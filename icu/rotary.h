#include "config.h"
#include <string>
#include <iostream>
#include <Arduino.h>
using namespace std;



  void displayRotary(int currentStateCLK, bool currentStateSW, int currentStateDT, int& lastStateCLK, int& displayScreen, int& rowCount, int &torque);
// #define CLK 9
// #define DT 10
// #define SW 11
