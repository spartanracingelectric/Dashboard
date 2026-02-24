// EVE_base.h  (STM32 HAL port)
#ifndef __EVE_BASE_H__
#define __EVE_BASE_H__

//============================================================================
// Low-Level routines for FTDI / BridgeTek EVE graphic accelerators.
// STM32 HAL port: replaces Arduino/AVR specifics with HAL SPI/GPIO.
//============================================================================

#include <stdint.h>
#include <stdarg.h>

// Public low-level register & command helpers
void _EVE_Select_and_Address(uint32_t Address, uint8_t Operation);
void _EVE_send_32(uint32_t Data);
void EVE_REG_Write_8(uint32_t REG_Address, uint8_t ftData8);
void EVE_REG_Write_16(uint32_t REG_Address, uint16_t ftData16);
void EVE_REG_Write_32(uint32_t REG_Address, uint32_t ftData32);

uint16_t EVE_Cmd_Dat_0(uint16_t FWol, uint32_t command);
uint16_t EVE_Cmd_Dat_1(uint16_t FWol, uint32_t command, uint32_t data0);
uint16_t EVE_Cmd_Dat_2(uint16_t FWol, uint32_t command, uint32_t data0, uint32_t data1);
uint16_t EVE_Cmd_Dat_3(uint16_t FWol, uint32_t command, uint32_t data0, uint32_t data1, uint32_t data2);

uint8_t  EVE_REG_Read_8(uint32_t REG_Address);
uint16_t EVE_REG_Read_16(uint32_t REG_Address);
uint32_t EVE_REG_Read_32(uint32_t REG_Address);
void     EVE_Read_Array(uint32_t EVE_Address, uint16_t length, uint8_t *destination);

// Execution wait
#define ROBUST_EXECUTION_COMPLETE (1)
#if (0 == ROBUST_EXECUTION_COMPLETE)
  #define Wait_for_EVE_Execution_Complete(SW_write_offset) while(EVE_REG_Read_16(EVE_REG_CMD_READ) != SW_write_offset)
#else
  uint16_t Wait_for_EVE_Execution_Complete(uint16_t SW_write_offset);
#endif

uint16_t Get_Free_CMD_Space(uint16_t FWol);
uint16_t Get_RAM_G_Pointer_After_INFLATE(uint16_t FWol, uint32_t *RAM_G_First_Available);
uint16_t Get_RAM_G_Properties_After_LOADIMAGE(uint16_t FWol, uint32_t *RAM_G_First_Available, uint32_t *Width, uint32_t *Height);

void     DBG_GEEK_Decode_Flash_Status(uint8_t EVE_flash_status);
uint16_t EVE_Initialize_Flash(uint16_t FWol);

#if TOUCH_TYPE==TOUCH_RESISTIVE
uint8_t Read_Touch(int16_t x_points[1], int16_t y_points[1]);
#endif
#if TOUCH_TYPE==TOUCH_CAPACITIVE
uint8_t Read_Touch(int16_t x_points[5], int16_t y_points[5]);
#endif
uint8_t  Read_Touch(int16_t x_points[5], int16_t y_points[5]);  // keeps original overload
uint8_t  EVE_Initialize(void);

uint16_t Write_BLOB_to_Flash_Sector_0(uint16_t FWol, uint32_t First_Unused_RAM_G_Address, uint32_t *Flash_Sector);
uint16_t Inflate_uSD_File_To_Flash(uint16_t FWol, const char *File_Name, uint32_t First_Unused_RAM_G_Address, uint32_t *Flash_Sector, uint32_t *Data_Length);
uint16_t Write_uSD_File_To_Flash(uint16_t FWol, const char *File_Name, uint32_t First_Unused_RAM_G_Address, uint32_t *Flash_Sector, uint32_t *Data_Length);

uint16_t EVE_Set_Bitmap(uint16_t FWol,
                        uint8_t  handle,
                        uint32_t source,
                        uint16_t format,
                        uint16_t width,
                        uint16_t height,
                        uint16_t colstride,
                        uint16_t rowstride,
                        uint8_t  filter,
                        uint8_t  wrapx,
                        uint8_t  wrapy);

//============================================================================
// Debug printing (STM32 version)
// Don't call SerPrintFF() directly; use DBG_STAT / DBG_GEEK macros.
//============================================================================
void SerPrintFF(const char *fmt, ...);

// Debug macros (no Arduino F() / flash strings)
#if (DEBUG_LEVEL == DEBUG_NONE)
  #define    DBG_STAT(f,...)
  #define    DBG_GEEK(f,...)
  #define    DBG_GEEK_READ_AND_DUMP_TOUCH_MATRIX(m)
#endif

#if (DEBUG_LEVEL == DEBUG_STATUS)
  #define    DBG_STAT(f,...)    SerPrintFF(f, ##__VA_ARGS__)
  #define    DBG_GEEK(f,...)
  #define    DBG_GEEK_READ_AND_DUMP_TOUCH_MATRIX(m)
#endif

#if (DEBUG_LEVEL == DEBUG_GEEK)
  void Read_and_Dump_Touch_Matrix(const char *message);
  #define    DBG_STAT(f,...)    SerPrintFF(f, ##__VA_ARGS__)
  #define    DBG_GEEK(f,...)    SerPrintFF(f, ##__VA_ARGS__)
  #define    DBG_GEEK_READ_AND_DUMP_TOUCH_MATRIX(m) Read_and_Dump_Touch_Matrix(m)
#endif

#endif // __EVE_BASE_H__
