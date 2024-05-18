//#include "rotarySelection.h"
//const uint8_t butnum = 1; // number of buttons
//const byte button_pins[] = {BUT1}; //, BUT2, BUT3, BUT4};
//int button_flag[ butnum ];
//
//uint32_t last_debounce_time = 0;
//
//// Initialize the buttons, run in the setup function
//void buttons__init() {
//  for (int i = 0; i < butnum; i++) {
//    pinMode(button_pins[i], INPUT);
//    button_flag[i] = 0;
//  }
//  Serial.println("Button init done");
//}
//
//// Poll the buttons and update the respective button flags
//void buttons__poll() {
//  int but1_state = digitalRead(BUT1);
////  int but2_state = digitalRead(BUT2);
////  int but3_state = digitalRead(BUT3);
////  int but4_state = digitalRead(BUT4);
//
//  if (millis() - last_debounce_time > DEBOUNCE_TIME) {
//    if (but1_state == LOW)
//      button_flag[0] = 1;
////    if (but2_state == LOW)
////      button_flag[1] = 1;
////    if (but3_state == LOW)
////      button_flag[2] = 1;
////    if (but4_state == LOW)
////      button_flag[3] = 1;
//  } else {
//    last_debounce_time = millis();
//  }
//  //Serial.println();
//}
//
//// Reset the button flags
//// Usually only need to call after using the update lcd function
//void buttons__flag_reset() {
//  for (int i = 0; i < butnum; i++) {
//    button_flag[i] = 0;
//  }
//}
//
//// Update the lcd to the correct menu
//void buttons__update_LCD() {
//  switch(DISPLAY_SCREEN) {
//    case DEFAULT_SCREEN: //DISPLAY_SCREEN = 0
//      if (button_flag[0]) {
//        DISPLAY_SCREEN = 1;
//      }
////      else if (button_flag[1]) {
//////        indicator__timestamp();
//////        CAN__transmit_timestamp();
////      }
////      else if (button_flag[2] && button_flag[3]) {
////        lcd__menu();
////      }
////      //else if (button_flag[3])
////        //lcd__menu();
//      buttons__flag_reset();
////      
//      break;
////      
//    case MENU_SCREEN: //DISPLAY_SCREEN = 1
//      if (button_flag[0])
//        DISPLAY_SCREEN = 2;
////      else if (button_flag[1])
////        lcd__optionx();
////      else if (button_flag[2])
////        lcd__back();
////      else if (button_flag[3])
////        lcd__optiony(launch_flag);
//      buttons__flag_reset();
//      break;
////    
//    case DIAGNOSTICS_SCREEN: //DISPLAY_SCREEN = 2
//      if (button_flag[0])
//        DISPLAY_SCREEN = 3;
//      buttons__flag_reset();
//      break;
////    
//    case RPM_SCREEN: //DISPLAY_SCREEN = 3
//      if (button_flag[0])
//        DISPLAY_SCREEN = 0;
//      buttons__flag_reset();
//      break;
////    
////    case OPTIONY_SCREEN: //DISPLAY_SCREEN = 4
////      if (button_flag[0]) {
////        CAN__toggle_launch();
////        indicator__blink_bottom();
////        lcd__optiony(launch_flag);
////      }
////      if (button_flag[2])
////        lcd__back();
////      buttons__flag_reset();
////      break;
//  }
//}
