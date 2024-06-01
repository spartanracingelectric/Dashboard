#include <U8g2lib.h>
#include "lcd.h"
#include "leds.h"

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *lcd; // changed from SW -> HW

uint32_t prev_millis_lcd = 0;

// values to check current values and help with refresh rate on LCD
uint16_t rpm_prev = -1;
uint8_t gear_prev = -1;
float soc_prev = -1.0f;
float hv_prev = -1.0f; 
float lv_prev = -1.0f;
float hvtemp_prev = -1.0f;
float hvlow_prev = -1.0f;
float oilpress_prev = -1.0f; // float or uint8p
uint8_t watertemp_prev = -1;
uint8_t drs_prev = -1;

// LCD Set-up --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__init(U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *lcd_ptr) // changed from SW -> HW
{
  // Set internal lcd.cpp ptr to whatever is passed into the function
  lcd = lcd_ptr;
  // Begin
  lcd->begin();
}

void lcd__clear_screen() 
{
  lcd->clearBuffer();
}

void lcd__print8(uint8_t x, uint8_t y, const char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  //lcd->clearBuffer();          // clear the internal memory

  //Refuses to take in a passed parameter for some reason
  //Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB08_tr); // choose a suitable font

  lcd->drawStr(x, y, str);  // write something to the internal memory
  lcd->sendBuffer();          // transfer internal memory to the display
}

void lcd__print14(uint8_t x, uint8_t y, const char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  //lcd->clearBuffer();					// clear the internal memory

  //Refuses to take in a passed parameter for some reason
  //Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB14_tr);	// choose a suitable font

  lcd->drawStr(x, y, str);	// write something to the internal memory
  lcd->sendBuffer();					// transfer internal memory to the display
}

void lcd__print18(uint8_t x, uint8_t y, char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  //lcd->clearBuffer();					// clear the internal memory

  //Refuses to take in a passed parameter for some reason
  //Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB18_tr);	// choose a suitable font
  //lcd->setFont(u8g2_font_luRS18_tr); // lucida monospaced font for rpm?

  lcd->drawStr(x, y, str);	// write something to the internal memory
  lcd->sendBuffer();					// transfer internal memory to the display
}

void lcd__print24(uint8_t x, uint8_t y, char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  //lcd->clearBuffer();          // clear the internal memory

  //Refuses to take in a passed parameter for some reason
  //Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB24_tr); // choose a suitable font

  lcd->drawStr(x, y, str);  // write something to the internal memory
  lcd->sendBuffer();          // transfer internal memory to the display
}
void lcd__print_default_screen_template()
{
  char default_str[] = "Created by: johnathon lu";
  lcd__print14(0, 45, default_str);
  delay(100);

  lcd__clear_screen();

  #if (DISPLAY_SCREEN == 0)
    #if (POWERTRAIN_TYPE == 'E')
    lcd__print8(104, 45, "HV T");
    lcd__print8(0, 45, "HV L");
    lcd__print8(45, 28, "VOLTS");
    lcd__print8(47, 40, "SOC%");
    lcd__print8(0, 0, "RPM Screen");
    
    /* #elif(POWERTRAIN_TYPE == 'C')
    lcd__print8(128 - 20, 18, "rpm");
    lcd__print8(52, 37, "Gear");
    lcd__print8(95, 45, "Oil PSI");
    lcd__print8(0, 45, "LV");
    lcd__print8(90, 35, "DRS");
    */
    
    #endif
  #elif (DISPLAY_SCREEN == 1)
    
  #endif
}

void lcd__clear_section (uint8_t sect)
{
  int hvtemp[] = {90, 64-14, 40, 14};
  int hv[] = {30, 0, 70, 18};
  int lv[] = {0, 64-14, 45, 14};
  int soc[] = {40, 64-24, 45, 24};
  int rpm[] = {30, 0, 75,18};
  int gear[] = {50, 64-24, 30, 24};
  int* sections[] = {hvtemp, hv, lv, soc, rpm, gear};
  
  lcd->setDrawColor(0);
  lcd->drawBox(sections[sect][0], sections[sect][1], sections[sect][2], sections[sect][3]);
  lcd->setDrawColor(1);
}

// Combustion Car --------------------------------------------------------------- ---------------------------------------------------------------
/* void lcd__print_rpm(uint16_t rpm)
{
  if (rpm == rpm_prev) return; // if the value is the same, don't update that "section" 
  
  rpm_prev = rpm; // else, update value_prev and redraw that section
  
  char rpm_str[6] = "     ";
  //RPM up to 5 digits
  sprintf(rpm_str, "%5hu", rpm); // transforms int or float or # into a string with a specifying operator in the middle.
  
  lcd__clear_section(4);
  lcd__print18(35, 18, rpm_str);
}
*/

/* void lcd__print_gear(uint8_t gear)
{
  if (gear == gear_prev) return; // if the value is the same, don't update that "section" 
  
  gear_prev = gear; // else, update value_prev and redraw that section
  
  char gear_str[2] = " ";
  //gear is uint8_t, so no negative values expected
  //We only need to compare for gear values past 5
  //If gear out of range
  if (gear > 5) {
    lcd__print24(56, 64, "¡");
    return;
  } else {
    sprintf(gear_str, "%1d", gear);
    lcd__clear_section(5);
    lcd__print24(56, 64, gear_str);
  }
}
*/

/* void lcd__print_oilpress(float oilpress) // Oil coolant? pressure // warn if below 15 psi // float or uint8
{
  if (oilpress == oilpress_prev) return; // if the value is the same, don't update that "section" 
  
  oilpress_prev = oilpress; // else, update value_prev and redraw that section
  
  char oil_str[4] = "   ";
  
  leds__oilpress(oilpress); // updates RGB Oilpress led (bottom right)
  
  sprintf(oil_str, "%3.1f", oilpress); // float or unsigned int?
  lcd__clear_section(0);
  lcd__print14(94, 64, oil_str);
}
*/
// E & C car --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__print_lv(float lv) // low voltage battery
{
  if (lv == lv_prev) return; // if the value is the same, don't update that "section" 
  
  lv_prev = lv; // else, update value_prev and redraw that section
  
  char lv_str[5] = "   ";
  // leds__lv(lv); // update low voltage led (bottom left)
  
  sprintf(lv_str, "%0.1f", lv);
  
  lcd__clear_section(2);
  lcd__print14(0, 64, lv_str);
}

void lcd__print_hvlow(float hvlow) // low voltage battery
{
  if (hvlow == hvlow_prev) return; // if the value is the same, don't update that "section" 
  
  hvlow_prev = hvlow; // else, update value_prev and redraw that section
  
  char hvlow_str[5] = "   ";
  
  sprintf(hvlow_str, "%1.2f", hvlow);
  
  lcd__clear_section(2);
  lcd__print14(0, 64, hvlow_str);
}

void lcd__print_hvtemp(float hvtemp) // Accumulator/Engine temperature
{
  if (hvtemp == hvtemp_prev) return; // if the value is the same, don't update that "section" 
  
  hvtemp_prev = hvtemp; // else, update value_prev and redraw that section
  
  char hvtemp_str[5] = "    ";
  // leds__hvtemp(hvtemp);

  sprintf(hvtemp_str, "%2.1f", hvtemp);

  lcd__clear_section(0);
  lcd__print14(94, 64, hvtemp_str);
}

void lcd__print_drs(uint8_t drs) // DRS Open or Closed: 0 or 1
{
  if (drs == 0){
    lcd__print14(113, 35, "O");
  } else if (drs == 1)
  {
    lcd__print14(113, 35, "M");
  } else if (drs == 2)
  {
    lcd__print14(113, 35, "A");
  } else if (drs == 3)
  {
    lcd__print14(113, 35, "C");
  }

}

// Electric car --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__print_hv(float hv) // accumulator voltage (comes in float or integer?)
{
  if (hv == hv_prev) return; // if the value is the same, don't update that "section" }
    
  hv_prev = hv; // else, update value_prev=value and redraw that section
  // to test: 0 == hv_prev & hv=hv_prev--
  
  char hv_str[6] = "   ";
  // Round to one decimal place
  sprintf(hv_str, "%05.1f", hv);

  lcd__clear_section(1);
  lcd__print18(35, 18, hv_str);
}

void lcd__print_soc(float soc)
{
  if (soc == soc_prev) return; // if the value is the same, don't update

  soc_prev = soc; // else update value_prev = value

  char soc_str[] = "   ";
  // Round to ?? decimal place 
  // sprintf(soc_str, "%05f", soc);

  // lcd__clear_screen(1);
  // lcd__print18(35, 36, soc_str);  
}


// Menu Functions --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__highlight_screen(uint8_t row, const char* screen) // number 0-5
{
  if (row == 5) {
    lcd->drawBox(128 - lcd->getStrWidth(screen) - 2, 64 - lcd->getAscent() - 2, lcd->getStrWidth(screen) + 2, lcd->getAscent() + 2);
    lcd->setDrawColor(0);/* color 0 for the text */
    lcd__print8(128 - lcd->getStrWidth(screen) - 1, 63, screen);
    lcd->setDrawColor(1);
  } else {
    lcd->drawBox(0, row * 12, lcd->getStrWidth(screen) + 2, lcd->getAscent() + 2);
    lcd->setDrawColor(0);/* color 0 for the text */
    lcd__print8(1, 1 + 8 + 12 * row, screen);
    lcd->setDrawColor(1);
  }
}

void lcd__print_screen(uint8_t selection, uint8_t row, const char* screen[]) // 5 row + Back template
{
  // Overlay Highlight Button Selected Screen
  lcd__highlight_screen(selection, screen[selection]);
  
  // Display screens that are not the selected screen
  for (int i = 0; i < row - 1; i++){
    if (selection != i) lcd__print8(1, 1 + 8 + 12 * i, screen[i]);
  }
  if (selection != 5) lcd__print8(128 - lcd->getStrWidth(screen[5]) - 1, 63, screen[5]);
}


void lcd__diagnostics(uint8_t cellfault, uint8_t cellwarn, uint8_t bmsstate)
{
  // Screens
  const char* zero = "Cell Under Voltage Fault";
  const char* one = "Current BMS Status";
  const char* two = "Plceholder";
  const char* three = "Holder of Places";
  const char* four = "Placeholder";
  const char* back = "Back";
  const char* screens[6] = {zero, one, two, three, four, back};
  
  lcd__print_screen(5, 6, screens);
  
  char cellfault_str[2] = " ";
  sprintf(cellfault_str, "%hu", cellfault);
  lcd__print8(56, 9, cellfault_str);

  char bmsstate_str[2] = " ";
  sprintf(bmsstate_str, "%hu", bmsstate);
  lcd__print8(56, 9+12, bmsstate_str);
}

void lcd__print_rpm_diag(uint16_t rpm)
{ 
  char rpm_str[6] = "     ";
  //RPM up to 5 digits
  sprintf(rpm_str, "%5hu", rpm); // transforms int or float or # into a string with a specifying operator in the middle.
  
  lcd__clear_section(4);
  lcd__print18(35, 18, rpm_str);
}

// LCD Screen Update --------------------------------------------------------------- ---------------------------------------------------------------
/* void lcd__update_screen(uint16_t rpm, uint8_t gear, float lv, float oilpress, uint8_t drs, uint32_t curr_millis_lcd) // C Car
{
  if (curr_millis_lcd - prev_millis_lcd >= LCD_UPDATE_MS) {
    prev_millis_lcd = curr_millis_lcd;
    if (DISPLAY_SCREEN == 0) {
      //lcd__print_rpm(rpm);
      //lcd__print_gear(gear);
      lcd__print_lv(lv);
      //lcd__print_oilpress(oilpress);
      lcd__print_drs(drs);
    }
  }
}
*/
void lcd__update_screenE(float hv, float soc, float lv, float hvlow, float hvtemp, uint32_t curr_millis_lcd)
{
  if (curr_millis_lcd - prev_millis_lcd >= LCD_UPDATE_MS) {
    prev_millis_lcd = curr_millis_lcd;

      lcd__print_hv(hv);
      // lcd__print_hvlow(hvlow);
      // lcd__print_lv(lv);
      lcd__print_hvtemp(hvtemp);
    
  }
}
