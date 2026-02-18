//============================================================================
// Drawing and helper routines for EVE accelerators. (STM32 HAL port)
//============================================================================

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "CFA10100_defines.h"
#include "EVE_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include "stm32h7xx_hal.h"


/* ------- SPI byte/blocked transmit (matches EVE_base.cpp style) ------- */
#ifndef EVE_SPI_HANDLE
extern SPI_HandleTypeDef hspi1;
#define EVE_SPI_HANDLE hspi1
#endif

static inline void eve_spi_tx(uint8_t b) {
  HAL_SPI_Transmit(&EVE_SPI_HANDLE, &b, 1, HAL_MAX_DELAY);
}
static inline void eve_spi_tx_buf(const uint8_t* buf, uint32_t len) {
  if (len) HAL_SPI_Transmit(&EVE_SPI_HANDLE, const_cast<uint8_t*>(buf), len, HAL_MAX_DELAY);
}

/* ---------------------------- Primitives ------------------------------- */
uint16_t EVE_Point(uint16_t FWol,
                   uint16_t point_x,
                   uint16_t point_y,
                   uint16_t ball_size)
{
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_POINT_SIZE(ball_size));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_BEGIN(EVE_BEGIN_POINTS));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(point_x,point_y));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_END());
  return FWol;
}

uint16_t EVE_Line(uint16_t FWol,
                  uint16_t x0, uint16_t y0,
                  uint16_t x1, uint16_t y1,
                  uint16_t width)
{
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_LINE_WIDTH(width*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_BEGIN(EVE_BEGIN_LINES));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x0*16,y0*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x1*16,y1*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_END());
  return FWol;
}

uint16_t EVE_Filled_Rectangle(uint16_t FWol,
                              uint16_t x0, uint16_t y0,
                              uint16_t x1, uint16_t y1)
{
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_LINE_WIDTH(16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_BEGIN(EVE_BEGIN_RECTS));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x0*16,y0*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x1*16,y1*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_END());
  return FWol;
}

uint16_t EVE_Open_Rectangle(uint16_t FWol,
                            uint16_t x0, uint16_t y0,
                            uint16_t x1, uint16_t y1,
                            uint16_t width)
{
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_LINE_WIDTH(width*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_BEGIN(EVE_BEGIN_LINES));
  // top
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x0*16,y0*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x1*16,y0*16));
  // right
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x1*16,y0*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x1*16,y1*16));
  // bottom
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x1*16,y1*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x0*16,y1*16));
  // left
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x0*16,y1*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_VERTEX2F(x0*16,y0*16));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_END());
  return FWol;
}

/* ----------------------------- Text ----------------------------------- */
uint16_t EVE_Text(uint16_t FWol,
                  uint16_t x,
                  uint16_t y,
                  uint16_t Font,
                  uint16_t Options,
                  const char *message)
{
#if (0 == VIDEO_DEMO)
  _EVE_Select_and_Address(EVE_RAM_CMD|FWol, EVE_MEM_WRITE);

  _EVE_send_32(EVE_ENC_CMD_TEXT);
  _EVE_send_32((((uint32_t)y)<<16) | (x));
  _EVE_send_32((((uint32_t)Options)<<16) | (Font));
  FWol = (FWol + 12) & 0x0FFF;

  // write string bytes
  const uint8_t *p = reinterpret_cast<const uint8_t*>(message);
  while (*p) { eve_spi_tx(*p++); FWol = (FWol + 1) & 0x0FFF; }

  // terminating NUL
  eve_spi_tx(0x00);
  FWol = (FWol + 1) & 0x0FFF;

  // pad to 4-byte alignment
  while (FWol & 0x03) { eve_spi_tx(0x00); FWol = (FWol + 1) & 0x0FFF; }

  SET_EVE_CS_NOT();
#endif
  return FWol;
}

/* printf-style formatted text (no Arduino flash strings) */
uint16_t _EVE_PrintFF(uint16_t FWol,
                      uint16_t x,
                      uint16_t y,
                      uint16_t Font,
                      uint16_t Options,
                      const char *fmt, ... )
{
  char tmp[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  return EVE_Text(FWol, x, y, Font, Options, tmp);
}

/* ------------------------- Spinner screens ----------------------------- */
uint16_t Start_Busy_Spinner_Screen(uint16_t FWol,
                                   uint32_t Clear_Color,
                                   uint32_t Text_Color,
                                   uint32_t Spinner_Color,
                                   const char *message)
{
  FWol = Wait_for_EVE_Execution_Complete(FWol);

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_DLSTART);
  FWol=EVE_Cmd_Dat_0(FWol, Clear_Color);
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CLEAR(1,1,1));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_A(255));

  FWol=EVE_Cmd_Dat_0(FWol, Text_Color);
  FWol=EVE_Text(FWol, LCD_WIDTH/2, LCD_HEIGHT/2, 25, EVE_OPT_CENTER, message);

  FWol=EVE_Cmd_Dat_0(FWol, Spinner_Color);
  /* If you want spinner, issue CMD_SPINNER here once the silicon bug is resolved */

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_DISPLAY());
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_SWAP);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);
  return FWol;
}

uint16_t Stop_Busy_Spinner_Screen(uint16_t FWol,
                                  uint32_t Clear_Color,
                                  uint32_t Text_Color,
                                  const char *message)
{
  FWol = Wait_for_EVE_Execution_Complete(FWol);

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_DLSTART);
  FWol=EVE_Cmd_Dat_0(FWol, Clear_Color);
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CLEAR(1,1,1));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_A(255));

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_STOP);

  FWol=EVE_Cmd_Dat_0(FWol, Text_Color);
  FWol=EVE_Text(FWol, LCD_WIDTH/2, LCD_HEIGHT/2, 25, EVE_OPT_CENTER, message);

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_DISPLAY());
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_SWAP);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);
  return FWol;
}

/* ------------------------- Touch calibration --------------------------- */
uint16_t Calibrate_Touch(uint16_t FWol)
{
  FWol = Wait_for_EVE_Execution_Complete(FWol);

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_DLSTART);
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(0xFF,0xFF,0xFF));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_A(255));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CLEAR_COLOR_RGB(0,0,0xFF));
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CLEAR(1,1,1));

  FWol=EVE_PrintF(FWol, LCD_WIDTH/2, LCD_HEIGHT/2, 25, EVE_OPT_CENTER, "Touch dot to calibrate");
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_CALIBRATE);

  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_DISPLAY());
  FWol=EVE_Cmd_Dat_0(FWol, EVE_ENC_CMD_SWAP);
  EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);

  FWol = Wait_for_EVE_Execution_Complete(FWol);
  return FWol;
}

/* --------------------- PNG/Inflate helpers (no pgm_) ------------------- */
uint16_t EVE_Load_PNG_to_RAM_G(uint16_t FWol,
                               const uint8_t *PNG_data,
                               uint32_t PNG_length,
                               uint32_t *RAM_G_Address,
                               uint32_t *Image_Width,
                               uint32_t *Image_Height)
{
#if (1==LOGO_DEMO) && (0==LOGO_PNG_0_ARGB2_1)
  // width/height read from fixed IHDR offsets (big-endian)
  Image_Width [0] = (uint32_t)PNG_data[16] << 24 | (uint32_t)PNG_data[17] << 16 |
                    (uint32_t)PNG_data[18] << 8  | (uint32_t)PNG_data[19];
  Image_Height[0] = (uint32_t)PNG_data[20] << 24 | (uint32_t)PNG_data[21] << 16 |
                    (uint32_t)PNG_data[22] << 8  | (uint32_t)PNG_data[23];

  uint32_t RAM_G_Needed = (*Image_Width) * (*Image_Height) * 2; // RGB565
  if ((EVE_RAM_G_SIZE - *RAM_G_Address) < RAM_G_Needed) {
    DBG_STAT("EVE_Load_PNG_to_RAM_G(): Need %lu bytes, only %lu available.\n",
             RAM_G_Needed, EVE_RAM_G_SIZE - (*RAM_G_Address));
    return FWol;
  }

  FWol = EVE_Cmd_Dat_2(FWol, EVE_ENC_CMD_LOADIMAGE, *RAM_G_Address, EVE_OPT_NODL);

  PNG_length = (PNG_length + 3) & ~3U;
  while (PNG_length) {
    uint32_t bytes_free = Get_Free_CMD_Space(FWol);
    uint32_t bytes_this_block = (PNG_length <= bytes_free) ? PNG_length : bytes_free;

    _EVE_Select_and_Address((uint32_t)EVE_RAM_CMD | (uint32_t)FWol, EVE_MEM_WRITE);
    eve_spi_tx_buf(PNG_data, bytes_this_block);

    FWol       = (FWol + bytes_this_block) & 0x0FFF;
    PNG_data  += bytes_this_block;
    PNG_length-= bytes_this_block;

    SET_EVE_CS_NOT();
    EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);
    FWol = Wait_for_EVE_Execution_Complete(FWol);
  }

  *RAM_G_Address += RAM_G_Needed;
  *RAM_G_Address  = (*RAM_G_Address + 7) & ~7U;

  uint32_t rb_ptr=0, rb_w=0, rb_h=0;
  FWol = Get_RAM_G_Properties_After_LOADIMAGE(FWol, &rb_ptr, &rb_w, &rb_h);
  if ((*RAM_G_Address!=rb_ptr) || (*Image_Width!=rb_w) || (*Image_Height!=rb_h)) {
    DBG_STAT("EVE_Load_PNG_to_RAM_G(): Read-back mismatch.\n");
  }
#endif
  return FWol;
}

uint16_t EVE_Inflate_to_RAM_G(uint16_t FWol,
                              const uint8_t *Flash_Data,
                              uint32_t data_length,
                              uint32_t *RAM_G_Address)
{
  DBG_GEEK("\n");
  FWol = EVE_Cmd_Dat_1(FWol, EVE_ENC_CMD_INFLATE, *RAM_G_Address);

  data_length = (data_length + 3) & ~3U;
  while (data_length) {
    uint32_t bytes_free = Get_Free_CMD_Space(FWol);
    uint32_t bytes_this_block = (data_length <= bytes_free) ? data_length : bytes_free;

    _EVE_Select_and_Address((uint32_t)EVE_RAM_CMD | (uint32_t)FWol, EVE_MEM_WRITE);
    eve_spi_tx_buf(Flash_Data, bytes_this_block);

    FWol        = (FWol + bytes_this_block) & 0x0FFF;
    Flash_Data += bytes_this_block;
    data_length-= bytes_this_block;

    SET_EVE_CS_NOT();
    EVE_REG_Write_16(EVE_REG_CMD_WRITE, FWol);
    FWol = Wait_for_EVE_Execution_Complete(FWol);
  }

  FWol = Get_RAM_G_Pointer_After_INFLATE(FWol, RAM_G_Address);
  *RAM_G_Address = (*RAM_G_Address + 7) & ~7U;
  return FWol;
}

/* ---------------- uSD → RAM_G loader (stubbed on STM32 by default) ----- */
void EVE_Load_File_To_RAM_G(uint32_t /*RAM_G_Address*/,
                            const char * /*File_Name*/,
                            uint32_t *RAM_G_Used)
{
  /* On STM32, implement with FatFs if needed. For now, report 0 used. */
  if (RAM_G_Used) *RAM_G_Used = 0;
}
