  #include "can.h"
  #include "config.h"
  #include "lcd.h"
  #include "rotary.h"
  extern int displayScreen;
  #define screen_0 (displayScreen == 0)

  //Skip INT pin for Rev A, set to 0
  #if (BOARD_REVISION == 'A')
  ACAN2515 can (PICO_CAN_SPI_CS, SPI, 15);
  #elif (BOARD_REVISION == 'B')
  ACAN2515 can (PICO_CAN_SPI_CS, SPI1, PICO_CAN_INT);
  #endif

  static const uint32_t QUARTZ_FREQUENCY = 16UL * 1000UL * 1000UL; // 16 MHz
  ACAN2515Settings settings (QUARTZ_FREQUENCY, 500UL * 1000UL) ; // CAN bit rate 500s kb/s

  #if (POWERTRAIN_TYPE == 'E') // ------------------------------------------------
  float curr_hv = 0;
  float curr_soc = 0;
  float curr_lv = 0;
  float curr_hvlow = 0;
  float curr_hvtemp = 0;
  float curr_hv_current = 0;
  int curr_regenmode = 0;
  float curr_drsEnable = 0; // Ensure to make this an LED which turns on when drsEnable is 1
  int curr_drsMode = 0;
  float curr_launchReady = 0;
  float curr_launchStatus = 0;
  float curr_tps0voltage = 0;
  float curr_tps0calibmax = 0;
  float curr_tps1voltage = 0;
  float curr_tps1calibmax = 0;
  float curr_bps0voltage = 0;
  float curr_bps0calibmax = 0;
  int curr_cellovervoltage = 0;
  int curr_packovervoltage = 0;
  int curr_monitorcommfault = 0;
  int curr_prechargefault = 0;
  int curr_failedthermistor = 0;
  float curr_maxtorque;
  int curr_hvil = 1;
  // diagnostics ---------------------------------
  float curr_rpm = 0;
  float curr_pack_voltage = 0;
  float curr_bms_fault = 0;
  float curr_bms_warn = 0;
  float curr_bms_stat = 0;
  //


  static void can__launch_receive(const CANMessage & inMessage){
    curr_launchReady = inMessage.data[0]; // Launch Ready
    /*if (curr_launchReady == 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
    
    curr_launchStatus = inMessage.data[1];
    /* if (curr_launchStatus == 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
    
    
    

  }

  static void can__drs_receive(const CANMessage & inMessage){
    curr_drsMode = inMessage.data[3]; // DRS Mode
    /*if (curr_drsMode == 3){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
    
  curr_drsEnable = inMessage.data[2]; // DRS Enable
  /*if (curr_drsEnable == 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }
  */
  
      
  }

  static void can__regenmode_receive(const CANMessage & inMessage){
    curr_regenmode =  inMessage.data[0];
    curr_maxtorque = inMessage.data[2];
    /*if (curr_regenmode == 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */

    

  }

  static void can__lv_receive (const CANMessage & inMessage)
  {
    curr_lv = ((inMessage.data[0]) | (inMessage.data[1] << 8)) * 0.001f; // for e car
  }

  static void can__hv_receive (const CANMessage & inMessage)
  {
    curr_hv = ((inMessage.data[4]) | (inMessage.data[5] << 8) | (inMessage.data[6] << 16) | (inMessage.data[7] << 24)) * .001f;
    curr_hv_current = ((inMessage.data[0]) | (inMessage.data[1] << 8) | (inMessage.data[2] << 16) | (inMessage.data[3] << 24)) * .001f;
    /* if (curr_hv == 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
  }

  static void can__soc_receive (const CANMessage & inMessage)
  {
    curr_soc = ((inMessage.data[6]) | (inMessage.data[7] << 8)) * 0.1f;
  }

  static void can__hvlow_receive (const CANMessage & inMessage)
  {
    curr_hvlow = ((inMessage.data[5] << 8) | (inMessage.data[4])) * 0.001f;
    /*
    if(curr_hvlow == 5){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
    
    
    
  }

  static void can__hvtemp_receive (const CANMessage & inMessage)
  {
    curr_hvtemp = ((inMessage.data[5] << 8)  | (inMessage.data[4])) * 0.1f;
    /* if (curr_hvtemp == 50){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
  }

  static void can__tps0voltage_receive(const CANMessage & inMessage) 
  {
    curr_tps0voltage = ((inMessage.data[3] << 8) | inMessage.data[2]) * 0.001f;
    curr_tps0calibmax = ((inMessage.data[7] << 8) | inMessage.data[6]) * 0.001f;
  }
  static void can__tps1voltage_receive(const CANMessage & inMessage) 
  {
    curr_tps1voltage = ((inMessage.data[3] << 8) | inMessage.data[2]) * 0.001f;
    curr_tps1calibmax = ((inMessage.data[7] << 8) | inMessage.data[6]) * 0.001f;
  }
  static void can__bps0voltage_receive(const CANMessage & inMessage) 
  {
    curr_bps0voltage = ((inMessage.data[3] << 8) | inMessage.data[2]) * 0.001f;
    curr_bps0calibmax = ((inMessage.data[7] << 8) | inMessage.data[6]) * 0.001f;
  }
  static void can__maxtorque_receive(const CANMessage & inMessage) 
  {
  //  curr_maxtorque = (inMessage.data[2] << 8);
  }
  // diagnostics ---------------------------------
  static void can__rpm_receive (const CANMessage & inMessage)
  {
    curr_rpm = ((inMessage.data[2]) | (inMessage.data[3] << 8));
    /* if (curr_rpm == 1100){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    */
  }
  static void can__bms_safety_checker_receive(const CANMessage & inMessage)
  {
    curr_pack_voltage = ((inMessage.data[3]) | (inMessage.data[4] >> 8) | (inMessage.data[5] >> 16) | (inMessage.data[6] >> 24));
    if(curr_pack_voltage == 100){
      digitalWrite(LED_BUILTIN,HIGH);
    }
  }
  static void can__bms_fault_receive (const CANMessage & inMessage)
  {
    curr_bms_fault = inMessage.data[0] << 16;
    curr_cellovervoltage = (inMessage.data[1] >> 2);
    curr_packovervoltage = inMessage.data[1] >> 4;
    curr_monitorcommfault = inMessage.data[0] >> 1;
    curr_prechargefault = inMessage.data[0] >> 2;
    curr_failedthermistor = inMessage.data[0] >> 5;
    
  }
  static void can__bms_warn_receive (const CANMessage & inMessage)
  {
    curr_bms_warn = inMessage.data[1];
  }
  static void can__bms_stat_receive (const CANMessage & inMessage)
  {
    curr_bms_stat = inMessage.data[6];
  }

  static void can__vcu_safety_receive(const CANMessage &inMessage)
  {
    curr_hvil = inMessage.data[6];
    if (curr_hvil == 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    

  }
  //


  //Accessors
  int can__get_hvil(){
    return curr_hvil;
  }
  float can__get_launchReady(){
    return curr_launchReady;
  }

  float can__get_launchStatus(){
    return curr_launchStatus;
  }


  float can__get_drsEnable(){
    return curr_drsEnable;
  }

  int can__get_drsMode(){
    return curr_drsMode;

  }
  int can__get_regenmode(){
    return curr_regenmode;

  }
  float can__get_hv_current()
  {
    return curr_hv_current;
  }

  float can__get_hv()
  {
    return curr_hv;
  }
  float can__get_soc()
  {
    return curr_soc;
  }

  float can__get_hvtemp() // E car accumulator
  {
    return curr_hvtemp;
  }

  float can__get_lv()
  {
    return curr_lv;
  }

  float can__get_hvlow()
  {
    return curr_hvlow;
  }

  // diagnostics ---------------------------------
  float can__get_rpm()
  {
    return curr_rpm;
  }
  float can__get_bms_safety_checker()
  {
    return curr_pack_voltage; 
  }
  float can__get_bms_fault()
  {
    return curr_bms_fault;
  }

  float can__get_bms_warn()
  {
    return curr_bms_warn;
  }

  float can__get_bms_stat()
  {
    return curr_bms_stat;
  }

  float can__get_tps0voltage() 
  {
    return curr_tps0voltage;
  }
  float can__get_tps0calibmax() 
  {
    return curr_tps0calibmax;
  }
  float can__get_tps1voltage() 
  {
    return curr_tps1voltage;
  }
  float can__get_tps1calibmax() 
  {
    return curr_tps1calibmax;
  }
  float can__get_bps0voltage() 
  {
    return curr_bps0voltage;
  }
  float can__get_bps0calibmax() 
  {
    return curr_bps0calibmax;
  }
  int can__get_cellovervoltage() 
  {
    return curr_cellovervoltage;
  }
  int can__get_packovervoltage() 
  {
    return curr_packovervoltage;
  }
  int can__get_monitorcommfault() 
  {
    return curr_monitorcommfault;
  }
  int can__get_prechargefault()
  {
    return curr_prechargefault;
  }
  int can__get_failedthermistor() 
  {
    return curr_failedthermistor;
  }
  float can__get_maxtorque() {
    return curr_maxtorque;
  }
  //
  #endif


  const ACAN2515Mask rxm0 = standard2515Mask (0x7FF, 0, 0) ;
  const ACAN2515Mask rxm1 = standard2515Mask (0x7FF, 0, 0) ;

  ACAN2515AcceptanceFilter filters0 [] = 
  {
    //Must have addresses in increasing order
    {standard2515Filter (CAN_RPM_ADDR, 0, 0), can__rpm_receive}, //0x0A5
    {standard2515Filter (CAN_REGEN_ADDR, 0, 0), can__regenmode_receive}, //0x508
    //{standard2515Filter (CAN_LAUNCH_ADDR, 0, 0), can__launch_receive}, //0x50B
    {standard2515Filter (CAN_DRS_ADDR, 0,0), can__drs_receive}, //0x50C
    {standard2515Filter (CAN_SAFETY, 0, 0), can__vcu_safety_receive},
    {standard2515Filter (CAN_BMS_SAFETY_CHECKER, 0, 0), can__bms_safety_checker_receive}, // 0x600
    {standard2515Filter (CAN_BAT_TEMP_ADDR, 0, 0), can__hvtemp_receive}, // 0x623
    
  };

  ACAN2515AcceptanceFilter filters1[] = 
  {
    {standard2515Filter (CAN_REGEN_ADDR, 0, 0), can__regenmode_receive}, //0x508
  };

  ACAN2515AcceptanceFilter filters2[] = 
  {
    {standard2515Filter (CAN_TPS0, 0, 0), can__tps0voltage_receive},// 0x500
    {standard2515Filter (CAN_TPS1, 0, 0), can__tps1voltage_receive}, // 0x501
    {standard2515Filter (CAN_BPS0, 0, 0), can__bps0voltage_receive}, // 0x502
  };

   ACAN2515AcceptanceFilter filters3[] = 
   {
    {standard2515Filter (CAN_BPS0, 0, 0), can__bps0voltage_receive}, // 0x502
    // {standard2515Filter (CAN_BMS_FAULT_ADDR, 0, 0), can__bms_fault_receive}, // 0x602
   };

  ACAN2515AcceptanceFilter filters4[] = 
  {
    {standard2515Filter (CAN_LV_ADDR, 0, 0), can__lv_receive}, // 0x507
    {standard2515Filter (CAN_REGEN_ADDR, 0, 0), can__regenmode_receive}, //0x508
    // {standard2515Filter (CAN_BMS_FAULT_ADDR, 0, 0), can__bms_fault_receive}, // 0x602
  };



  void can__start()
  {
    //--- Configure ACAN2515
    
    const uint16_t errorCode = can.begin(settings, [] { can.isr () ; },
                                         rxm0, rxm1, filters0, 6);

    if (errorCode == 0) {
      Serial.print ("Bit Rate prescaler: ") ;
      Serial.println (settings.mBitRatePrescaler) ;
      Serial.print ("Propagation Segment: ") ;
      Serial.println (settings.mPropagationSegment) ;
      Serial.print ("Phase segment 1: ") ;
      Serial.println (settings.mPhaseSegment1) ;
      Serial.print ("Phase segment 2: ") ;
      Serial.println (settings.mPhaseSegment2) ;
      Serial.print ("SJW: ") ;
      Serial.println (settings.mSJW) ;
      Serial.print ("Triple Sampling: ") ;
      Serial.println (settings.mTripleSampling ? "yes" : "no") ;
      Serial.print ("Actual bit rate: ") ;
      Serial.print (settings.actualBitRate ()) ;
      Serial.println (" bit/s") ;
      Serial.print ("Exact bit rate ? ") ;
      Serial.println (settings.exactBitRate () ? "yes" : "no") ;
      Serial.print ("Sample point: ") ;
      Serial.print (settings.samplePointFromBitStart ()) ;
      Serial.println ("%") ;
    } else {
      Serial.print ("Configuration error 0x") ;
      Serial.println (errorCode, HEX) ;
    }
    
    //Non-zero indicates error
    if (errorCode) {
      Serial.print ("Configuration error 0x") ;
      Serial.println (errorCode, HEX);
    }
  
  }

   void can__filtersetup(){
    if (displayScreen == 1){
      can.setFiltersOnTheFly(rxm0, rxm1, filters1, 1);    
    }
    else if(displayScreen == 2){
      can.setFiltersOnTheFly(rxm0, rxm1, filters2, 3);    
    }
    else if(displayScreen == 3){
      can.setFiltersOnTheFly(rxm0, rxm1, filters3, 3);    
    }
    else if(displayScreen == 4){
      can.setFiltersOnTheFly(rxm0, rxm1, filters4, 3);
    }    
    else{
      can.setFiltersOnTheFly(rxm0, rxm1, filters0, 6);
    }
  }

  void can__stop()
  {
    can.end();
  }

  static uint32_t gBlinkLedDate = 0 ;
  static uint32_t gReceivedFrameCount = 0 ;
  static uint32_t gSentFrameCount = 0 ;

  void can__send(uint8_t regenmode)
  {
    CANMessage frame;
    frame.id = 0x702;
    frame.len = 8;
    frame.data[0] = regenmode; 
    if (gBlinkLedDate < millis ()) {
      gBlinkLedDate += 200 ;
      const bool ok = can.tryToSend (frame) ;
      if (ok) {
        gSentFrameCount += 1 ;
        Serial.print ("Sent: ") ;
        Serial.println (gSentFrameCount) ;
      } else {
        Serial.println ("Send failure") ;
      }
    }
  }


  void can__receive()
  {
    can.dispatchReceivedMessage();
    CANMessage frame ;

    if (can.available ()) {
      can.receive (frame) ;
      //Serial.println((frame.data[1]) | (frame.data[0] << 8));
    }
    
  }
