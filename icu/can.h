#ifndef CAN_H_
#define CAN_H_

#include <SPI.h>
#include <ACAN2515.h>
#include <Arduino.h>


void can__start();
void can__send_test();
void can__receive();

// C car
/* #if (POWERTRAIN_TYPE == 'C')
static void can__gear_receive (const CANMessage & inMessage);
static void can__oilpress_receive (const CANMessage & inMessage);
static void can__lv_receive (const CANMessage & inMessage);
static void can__drs_receive (const CANMessage & inMessage);

uint16_t can__get_rpm();
uint8_t can__get_gear();
float can__get_oilpress();
float can__get_lv();
uint8_t can__get_drs();
*/

// E car
#if (POWERTRAIN_TYPE == 'E')
static void can__hv_receive (const CANMessage & inMessage);
static void can__hv_current_receive (const CANMessage & inMessage);
static void can__soc_receive (const CANMessage & inMessage);
static void can__lv_receive (const CANMessage & inMessage);
static void can__hv_low_receive (const CANMessage & inMessage);
static void can__hvtemp_receive (const CANMessage & inMessage);

// diagnostics ---------------------------
static void can__rpm_receive (const CANMessage & inMessage); // rpm
static void can__bms_fault_receive (const CANMessage & inMessage); // cell under voltage fault
static void can__bms_warn_receive (const CANMessage & inMessage); // cell under voltage warn
static void can__bms_stat_receive (const CANMessage & inMessage); // bms status 0-5

float can__get_hv();
float can__get_soc();
float can__get_lv();
float can__get_hvtemp(); // E car accumulator
float can__get_hv_current();
float can__get_hvlow(); // E car accumulator

// diagnostics ---------------------------
float can__get_rpm(); // not tested
float can__get_bms_fault(); //
float can__get_bms_warn(); //
float can__get_bms_stat(); //

#endif

void can__stop();

#endif /* CAN_H_ */
