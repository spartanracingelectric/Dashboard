#ifndef LCD_H
#define LCD_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HOST_COMMAND_LENGTH 3
#define HOST_MEMORY_READ_COMMAND_LENGTH 4


void LCD_init(void);
void LCD_csLow(void);
void LCD_csHigh(void);
void LCD_pdLow(void);
void LCD_pdHigh(void);
void LCD_sendHostCommand(uint8_t command, uint8_t param);
void LCD_readMemory(uint32_t address, uint8_t *rxBuffer, uint16_t registerSize);
uint8_t LCD_readRegister8(uint32_t address);
uint16_t LCD_readRegister16(uint32_t address);
uint32_t LCD_readRegister32(uint32_t address);
void LCD_writeMemory(uint32_t address, uint8_t *data, uint16_t dataLength);
void LCD_writeRegister8(uint32_t address, uint8_t data);
void LCD_writeRegister16(uint32_t address, uint16_t data);
void LCD_writeRegister32(uint32_t address, uint32_t data);
void LCD_showRed();
void LCD_drawLineOnce(void);
void LCD_demoCodeTest(void);

// Values from Example code
#define HPX   (800)    // Horizontal Pixel Width
#define HSW   (4)      // Horizontal Sync Width (1~40)
#define HBP   (8)      // Horizontal Back Porch (must be 46, includes HSW)
#define HFP   (8)      // Horizontal Front Porch (16~210~354)
#define HPP   (178)    // Horizontal Pixel Padding (tot=863: 862~1056~1200)

#define LCD_WIDTH_PX (HPX)
#define LCD_HCYCLE_VALUE (HPX + HSW + HBP + HFP + HPP)
#define LCD_HOFFSET_VALUE (HFP + HSW + HBP)
#define LCD_HSYNC0_VALUE (HFP)
#define LCD_HSYNC1_VALUE (HFP + HSW)


// Values from Example code
#define VLH   (480)   // Vertical Line Height
#define VS    (4)     // Vertical Sync (in lines)  (1~20)
#define VBP   (8)     // Vertical Back Porch (must be 23, includes VS)
#define VFP   (8)     // Vertical Front Porch (7~22~147)
#define VLP   (1)     // Vertical Line Padding (tot=511: 510~525~650). EVE needs at least 1 here

#define LCD_HEIGHT_PX (VLH)
#define LCD_VSYNC0_VALUE (VFP)
#define LCD_VSYNC1_VALUE (VFP + VS)
#define LCD_VOFFSET_VALUE (VFP + VS + VBP)
#define LCD_VCYCLE_VALUE (VLH + VFP + VS + VBP + VLP)

#define CLKEXT 0x44
#define RST_PULSE 0x68
#define ACTIVE 0x00
#define CLKSEL 0x61

#define EXTERNAL_CLOCK_72MHz 0x46

#define REG_ID_ADDRESS 0x302000
#define REG_ID_SIZE_BYTES 1
#define REG_CPURESET_ADDRESS 0x302020
#define REG_CPURESET_SIZE_BYTES 1

#define REG_HSIZE_ADDRESS 0x302034
#define REG_HCYCLE_ADDRESS 0x30202C
#define REG_HOFFSET_ADDRESS 0x302030
#define REG_HSYNC0_ADDRESS 0x302038
#define REG_HSYNC1_ADDRESS 0x30203C

#define REG_VSIZE_ADDRESS 0x302048
#define REG_VCYCLE_ADDRESS 0x302040
#define REG_VOFFSET_ADDRESS 0x302044
#define REG_VSYNC0_ADDRESS 0x30204C
#define REG_VSYNC1_ADDRESS 0x302050

#define REG_SWIZZLE_ADDRESS 0x302064
#define LCD_SWIZZLE_VALUE 0 // Defines RGB output pins order, determined by PCB layout

#define REG_PCLK_POL_ADDRESS 0x30206C
#define LCD_PCLK_POL 0 // Define active edge of PCLK. 0 is falling edge, 1 is rising edge

#define REG_CSPREAD_ADDRESS 0x302068
#define LCD_CSPREAD 0

#define RAM_DL_START_ADDRESS 0x300000

#define REG_DLSWAP_ADDRESS 0x302054
#define DLSWAP_FRAME 0x02

#define REG_GPIOX_DIR_ADDRESS 0x302098
#define REG_GPIOX_ADDRESS 0x30209C

#define REG_PCLK_ADDRESS 0x302070
#define LCD_PCLK (2)

#define REG_FREQUENCY_ADDRESS 0x30200C
#define CLOCK_SPEED ((uint32_t)(EXTERNAL_CLOCK_72MHz & 0x1F) * (uint32_t)12000000)

#define REG_PWM_HZ_ADDRESS 0x3020D0
#define REG_PWM_DUTY_ADDRESS 0x3020D4

#define REG_PCLK_FREQ_ADDRESS 0x302614
#define DispPLCLKFREQ  0x232

#define REG_PCLK_2X_ADDRESS 0x302618
#define DispPCLK2x	 0

#define REG_DITHER_ADDRESS 0x302060
#define LCD_DITHER_VALUE 0


#ifdef __cplusplus
}
#endif


#endif
