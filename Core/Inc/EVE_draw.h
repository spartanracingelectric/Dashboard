#ifndef __EVE_DRAW_H__
#define __EVE_DRAW_H__
//============================================================================
// Drawing and helper routines for EVE accelerators. (STM32 HAL port)
//============================================================================

#include <stdint.h>

uint16_t EVE_Point(uint16_t FWol,
                   uint16_t point_x,
                   uint16_t point_y,
                   uint16_t ball_size);

uint16_t EVE_Line(uint16_t FWol,
                  uint16_t x0,
                  uint16_t y0,
                  uint16_t x1,
                  uint16_t y1,
                  uint16_t width);

uint16_t EVE_Filled_Rectangle(uint16_t FWol,
                              uint16_t x0,
                              uint16_t y0,
                              uint16_t x1,
                              uint16_t y1);

uint16_t EVE_Open_Rectangle(uint16_t FWol,
                            uint16_t x0,
                            uint16_t y0,
                            uint16_t x1,
                            uint16_t y1,
                            uint16_t width);

/* Text helpers use standard C strings now (no Arduino FlashString) */
uint16_t EVE_Text(uint16_t FWol,
                  uint16_t x,
                  uint16_t y,
                  uint16_t Font,
                  uint16_t Options,
                  const char *message);

/* Don't call _EVE_PrintFF() directly; use EVE_PrintF(...) */
uint16_t _EVE_PrintFF(uint16_t FWol,
                      uint16_t x,
                      uint16_t y,
                      uint16_t Font,
                      uint16_t Options,
                      const char *fmt, ...);

/* Simple passthrough macro (no F() needed) */
#define EVE_PrintF(FWol,x,y,Font,Options,fmt,...) _EVE_PrintFF((FWol),(x),(y),(Font),(Options),(fmt),##__VA_ARGS__)

uint16_t Start_Busy_Spinner_Screen(uint16_t FWol,
                                   uint32_t Clear_Color,
                                   uint32_t Text_Color,
                                   uint32_t Spinner_Color,
                                   const char *message);

uint16_t Stop_Busy_Spinner_Screen(uint16_t FWol,
                                  uint32_t Clear_Color,
                                  uint32_t Text_Color,
                                  const char *message);

uint16_t Calibrate_Touch(uint16_t FWol);

/* Optional PNG loader (works with const data arrays stored in flash/ROM) */
uint16_t EVE_Load_PNG_to_RAM_G(uint16_t FWol,
                               const uint8_t *PNG_data,
                               uint32_t PNG_length,
                               uint32_t *RAM_G_Address,
                               uint32_t *Image_Width,
                               uint32_t *Image_Height);

/* Inflate helpers for pre-compressed (zlib) data blobs */
uint16_t EVE_Inflate_to_RAM_G(uint16_t FWol,
                              const uint8_t *Flash_Data,
                              uint32_t data_length,
                              uint32_t *RAM_G_Address);

/* uSD-to-RAM_G loader: Arduino SD removed; provide your own impl if needed */
void EVE_Load_File_To_RAM_G(uint32_t RAM_G_Address,
                            const char *File_Name,
                            uint32_t *RAM_G_Used);

#endif // __EVE_DRAW_H__
