#include "can.h"
#include "config.h"

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
float curr_tps0voltage = 0;
float curr_tps0percent = 0;
float curr_tps1voltage = 0;
float curr_tps1percent = 0;

// diagnostics ---------------------------------
float curr_rpm = 0;
float curr_bms_fault = 0;
float curr_bms_warn = 0;
float curr_bms_stat = 0;
//

static void can__lv_receive (const CANMessage & inMessage)
{
  curr_lv = ((inMessage.data[0]) | (inMessage.data[1] << 8)) * 0.001f; // for e car
}

static void can__hv_receive (const CANMessage & inMessage)
{
  curr_hv = ((inMessage.data[4]) | (inMessage.data[5] << 8) | (inMessage.data[6] << 16) | (inMessage.data[7] << 24)) * .001f;
}

static void can__hv_current_receive (const CANMessage & inMessage)
{
  curr_hv_current = ((inMessage.data[0]) | (inMessage.data[1] << 8) | (inMessage.data[2] << 16) | (inMessage.data[3] << 24)) * .001f;
}

static void can__soc_receive (const CANMessage & inMessage)
{
  curr_soc = ((inMessage.data[6]) | (inMessage.data[7] << 8)) * 0.1f;
}

static void can__hvlow_receive (const CANMessage & inMessage)
{
  curr_hvlow = ((inMessage.data[4]) | (inMessage.data[5] << 8)) * 0.001f; // for e car
}

static void can__hvtemp_receive (const CANMessage & inMessage)
{
  curr_hvtemp = ((inMessage.data[7] << 8)  | (inMessage.data[6])) * 0.1f;
}
static void can__tps0_receive(const CANMessage & inMessage) 
{
  curr_tps0percent = (inMessage.data[0]) * 0.392156862746f; // TPS0ThrottlePercentOFF
  curr_tps0voltage = ((inMessage.data[3] << 8) | inMessage.data[2]) * 0.001f;
}
static void can__tps1_receive(const CANMessage & inMessage) 
{
  curr_tps1percent = (inMessage.data[0]) * 0.392156862746; // TPS1ThrottlePercentOFF
  curr_tps1voltage = ((inMessage.data[3] << 8) | inMessage.data[2]) * 0.001f;
}

// diagnostics ---------------------------------
static void can__rpm_receive (const CANMessage & inMessage)
{
  curr_rpm = ((inMessage.data[2]) | (inMessage.data[1] << 8));
  //Serial.println ("Received RPM " + curr_rpm) ;
}
static void can__bms_fault_receive (const CANMessage & inMessage)
{
  curr_bms_fault = inMessage.data[1];
}
static void can__bms_warn_receive (const CANMessage & inMessage)
{
  curr_bms_warn = inMessage.data[1];
}
static void can__bms_stat_receive (const CANMessage & inMessage)
{
  curr_bms_stat = inMessage.data[6];
}
//


//Accessors
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
float can__get_tps0percent()
{
  return curr_tps0percent;
}
float can__get_tps0voltage() 
{
  return curr_tps0voltage;
}
float can__get_tps1percent()
{
  return curr_tps1percent;
}
float can__get_tps1voltage() 
{
  return curr_tps1voltage;
}

// diagnostics ---------------------------------
float can__get_rpm()
{
  return curr_rpm;
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
//
#endif


const ACAN2515Mask rxm0 = standard2515Mask (0x7FF, 0, 0) ;
const ACAN2515Mask rxm1 = standard2515Mask (0x7FF, 0, 0) ;
const ACAN2515Mask rxm2 = standard2515Mask (0x7FF, 0, 0) ;

// POWERTRAIN_TYPE == 'E'
#if (POWERTRAIN_TYPE == 'E')
const ACAN2515AcceptanceFilter filters [] =
{
  
  {standard2515Filter (CAN_TPS0, 0, 0), can__tps0_receive},  // 0x500
  {standard2515Filter (CAN_TPS1, 0, 0), can__tps1_receive}, // 0x501
  {standard2515Filter (CAN_HV_ADDR, 0, 0), can__hv_receive}, // 0x620
  {standard2515Filter (CAN_BAT_TEMP_ADDR, 0, 0), can__hvtemp_receive},  //0x623
  

} ;

#endif

void can__start()
{

  const uint16_t errorCode = can.begin (settings, [] { can.isr () ; },
                                        rxm0, rxm1, filters, 6) ;
  
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
  
  //Non-4eqwd indicates error
  if (errorCode) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX);
  }
 
}

void can__stop()
{
  can.end();
}

static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;

void can__send_test()
{
  CANMessage frame;
  frame.id = 0x7EE;
  frame.len = 8;
  frame.data[0] = 0x53; 
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
  }
  
}