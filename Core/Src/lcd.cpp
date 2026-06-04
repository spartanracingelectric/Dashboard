#include "main.h"
#include "lcd.h"
#include "leds.h"
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include "can_service.h"
#include "dash_fault.h"
#include <stdint.h>

const uint8_t DLCODE_BOOTUP[12] =
{
  0,0,0,2,	//GPU instruction CLEAR_COLOR_RGB
  7,0,0,38,	//GPU instruction CLEAR
  0,0,0,0,	//GPU instruction DISPLAY
};

float max_power = 0.0f;

#define DASH_FAULT_DISPLAY_MS 10000u
#define MAX_POWER_DEBOUNCE_MS 100u
#define PACK_CAPACITY_KWH     6.0f

/* ============================================================================
 *  Dash rendering
 *  ----------------------------------------------------------------------------
 *  The screen is 800 x 480, but the bottom third (y >= 320) is hidden behind
 *  the steering wheel, so everything is packed into the visible top region:
 *      Top row    (y=8)   : Pack V | High Cell Temp | Low Cell V
 *      Energy bar (y=150) : full-width energy-remaining bar
 *      Bottom row (y=196) : TPS    | PL             | Max Power
 *
 *  dash_mode controls LCD brightness (1 = brightest, 6 = dimmest); see
 *  LCD_demoCodeTest for the mapping.
 *
 *  A BMS fault in DF_DisplayMask pre-empts everything and shows the overlay.
 * ========================================================================= */

// ---- Layout ----------------------------------------------------------------
// The bottom third of the panel (y >= VISIBLE_H) is hidden behind the steering
// wheel, so the whole dashboard is packed into the visible top region above it.
#define LCD_W       800
#define LCD_H       480
#define VISIBLE_H   320          // steering wheel hides everything below this y
#define BOX_W       200
#define BOX_H       100
#define BOX_GAP     40
#define ROW_MARGIN  ((LCD_W - (3 * BOX_W + 2 * BOX_GAP)) / 2) // centers the 3-box row
#define TOP_ROW_Y   8
#define BOT_ROW_Y   196
#define BAR_Y       150
#define BAR_H       32

// ---- Theme -----------------------------------------------------------------
typedef struct { uint8_t r, g, b; } Color;

// All colors used by the data dashboard, grouped so the draw routines stay
// theme-agnostic.
typedef struct {
    Color bg;     // background
    Color border; // box & bar outlines (team accent)
    Color value;  // big numeric value (highest contrast)
    Color label;  // metric name above the value
    Color unit;   // unit suffix below the value
} Theme;

// Spartan Racing theme: dark gray background, team-gold values framed in team blue.
static const Theme kTheme = {
    { 40,  40,  40},   // bg:     dark gray
    { 56, 162, 219},   // border: Spartan blue
    {255, 199,   0},   // value:  Spartan gold
    {255, 255, 255},   // label:  white
    {170, 170, 170},   // unit:   dim gray
};

static inline uint16_t setColor(uint16_t FWo, Color c) {
    return EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(c.r, c.g, c.b));
}

// ---- Frame begin / end -----------------------------------------------------
static uint16_t beginFrame(uint16_t FWo, const Theme& th) {
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_DLSTART);
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR_COLOR_RGB(th.bg.r, th.bg.g, th.bg.b));
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR(1, 1, 1));
    return FWo;
}

static uint16_t endFrame(uint16_t FWo) {
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_DISPLAY());
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_SWAP);
    return FWo;
}

// ---- Building blocks for the data dashboard --------------------------------

typedef struct {
    const char* label;
    float       value;
    const char* unit;
} MetricCell;

// Draws one labeled metric box. `decimals==0` prints an integer value;
// `decimals==2` prints two decimal places.
static uint16_t drawMetricBox(uint16_t FWo, const Theme& th, int x0, int y0,
                              const MetricCell& m, int decimals) {
    int x1 = x0 + BOX_W;
    int y1 = y0 + BOX_H;
    int cx = (x0 + x1) / 2;

    FWo = setColor(FWo, th.border);
    FWo = EVE_Open_Rectangle(FWo, x0, y0, x1, y1, 2);

    FWo = setColor(FWo, th.label);
    FWo = EVE_PrintF(FWo, cx, y0 + 16, 27, EVE_OPT_CENTER, "%s", m.label);

    FWo = setColor(FWo, th.value);
    if (decimals == 0) {
        FWo = EVE_PrintF(FWo, cx, y0 + 52, 31, EVE_OPT_CENTER,
                         "%ld", (long)(int32_t)m.value);
    } else {
        int32_t v_int = (int32_t)m.value;
        int32_t v_dec = (int32_t)((m.value - (float)v_int) * 100);
        if (v_dec < 0) v_dec = -v_dec;
        FWo = EVE_PrintF(FWo, cx, y0 + 52, 31, EVE_OPT_CENTER,
                         "%ld.%02ld", (long)v_int, (long)v_dec);
    }

    FWo = setColor(FWo, th.unit);
    FWo = EVE_PrintF(FWo, cx, y0 + 84, 26, EVE_OPT_CENTER, "%s", m.unit);
    return FWo;
}

static uint16_t drawMetricRow(uint16_t FWo, const Theme& th, int yOff,
                              const MetricCell metrics[3], int decimals) {
    for (int i = 0; i < 3; i++) {
        int x0 = ROW_MARGIN + i * (BOX_W + BOX_GAP);
        FWo = drawMetricBox(FWo, th, x0, yOff, metrics[i], decimals);
    }
    return FWo;
}

static uint16_t drawEnergyBar(uint16_t FWo, const Theme& th, float pct) {
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

    int barX0  = ROW_MARGIN;
    int barX1  = ROW_MARGIN + 3 * BOX_W + 2 * BOX_GAP;
    int barY0  = BAR_Y;
    int barY1  = barY0 + BAR_H;
    int fillX1 = barX0 + (int)((float)(barX1 - barX0) * pct / 100.0f);

    // Battery-style gauge: bar drains as energy is spent, green when full -> red when low.
    uint8_t r = (uint8_t)((100.0f - pct) * 255.0f / 100.0f);
    uint8_t g = (uint8_t)(pct * 255.0f / 100.0f);
    if (fillX1 > barX0) {
        FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(r, g, 0));
        FWo = EVE_Filled_Rectangle(FWo, barX0, barY0, fillX1, barY1);
    }

    FWo = setColor(FWo, th.border);
    FWo = EVE_Open_Rectangle(FWo, barX0, barY0, barX1, barY1, 2);

    int cx = (barX0 + barX1) / 2;
    int cy = barY0 + BAR_H / 2;
    FWo = setColor(FWo, th.value);
    FWo = EVE_PrintF(FWo, cx, cy, 28, EVE_OPT_CENTER,
                     "Energy  %ld%%", (long)(int32_t)pct);

    FWo = setColor(FWo, th.label);
    FWo = EVE_PrintF(FWo, cx, barY0 - 15, 27, EVE_OPT_CENTER, "Energy Remaining");
    return FWo;
}

// ---- Mode renderers --------------------------------------------------------

// Renders the normal data dashboard using the single Spartan theme.
static uint16_t drawDataDashboard(uint16_t FWo, const Theme& th,
                                  float voltage, float cell_high, float cell_low,
                                  float TPS, float PL, float power, float energy) { // add brake pressure parameter
    const MetricCell top[3] = {
        {"Pack V",     voltage,   "V"},
        {"High Temp",  cell_high, "C"},
        {"Low Cell V", cell_low,  "V"},
    };
    const MetricCell bot[3] = {
        {"TPS",   TPS,   "%"},
        {"PL",    PL,    "kW"},
        {"Power", power, "kW"},
    };

    FWo = drawMetricRow(FWo, th, TOP_ROW_Y, top, /*decimals=*/2);
    FWo = drawEnergyBar(FWo, th, energy);
    FWo = drawMetricRow(FWo, th, BOT_ROW_Y, bot, /*decimals=*/0);
    return FWo;
}

static uint16_t drawFaultOverlay(uint16_t FWo, uint8_t fault_mask,
                                 float low_cell_v, float high_cell_temp) {
    bool flash_on = ((HAL_GetTick() / 200u) & 1u) == 0u;
    uint8_t bg_v = flash_on ? 180 : 110;

    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(bg_v, bg_v, 0));
    FWo = EVE_Filled_Rectangle(FWo, 0, 0, LCD_W, LCD_H);

    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWo = EVE_Open_Rectangle(FWo, 20, 20, 780, 460, 8);
    FWo = EVE_PrintF(FWo, 400, 90, 31, EVE_OPT_CENTER, "!! BMS FAULT !!");

    int line_y = 180;
    if (fault_mask & DF_CellUndervolt) {
        FWo = EVE_PrintF(FWo, 400, line_y, 31, EVE_OPT_CENTER, "Cell Undervoltage");
        int32_t v_int = (int32_t)low_cell_v;
        int32_t v_mv  = (int32_t)((low_cell_v - (float)v_int) * 1000.0f);
        if (v_mv < 0) v_mv = -v_mv;
        FWo = EVE_PrintF(FWo, 400, line_y + 40, 30, EVE_OPT_CENTER,
                         "%ld.%03ld V", (long)v_int, (long)v_mv);
        line_y += 110;
    }
    if (fault_mask & DF_CellHighTemp) {
        FWo = EVE_PrintF(FWo, 400, line_y, 31, EVE_OPT_CENTER, "High Cell Temp");
        FWo = EVE_PrintF(FWo, 400, line_y + 40, 30, EVE_OPT_CENTER,
                         "%ld C", (long)(int32_t)high_cell_temp);
    }

    FWo = EVE_PrintF(FWo, 400, 420, 28, EVE_OPT_CENTER, "Check Vehicle");
    return FWo;
}

// Latches BMS fault bits (masked to DF_DisplayMask) for DASH_FAULT_DISPLAY_MS
// so flapping bits don't make the overlay flicker. Accumulates within the
// window without restarting the timer; re-arms once the mask returns to 0.
static uint8_t latchedFaultMask(uint8_t cur_fault) {
    static uint8_t  latched  = 0;
    static uint32_t start_ms = 0;

    cur_fault &= DF_DisplayMask;             // only the bits we display

    uint32_t now = HAL_GetTick();
    bool in_window = (latched != 0) && ((now - start_ms) < DASH_FAULT_DISPLAY_MS);

    if (in_window) {
        latched |= cur_fault;                // accumulate without restarting timer
    } else if (cur_fault != 0 && latched == 0) {
        latched   = cur_fault;               // rising edge: open a fresh 10 s window
        start_ms  = now;
        in_window = true;
    } else if (cur_fault == 0) {
        latched = 0;                         // fault cleared: re-arm for the next edge
    }
    return in_window ? latched : 0;
}


static void renderDash(float voltage, float max_power, float cell_high, float cell_low,
                       float PL, float TPS, float energy, uint8_t fault_mask)
{
    uint16_t FWo = EVE_REG_Read_16(EVE_REG_CMD_WRITE);
    FWo = Wait_for_EVE_Execution_Complete(FWo);

    FWo = beginFrame(FWo, kTheme);
    if (fault_mask != 0) {
        FWo = drawFaultOverlay(FWo, fault_mask, cell_low, cell_high);
    }
    else {
        FWo = drawDataDashboard(FWo, kTheme,
                                voltage, cell_high, cell_low,
                                TPS, PL, max_power, energy);
    }

    FWo = endFrame(FWo);

    EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWo);
    Wait_for_EVE_Execution_Complete(FWo);
}

void LCD_demoCodeTest(void)
{
    static uint32_t power_above_start_ms = 0;
    static uint8_t  last_pwm_duty        = 0xFF; // force a write on the first frame

    float tps_avg    = (cansvc::tps0_percent() + cansvc::tps1_percent()) / 2.0f;
    float inst_power = cansvc::shunt_voltage() * cansvc::shunt_current() / 1000.0f;

    uint32_t now = HAL_GetTick();

    if (inst_power > max_power) {
        if (power_above_start_ms == 0) {
            power_above_start_ms = now;
        }

        if ((now - power_above_start_ms) >= MAX_POWER_DEBOUNCE_MS) {
            max_power = inst_power;
            power_above_start_ms = 0;
        }
    } else {
        power_above_start_ms = 0;
    }

    // dash_mode -> LCD backlight: 1 = brightest (PWM 128), 6 = dimmest (PWM 28).
    // VCU sends 1..6 from PL knob; 0 only appears before the first CAN frame, treat as brightest.
    uint8_t mode = (uint8_t)cansvc::dash_mode();
    if (mode == 0) mode = 1;
    if (mode > 6) mode = 6;
    uint8_t pwm_duty = (uint8_t)(128 - (mode - 1) * 20);
    if (pwm_duty != last_pwm_duty) {
        LCD_writeRegister8(REG_PWM_DUTY_ADDRESS, pwm_duty);
        last_pwm_duty = pwm_duty;
    }

    uint8_t fault_to_show = latchedFaultMask((uint8_t)cansvc::bms_fault());

    renderDash(cansvc::hv(),
               max_power,
               cansvc::celltemp(),
               cansvc::hv_low(),
               cansvc::pl(),
               tps_avg,
               cansvc::energy_pct(),
               fault_to_show);
}

void LCD_drawLineOnce(void)
{
    uint16_t FWo;


    FWo = EVE_REG_Read_16(EVE_REG_CMD_WRITE);
    FWo = Wait_for_EVE_Execution_Complete(FWo);

    // Start display list
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_DLSTART);

    // Clear background to green
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR_COLOR_RGB(0, 255, 0));
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR(1, 1, 1));

    // White drawing color
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(255, 255, 255));

    // Sets opacity to 1
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_A(255));

    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWo = EVE_Text(FWo, 250, 190, 31, 0, (char*)"AKASH WAS HERE");

    // Finish and swap
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_DISPLAY());
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_SWAP);

    EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWo);
    Wait_for_EVE_Execution_Complete(FWo);
}


void LCD_csLow(void)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

void LCD_csHigh(void)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void LCD_pdLow(void)
{
    HAL_GPIO_WritePin(LCD_PD_GPIO_Port, LCD_PD_Pin, GPIO_PIN_RESET);
}

void LCD_pdHigh(void)
{
    HAL_GPIO_WritePin(LCD_PD_GPIO_Port, LCD_PD_Pin, GPIO_PIN_SET);
}

void LCD_sendHostCommand(uint8_t command, uint8_t param)
{
    uint8_t txBuffer[3];

    txBuffer[0] = command;
    txBuffer[1] = param;
    txBuffer[2] = 0x00;

    LCD_csLow();
    HAL_SPI_Transmit(&hspi4, txBuffer, 3, HAL_MAX_DELAY);
    LCD_csHigh();
}

void LCD_writeMemory(uint32_t address, uint8_t *data, uint16_t dataLength)
{
    uint8_t txBuffer[3];

    txBuffer[0] = (uint8_t)(0x80 | ((address >> 16) & 0x3F));
    txBuffer[1] = (uint8_t)(address >> 8);
    txBuffer[2] = (uint8_t)address;

    LCD_csLow();

    HAL_SPI_Transmit(&hspi4, txBuffer, 3, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi4, (uint8_t*)data, dataLength, HAL_MAX_DELAY);

    LCD_csHigh();
}

void LCD_writeRegister8(uint32_t address, uint8_t data)
{
   LCD_writeMemory(address, &data, 1);
}

void LCD_writeRegister16(uint32_t address, uint16_t data)
{
    uint8_t txBuffer[2];

    txBuffer[0] = (uint8_t)data;
    txBuffer[1] = (uint8_t)(data >> 8);

    LCD_writeMemory(address, txBuffer, 2);
}

void LCD_writeRegister32(uint32_t address, uint32_t data)
{
    uint8_t txBuffer[4];

    txBuffer[0] = (uint8_t)data;
    txBuffer[1] = (uint8_t)(data >> 8);
    txBuffer[2] = (uint8_t)(data >> 16);
    txBuffer[3] = (uint8_t)(data >> 24);

    LCD_writeMemory(address, txBuffer, 4);
}


void LCD_readMemory(uint32_t address, uint8_t *rxBuffer, uint16_t registerSize)
{
    uint8_t txBuffer[4];
    uint8_t dummyTx[32] = {0};

    txBuffer[0] = (uint8_t)((address >> 16) & 0x3F);
    txBuffer[1] = (uint8_t)(address >> 8);
    txBuffer[2] = (uint8_t)address;
    txBuffer[3] = 0x00;

    LCD_csLow();
    HAL_SPI_Transmit(&hspi4, txBuffer, 4, HAL_MAX_DELAY);
    HAL_SPI_TransmitReceive(&hspi4, dummyTx, rxBuffer, registerSize, HAL_MAX_DELAY);
    LCD_csHigh();
}

uint8_t LCD_readRegister8(uint32_t address)
{
    uint8_t value;
    LCD_readMemory(address, &value, 1);
    return value;
}

uint16_t LCD_readRegister16(uint32_t address)
{
    uint8_t rxBuffer[2];
    LCD_readMemory(address, rxBuffer, 2);

    return (uint16_t)(rxBuffer[0] | (rxBuffer[1] << 8));
}

uint32_t LCD_readRegister32(uint32_t address)
{
    uint8_t rxBuffer[4];
    LCD_readMemory(address, rxBuffer, 4);

    return ((uint32_t)rxBuffer[0]) | ((uint32_t)rxBuffer[1] << 8) | ((uint32_t)rxBuffer[2] << 16) | ((uint32_t)rxBuffer[3] << 24);
}

void LCD_showRed()
{
    LCD_writeRegister32(RAM_DL_START_ADDRESS + 0, EVE_ENC_CLEAR_COLOR_RGB(255, 0, 0));
    LCD_writeRegister32(RAM_DL_START_ADDRESS + 4, EVE_ENC_CLEAR(1, 1, 1));
    LCD_writeRegister32(RAM_DL_START_ADDRESS + 8, EVE_ENC_DISPLAY());

    LCD_writeRegister8(REG_DLSWAP_ADDRESS, DLSWAP_FRAME);
}

void LCD_init(void)
{
	HAL_Delay(20);
	LCD_pdLow();
	HAL_Delay(6);
	LCD_pdHigh();
	HAL_Delay(21);

	uint32_t startTick;
	LCD_sendHostCommand(CLKEXT, 0x00);
	LCD_sendHostCommand(CLKSEL, EXTERNAL_CLOCK_72MHz);

	LCD_sendHostCommand(ACTIVE, 0x00);
	HAL_Delay(40);

	//LCD_sendHostCommand(RST_PULSE, 0x00);



	startTick = HAL_GetTick();
	while (0x7C != LCD_readRegister8(REG_ID_ADDRESS))
	{
		//leds::led0_on();
		if (HAL_GetTick() - startTick > 500)
		{
			return;
		}
		HAL_Delay(1);
	}


	// SUCCESS
	startTick = HAL_GetTick();
	while (0x00 != LCD_readRegister8(REG_CPURESET_ADDRESS))
	{
		if (HAL_GetTick() - startTick > 500)
		{

			return;
		}
		HAL_Delay(1);
	}

	LCD_writeRegister32(REG_FREQUENCY_ADDRESS, CLOCK_SPEED);


	//LCD_writeRegister16(REG_PWM_HZ_ADDRESS, 4000);
	LCD_writeRegister8(REG_PCLK_ADDRESS, 0);

	LCD_writeRegister8(REG_PWM_DUTY_ADDRESS, 0);

//	LCD_writeRegister16(REG_PCLK_FREQ_ADDRESS, DispPLCLKFREQ);
//	LCD_writeRegister8(REG_PCLK_2X_ADDRESS, DispPCLK2x);

	LCD_writeRegister16(REG_HSIZE_ADDRESS, LCD_WIDTH_PX);
	LCD_writeRegister16(REG_HCYCLE_ADDRESS, LCD_HCYCLE_VALUE);
	LCD_writeRegister16(REG_HOFFSET_ADDRESS, LCD_HOFFSET_VALUE);
	LCD_writeRegister16(REG_HSYNC0_ADDRESS, LCD_HSYNC0_VALUE);
	LCD_writeRegister16(REG_HSYNC1_ADDRESS, LCD_HSYNC1_VALUE);
	LCD_writeRegister16(REG_VSIZE_ADDRESS, LCD_HEIGHT_PX);
	LCD_writeRegister16(REG_VCYCLE_ADDRESS, LCD_VCYCLE_VALUE);
	LCD_writeRegister16(REG_VOFFSET_ADDRESS, LCD_VOFFSET_VALUE);
	LCD_writeRegister16(REG_VSYNC0_ADDRESS, LCD_VSYNC0_VALUE);
	LCD_writeRegister16(REG_VSYNC1_ADDRESS, LCD_VSYNC1_VALUE);
	LCD_writeRegister8(REG_SWIZZLE_ADDRESS, LCD_SWIZZLE_VALUE);
	LCD_writeRegister8(REG_PCLK_POL_ADDRESS, LCD_PCLK_POL);

	LCD_writeRegister16(REG_CSPREAD_ADDRESS, LCD_CSPREAD);
	LCD_writeRegister16(REG_DITHER_ADDRESS, LCD_DITHER_VALUE);



	LCD_writeRegister32(RAM_DL_START_ADDRESS + 0, EVE_ENC_CLEAR_COLOR_RGB(0, 0, 0));
	LCD_writeRegister32(RAM_DL_START_ADDRESS + 4, EVE_ENC_CLEAR(1, 1, 1));
	LCD_writeRegister32(RAM_DL_START_ADDRESS + 8, EVE_ENC_DISPLAY());


	LCD_writeRegister8(REG_DLSWAP_ADDRESS, DLSWAP_FRAME);

	LCD_writeRegister16(REG_GPIOX_DIR_ADDRESS, 0xFFFF);
	LCD_writeRegister16(REG_GPIOX_ADDRESS, LCD_readRegister16(REG_GPIOX_ADDRESS) | 0x8000);



	LCD_writeRegister8(REG_PCLK_ADDRESS, LCD_PCLK);
	LCD_writeRegister16(REG_PWM_HZ_ADDRESS, 250);
	LCD_writeRegister8(REG_PWM_DUTY_ADDRESS, 128);

	//leds::led0_on();


}
