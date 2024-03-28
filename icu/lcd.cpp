#include <U8g2lib.h>
#include "lcd.h"
#include "leds.h"

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *lcd; // changed from SW -> HW

uint32_t prev_millis_lcd = 0;

// values to check current values and help with refresh rate on LCD
uint16_t rpm_prev = -1;
uint8_t gear_prev = -1;
float soc_prev = -1.0f;
float hvil_prev = -1.0f;
float hv_prev = -1.0f;
float lv_prev = -1.0f;
float tps0_voltprev = -1.0f;
float tps0_calibprev = -1.0f;
float tps1_voltprev = -1.0f;
float tps1_calibprev = -1.0f;
float bps0_voltprev = -1.0f;
float bps0_calibprev = -1.0f;
int cov_prev = -1;
int pov_prev = -1;
int mc_prev = -1;
int pc_prev = -1;
int ft_prev = -1;
float maxtorque_prev = -1.0f;
float hvtemp_prev = -1.0f;
float hvlow_prev = -1.0f;
float hvcurr = -1.0f;
float hvcurr_prev = -1.0f;
float oilpress_prev = -1.0f; // float or uint8
uint8_t watertemp_prev = -1;
int drs_prev = -1;
int rgm_prev = -1;
float launch_prev = -1;

// LCD Set-up --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__init(U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *lcd_ptr) // changed from SW -> HW
{
  // Set internal lcd.cpp ptr to whatever is passed into the function
  lcd = lcd_ptr;
  // Begin
  lcd->begin();
}

void lcd__print8(uint8_t x, uint8_t y, const char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  // lcd->clearBuffer();          // clear the internal memory
  // Will print the characters using 8 pixels
  // Refuses to take in a passed parameter for some reason
  // Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB08_tr); // choose a suitable font

  lcd->drawStr(x, y, str); // write something to the internal memory
  lcd->sendBuffer();       // transfer internal memory to the display
}

void lcd__print14(uint8_t x, uint8_t y, const char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  // lcd->clearBuffer();					// clear the internal memory

  // Refuses to take in a passed parameter for some reason
  // Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB14_tr); // choose a suitable font

  lcd->drawStr(x, y, str); // write something to the internal memory
  lcd->sendBuffer();       // transfer internal memory to the display
}

void lcd__print18(uint8_t x, uint8_t y, char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  // lcd->clearBuffer();					// clear the internal memory

  // Refuses to take in a passed parameter for some reason
  // Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB18_tr); // choose a suitable font
  // lcd->setFont(u8g2_font_luRS18_tr); // lucida monospaced font for rpm?

  lcd->drawStr(x, y, str); // write something to the internal memory
  lcd->sendBuffer();       // transfer internal memory to the display
}

void lcd__print24(uint8_t x, uint8_t y, char *str)
{
  // Need to implement a way to update ONLY the space that is to be printed
  // lcd->clearBuffer();          // clear the internal memory

  // Refuses to take in a passed parameter for some reason
  // Bandaid fix to make multiple functions
  lcd->setFont(u8g2_font_helvB24_tr); // choose a suitable font

  lcd->drawStr(x, y, str); // write something to the internal memory
  lcd->sendBuffer();       // transfer internal memory to the display
}

void lcd__clear_screen()
{
  lcd->clearBuffer();
}

void lcd_welcome_screen()
{
  // char default_str[] = "Created by: Sarthak Chauhan";
  // lcd__print14(0, 45, default_str);
  // delay(100);
  // Welcome screen with the Logo.
  // lcd->setFont(u8g2_font_luRS18_tr);
  // Setting the font monospace for the intial welcome screen
  char LOGO_S[] = "S";
  char LOGO_R[] = "R";
  //
  char heading_One[] = "SPARTAN";
  char heading_Two[] = "RACING";

  lcd__print24(5, 45, LOGO_S);
  lcd__print24(25, 45, LOGO_R);

  lcd__print8(50, 27, heading_One);
  lcd__print14(50, 45, heading_Two);

  lcd__clear_screen();
}

void lcd__print_default_screen_template()
{
  // lcd__print8(0, 45, "LV");
  // lcd__print8(45, 28, "VOLTS");
  // lcd__print8(47, 40, "SOC%");
  // lcd__print8(0, 0, "RPM Screen");
  // lcd__print8(0, 10, "HV CURR");
  lcd__print8(101, 28, "BSPD");
  // lcd__print8(100, 43, "REGEN");
  // lcd__print8(57, 58, "LAUNCH"); // Launch Control
  //  lcd__print8(10, 10, "SOC"); // State of Charge
  // lcd__print8(10, 10, "VOL");
  // lcd__print8(10, 20, "0/400"); // Max Pack Voltage
  lcd__print8(0, 20, "HV T"); // Lowest Cell Temp
  lcd__print8(0, 35, "HV");   // Low Voltage
  // lcd__print14(10, 62, "SOC");
  //  lcd__print8(5,5,"------");

  /* #elif(POWERTRAIN_TYPE == 'C')
    lcd__print8(128 - 20, 18, "rpm");
    lcd__print8(52, 37, "Gear");
    lcd__print8(95, 45, "Oil PSI");
    lcd__print8(0, 45, "LV");
    lcd__print8(90, 35, "DRS");
  */
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

void lcd__clear_section(uint8_t sect)
{
  int rgm[] = {100, 35, 20, 14};
  int drs[] = {100, 15, 30, 14};
  int launch[] = {100, 49, 40, 14};
  int hv[] = {35, 10, 65, 35};
  int torque[] = {59, 15, 20, 9};
  int tps0volt[] = {70, 2, 20, 10};
  int tps0calib[] = {80, 14, 20, 10};
  int tps1volt[] = {70, 26, 20, 10};
  int tps1calib[] = {80, 38, 20, 10};
  int bps0volt[] = {70, 50, 20, 10};
  int bps0calib[] = {80, 2, 20, 10};
  int cov[] = {90, 14, 10, 10};
  int pov[] = {90, 26, 10, 10};
  int mc[] = {100, 38, 10, 10};
  int precharge[] = {80, 50, 10, 10};
  int failedtherm[] = {90, 2, 10, 10};
  int lv[] = {65, 14, 20, 10};
  int maxtorque[] = {80, 26, 20, 10};
  int rgm2[] = {81, 13, 10, 10};
  int *sections[] = {rgm, drs, launch, hv, torque, tps0volt, tps0calib, tps1volt, tps1calib, bps0volt, bps0calib, cov, pov, mc, precharge, failedtherm, lv, maxtorque, rgm2};

  lcd->setDrawColor(0);
  lcd->drawBox(sections[sect][0], sections[sect][1], sections[sect][2], sections[sect][3]);

  lcd->sendBuffer();
  lcd->setDrawColor(1);
}

void lcd__print_launch(float launch, int displayScreen)
{

  if (displayScreen == 0)
  {
    if (launch == launch_prev)
    {
      launch_prev = -1;
      return;
    }
    launch_prev = launch;

    char launch_str[5] = "   ";
    if (launch == 1)
    {
      sprintf(launch_str, "%s", "ON");
    }

    else
    {
      sprintf(launch_str, "%s", "OFF");
    }

    lcd__clear_section(2);
    lcd__print8(110, 58, launch_str);
  }
}

void lcd__print_rgm(int rgm, int displayScreen)
{

  if (displayScreen != 0 && displayScreen != 1)
    return;
  if (rgm == rgm_prev)
  {
    rgm_prev = -1;
    return;
  }
  rgm_prev = rgm;
  char rgm_str[5] = "   ";
  if (displayScreen == 0)
  {
    if (rgm == 0)
    {
      sprintf(rgm_str, "%d", rgm);
    }

    if (rgm == 1)
    {
      sprintf(rgm_str, "%s", "H");
    }

    if (rgm == 4)
    {
      sprintf(rgm_str, "%s", "N");
    }

    lcd__clear_section(0);
    lcd__print8(100, 43, rgm_str);
  }

  else if (displayScreen == 1)
  {
    sprintf(rgm_str, "%d", rgm);
    lcd__clear_section(18);
    lcd->sendBuffer();
    lcd__print8(85, 21.5, rgm_str);
  }
}

void lcd__print_tps0voltage(float tps0, int displayScreen)
{
  if (tps0 == tps0_voltprev)
  {
    tps0_voltprev = -1;
    return;
  }
  if (displayScreen == 2)
  {
    tps0_voltprev = tps0;
    char tps0_str[5] = "   ";
    sprintf(tps0_str, "%0.1f", tps0);
    lcd__clear_section(5);
    lcd->sendBuffer();
    lcd__print8(70, 10, tps0_str);
  }
}

void lcd__print_tps0calibmax(float tps0_calib, int displayScreen)
{
  if (tps0_calib == tps0_calibprev)
  {
    tps0_calibprev = -1;
    return;
  }
  if (displayScreen == 2)
  {
    tps0_calibprev = tps0_calib;
    char tps0_str[5] = "   ";
    sprintf(tps0_str, "%0.1f", tps0_calib);
    lcd__clear_section(6);
    lcd->sendBuffer();
    lcd__print8(80, 23, tps0_str);
  }
}

void lcd__print_tps1voltage(float tps1, int displayScreen)
{
  if (tps1 == tps1_voltprev)
  {
    tps1_voltprev = -1;
    return;
  }
  if (displayScreen == 2)
  {
    tps1_voltprev = tps1;
    char tps1_str[5] = "   ";
    sprintf(tps1_str, "%0.1f", tps1);
    lcd__clear_section(7);
    lcd->sendBuffer();
    lcd__print8(70, 34, tps1_str);
  }
}

void lcd__print_tps1calibmax(float tps1_calib, int displayScreen)
{
  if (tps1_calib == tps1_calibprev)
  {
    tps1_calibprev = -1;
    return;
  }
  if (displayScreen == 2)
  {
    tps1_calibprev = tps1_calib;
    char tps1_str[5] = "   ";
    sprintf(tps1_str, "%0.1f", tps1_calib);
    lcd__clear_section(8);
    lcd->sendBuffer();
    lcd__print8(80, 45, tps1_str);
  }
}

void lcd__print_bps0voltage(float bps0, int displayScreen)
{
  if (bps0 == bps0_voltprev)
  {
    bps0_voltprev = -1;
    return;
  }
  if (displayScreen == 2)
  {
    bps0_voltprev = bps0;
    char bps0_str[5] = "   ";
    sprintf(bps0_str, "%0.1f", bps0);
    lcd__clear_section(9);
    lcd->sendBuffer();
    lcd__print8(70, 57, bps0_str);
  }
}

void lcd__print_bps0calib(float bps0_calib, int displayScreen)
{
  if (bps0_calib == bps0_calibprev)
  {
    bps0_calibprev = -1;
    return;
  }
  if (displayScreen == 3)
  {
    bps0_calibprev = bps0_calib;
    char bps0_str[5] = "   ";
    sprintf(bps0_str, "%0.1f", bps0_calib);
    lcd__clear_section(10);
    lcd__print8(80, 10, bps0_str);
  }
}

void lcd__print_cellovervoltage(int cov, int displayScreen)
{
  if (cov == 5)
    cov = 1;
  if (cov == 4)
    cov = 0;
  if (cov == cov_prev)
  {
    cov_prev = -1;
    return;
  }
  if (displayScreen == 3)
  {
    cov_prev = cov;
    char cov_str[5] = "   ";
    sprintf(cov_str, "%d", cov);
    lcd__clear_section(11);
    lcd__print8(90, 23, cov_str);
  }
}

void lcd__print_packovervoltage(int pov, int displayScreen)
{
  if (pov == pov_prev)
  {
    pov_prev = -1;
    return;
  }
  if (displayScreen == 3)
  {
    pov_prev = pov;
    char pov_str[5] = "   ";
    sprintf(pov_str, "%d", pov);
    lcd__clear_section(12);
    lcd__print8(90, 35, pov_str);
  }
}

void lcd__print_monitorcomm(int mc, int displayScreen)
{
  if (mc == 3 || mc == 19 || mc == 17)
    mc = 1;
  if (mc == 2 || mc == 18 || mc == 16)
    mc = 0;
  if (mc == mc_prev)
  {
    mc_prev = -1;
    return;
  }
  if (displayScreen == 3)
  {
    mc_prev = mc;
    char mc_str[5] = "   ";
    sprintf(mc_str, "%d", mc);
    lcd__clear_section(13);
    lcd__print8(100, 46, mc_str);
  }
}

void lcd__print_precharge(int pc, int displayScreen)
{
  if (pc == 8)
    pc = 0;
  if (pc == 9)
    pc = 1;
  if (pc == pc_prev)
  {
    pc_prev = -1;
    return;
  }
  if (displayScreen == 3)
  {
    pc_prev = pc;
    char pc_str[5] = "   ";
    sprintf(pc_str, "%d", pc);
    lcd__clear_section(14);
    lcd__print8(80, 57, pc_str);
  }
}

void lcd__print_failedthermistor(int ft, int displayScreen)
{
  if (ft == ft_prev)
  {
    ft_prev = -1;
    return;
  }
  if (displayScreen == 4)
  {
    ft_prev = ft;
    char ft_str[5] = "   ";
    sprintf(ft_str, "%d", ft);
    lcd__clear_section(15);
    lcd__print8(90, 10, ft_str);
  }
}

void lcd__print_lv(float lv, int displayScreen) // low voltage battery
{
  if (lv == lv_prev)
  {
    lv_prev = -1;
    return;
  }
  if (displayScreen == 4)
  {
    lv_prev = lv;
    char lv_str[5] = "   ";
    sprintf(lv_str, "%0.1f", lv);
    lcd__clear_section(16);
    lcd__print8(65, 22, lv_str);
  }
}

void lcd__print_maxtorque(float mt, int displayScreen)
{
  if (mt == maxtorque_prev)
  {
    maxtorque_prev = -1;
    return;
  }

  if (displayScreen == 4)
  {
    maxtorque_prev = mt;
    char mt_str[5] = "   ";
    sprintf(mt_str, "%0.1f", mt);
    lcd__clear_section(17);
    lcd__print8(80, 32, mt_str);
  }
}

void lcd__print_hvlow(float hvlow) // low voltage battery
{
  if (hvlow == hvlow_prev)
    return; // if the value is the same, don't update that "section"

  hvlow_prev = hvlow; // else, update value_prev and redraw that section

  char hvlow_str[5] = "   ";

  sprintf(hvlow_str, "%1.2f", hvlow);

  // lcd__clear_section(2);
  lcd__print14(0, 64, hvlow_str);
}

void lcd__print_hvcurr(float hvcurr) // hv current
{
  if (hvcurr == hvcurr_prev)
    return; // if the value is the same, don't update that "section"

  hvcurr_prev = hvcurr; // else, update value_prev and redraw that section

  char hvcurr_str[5] = "   ";

  sprintf(hvcurr_str, "%1.0f", hvcurr);

  // lcd__clear_section(2);
  lcd__print14(0, 34, hvcurr_str);
}

void lcd__print_hvtemp(float hvtemp) // Accumulator/Engine temperature
{
  if (hvtemp == hvtemp_prev)
    return; // if the value is the same, don't update that "section"

  hvtemp_prev = hvtemp; // else, update value_prev and redraw that section

  char hvtemp_str[5] = "    ";
  //  leds__hvtemp(hvtemp);

  sprintf(hvtemp_str, "%2.1f", hvtemp);

  // lcd__clear_section(0);
  lcd__print14(94, 64, hvtemp_str);
}

void lcd__print_drs(int drs, int displayScreen)
{
  if (displayScreen == 0)
  {
    if (drs == drs_prev)
    {
      drs_prev = -1;
      return;
    }
    drs_prev = drs;

    char drs_str[5] = "    ";

    if (drs == 0)
    {
      sprintf(drs_str, "%s", "OFF");
    }
    else if (drs == 1)
    {
      sprintf(drs_str, "%s", "ON");
    }
    else if (drs == 2)
    {
      sprintf(drs_str, "%s", "MAN");
    }
    else if (drs == 3)
    {
      sprintf(drs_str, "%s", "AUTO");
    }

    lcd__clear_section(1);
    lcd__print8(100, 25, drs_str);
  }
}

// Electric car --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__print_hv(float hv) // accumulator voltage (comes in float or integer?)
{
    if (hv == hv_prev)
    { // if the value is the same, don't update that "section"
      hv_prev = -1;
      return;
    }

    hv_prev = hv; // else, update value_prev=value and redraw that section
    // to test: 0 == hv_prev & hv=hv_prev--

    char hv_str[6] = "   ";
    // Round to one decimal place
    sprintf(hv_str, "%05.1f", hv);

    lcd__clear_section(3);
    lcd__print18(35, 40, hv_str);
}

void lcd__print_hvil(float hvil, int displayScreen, int prevDisplayScreen)
{
  if (displayScreen == 0)
  {
    if (hvil == hvil_prev && displayScreen == prevDisplayScreen)
    { // if the value is the same, don't update that "section"
      hvil = -1;

      return;
    }

    hvil_prev = hvil; // else, update value_prev=value and redraw that section
    // to test: 0 == hv_prev & hv=hv_prev--

    char hvil_str[5] = " ";
    sprintf(hvil_str, "%5.1f", hvil);

    lcd__clear_section(3);
    lcd__print14(35, 60, hvil_str);
  }
}

void lcd__print_soc(float soc) // State of charge 0-100%
{
  if (soc == soc_prev)
    return; // if the value is the same, don't update that "section"

  soc_prev = soc; // else, update value_prev=value and redraw that section

  char soc_str[5] = "    ";

  if (soc_prev == 100)
  {
    sprintf(soc_str, "%3.0f", soc);
    // sprintf loads the string to the first argument(soc_str)
  }
  else
  {
    sprintf(soc_str, "%3.1f", soc);
  }

  // lcd__clear_section(3);
  lcd__print14(70, 62, soc_str);
}
// Menu Functions --------------------------------------------------------------- ---------------------------------------------------------------
void lcd__highlight_screen(uint8_t row, const char *screen) // number 0-5
{
  if (row == 5)
  {
    lcd->drawBox(128 - lcd->getStrWidth(screen) - 2, 64 - lcd->getAscent() - 2, lcd->getStrWidth(screen) + 2, lcd->getAscent() + 2);
    lcd->setDrawColor(0); /* color 0 for the text */
    lcd__print8(128 - lcd->getStrWidth(screen) - 1, 63, screen);
    lcd->setDrawColor(1);
  }
  else
  {
    lcd->drawBox(0, row * 12, lcd->getStrWidth(screen) + 2, lcd->getAscent() + 2);
    lcd->setDrawColor(0); /* color 0 for the text */
    lcd__print8(1, 1 + 8 + 12 * row, screen);
    lcd->setDrawColor(1);
  }
}

void lcd__print_screen(uint8_t selection, uint8_t row, const char *screen[], int &prevRowCount) // 5 row + Back template
{
  // Overlay Highlight Button Selected Screen
  lcd__highlight_screen(selection, screen[selection]);

  // Display screens that are not the selected screen
  for (int i = 0; i < row - 1; i++)
  {
    if (selection != i)
      lcd__print8(1, 1 + 8 + 12 * i, screen[i]);
  }
  if (selection != 5)
    lcd__print8(128 - lcd->getStrWidth(screen[5]) - 1, 63, screen[5]);
}

// isn't in use
void lcd__menu(int rowCount, int prevRowCount)
{
  //  #define TESTING_SCREEN 0
  // #define MINIMALIST_SCREEN 1
  // #define DEBUG_BMS_SCREEN 2
  // #define DEBUG_VCU_SCREEN 3
  // #define MENU_SCREEN 4
  // Screens
  const char *zero = "Driver Screen";
  const char *one = "Debug Screen";
  const char *two = "------------";
  const char *three = "------------";
  const char *four = "------------";
  const char *back = "Back";
  const char *screens[6] = {zero, one, two, three, four, back};

  lcd__print_screen(5, 6, screens, prevRowCount);
}

void lcd__debugscreen(int rowCount, int prevRowCount)
{
  // Screens
  const char *zero = "TPS0Voltage";
  const char *one = "TPS0CalibMax";
  const char *two = "TPS1Voltage";
  const char *three = "TPS1CalibMax";
  const char *four = "BPS0Voltage";
  const char *back = "Back";
  const char *screens[6] = {zero, one, two, three, four, back};

  lcd__print_screen(5, 6, screens, prevRowCount);
}

void lcd__debugscreen2(int rowCount, int prevRowCount)
{
  // Screens
  const char *zero = "BPS0CalibMax";
  const char *one = "CellOverVoltage";
  const char *two = "PackOverVoltage";
  const char *three = "MonitorCommFault";
  const char *four = "PrechargeFault";
  const char *back = "Back";
  const char *screens[6] = {zero, one, two, three, four, back};

  lcd__print_screen(5, 6, screens, prevRowCount);
}

void lcd__debugscreen3(int rowCount, int prevRowCount)
{
  // Screens
  const char *zero = "FailedThermistor";
  const char *one = "LowVoltage";
  const char *two = "MaxTorqueSet";
  const char *three = "------------";
  const char *four = "------------";
  const char *back = "Back";
  const char *screens[6] = {zero, one, two, three, four, back};

  lcd__print_screen(5, 6, screens, prevRowCount);
}

void lcd_settings(int rowCount, int prevRowCount)
{
  const char *zero = "                Settings";
  const char *one = "Regen on Car: ";
  const char *two = "------------";
  const char *three = "------------";
  const char *four = "------------";
  const char *back = "Back";
  const char *screens[6] = {zero, one, two, three, four, back};
  lcd__print_screen(1, 6, screens, prevRowCount);
}

void lcd__update_screenE(float hvil, float hv, float soc, float lv, float hvlow, float hvtemp, float hvcurr, int drsMode, int regenmode,
                         float launchReady, float tps0volt, float tps0calib, float tps1volt, float tps1calib, float bps0volt,
                         float bps0calib, int cell_over_volt, int pack_over_volt, int monitor_comm, int precharge, int failedthermistor, float maxtorque, uint32_t curr_millis_lcd)
{
  if (curr_millis_lcd - prev_millis_lcd >= LCD_UPDATE_MS)
  {
    prev_millis_lcd = curr_millis_lcd;
    lcd__clear_screen();
    lcd__print_default_screen_template();
    lcd__print_hv(hv);
  }
}
