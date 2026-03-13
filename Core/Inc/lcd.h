#ifndef LCD_H
#define LCD_H

#include "main.h"

#define HOST_COMMAND_LENGTH 3
#define HOST_MEMORY_READ_COMMAND_LENGTH 4

extern SPI_HandleTypeDef hspi4;

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

// Values from Example code
#define HPX   (800)    // Horizontal Pixel Width
#define HSW   (4)      // Horizontal Sync Width (1~40)
#define HBP   (8)      // Horizontal Back Porch (must be 46, includes HSW)
#define HFP   (8)      // Horizontal Front Porch (16~210~354)
#define HPP   (178)    // Horizontal Pixel Padding (tot=863: 862~1056~1200)

#define LCD_WIDTH_PX (HPX)
#define LCD_HCYCLE (HPX + HSW + HBP + HFP + HPP)
#define LCD_HOFFSET (HFP + HSW + HBP)
#define LCD_HSYNC0 (HFP)
#define LCD_HSYNC1 (HFP + HSW)


// Values from Example code
#define VLH   (480)   // Vertical Line Height
#define VS    (4)     // Vertical Sync (in lines)  (1~20)
#define VBP   (8)     // Vertical Back Porch (must be 23, includes VS)
#define VFP   (8)     // Vertical Front Porch (7~22~147)
#define VLP   (1)     // Vertical Line Padding (tot=511: 510~525~650). EVE needs at least 1 here

#define LCD_HEIGHT_PX (VLH)
#define LCD_VSYNC0 (VFP)
#define LCD_VSYNC1 (VFP + VS)
#define LCD_VOFFSET (VFP + VS + VBP)
#define LCD_VCYCLE (VLH + VFP + VS + VBP + VLP)

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
#define LCD_SWIZZLE 0x0 // Defines RGB output pins order, determined by PCB layout

#define REG_PCLK_POL_ADDRESS 0x30206C
#define LCD_PCLK_POL (1) // Define active edge of PCLK. 0 is falling edge, 1 is rising edge

#define REG_CSPREAD_ADDRESS 0x302068
#define LCD_CSPREAD (0)

#define RAM_DL_START_ADDRESS 0x300000

#define REG_DLSWAP_ADDRESS 0x302054
#define DLSWAP_FRAME 0x02

#define REG_GPIOX_DIR_ADDRESS 0x302094
#define REG_GPIOX_ADDRESS 0x30209C

#define REG_PCLK_ADDRESS 0x302070
#define LCD_PCLK (2)

#define REG_FREQUENCY_ADDRESS 0x30200C
#define CLOCK_SPEED ((uint32_t)(EXTERNAL_CLOCK_72MHz & 0x1F) * (uint32_t)12000000)

#define EVE_ENC_ALPHA_FUNC(func,ref)                         ((0x9UL << 24)|(((func) & 0x7UL) << 8)|(((ref) & 0xffUL) << 0))
#define EVE_ENC_BEGIN(prim)                                  ((0x1fUL << 24)|(((prim)&15UL) << 0))
#define EVE_ENC_BITMAP_HANDLE(handle)                        ((0x5UL << 24)|(((handle) & 0x1fUL) << 0))
#define EVE_ENC_BITMAP_LAYOUT_H(linestride,height)           ((0x28UL << 24)|(((linestride) & 0x3UL) << 2)|(((height) & 0x3UL) << 0))
#define EVE_ENC_BITMAP_LAYOUT(format,linestride,height)      ((0x7UL << 24)|(((format) & 0x1fUL) << 19)|(((linestride) & 0x3ffUL) << 9)|(((height) & 0x1ffUL) << 0))
#define EVE_ENC_BITMAP_SIZE_H(width,height)                  ((0x29UL << 24)|(((width) & 0x3UL) << 2)|(((height) & 0x3UL) << 0))
#define EVE_ENC_BITMAP_SIZE(filter,wrapx,wrapy,width,height) ((0x8UL << 24)|(((filter) & 0x1UL) << 20)|(((wrapx) & 0x1UL) << 19)|(((wrapy) & 0x1UL) << 18)|(((width) & 0x1ffUL) << 9)|(((height) & 0x1ffUL) << 0))
#define EVE_ENC_BITMAP_SOURCE(addr)                          ((0x1UL << 24)|(((addr) & 0x3FFFFFUL) << 0))
#define EVE_ENC_BITMAP_TRANSFORM_A(a)                        ((0x15UL << 24)|((((uint32_t)(a)) & 0x1FFFFUL) << 0))
#define EVE_ENC_BITMAP_TRANSFORM_B(b)                        ((0x16UL << 24)|((((uint32_t)(b)) & 0x1FFFFUL) << 0))
#define EVE_ENC_BITMAP_TRANSFORM_C(c)                        ((0x17UL << 24)|((((uint32_t)(c)) & 0xFFFFFFUL) << 0))
#define EVE_ENC_BITMAP_TRANSFORM_D(d)                        ((0x18UL << 24)|((((uint32_t)(d)) & 0x1FFFFUL) << 0))
#define EVE_ENC_BITMAP_TRANSFORM_E(e)                        ((0x19UL << 24)|((((uint32_t)(e)) & 0x1FFFFUL) << 0))
#define EVE_ENC_BITMAP_TRANSFORM_F(f)                        ((0x1aUL << 24)|((((uint32_t)(f)) & 0xFFFFFFUL) << 0))
#define EVE_ENC_BLEND_FUNC(src,dst)                          ((0xbUL << 24)|(((src) & 0x7UL) << 3)|(((dst) & 0x7UL) << 0))
#define EVE_ENC_CALL(dest)                                   ((0x1dUL << 24)|(((dest) & 0xFFFFUL) << 0))
#define EVE_ENC_CELL(cell)                                   ((0x6UL << 24)|(((cell) & 0x7fUL) << 0))
#define EVE_ENC_CLEAR_COLOR_A(alpha)                         ((0xfUL << 24)|(((alpha) & 0xffUL) << 0))
#define EVE_ENC_CLEAR_COLOR_RGB(red,green,blue)              ((0x2UL << 24)|(((red) & 0xffUL) << 16)|(((green) & 0xffUL) << 8)|(((blue) & 0xffUL) << 0))
#define EVE_ENC_CLEAR_COLOR(c)                               ((0x2UL << 24)|(((uint32_t)(c)) & 0x00ffffffUL))
#define EVE_ENC_CLEAR_STENCIL(s)                             ((0x11UL << 24)|((((uint32_t)(s)) & 0xffUL) << 0))
#define EVE_ENC_CLEAR_TAG(s)                                 ((0x12UL << 24)|((((uint32_t)(s)) & 0xffUL) << 0))
#define EVE_ENC_CLEAR(c,s,t)                                 ((0x26UL << 24)|((((uint32_t)(c)) & 0x1UL) << 2)|((((uint32_t)(s)) & 0x1UL) << 1)|((((uint32_t)(t)) & 0x1UL) << 0))
#define EVE_ENC_COLOR_A(alpha)                               ((0x10UL << 24)|(((alpha) & 0xffUL) << 0))
#define EVE_ENC_COLOR_MASK(r,g,b,a)                          ((0x20UL << 24)|((((uint32_t)(r)) & 0x1UL) << 3)|((((uint32_t)(g)) & 0x1UL) << 2)|((((uint32_t)(b)) & 0x1UL) << 1)|((((uint32_t)(a)) & 0x1UL) << 0))
#define EVE_ENC_COLOR_RGB(red,green,blue)                    ((0x4UL << 24)|(((red) & 0xffUL) << 16)|(((green) & 0xffUL) << 8)|(((blue) & 0xffUL) << 0))
#define EVE_ENC_COLOR(c)                                     ((0x4UL << 24)|(((uint32_t)(c)) & 0x00ffffffUL))
#define EVE_ENC_DISPLAY()                                    ((0x0UL << 24))
#define EVE_ENC_END()                                        ((0x21UL << 24))
#define EVE_ENC_JUMP(dest)                                   ((0x1eUL << 24)|(((dest) & 0xFFFFUL) << 0))
#define EVE_ENC_LINE_WIDTH(width)                            ((0xeUL << 24)|(((width) & 0xFFFUL) << 0))
#define EVE_ENC_MACRO(m)                                     ((0x25UL << 24)|((((uint32_t)(m)) & 0x1UL) << 0))
#define EVE_ENC_NOP()                                        ((0x2dUL << 24))
#define EVE_ENC_PALETTE_SOURCE(addr)                         ((0x2aUL << 24)|(((addr) & 0x3FFFFFUL) << 0))
#define EVE_ENC_POINT_SIZE(size)                             ((0xdUL << 24)|(((size) & 0x1FFFUL) << 0))
#define EVE_ENC_RESTORE_CONTEXT()                            ((0x23UL << 24))
#define EVE_ENC_RETURN()                                     ((0x24UL << 24))
#define EVE_ENC_SAVE_CONTEXT()                               ((0x22UL << 24))
#define EVE_ENC_SCISSOR_SIZE(width,height)                   ((0x1cUL << 24)|(((width) & 0xFFFUL) << 12)|(((height) & 0xFFFUL) << 0))
#define EVE_ENC_SCISSOR_XY(x,y)                              ((0x1bUL << 24)|((((uint32_t)(x)) & 0x7FFUL) << 11)|((((uint32_t)(y)) & 0x7FFUL) << 0))
#define EVE_ENC_STENCIL_FUNC(func,ref,mask)                  ((0xaUL << 24)|(((func) & 0x7UL) << 16)|(((ref) & 0xffUL) << 8)|(((mask) & 0xffUL) << 0))
#define EVE_ENC_STENCIL_MASK(mask)                           ((0x13UL << 24)|(((mask) & 0xffUL) << 0))
#define EVE_ENC_STENCIL_OP(sfail,spass)                      ((0xcUL << 24)|(((sfail) & 0x7UL) << 3)|(((spass) & 0x7UL) << 0))
#define EVE_ENC_TAG_MASK(mask)                               ((0x14UL << 24)|(((mask) & 0x1UL) << 0))
#define EVE_ENC_TAG(s)                                       ((0x3UL << 24)|((((uint32_t)(s)) & 0xffUL) << 0))
#define EVE_ENC_VERTEX_FORMAT(frac)                          ((0x27UL << 24)|(((frac) & 0x7UL) << 0))
#define EVE_ENC_VERTEX_TRANSLATE_X(x)                        ((0x2bUL << 24)|((((uint32_t)(x)) & 0x1FFFFUL) << 0))
#define EVE_ENC_VERTEX_TRANSLATE_Y(y)                        ((0x2cUL << 24)|((((uint32_t)(y)) & 0x1FFFFUL) << 0))
#define EVE_ENC_VERTEX2F(x,y)                                ((0x1UL << 30)|((((uint32_t)(x)) & 0xffffUL) << 15)|((((uint32_t)(y)) & 0xffffUL) << 0))
#define EVE_ENC_VERTEX2II(x,y,handle,cell)                   ((0x2UL << 30)|((((uint32_t)(x)) & 0x1ffUL) << 21)|((((uint32_t)(y)) & 0x1ffUL) << 12)|(((handle) & 0x1fUL) << 7)|(((cell) & 0x7fUL) << 0))





#endif
