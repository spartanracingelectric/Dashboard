// EVE_base.cpp  (STM32 HAL port)
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "CFA10100_defines.h"
#include "EVE_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include "stm32h7xx_hal.h"



// ---------------- SPI shim (single-byte full-duplex like Arduino's transfer) ---
#ifndef EVE_SPI_HANDLE
extern SPI_HandleTypeDef hspi4;
#define EVE_SPI_HANDLE hspi4
#endif

static inline uint8_t eve_spi_txrx(uint8_t out)
{
  uint8_t in = 0;
  HAL_SPI_TransmitReceive(&EVE_SPI_HANDLE, &out, &in, 1, HAL_MAX_DELAY);
  return in;
}

// ---------------- Delay ----------------------------------------------------
static inline void eve_delay_ms(uint32_t ms) { HAL_Delay(ms); }

// ---------------- Debug print --------------
void SerPrintFF(const char *fmt, ... )
{
  char tmp[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);

  printf("%s", tmp);
}

// ---------------- Helpers from original implementation -------------------------
void DBG_GEEK_Decode_Flash_Status(uint8_t EVE_flash_status)
{
#if (DEBUG_LEVEL == DEBUG_GEEK)
  DBG_GEEK("REG_FLASH_STATUS = 0x%02X = %3d = \"FLASH_STATUS_",EVE_flash_status,EVE_flash_status);
  switch(EVE_flash_status)
  {
    case EVE_FLASH_STATUS_INIT:     DBG_GEEK("INIT\"\n"); break;
    case EVE_FLASH_STATUS_DETACHED: DBG_GEEK("DETACHED\"\n"); break;
    case EVE_FLASH_STATUS_BASIC:    DBG_GEEK("BASIC (slow)\"\n"); break;
    case EVE_FLASH_STATUS_FULL:     DBG_GEEK("FULL (fast)\"\n"); break;
    default:                        DBG_GEEK("Error/Unknown\"\n"); break;
  }
#endif
}

void DBG_GEEK_Decode_FastFlash_Status(uint32_t EVE_fast_flash_result)
{
#if (DEBUG_LEVEL == DEBUG_GEEK)
  DBG_GEEK("CMD_FLASHFAST result: 0x%08lX = \"",EVE_fast_flash_result);
  switch(EVE_fast_flash_result)
  {
    case 0x00000000: DBG_GEEK("success, no error\"\n"); break;
    case 0x0000E001: DBG_GEEK("flash is not supported\"\n"); break;
    case 0x0000E002: DBG_GEEK("no header detected in sector 0\"\n"); break;
    case 0x0000E003: DBG_GEEK("sector 0 data failed integrity check\"\n"); break;
    case 0x0000E004: DBG_GEEK("device/blob mismatch\"\n"); break;
    case 0x0000E005: DBG_GEEK("failed full-speed test\"\n"); break;
    default:         DBG_GEEK("invalid result returned\"\n"); break;
  }
#endif
}

uint8_t Validate_and_Print_Chip_ID(uint32_t Chip_ID)
{
  uint8_t return_value = 0; // 0 success to match original usage

  uint8_t this_byte;
  this_byte = (Chip_ID >> 24);
  if(0x00 != this_byte) { DBG_GEEK("Chip ID first byte fail. Expected 0x00, got 0x%02X.\n",this_byte); return_value=0; }

  this_byte = (Chip_ID>>16) & 0xFF;
  if(0x01 != this_byte) { DBG_GEEK("Chip ID second byte fail. Expected 0x01, got 0x%02X.\n",this_byte); return_value=0; }

  this_byte = Chip_ID & 0xFF;
  if(0x08 != this_byte) { DBG_GEEK("Chip ID fourth byte fail. Expected 0x08, got 0x%02X.\n",this_byte); return_value=0; }

  this_byte = (Chip_ID>>8) & 0xFF;
  if(this_byte==EVE_DEVICE)
    DBG_GEEK("Chip ID indicates %s8%02X as expected.\n",EVE_DEVICE<0x14?"FT":"BT",EVE_DEVICE);
  else
  {
    DBG_STAT("Unexpected chip if of 0x%02X = %s8%02X, code compiled for %s8%02X\n",
             this_byte, this_byte<0x14?"FT":"BT",this_byte, EVE_DEVICE<0x14?"FT":"BT",EVE_DEVICE);
    return_value=0;
  }
  return return_value;
}

// ---------------- SPI-level accessors (HAL) ------------------------------------
void _EVE_Select_and_Address(uint32_t Address, uint8_t Operation)
{
  CLR_EVE_CS_NOT();
  eve_spi_txrx((uint8_t)(Address >> 16) | Operation);
  eve_spi_txrx((uint8_t)(Address >> 8));
  eve_spi_txrx((uint8_t)(Address));
}

void EVE_Command_Write(uint8_t Command, uint8_t Parameter)
{
  CLR_EVE_CS_NOT();
  eve_spi_txrx(Command);
  eve_spi_txrx(Parameter);
  eve_spi_txrx(0);
  SET_EVE_CS_NOT();
}

void _EVE_send_32(uint32_t Data)
{
  eve_spi_txrx((uint8_t)(Data      ));
  eve_spi_txrx((uint8_t)(Data >>  8));
  eve_spi_txrx((uint8_t)(Data >> 16));
  eve_spi_txrx((uint8_t)(Data >> 24));
}

void EVE_REG_Write_8(uint32_t REG_Address, uint8_t ftData8)
{
  _EVE_Select_and_Address(REG_Address, EVE_MEM_WRITE);
  eve_spi_txrx(ftData8);
  SET_EVE_CS_NOT();
}

void EVE_REG_Write_16(uint32_t REG_Address, uint16_t ftData16)
{
  _EVE_Select_and_Address(REG_Address, EVE_MEM_WRITE);
  eve_spi_txrx((uint8_t)(ftData16));
  eve_spi_txrx((uint8_t)(ftData16 >> 8));
  SET_EVE_CS_NOT();
}

void EVE_REG_Write_32(uint32_t REG_Address, uint32_t ftData32)
{
  _EVE_Select_and_Address(REG_Address, EVE_MEM_WRITE);
  _EVE_send_32(ftData32);
  SET_EVE_CS_NOT();
}

uint16_t EVE_Cmd_Dat_0(uint16_t FWol, uint32_t command)
{
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol,EVE_MEM_WRITE);
  _EVE_send_32(command);
  SET_EVE_CS_NOT();
  return((FWol+4)&0xFFF);
}

uint16_t EVE_Cmd_Dat_1(uint16_t FWol, uint32_t command,uint32_t data0)
{
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol,EVE_MEM_WRITE);
  _EVE_send_32(command);
  _EVE_send_32(data0);
  SET_EVE_CS_NOT();
  return((FWol+8)&0xFFF);
}

uint16_t EVE_Cmd_Dat_2(uint16_t FWol, uint32_t command,uint32_t data0, uint32_t data1)
{
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol,EVE_MEM_WRITE);
  _EVE_send_32(command);
  _EVE_send_32(data0);
  _EVE_send_32(data1);
  SET_EVE_CS_NOT();
  return((FWol+12)&0xFFF);
}

uint16_t EVE_Cmd_Dat_3(uint16_t FWol, uint32_t command, uint32_t data0, uint32_t data1, uint32_t data2)
{
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol,EVE_MEM_WRITE);
  _EVE_send_32(command);
  _EVE_send_32(data0);
  _EVE_send_32(data1);
  _EVE_send_32(data2);
  SET_EVE_CS_NOT();
  return((FWol+16)&0xFFF);
}

uint8_t EVE_REG_Read_8(uint32_t REG_Address)
{
  _EVE_Select_and_Address(REG_Address, EVE_MEM_READ);
  eve_spi_txrx(0);
  uint8_t v = eve_spi_txrx(0);
  SET_EVE_CS_NOT();
  return v;
}

uint16_t EVE_REG_Read_16(uint32_t REG_Address)
{
  _EVE_Select_and_Address(REG_Address, EVE_MEM_READ);
  eve_spi_txrx(0);
  uint16_t v = eve_spi_txrx(0);
  v |= (uint16_t)eve_spi_txrx(0) << 8;
  SET_EVE_CS_NOT();
  return v;
}

uint32_t EVE_REG_Read_32(uint32_t REG_Address)
{
  _EVE_Select_and_Address(REG_Address, EVE_MEM_READ);
  eve_spi_txrx(0);
  uint32_t v  = (uint32_t)eve_spi_txrx(0);
  v |= (uint32_t)eve_spi_txrx(0) << 8;
  v |= (uint32_t)eve_spi_txrx(0) << 16;
  v |= (uint32_t)eve_spi_txrx(0) << 24;
  SET_EVE_CS_NOT();
  return v;
}

void EVE_Read_Array(uint32_t EVE_Address, uint16_t length, uint8_t *destination)
{
  _EVE_Select_and_Address(EVE_Address, EVE_MEM_READ);
  eve_spi_txrx(0);
  while(length--) *destination++ = eve_spi_txrx(0);
  SET_EVE_CS_NOT();
}

// ---------------- Robust coprocessor wait (unchanged logic) --------------------
#if (0 != ROBUST_EXECUTION_COMPLETE)
static uint16_t Reset_EVE_Coprocessor(void)
{
#if ((EVE_DEVICE==BT815)||(EVE_DEVICE==BT816)||(EVE_DEVICE==BT817)||(EVE_DEVICE==BT818))
#if (DEBUG_LEVEL != DEBUG_NONE)
  uint8_t BT81x_error_string[129]; BT81x_error_string[128]=0;
  EVE_Read_Array(EVE_RAM_ERR_REPORT,128,BT81x_error_string);
  DBG_GEEK("  %s\n",BT81x_error_string);
#endif
  uint16_t copro_patch_pointer = EVE_REG_Read_16(EVE_REG_COPRO_PATCH_PTR);
#endif

  uint16_t FWol;
  EVE_REG_Write_32(EVE_REG_CPURESET, 1);
  EVE_REG_Write_32(EVE_REG_CMD_READ, 0);
  EVE_REG_Write_32(EVE_REG_CMD_WRITE, 0);
  EVE_REG_Write_32(EVE_REG_CMD_DL, 0);
  EVE_REG_Write_8(EVE_REG_PCLK, LCD_PCLK);
  FWol = 0;
  EVE_REG_Write_8(EVE_REG_CPURESET, 0);
  eve_delay_ms(100);

#if ((EVE_DEVICE==BT815)||(EVE_DEVICE==BT816)||(EVE_DEVICE==BT817)||(EVE_DEVICE==BT818))
  EVE_REG_Write_16(EVE_REG_COPRO_PATCH_PTR, copro_patch_pointer);
  eve_delay_ms(5);
#endif
  return(FWol);
}

uint16_t Wait_for_EVE_Execution_Complete(uint16_t SW_write_offset)
{
  uint32_t timeout=100000;
  uint16_t reported_read_address=0, reported_write_address=0;

  while (true)
  {
    uint16_t read_address  = EVE_REG_Read_16(EVE_REG_CMD_READ);
    if(0xFFF == read_address)
    {
      DBG_GEEK("Coprocessor Fault detected. Resetting coprocessor.\n");
      return Reset_EVE_Coprocessor();
    }
    uint16_t write_address = EVE_REG_Read_16(EVE_REG_CMD_WRITE);

    if((read_address&0xF003)||(write_address&0xF003)||(SW_write_offset&0xF003)||(write_address!=SW_write_offset))
    {
      if((reported_read_address!=read_address)||(reported_write_address!=write_address))
      {
        if(write_address!=SW_write_offset)
          DBG_GEEK("Write Mismatch: HDW_R=(%5u,0x%04X) HDW_W=(%5u,0x%04X) != SW_W=(%5u,0x%04X)\n",
                   read_address,read_address,write_address,write_address,SW_write_offset,SW_write_offset);
        if((read_address&0xF003)||(write_address&0xF003)||(SW_write_offset&0xF003))
          DBG_GEEK("DWORD Alignment: HDW_R=(%5u,0x%04X) HDW_W=(%5u,0x%04X) SW_W=(%5u,0x%04X)\n",
                   read_address,read_address,write_address,write_address,SW_write_offset,SW_write_offset);
        reported_read_address=read_address;
        reported_write_address=write_address;
      }
    }

    if(timeout) --timeout;
    else {
      DBG_GEEK("Wait_for_EVE_Execution_Complete: 100K tries, not complete.\n");
      DBG_GEEK("  Pointers: HDW_R=(%5u,0x%04X) HDW_W=(%5u,0x%04X) != SW_W=(%5u,0x%04X)\n",
               read_address,read_address,write_address,write_address,SW_write_offset,SW_write_offset);
      timeout=100000;
    }

    if(read_address==SW_write_offset) break;
  }
  return SW_write_offset;
}
#endif

uint16_t Get_Free_CMD_Space(uint16_t FWol)
{
  return((4096-4)-((FWol-EVE_REG_Read_16(EVE_REG_CMD_READ))&0x0FFF));
}

// ---------------- INFLATE / LOADIMAGE helpers (unchanged logic) ---------------
uint16_t Get_RAM_G_Pointer_After_INFLATE(uint16_t FWol, uint32_t *RAM_G_First_Available)
{
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  FWol=EVE_Cmd_Dat_1(FWol, EVE_ENC_CMD_GETPTR,0);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE,FWol);
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  *RAM_G_First_Available=EVE_REG_Read_32(EVE_RAM_CMD+((FWol-4) & 0x0FFF));
  return(FWol);
}

uint16_t Get_RAM_G_Properties_After_LOADIMAGE(uint16_t FWol, uint32_t *RAM_G_First_Available, uint32_t *Width, uint32_t *Height)
{
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  FWol=EVE_Cmd_Dat_3(FWol, EVE_ENC_CMD_GETPROPS,0,0,0);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE,FWol);
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  *Height=EVE_REG_Read_32(EVE_RAM_CMD+((FWol-4) & 0x0FFF));
  *Width =EVE_REG_Read_32(EVE_RAM_CMD+((FWol-8) & 0x0FFF));
  *RAM_G_First_Available=EVE_REG_Read_32(EVE_RAM_CMD+((FWol-12) & 0x0FFF));
  return(FWol);
}

// ---------------- Flash helpers & init (unchanged logic) -----------------------
uint32_t EVE_Set_Flash_to_Fast(uint16_t FWol)
{
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  FWol=EVE_Cmd_Dat_1(FWol, EVE_ENC_CMD_FLASHFAST, 0);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE, (FWol));
  FWol=Wait_for_EVE_Execution_Complete(FWol);

  uint32_t CMD_FastFlash_Result = EVE_REG_Read_32(EVE_RAM_CMD+((FWol-4) & 0x0FFF));

#if (DEBUG_LEVEL == DEBUG_GEEK)
  uint8_t status = EVE_REG_Read_8(EVE_REG_FLASH_STATUS);
  DBG_GEEK_Decode_Flash_Status(status);
  DBG_GEEK_Decode_FastFlash_Status(CMD_FastFlash_Result);
#endif
  return(FWol);
}

uint16_t EVE_Initialize_Flash(uint16_t FWol)
{
  uint8_t status = EVE_REG_Read_8(EVE_REG_FLASH_STATUS);
  DBG_GEEK("\n");
  DBG_GEEK_Decode_Flash_Status(status);
  switch(status)
  {
    case EVE_FLASH_STATUS_INIT:     DBG_STAT("EVE_init_flash(): Error - status is INIT, not BASIC.\n"); break;
    case EVE_FLASH_STATUS_DETACHED: DBG_STAT("EVE_init_flash(): Error - status is DETACHED, not BASIC.\n"); break;
    case EVE_FLASH_STATUS_BASIC:
      FWol=EVE_Set_Flash_to_Fast(FWol);
      { uint32_t flash_size = EVE_REG_Read_32(EVE_REG_FLASH_SIZE);
        DBG_STAT("EVE_init_flash(): EVE_REG_FLASH_SIZE = %lu MB\n",flash_size); }
      break;
    case EVE_FLASH_STATUS_FULL:     DBG_STAT("EVE_init_flash(): Warning - status already FULL.\n"); break;
    default:                        DBG_STAT("EVE_init_flash(): Error - invalid status.\n"); break;
  }
  return(FWol);
}

uint16_t Erase_Entire_Flash_Chip(uint16_t FWol)
{
  DBG_GEEK("Erasing flash (takes a long time) . . . ");
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_FLASHERASE);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE, (FWol));
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  DBG_GEEK("done. ");
  return(FWol);
}

// ---------------- GT911 & Pen-up blobs: drop PROGMEM/pgm_read_byte ------------
#if (EVE_TOUCH_TYPE==EVE_TOUCH_CAPACITIVE) && (EVE_TOUCH_CAP_DEVICE==EVE_CAP_DEV_GT911)
#define GOODIX_GT911_INIT_DATA_LENGTH (1216)
extern const uint8_t Goodix_GT911_Init_Data[GOODIX_GT911_INIT_DATA_LENGTH]; // define in a .c/.cpp or keep here as in original

uint16_t EVE_Init_Goodix_GT911(uint16_t FWol)
{
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol,EVE_MEM_WRITE);

  const uint8_t *Flash_Data = Goodix_GT911_Init_Data;
  uint32_t data_length = GOODIX_GT911_INIT_DATA_LENGTH;
  while(data_length--)
    eve_spi_txrx(*Flash_Data++);

  FWol=(FWol+GOODIX_GT911_INIT_DATA_LENGTH)&0xFFF;
  SET_EVE_CS_NOT();

  EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);
  FWol=Wait_for_EVE_Execution_Complete(FWol);

  EVE_REG_Write_8 (EVE_REG_CPURESET, 0x02);
  EVE_REG_Write_16(EVE_REG_TOUCH_CONFIG, 0x05D0);
  EVE_REG_Write_16(EVE_REG_GPIOX_DIR,0x8008);
  eve_delay_ms(1);
  EVE_REG_Write_8 (EVE_REG_CPURESET, 0x00);
  eve_delay_ms(110);
  EVE_REG_Write_16(EVE_REG_GPIOX_DIR,0x8000);

  return(FWol);
}
#endif

#if (0 != EVE_PEN_UP_BUG_FIX)
#define PEN_UP_BUG_FIX_INIT_DATA_LENGTH (1172)
extern const uint8_t Pen_Up_Bug_Fix_Init_Data[PEN_UP_BUG_FIX_INIT_DATA_LENGTH];

uint16_t EVE_Init_Pen_Up_Bug_Fix(uint16_t FWol)
{
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol,EVE_MEM_WRITE);

  const uint8_t *Flash_Data = Pen_Up_Bug_Fix_Init_Data;
  uint32_t data_length = PEN_UP_BUG_FIX_INIT_DATA_LENGTH;
  while(data_length--) eve_spi_txrx(*Flash_Data++);

  FWol=(FWol+PEN_UP_BUG_FIX_INIT_DATA_LENGTH)&0xFFF;
  SET_EVE_CS_NOT();

  EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);
  FWol=Wait_for_EVE_Execution_Complete(FWol);
  return(FWol);
}
#endif

// ---------------- Touch read (unchanged logic) --------------------------------
#if EVE_TOUCH_TYPE==EVE_TOUCH_CAPACITIVE
uint8_t Read_Touch(int16_t x_points[5], int16_t y_points[5])
{
  uint32_t temp;
  temp = EVE_REG_Read_32(EVE_REG_CTOUCH_TOUCH0_XY); x_points[0]=(uint16_t)(temp>>16); y_points[0]=(uint16_t)temp;
  temp = EVE_REG_Read_32(EVE_REG_CTOUCH_TOUCH1_XY); x_points[1]=(uint16_t)(temp>>16); y_points[1]=(uint16_t)temp;
  temp = EVE_REG_Read_32(EVE_REG_CTOUCH_TOUCH2_XY); x_points[2]=(uint16_t)(temp>>16); y_points[2]=(uint16_t)temp;
  temp = EVE_REG_Read_32(EVE_REG_CTOUCH_TOUCH3_XY); x_points[3]=(uint16_t)(temp>>16); y_points[3]=(uint16_t)temp;
  x_points[4]=EVE_REG_Read_16(EVE_REG_CTOUCH_TOUCH4_X);
  y_points[4]=EVE_REG_Read_16(EVE_REG_CTOUCH_TOUCH4_Y);

  uint8_t points_touched_mask=0, mask=0x01;
  for(uint8_t i=0;i<5;i++)
  {
    if((0==(x_points[i]&0x8000))&&(0==(y_points[i]&0x8000))) points_touched_mask|=mask;
    mask<<=1;
  }
  return(points_touched_mask);
}
#endif

#if EVE_TOUCH_TYPE==EVE_TOUCH_RESISTIVE
uint8_t Read_Touch(int16_t x_points[1], int16_t y_points[1])
{
  uint32_t temp = EVE_REG_Read_32(EVE_REG_TOUCH_SCREEN_XY);
  x_points[0]=(uint16_t)(temp>>16);
  y_points[0]=(uint16_t)temp;
  return ((0==(x_points[0]&0x8000))&&(0==(y_points[0]&0x8000))) ? 0x01 : 0x00;
}
#endif

// ---------------- Debug touch matrix dump (drop AVR-only bits) -----------------
#if (DEBUG_LEVEL==DEBUG_GEEK)
void Read_and_Dump_Touch_Matrix(const char *message)
{
  int32_t touch_transform[6];
  touch_transform[0] = EVE_REG_Read_32(EVE_REG_TOUCH_TRANSFORM_A);
  touch_transform[1] = EVE_REG_Read_32(EVE_REG_TOUCH_TRANSFORM_B);
  touch_transform[2] = EVE_REG_Read_32(EVE_REG_TOUCH_TRANSFORM_C);
  touch_transform[3] = EVE_REG_Read_32(EVE_REG_TOUCH_TRANSFORM_D);
  touch_transform[4] = EVE_REG_Read_32(EVE_REG_TOUCH_TRANSFORM_E);
  touch_transform[5] = EVE_REG_Read_32(EVE_REG_TOUCH_TRANSFORM_F);

  DBG_GEEK("Touch Transform Matrix, %s:\n    {\n", message);
  for(uint8_t i=0;i<=5;i++)
  {
    // Show fixed-point 16.16 as float
    float f = (float)touch_transform[i]/65536.0f;
    DBG_GEEK("    (int32_t)0x%08lX%c // [%c] = %0.4f\n",
              touch_transform[i], i==5?' ':',', i+'A', f);
  }
  DBG_GEEK("    };\n");
}
#endif

// ---------------- Optional: force matrix (drop PROGMEM) -----------------------
#if ((EVE_TOUCH_TYPE!=EVE_TOUCH_NONE) && (0 == EVE_TOUCH_CAL_NEEDED))
void Force_Touch_Matrix(void)
{
  const int32_t touch_transform[6] =
  {
    (int32_t)0x0000FFBD, (int32_t)0xFFFFFE55, (int32_t)0x00085DC6,
    (int32_t)0xFFFFFF00, (int32_t)0x0000F08D, (int32_t)0x0005FF21
  };
  EVE_REG_Write_32(EVE_REG_TOUCH_TRANSFORM_A,touch_transform[0]);
  EVE_REG_Write_32(EVE_REG_TOUCH_TRANSFORM_B,touch_transform[1]);
  EVE_REG_Write_32(EVE_REG_TOUCH_TRANSFORM_C,touch_transform[2]);
  EVE_REG_Write_32(EVE_REG_TOUCH_TRANSFORM_D,touch_transform[3]);
  EVE_REG_Write_32(EVE_REG_TOUCH_TRANSFORM_E,touch_transform[4]);
  EVE_REG_Write_32(EVE_REG_TOUCH_TRANSFORM_F,touch_transform[5]);
}
#endif

// ---------------- Pin drive table instantiation (unchanged macro) -------------
PIN_DRIVE_TABLE;

// ---------------- EVE_Initialize (replace delay() with HAL_Delay) -------------
uint8_t EVE_Initialize(void)
{
  eve_delay_ms(20);
  CLR_EVE_PD_NOT();
  eve_delay_ms(6);
  SET_EVE_PD_NOT();
  eve_delay_ms(21);

#if (EVE_CLOCK_SOURCE == EVE_CLOCK_SOURCE_EXTERNAL)
  EVE_Command_Write(EVE_CLKEXT,0);
  EVE_Command_Write(EVE_CLKSEL,EVE_CLOCK_MUL);
#endif
  DBG_GEEK("EVE speed set to: %u MHz\n",EVE_CLOCK_SPEED/1000000UL);

  EVE_Command_Write(EVE_ACTIVE,0);
  eve_delay_ms(40);

  uint8_t timeout=0, received_register;
  while(0x7C != (received_register=EVE_REG_Read_8(EVE_REG_ID)))
  {
    timeout++;
    if(250 <= timeout)
    {
      DBG_STAT("After %d tries, have not received ID of 0x7C. Last received was 0x%02X\n",timeout,received_register);
      DBG_STAT("Is the device connected? Is the right EVE device selected?");
      timeout=0;
    }
    else eve_delay_ms(1);
  }
  DBG_GEEK("Polled EVE_REG_ID register %d times.\n",timeout);

  timeout=0;
  while(0x00 != (received_register=EVE_REG_Read_8(EVE_REG_CPURESET)))
  {
    timeout++;
    if(250 <= timeout)
    {
      DBG_STAT("After %d tries, have not received EVE_REG_CPURESET of 0x00. Last received was 0x%02X\n",timeout,received_register);
      DBG_STAT("Is the device connected? Is the right EVE device selected?");
      timeout=0;
    }
    else eve_delay_ms(1);
  }
  DBG_GEEK("Polled EVE_REG_CPURESET register %d times.\n",timeout);

  uint32_t Chip_ID = EVE_REG_Read_32(EVE_CHIP_ID_ADDRESS);
  Validate_and_Print_Chip_ID(Chip_ID);

  EVE_REG_Write_32(EVE_REG_FREQUENCY,EVE_CLOCK_SPEED);

#if (0 != EVE_TOUCH_CAL_NEEDED) || ((EVE_TOUCH_TYPE==EVE_TOUCH_CAPACITIVE) && (EVE_TOUCH_CAP_DEVICE==EVE_CAP_DEV_GT911)) || (0 != EVE_PEN_UP_BUG_FIX)
  uint16_t FWo = EVE_REG_Read_16(EVE_REG_CMD_WRITE);
  DBG_GEEK("Initial FWo read from EVE: %u\n",FWo);
#endif

#if (EVE_TOUCH_TYPE==EVE_TOUCH_CAPACITIVE) && (EVE_TOUCH_CAP_DEVICE==EVE_CAP_DEV_GT911)
  FWo=EVE_Init_Goodix_GT911(FWo);
  DBG_GEEK("FWo read from EVE after GT911 initialization: %u Our SW copy: %u\n",EVE_REG_Read_16(EVE_REG_CMD_WRITE),FWo);
#endif
#if (0 != EVE_PEN_UP_BUG_FIX)
  FWo=EVE_Init_Pen_Up_Bug_Fix(FWo);
  DBG_GEEK("FWo read from EVE after Pen Up Bug Fix: %u Our SW copy: %u\n",EVE_REG_Read_16(EVE_REG_CMD_WRITE),FWo);
#endif

  EVE_REG_Write_8(EVE_REG_PCLK, 0);
  EVE_REG_Write_8(EVE_REG_PWM_DUTY, 0);

  EVE_REG_Write_16(EVE_REG_HSIZE,   LCD_WIDTH);
  EVE_REG_Write_16(EVE_REG_HCYCLE,  LCD_HCYCLE);
  EVE_REG_Write_16(EVE_REG_HOFFSET, LCD_HOFFSET);
  EVE_REG_Write_16(EVE_REG_HSYNC0,  LCD_HSYNC0);
  EVE_REG_Write_16(EVE_REG_HSYNC1,  LCD_HSYNC1);
  EVE_REG_Write_16(EVE_REG_VSIZE,   LCD_HEIGHT);
  EVE_REG_Write_16(EVE_REG_VCYCLE,  LCD_VCYCLE);
  EVE_REG_Write_16(EVE_REG_VOFFSET, LCD_VOFFSET);
  EVE_REG_Write_16(EVE_REG_VSYNC0,  LCD_VSYNC0);
  EVE_REG_Write_16(EVE_REG_VSYNC1,  LCD_VSYNC1);
  EVE_REG_Write_8 (EVE_REG_SWIZZLE,  LCD_SWIZZLE);
  EVE_REG_Write_8 (EVE_REG_PCLK_POL, LCD_PCLKPOL);

#if ((EVE_DEVICE == BT815) ||(EVE_DEVICE == BT816) ||(EVE_DEVICE == BT817) || (EVE_DEVICE == BT818))
  for(uint8_t i=0;i< sizeof(Pin_Drive_Table);i++)
  {
    EVE_Command_Write(EVE_PINDRIVE, Pin_Drive_Table[i]);
    DBG_GEEK("Pin_Drive_Table[%2d] = 0x%02x = %3d\n", i, Pin_Drive_Table[i], Pin_Drive_Table[i]);
  }
  EVE_REG_Write_16(EVE_REG_ADAPTIVE_FRAMERATE, 0);
#else
  #if (0 != LCD_DRIVE_10MA)
    EVE_REG_Write_16(EVE_REG_GPIOX,EVE_REG_Read_16(EVE_REG_GPIOX) | 0x1000);
  #else
    EVE_REG_Write_16(EVE_REG_GPIOX,EVE_REG_Read_16(EVE_REG_GPIOX) & ~0x1000);
  #endif
#endif

  #if (0 != LCD_PCLK_CSPREAD)  /* Spread-spectrum */
    EVE_REG_Write_8(EVE_REG_CSPREAD,1);
  #else
    EVE_REG_Write_8(EVE_REG_CSPREAD,0);
  #endif

  #if (0 != LCD_DITHER)
    EVE_REG_Write_8(EVE_REG_DITHER,1);
  #else
    EVE_REG_Write_8(EVE_REG_DITHER,0);
  #endif

#if (EVE_TOUCH_TYPE==EVE_TOUCH_NONE)
  EVE_REG_Write_8(EVE_REG_TOUCH_MODE, EVE_TOUCHMODE_OFF);
  EVE_REG_Write_16(EVE_REG_TOUCH_RZTHRESH, 0);
#endif

#if (EVE_TOUCH_TYPE==EVE_TOUCH_RESISTIVE)
  EVE_REG_Write_16(EVE_REG_TOUCH_RZTHRESH, 1200);
  EVE_REG_Write_8 (EVE_REG_TOUCH_OVERSAMPLE, 6);
  EVE_REG_Write_8 (EVE_REG_TOUCH_MODE, EVE_TOUCHMODE_FRAME);
#endif

#if (EVE_TOUCH_TYPE==EVE_TOUCH_CAPACITIVE)
  EVE_REG_Write_8(EVE_REG_TOUCH_MODE, EVE_TOUCHMODE_FRAME);
  EVE_REG_Write_8(EVE_REG_CTOUCH_EXTENDED, EVE_CTOUCH_MODE_COMPATIBILITY);
#endif

  EVE_REG_Write_8 (EVE_REG_VOL_PB, 0);
  EVE_REG_Write_32(EVE_REG_PLAYBACK_PLAY,0);
  EVE_REG_Write_8 (EVE_REG_VOL_SOUND, 0);
  EVE_REG_Write_16(EVE_REG_SOUND, 0x0060);
  EVE_REG_Write_8 (EVE_REG_PLAY,1);

  EVE_REG_Write_32(EVE_RAM_DL + 0, EVE_ENC_CLEAR_COLOR_RGB(0x00,0x00,0x00));
  EVE_REG_Write_32(EVE_RAM_DL + 4, EVE_ENC_CLEAR(1,1,1));
  EVE_REG_Write_32(EVE_RAM_DL + 8, EVE_ENC_DISPLAY());
  EVE_REG_Write_32(EVE_REG_DLSWAP, EVE_DLSWAP_FRAME);

  EVE_REG_Write_16(EVE_REG_GPIOX,EVE_REG_Read_16(EVE_REG_GPIOX) | 0x8000);
  EVE_REG_Write_8 (EVE_REG_PCLK, LCD_PCLK);

  EVE_REG_Write_16(EVE_REG_PWM_HZ,250);
  EVE_REG_Write_8 (EVE_REG_PWM_DUTY,128);

#if (EVE_TOUCH_TYPE != EVE_TOUCH_NONE)
  #if (0 != EVE_TOUCH_CAL_NEEDED)
    DBG_STAT("Touch calibration . . .");
    FWo=Calibrate_Touch(FWo);
    DBG_GEEK_READ_AND_DUMP_TOUCH_MATRIX("after touch cal");
    DBG_STAT("done.\n");
  #else
    DBG_GEEK("Display specifies no touch calibration is needed.\n");
    Force_Touch_Matrix();
    DBG_GEEK_READ_AND_DUMP_TOUCH_MATRIX("recalled from flash");
  #endif

  #if (EVE_TOUCH_TYPE==EVE_TOUCH_CAPACITIVE) && (EVE_TOUCH_CAP_DEVICE==EVE_CAP_DEV_FT5316)
    EVE_REG_Write_8(EVE_REG_CPURESET, 2);
    eve_delay_ms(1);
    EVE_REG_Write_8(EVE_REG_CPURESET, 0);
    eve_delay_ms(1);
    DBG_GEEK("FT5316 touch device reset.\n");
  #endif

  DBG_GEEK("Waiting for no touch . . .");
  uint8_t  points_touched_mask;
  int16_t  x_points[5];
  int16_t  y_points[5];
  #if (DEBUG_LEVEL == DEBUG_GEEK)
  uint32_t touch_release_polls=0;
  #endif
  do {
    points_touched_mask=Read_Touch(x_points,y_points);
    #if (DEBUG_LEVEL == DEBUG_GEEK)
      touch_release_polls++;
      eve_delay_ms(1);
    #endif
  } while(0 != points_touched_mask);
  DBG_GEEK(" done. Polled %ld times(mS).\n",touch_release_polls);
#endif

  return 0;
}
