#include "rotary.h"
#define CLICK_THRESHOLD 100
int counter = 0;

unsigned long lastButtonPress = 0;

void displayRotary(int currentStateCLK, bool currentStateSW, int currentStateDT, int& lastStateCLK, int& displayScreen, int& rowCount, int &torque)
{
 if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
{
  // Serial.println(1);

  // If the DT state is different than the CLK state then
  // the encoder is rotating CCW so decrement
  
  if (currentStateDT == currentStateCLK)
  {
    torque = torque - 10;
  }
  else
  {
    // Encoder is rotating CW so increment
    torque = torque + 10;
  }
}
  // Save the current state of the CLK pin for the next iteration
  lastStateCLK = currentStateCLK;
  
if (currentStateSW == LOW) // LOW is 0V, testing if redefining works
{
  if (millis() - lastButtonPress > 5)
  {
    counter++;
    displayScreen = counter % NUMBER_OF_SCREENS;
    Serial.println("Button pressed!");
  }
 
  // Remember last button press event
  lastButtonPress = millis();
}

delay(1);
}
