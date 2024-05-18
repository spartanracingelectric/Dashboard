#include "leds.h"

MD_MAX72XX *leds;

//unsigned long delaytime=65;

// Keep state of leds to easily toggle later
// Default off
uint8_t pin_led_solid_state[NUM_LED_SOLID] = {0};

//Internal variables
uint32_t prev_millis_overrev = 0;
uint32_t prev_millis_revlim = 0;
uint32_t prev_millis_hvlow = 0;
uint32_t prev_millis_hvtemp = 0;


void leds__init(MD_MAX72XX *leds_ptr)
{
  leds = leds_ptr;
  leds->begin();
}

void leds__wake()
{
  for (int led_idx = 0; led_idx < NUM_LED_RGB; led_idx++) {
    leds->setPoint(PIN_LED_RGB_R[led_idx][0],PIN_LED_RGB_R[led_idx][1],true);
    delay(50);
    leds->setPoint(PIN_LED_RGB_G[led_idx][0],PIN_LED_RGB_G[led_idx][1],true);
    delay(50);
    leds->setPoint(PIN_LED_RGB_B[led_idx][0],PIN_LED_RGB_B[led_idx][1],true);
    delay(50);
  }
  for (int led_idx = 0; led_idx < NUM_LED_RGB; led_idx++) {
    leds->setPoint(PIN_LED_RGB_R[led_idx][0],PIN_LED_RGB_R[led_idx][1],false);
    delay(50);
    leds->setPoint(PIN_LED_RGB_G[led_idx][0],PIN_LED_RGB_G[led_idx][1],false);
    delay(50);
    leds->setPoint(PIN_LED_RGB_B[led_idx][0],PIN_LED_RGB_B[led_idx][1],false);
    delay(50);
  }
  //Turn on ALL LEDS
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID; led_idx++) {
    leds->setPoint(PIN_LED_SOLID[led_idx][0],PIN_LED_SOLID[led_idx][1],true);
  }
  delay(100);
  //Turn off ALL LEDS
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID; led_idx++) {
    leds->setPoint(PIN_LED_SOLID[led_idx][0],PIN_LED_SOLID[led_idx][1],false);
  }
  delay(100);
  //Turn on ALL LEDS
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID; led_idx++) {
    leds->setPoint(PIN_LED_SOLID[led_idx][0],PIN_LED_SOLID[led_idx][1],true);
  }
  delay(100);
  //Turn off ALL LEDS
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID; led_idx++) {
    leds->setPoint(PIN_LED_SOLID[led_idx][0],PIN_LED_SOLID[led_idx][1],false);
  }
}

void leds__enable_shift()
{
  //Change internal state of all shift leds to HIGH (exclude LED1 and LED10)
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID-2; led_idx++) {
    pin_led_solid_state[led_idx+1] = 1;
    leds->setPoint(PIN_LED_SOLID[led_idx+1][0], PIN_LED_SOLID[led_idx+1][1], true);
  }
  //If edge LEDs turned on, turn off
  if (pin_led_solid_state[1]) {
    pin_led_solid_state[0] = 0;
    pin_led_solid_state[NUM_LED_SOLID-1] = 0;
    leds->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], false);
    leds->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], false);
  }
}

void leds__disable_shift()
{
  //Change internal state of all shift leds to LOW (exclude LED1 and LED10)
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID-2; led_idx++) {
    pin_led_solid_state[led_idx+1] = 0;
    leds->setPoint(PIN_LED_SOLID[led_idx+1][0], PIN_LED_SOLID[led_idx+1][1], false);
  }
}

void leds__disable_all_solid()
{
  //Change internal state of all leds to LOW
  for (uint8_t led_idx = 0; led_idx < NUM_LED_SOLID; led_idx++) {
    pin_led_solid_state[led_idx] = 0;
    leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], false);
  }
}

void leds__disable_half_solid(bool firstHalf)
{
  uint8_t startIdx, endIdx;
  if (firstHalf) {
    startIdx = 0;
    endIdx = (NUM_LED_SOLID/2);
  }
  else {
    startIdx = NUM_LED_SOLID/2;
    endIdx = NUM_LED_SOLID;
  }
  //Change internal state of all leds to LOW
  for (uint8_t led_idx = startIdx; led_idx < endIdx; led_idx++) {
    pin_led_solid_state[led_idx] = 0;
    leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], false);
  }
}

void leds__toggle_overrev()
{
  for (uint8_t led_idx = 1; led_idx < NUM_LED_SOLID-1; led_idx++) {
    // If led on
    if (pin_led_solid_state[led_idx]) {
      // Then turn off
      pin_led_solid_state[led_idx] = 0;
      leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], false);
    }
    //If led off
    else {
      // Then turn off
      pin_led_solid_state[led_idx] = 1;
      leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], true);
    }
  }
  //If edge LEDs turned on, turn off
  if (pin_led_solid_state[1]) {
    pin_led_solid_state[0] = 0;
    pin_led_solid_state[NUM_LED_SOLID-1] = 0;
    leds->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], false);
    leds->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], false);
  }
}

void leds__toggle_half(bool firstHalf) {
  uint8_t startIdx, endIdx;
  if (firstHalf)
  {
    startIdx = 0;
    endIdx = (NUM_LED_SOLID/2);
  }
  else {
    startIdx = NUM_LED_SOLID/2;
    endIdx = NUM_LED_SOLID;
  }
  for (uint8_t led_idx = startIdx; led_idx < endIdx; led_idx++) {
    // If led on
    if (pin_led_solid_state[led_idx]) {
      // Then turn off
      pin_led_solid_state[led_idx] = 0;
      leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], false);
    }
    //If led off
    else {
      // Then turn off
      pin_led_solid_state[led_idx] = 1;
      leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], true);
    }
  }
}

void leds__toggle_revlim()
{
  for (uint8_t led_idx = 1; led_idx < NUM_LED_SOLID-1; led_idx++) {
    // If led on
    if (pin_led_solid_state[led_idx]) {
      // Then turn off
      pin_led_solid_state[led_idx] = 0;
      leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], false);
    }
    //If led off
    else {
      // Then turn off
      pin_led_solid_state[led_idx] = 1;
      leds->setPoint(PIN_LED_SOLID[led_idx][0], PIN_LED_SOLID[led_idx][1], true);
    }
  }
  // Only check first led state, assume rest are same state
  // Rewrite so we don't assume
  // If shift leds already on, turn off edge effect LEDs (mutually exclusive)
  if (pin_led_solid_state[1]) {
    pin_led_solid_state[0] = 0;
    pin_led_solid_state[NUM_LED_SOLID-1] = 0;
    leds->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], false);
    leds->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], false);
  }
  // Else if shift leds off, turn on edge effect LEDs (mutually exclusive)
  else {
    pin_led_solid_state[0] = 1;
    pin_led_solid_state[NUM_LED_SOLID-1] = 1;
    leds->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], true);
    leds->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], true);
  }
}

void leds__rpm_update_tach(uint16_t rpm) {
  uint8_t leds_to_turn_off;
  uint8_t leds_to_turn_on = 0;
  switch(rpm) {
    case 1 ... 550:
      leds_to_turn_on = 0;
      break;
    case 551 ... 1100:
      leds_to_turn_on = 1;
      break;
    case 1101 ... 1650:
      leds_to_turn_on = 2;
      break;
    case 1651 ... 2200:
      leds_to_turn_on = 3;
      break;
    case 2201 ... 2750:
      leds_to_turn_on = 4;
      break;
    case 2751 ... 3300:
      leds_to_turn_on = 5;
      break;
    case 3301 ... 3850:
      leds_to_turn_on = 6;
      break;
    case 3851 ... 4400:
      leds_to_turn_on = 7;
      break;
    case 4401 ... 4950:
      leds_to_turn_on = 8;
      break;
    case 4951 ... 5499:
      leds_to_turn_on = 9;
      break;
    case 5500: 
      leds_to_turn_on = 10;
    
  }
  //Inverse of # leds ON
  leds_to_turn_off = NUM_LED_SOLID-leds_to_turn_on;
  //Turn on LEDS
  for (uint8_t led_idx = 0; led_idx < leds_to_turn_on; led_idx++) {
    leds->setPoint(PIN_LED_SOLID[led_idx][0],PIN_LED_SOLID[led_idx][1],true);
  }
  //Turn off unused LEDS
  for (uint8_t led_idx = 0; led_idx < leds_to_turn_off; led_idx++) {
    leds->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-led_idx][0],PIN_LED_SOLID[NUM_LED_SOLID-led_idx][1],false);
  }
}

/*
void leds__rpm_update_flash(uint16_t rpm, uint8_t gear, uint32_t curr_millis_flash)
{
  //Variable to hold shift RPM value based on current gear
  uint16_t gear_shift_threshold_rpm;
  //Variable to hold overrev RPM value based on current gear
  uint16_t gear_overrev_threshold_rpm;

  switch (gear) {
    case 1:
      //If in gear 1, turn shift light on based on value of G12_SHIFT_THRESHOLD_RPM
      gear_shift_threshold_rpm = G12_SHIFT_THRESHOLD_RPM;
      break;
    case 2:
      gear_shift_threshold_rpm = G23_SHIFT_THRESHOLD_RPM;
      break;
    case 3:
      gear_shift_threshold_rpm = G34_SHIFT_THRESHOLD_RPM;
      break;
    case 4:
      gear_shift_threshold_rpm = G45_SHIFT_THRESHOLD_RPM;
      break;
//    case 5:
//      gear_shift_threshold_rpm = G56_SHIFT_THRESHOLD_RPM;
//      break;
    default:
      //Default to first gear value
      gear_shift_threshold_rpm = DEFAULT_SHIFT_THRESHOLD_RPM;
      break;
  }
  

  //Add overrev adder to shift threshold to find overrev shift point based on current gear
  gear_overrev_threshold_rpm = gear_shift_threshold_rpm + OVERREV_THRESHOLD_RPM_ADDER;

  // If between min. shift threshold and overrev threshold, solid shift LED bar
  if (rpm > gear_shift_threshold_rpm && rpm <= gear_overrev_threshold_rpm) {
    leds__enable_shift();
  }
  
  // Else if between min. overrev threshold and rev limit threshold, toggle LED bar at OVERREV_THRESHOLD_FLASH_MS Hz
  else if (rpm > gear_overrev_threshold_rpm && rpm <= REVLIM_THRESHOLD_RPM) {
    // Time difference between last known toggle (prev_millis_overrev)
    // and current time (curr_millis_flash)
    if (curr_millis_flash-prev_millis_overrev >= OVERREV_THRESHOLD_FLASH_MS) {
      prev_millis_overrev = curr_millis_flash;
      leds__toggle_overrev();
    }
  }
  // Else if between min. overrev threshold and rev limit threshold, toggle LED bar at REVLIM_THRESHOLD_FLASH_MS Hz
  else if (rpm > REVLIM_THRESHOLD_RPM) {
    // Time difference between last known toggle (prev_millis_revlim)
    // and current time (curr_millis_flash)
    if (curr_millis_flash-prev_millis_revlim >= REVLIM_THRESHOLD_FLASH_MS) {
      prev_millis_revlim = curr_millis_flash;
      leds__toggle_revlim();
    }
  }
  //Turn off all LEDs, nothing to show
  else {
    leds__disable_all_solid();
  }

}
*/

void leds__set_brightness(uint8_t brightness_value)
{
  // SETS BRIGHTNESS VALUE
  // Range is 0 (off) to 0xF (full bright)
  leds->control(MD_MAX72XX::INTENSITY, brightness_value);
}

/*
void leds__oilpress(float oilpress) // float or uint8
{
  if (oilpress < 15) {
    leds->setPoint(3, 4, false); // w/o this, combines Red and Green as both turn on
    leds->setPoint(2, 4, true);
  }
  else {
    leds->setPoint(2, 4, false);
    leds->setPoint(3, 4, true);
  }
}
*/

void leds__lv(float lv)
{
  if (lv < 5.0){
    leds->setPoint(3, 1, true);
  }
  else{
    leds->setPoint(3, 1, false);
  }
 
}

void leds__drsEnable(float drsEnable){
  if(drsEnable == 1){
    leds->setPoint(4, 0, true);
  }
  else{
    leds->setPoint(4, 0, false);
  }
}

void leds__launchReady(float launchReady){
  if(launchReady == 1){
    leds->setPoint(4,4, true);
  }
  else{
    leds->setPoint(4,4, false);
  }

}
void leds__regenMode(int regenmode){
  if(regenmode == 1 | regenmode == 4){
    leds->setPoint(4, 2, true);
  }

  else{
    leds->setPoint(4, 2, false);
  }
}
void leds__hvtemp(float hvtemp)
{
  if(hvtemp > 60){
    leds->setPoint(3, 3, true);
  }

  else{
    leds->setPoint(3, 3, false);
  }
}

void leds__hvlow(float hvlow){
  leds->setPoint(3,5,true);
}

void leds__safety_update_flash(float hvlow, float hvtemp, uint32_t curr_millis) {
  if (hvtemp > 50) { //50 C is VCU limit
    //Toggle 2nd half
    if (curr_millis-prev_millis_hvtemp >= HVTEMP_THRESHOLD_FLASH_MS) {
      prev_millis_hvtemp = curr_millis;
      leds__toggle_half(false);
    }
  }
  else {
    leds__disable_half_solid(false);
  }
  
  if (hvlow < 3.3f) { //3.3 V is VCU limit
    //Toggle 1st half
    if (curr_millis-prev_millis_hvlow >= HVLOW_THRESHOLD_FLASH_MS) {
      prev_millis_hvlow = curr_millis;
      leds__toggle_half(true);
    }
  }
  else {
    leds__disable_half_solid(true);
  }
}

/*
void leds__wake() {
  lc.shutdown(0,false);
  // Set the brightness to a medium values
  lc.setIntensity(0,8);
  // and clear the display
  lc.clearDisplay(0);
}

void leds__start(){
 
 
  lc.setLed(0,0,0,true); // First RGB
  lc.setLed(0,0,3,true); // Second RGB
  lc.setLed(0,1,1,true); // Third RGB
  lc.setLed(0,1,4,true); // Fourth RGB

  // Indicators

  lc.setLed(0,2,0,true); // Right Top LED
  lc.setLed(0,2,5,true); // Left Top LED


  for(int row = 0; row < 5; row++){
      for(int col = 0; col < 10; col++){
            lc.setLed(0, row, col, false);
            delay(delaytime);
            lc.setLed(0, row, col, true); 
      }
  }

   for(int row = 0; row < 5; row++){
      for(int col = 0; col < 10; col++){
            lc.setLed(0, row, col, false);
            delay(delaytime);
      }
  }
  

}
*/
