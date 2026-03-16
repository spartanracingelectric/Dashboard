#include "main.h"
#include "lcd.h"
#include "leds.h"
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"

const uint8_t DLCODE_BOOTUP[12] =
{
  0,0,0,2,	//GPU instruction CLEAR_COLOR_RGB
  7,0,0,38,	//GPU instruction CLEAR
  0,0,0,0,	//GPU instruction DISPLAY
};

void LCD_demoCodeTest(void)
{
	leds::led0_off();
	//EVE_Initialize();
	LCD_init();
	//leds::led0_on();
	LCD_drawLineOnce();
}


void LCD_drawLineOnce(void)
{
    uint16_t FWo;


    FWo = EVE_REG_Read_16(EVE_REG_CMD_WRITE);
    FWo = Wait_for_EVE_Execution_Complete(FWo);

    // Start display list
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_DLSTART);

    // Clear background to black
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR_COLOR_RGB(0, 255, 0));
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_CLEAR(1, 1, 1));

    // White drawing color
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(255, 255, 255));

    // Sets opacity to 1
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_A(255));

    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWo = EVE_Text(FWo, 250, 190, 31, 0, "HIREN WAS HERE");

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

	leds::led0_on();


}
