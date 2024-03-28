#ifndef CAN_H_
#define CAN_H_

#include <SPI.h>
#include <ACAN2515.h>
#include <Arduino.h>
// test1

void can__start();
void can__send(uint8_t regenmmode);
void can__receive();

// E car
#if (POWERTRAIN_TYPE == 'E')
static void can__hv_receive (const CANMessage & inMessage);
static void can__hv_current_receive (const CANMessage & inMessage);
static void can__soc_receive (const CANMessage & inMessage);
static void can__lv_receive (const CANMessage & inMessage);
static void can__hv_low_receive (const CANMessage & inMessage);
static void can__hvtemp_receive (const CANMessage & inMessage);
static void can__regenmode_receive(const CANMessage & inMessage);
static void can__drs_receive(const CANMessage & inMessage);
static void can__launch_receive(const CANMessage & inMessage);

static void can__tps0voltage_receive(const CANMessage & inMessage);
static void can__tps1voltage_receive(const CANMessage & inMessage);
static void can__bps0voltage_receive(const CANMessage & inMessage);
static void can__maxtorque_receive(const CANMessage & inMessage);
static void can__vcu_safety_receive(const CANMessage &inMessage);



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
int can__get_regenmode();
float can__get_drsEnable();
int can__get_drsMode();
float can__get_launchReady();
float can__get_launchStatus();
int can__get_hvil();
// diagnostics ---------------------------
float can__get_rpm(); // not tested
float can__get_bms_fault(); //
float can__get_bms_warn(); //
float can__get_bms_stat(); //

float can__get_tps0voltage();
float can__get_tps0calibmax();
float can__get_tps1voltage();
float can__get_tps1calibmax();
float can__get_bps0voltage();
float can__get_bps0calibmax();
int can__get_cellovervoltage();
int can__get_packovervoltage();
int can__get_monitorcommfault();
int can__get_prechargefault();
int can__get_failedthermistor();
float can__get_maxtorque();
void can__filtersetup();
#endif

void can__stop();

#endif /* CAN_H_ */
