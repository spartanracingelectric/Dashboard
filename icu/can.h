#ifndef CAN_H_
#define CAN_H_

#include <SPI.h>
#include <ACAN2515.h>
#include <Arduino.h>


void can__start();
void can__send_test();
void can__receive();


// E car
#if (POWERTRAIN_TYPE == 'E')
static void can__hv_receive (const CANMessage & inMessage);
static void can__hv_current_receive (const CANMessage & inMessage);
static void can__soc_receive (const CANMessage & inMessage);
static void can__lv_receive (const CANMessage & inMessage);
static void can__hv_low_receive (const CANMessage & inMessage);
static void can__hvtemp_receive (const CANMessage & inMessage);
static void can__tps0_receive(const CANMessage & inMessage);
static void can_tps1_receive(const CANMessage & inMessage);

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
float can__get_motortemp(); 
float can__get_tps0voltage(); 
float can__get_tps0percent(); 
float can__get_tps1voltage(); 
float can__get_tps1percent(); 

// diagnostics ---------------------------
float can__get_rpm(); // not tested
float can__get_bms_fault(); //
float can__get_bms_warn(); //
float can__get_bms_stat(); //

#endif

void can__stop();

#endif /* CAN_H_ */