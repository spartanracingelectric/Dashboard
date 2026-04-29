#include "main.h"
#include "lcd.h"
#include "leds.h"
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include "can_service.h"
#include "dash_fault.h"

const uint8_t DLCODE_BOOTUP[12] =
{
  0,0,0,2,	//GPU instruction CLEAR_COLOR_RGB
  7,0,0,38,	//GPU instruction CLEAR
  0,0,0,0,	//GPU instruction DISPLAY
};

float max_power = 0.0f;

#define DASH_FAULT_DISPLAY_MS 3000u

void LCD_demoCodeTest(void)
{
    float tps_avg = (cansvc::tps0_percent() + cansvc::tps1_percent()) / 2.0f;
    float inst_power = (cansvc::shunt_voltage() * cansvc::shunt_current()/1000); // V * A -> W

    if (inst_power > max_power) max_power = inst_power;

    float pl      = cansvc::pl();
    float hv_vol  = cansvc::hv();
    float t_high  = cansvc::celltemp();
    float v_low   = cansvc::hv_low();
    float dash_fault = cansvc::dash_fault_code();

    // Show fault overlay only for the first DASH_FAULT_DISPLAY_MS after faults
    // first appear. Trigger on the 0 -> nonzero rising edge only — changes in
    // the bitmask while the window is open must NOT restart the timer, or the
    // overlay never goes away when bits flap. Latch the displayed mask so the
    // listed faults stay stable for the duration of the window (otherwise the
    // top entry visibly cycles when bits toggle frame-to-frame).
    static uint32_t latched_fault = 0;
    static uint32_t fault_start_ms = 0;
    uint32_t cur_fault = (uint32_t)dash_fault;
    uint32_t now = HAL_GetTick();
    bool in_window = (latched_fault != 0) && ((now - fault_start_ms) < DASH_FAULT_DISPLAY_MS);

    if (in_window) {
        // Accumulate any newly-set bits, but keep the timer running.
        latched_fault |= cur_fault;
    } else if (cur_fault != 0 && latched_fault == 0) {
        // Rising edge: open a fresh window.
        latched_fault = cur_fault;
        fault_start_ms = now;
        in_window = true;
    } else if (cur_fault == 0) {
        // Re-arm so the next 0 -> nonzero edge can trigger again.
        latched_fault = 0;
    }
    // else: window expired and faults are still active — keep overlay hidden.

    float fault_to_show = in_window ? (float)latched_fault : 0.0f;

    renderDash(hv_vol, max_power, t_high, v_low, pl, tps_avg, cansvc::energy_pct(), fault_to_show);
}

void renderDash(float voltage, float max_power, float cell_high, float cell_low, float PL, float TPS, float energy, float dash_fault){
    /* Top row: Pack V | Highest Cell Temp | Lowest Cell Voltage
     * Bottom row: TPS | PL | PLTq
     * Middle: energy-used bar
     */

    /* initializing dash parameters */
    uint16_t FWo;
    float top_rect[3]    = {voltage, cell_high, cell_low};
    const char* top_labels[3] = {"Pack V", "High Temp", "Low Cell V"};
    const char* top_units[3]  = {"V", "C", "V"};

    float bot_rect[3]    = {TPS, PL, max_power};
    const char* bot_labels[3] = {"TPS", "PL", "Power"};
    const char* bot_units[3]  = {"%", "kW", "kW"};

    FWo = EVE_REG_Read_16(EVE_REG_CMD_WRITE);
    FWo = Wait_for_EVE_Execution_Complete(FWo);

    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_DLSTART);
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR_COLOR_RGB(150, 150, 150));
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR(1, 1, 1));

    /* Display resolution is 800 x 480 */
    const int gap = 40;
    const int rectWidth = 200, rectHeight = 120;
    const int xOff = (800 - (3 * rectWidth + 2 * gap)) / 2;

   /* checking for dash fault */
   if(dash_fault != 0){
       /* Flash background between bright and dark red ~2.5Hz to grab attention */
       bool flash_on = ((HAL_GetTick() / 200u) & 1u) == 0u;
       uint8_t bg_r = flash_on ? 220 : 110;

       /* Full-screen red fill overrides the gray clear */
       FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(bg_r, 0, 0));
       FWo = EVE_Filled_Rectangle(FWo, 0, 0, 800, 480);

       /* Thick white border frame */
       FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(255, 255, 255));
       FWo = EVE_Open_Rectangle(FWo, 20, 20, 780, 460, 8);

       /* Huge "FAULT" header */
       FWo = EVE_PrintF(FWo, 400, 90, 31, EVE_OPT_CENTER, "!! FAULT !!");

       /* List every active fault bit. Cap visible entries so the list cannot
        * overflow the action prompt at y=400. */
       uint32_t fault_mask = (uint32_t)dash_fault;
       const int max_visible = 5;
       int line_y = 180;
       int shown = 0;
       int total = 0;
       for (int b = 0; b < 32; b++) {
           uint32_t bit = (uint32_t)1 << b;
           if ((fault_mask & bit) == 0) continue;
           total++;
           if (shown < max_visible) {
               const char* name = GetSingleFaultName(bit);
               if (name == nullptr) name = "Unknown Fault";
               FWo = EVE_PrintF(FWo, 400, line_y, 31, EVE_OPT_CENTER, "%s", name);
               line_y += 45;
               shown++;
           }
       }
       if (total > shown) {
           FWo = EVE_PrintF(FWo, 400, line_y, 28, EVE_OPT_CENTER, "+%d more", total - shown);
       }

       /* Action prompt at bottom */
       FWo = EVE_PrintF(FWo, 400, 420, 28, EVE_OPT_CENTER, "Check Vehicle");
   }
   else{
        /* Top row */
        int yOff = 30;
        for (int i = 0; i < 3; i++) {
            int x0 = xOff + i * (rectWidth + gap);
            int y0 = yOff;
            int x1 = x0 + rectWidth;
            int y1 = y0 + rectHeight;
            int cx = (x0 + x1) / 2;

            int32_t val_int = (int32_t)top_rect[i];
            int32_t val_dec = (int32_t)((top_rect[i] - (float)val_int) * 100);
            if (val_dec < 0) val_dec = -val_dec;

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0, 0, 0));
            FWo = EVE_Open_Rectangle(FWo, x0, y0, x1, y1, 2);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(80, 80, 80));
            FWo = EVE_PrintF(FWo, cx, y0 + 20, 27, EVE_OPT_CENTER, "%s", top_labels[i]);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0, 0, 0));
            FWo = EVE_PrintF(FWo, cx, y0 + 65, 31, EVE_OPT_CENTER, "%ld.%02ld", (long)val_int, (long)val_dec);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(120, 120, 120));
            FWo = EVE_PrintF(FWo, cx, y0 + 100, 26, EVE_OPT_CENTER, "%s", top_units[i]);
        }

        /* Energy bar spans full width of the three boxes */
        {
            int barX0 = xOff;
            int barX1 = xOff + 3 * rectWidth + 2 * gap;
            int barY0 = 200;
            int barH  = 40;
            int barY1 = barY0 + barH;

            float pct = energy;
            if (pct < 0.0f) pct = 0.0f;
            if (pct > 100.0f) pct = 100.0f;

            int fillX1 = barX0 + (int)((float)(barX1 - barX0) * pct / 100.0f);

            uint8_t r = (uint8_t)(pct * 255.0f / 100.0f);
            uint8_t g = (uint8_t)((100.0f - pct) * 255.0f / 100.0f);
            if (fillX1 > barX0) {
                FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(r, g, 0));
                FWo = EVE_Filled_Rectangle(FWo, barX0, barY0, fillX1, barY1);
            }

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0, 0, 0));
            FWo = EVE_Open_Rectangle(FWo, barX0, barY0, barX1, barY1, 2);

            int barCx = (barX0 + barX1) / 2;
            int barCy = barY0 + barH / 2;
            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0, 0, 0));
            FWo = EVE_PrintF(FWo, barCx, barCy, 28, EVE_OPT_CENTER, "Energy  %ld%%", (long)(int32_t)pct);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(80, 80, 80));
            FWo = EVE_PrintF(FWo, barCx, barY0 - 15, 27, EVE_OPT_CENTER, "Energy Used");
        }

        /* Bottom row */
        yOff = 330;
        for (int i = 0; i < 3; i++) {
            int x0 = xOff + i * (rectWidth + gap);
            int y0 = yOff;
            int x1 = x0 + rectWidth;
            int y1 = y0 + rectHeight;
            int cx = (x0 + x1) / 2;

            int32_t val_int = (int32_t)bot_rect[i];

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0, 0, 0));
            FWo = EVE_Open_Rectangle(FWo, x0, y0, x1, y1, 2);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(80, 80, 80));
            FWo = EVE_PrintF(FWo, cx, y0 + 20, 27, EVE_OPT_CENTER, "%s", bot_labels[i]);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0, 0, 0));
            FWo = EVE_PrintF(FWo, cx, y0 + 65, 31, EVE_OPT_CENTER, "%ld", (long)val_int);

            FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(120, 120, 120));
            FWo = EVE_PrintF(FWo, cx, y0 + 100, 26, EVE_OPT_CENTER, "%s", bot_units[i]);
        }
    }

    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_DISPLAY());
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_SWAP);

    EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWo);
    Wait_for_EVE_Execution_Complete(FWo);
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
    FWo = EVE_Text(FWo, 250, 190, 31, 0, (char*)"HIREN WAS HERE");

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
