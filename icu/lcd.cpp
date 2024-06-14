#include <U8g2lib.h>
#include "lcd.h"
#include "leds.h"

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *lcd; // changed from SW -> HW

uint32_t prev_millis_lcd = 0;

// values to check current values and help with refresh rate on LCD
uint16_t rpm_prev = -1;
uint32_t power_prev = -1;
uint8_t gear_prev = -1;
float soc_prev = -1.0f;
float hv_prev = -1.0f; 
float lv_prev = -1.0f;
float hvtemp_prev = -1.0f;
float hvlow_prev = -1.0f;
float motortemp_prev = -1.0f;
float tps0_voltprev = -1.0f;
float tps1_voltprev = -1.0f;
float oilpress_prev = -1.0f; // float or uint8p
uint8_t watertemp_prev = -1;
uint8_t drs_prev = -1;

// Lookup table for cell voltage to soc (Ah, V, Wh, SOC%)
float soc_lookuptable[4][132] = {
{
0.023f, 0.046f, 0.069f, 0.093f, 0.117f, 0.140f, 0.164f, 0.188f, 0.212f, 0.235f, 
0.259f, 0.283f, 0.306f, 0.330f, 0.354f, 0.378f, 0.401f, 0.425f, 0.449f, 0.472f, 
0.496f, 0.520f, 0.544f, 0.567f, 0.591f, 0.615f, 0.638f, 0.662f, 0.686f, 0.710f, 
0.733f, 0.757f, 0.781f, 0.804f, 0.828f, 0.852f, 0.876f, 0.899f, 0.923f, 0.947f, 
0.970f, 0.994f, 1.018f, 1.042f, 1.065f, 1.089f, 1.113f, 1.136f, 1.160f, 1.184f, 
1.208f, 1.231f, 1.255f, 1.279f, 1.302f, 1.326f, 1.350f, 1.374f, 1.397f, 1.421f, 
1.445f, 1.468f, 1.492f, 1.516f, 1.540f, 1.563f, 1.587f, 1.611f, 1.634f, 1.658f, 
1.682f, 1.706f, 1.729f, 1.753f, 1.777f, 1.800f, 1.824f, 1.848f, 1.872f, 1.895f, 
1.919f, 1.943f, 1.966f, 1.990f, 2.014f, 2.038f, 2.061f, 2.085f, 2.109f, 2.132f, 
2.156f, 2.180f, 2.204f, 2.227f, 2.251f, 2.275f, 2.298f, 2.322f, 2.346f, 2.370f, 
2.393f, 2.417f, 2.441f, 2.463f, 2.485f, 2.505f, 2.524f, 2.545f, 2.561f, 2.577f, 
2.591f, 2.606f, 2.619f, 2.632f, 2.644f, 2.656f, 2.667f, 2.677f, 2.688f, 2.692f, 
2.699f, 2.706f, 2.713f, 2.719f, 2.726f, 2.729f, 2.733f, 2.736f, 2.739f, 2.743f, 
2.747f, 2.751f
}, {
4.137f, 4.118f, 4.103f, 4.093f, 4.085f, 4.077f, 4.072f, 4.069f, 4.065f, 4.062f,
4.060f, 4.057f, 4.055f, 4.052f, 4.050f, 4.047f, 4.044f, 4.039f, 4.034f, 4.028f,
4.019f, 4.010f, 4.001f, 3.992f, 3.981f, 3.970f, 3.959f, 3.949f, 3.939f, 3.929f,
3.919f, 3.911f, 3.902f, 3.894f, 3.886f, 3.879f, 3.872f, 3.865f, 3.858f, 3.851f,
3.843f, 3.835f, 3.826f, 3.819f, 3.812f, 3.803f, 3.795f, 3.788f, 3.778f, 3.771f,
3.763f, 3.754f, 3.746f, 3.739f, 3.729f, 3.721f, 3.712f, 3.704f, 3.695f, 3.688f,
3.678f, 3.670f, 3.664f, 3.656f, 3.649f, 3.642f, 3.635f, 3.628f, 3.621f, 3.614f,
3.609f, 3.602f, 3.595f, 3.590f, 3.583f, 3.576f, 3.569f, 3.565f, 3.557f, 3.549f,
3.540f, 3.533f, 3.526f, 3.516f, 3.508f, 3.499f, 3.489f, 3.479f, 3.469f, 3.459f,
3.448f, 3.437f, 3.429f, 3.422f, 3.415f, 3.407f, 3.400f, 3.390f, 3.379f, 3.362f,
3.341f, 3.319f, 3.296f, 3.271f, 3.246f, 3.219f, 3.189f, 3.154f, 3.125f, 3.100f,
3.074f, 3.048f, 3.023f, 2.998f, 2.974f, 2.946f, 2.921f, 2.895f, 2.868f, 2.842f,
2.817f, 2.788f, 2.759f, 2.730f, 2.700f, 2.672f, 2.647f, 2.619f, 2.592f, 2.563f,
2.532f, 2.504f
}, {
0.095f, 0.093f, 0.097f, 0.097f, 0.097f, 0.097f, 0.097f, 0.096f, 0.096f, 0.096f,
0.096f, 0.096f, 0.096f, 0.096f, 0.096f, 0.096f, 0.096f, 0.096f, 0.096f, 0.096f,
0.095f, 0.095f, 0.095f, 0.095f, 0.094f, 0.094f, 0.094f, 0.094f, 0.093f, 0.093f,
0.093f, 0.093f, 0.093f, 0.092f, 0.092f, 0.092f, 0.092f, 0.092f, 0.091f, 0.091f,
0.091f, 0.091f, 0.091f, 0.091f, 0.090f, 0.090f, 0.090f, 0.090f, 0.090f, 0.089f,
0.089f, 0.089f, 0.089f, 0.089f, 0.088f, 0.088f, 0.088f, 0.088f, 0.088f, 0.087f,
0.087f, 0.087f, 0.087f, 0.087f, 0.087f, 0.086f, 0.086f, 0.086f, 0.086f, 0.086f,
0.086f, 0.085f, 0.085f, 0.085f, 0.085f, 0.085f, 0.085f, 0.085f, 0.084f, 0.084f,
0.084f, 0.084f, 0.084f, 0.083f, 0.083f, 0.083f, 0.083f, 0.082f, 0.082f, 0.082f,
0.082f, 0.082f, 0.081f, 0.081f, 0.081f, 0.081f, 0.081f, 0.080f, 0.080f, 0.080f,
0.079f, 0.079f, 0.078f, 0.074f, 0.070f, 0.066f, 0.060f, 0.066f, 0.051f, 0.049f,
0.043f, 0.044f, 0.039f, 0.039f, 0.036f, 0.037f, 0.032f, 0.028f, 0.031f, 0.014f,
0.019f, 0.019f, 0.019f, 0.015f, 0.019f, 0.009f, 0.011f, 0.008f, 0.009f, 0.010f,
0.010f, 0.010f
}, {
99.1f, 98.1f, 97.2f, 96.2f, 95.3f, 94.3f, 93.3f, 92.4f, 91.4f, 90.5f, 
89.5f, 88.6f, 87.6f, 86.7f, 85.7f, 84.8f, 83.8f, 82.9f, 81.9f, 81.0f, 
80.0f, 79.1f, 78.1f, 77.2f, 76.3f, 75.3f, 74.4f, 73.5f, 72.6f, 71.6f, 
70.7f, 69.8f, 68.9f, 68.0f, 67.0f, 66.1f, 65.2f, 64.3f, 63.4f, 62.5f, 
61.6f, 60.7f, 59.8f, 58.9f, 58.0f, 57.1f, 56.2f, 55.3f, 54.5f, 53.6f, 
52.7f, 51.8f, 50.9f, 50.0f, 49.2f, 48.3f, 47.4f, 46.6f, 45.7f, 44.8f, 
44.0f, 43.1f, 42.2f, 41.4f, 40.5f, 39.7f, 38.8f, 38.0f, 37.1f, 36.3f, 
35.4f, 34.6f, 33.7f, 32.9f, 32.0f, 31.2f, 30.4f, 29.5f, 28.7f, 27.8f, 
27.0f, 26.2f, 25.4f, 24.5f, 23.7f, 22.9f, 22.1f, 21.2f, 20.4f, 19.6f, 
18.8f, 18.0f, 17.2f, 16.4f, 15.6f, 14.8f, 14.0f, 13.2f, 12.4f, 11.6f, 
10.8f, 10.0f, 9.3f, 8.5f, 7.8f, 7.2f, 6.6f, 5.9f, 5.4f, 4.9f, 
4.5f, 4.1f, 3.7f, 3.3f, 3.0f, 2.6f, 2.3f, 2.0f, 1.7f, 1.6f, 
1.4f, 1.2f, 1.0f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 
0.1f, 0.0f 
}
};

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
    lcd__pirnt8(0, 0, "POWER Screen");
    
    
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
  int tps0volt[] = {70, 2, 20, 10};
  int tps1volt[] = {70, 26, 20, 10};
  int rpm[] = {30, 0, 75,18};
  int power[] = {30, 0, 75, 18};
  int gear[] = {50, 64-24, 30, 24};
  int* sections[] = {hvtemp, hv, lv, soc, tps0volt, tps1volt, rpm, power, gear};
  
  lcd->setDrawColor(0);
  lcd->drawBox(sections[sect][0], sections[sect][1], sections[sect][2], sections[sect][3]);
  lcd->setDrawColor(1);
}


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
  
  // sprintf(hvlow_str, "%1.2f", hvlow);
  
  lcd__clear_section(2);
  // lcd__print14(0, 64, hvlow_str);
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

/*void lcd__print_motortemp(float motortemp){
  
}*/

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
  // Check
  leds__hvtemp(1000);
  if (hv == hv_prev) return; // if the value is the same, don't update that "section" }
    
  hv_prev = hv; // else, update value_prev=value and redraw that section
  // to test: 0 == hv_prev & hv=hv_prev--
  
  char hv_str[6] = "   ";
  // Round to one decimal place
  sprintf(hv_str, "%05.1f", hv);

  lcd__clear_section(1);
  lcd__print18(35, 18, hv_str);
}

void lcd__print_soc(float soc, float hv, float tps0percent, float tps1percent) // SOC percentage 
{
  if (soc == soc_prev) return; // If the value is the same, don't update

  soc_prev = soc; // Else update value_prev = value

  char soc_str[6] = "   ";
  if((tps0percent <= 5.0f) && (tps1percent <= 5.0f)){
    //  Convert cell voltage 
    float cellVoltage_converted = (hv/96);
    int indexLower = 131;
    int indexUpper = 0;
    float difference = 0;
    for(int index = 131; index > 0; index--) // Find closest voltage value based on the lookup table -> percentage
    {
      if(soc_lookuptable[2][index] < cellVoltage_converted) indexLower = index; // Saves index of the closeest number lower than cell voltage
      if(soc_lookuptable[2][index] > cellVoltage_converted) indexUpper = index; // Saves index of the closest number higher than cell voltage
      difference = soc_lookuptable[indexUpper] - soc_lookuptable[indexLower];
      if(cellVoltage_converted <= (soc_lookuptable[2][indexLower] + difference)){
        soc = soc_lookuptable[4][indexLower];
      }
      else{
        soc = soc_lookuptable[4][indexUpper];
      }
    }
    
    sprintf(soc_str, "%2.1f", soc);

    // lcd__clear_section(); //Whats the soc section number
    lcd__print18(43, 60, soc_str);  
  }  
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

void lcd__print_power_diag(uint32_t power)
{
  char power_str[6] = "      ";
  leds->setPoint(4, 4, true);
  if (power > 50)
  {
    leds->setPoint(4, 2 true);
    leds->setPoint(3, 2, true);
    if(power > 70){
      leds->setPoint(3, 1, true);
    }
  }
  
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
void lcd__update_screenE(float hv, float soc, float lv, float hvtemp, float hvlow, float tps0volt, float tps0percent, float tps1volt, float tps1percent, uint32_t curr_millis_lcd)
{
  if (curr_millis_lcd - prev_millis_lcd >= LCD_UPDATE_MS) {
    prev_millis_lcd = curr_millis_lcd;

      lcd__print_hv(hv);
      // lcd__print_hvlow(hvlow);
      // lcd__print_lv(lv);
      lcd__print_hvtemp(hvtemp);
      lcd__print_soc(soc, hv, tps0percent, tps1percent);
    
  }
}
