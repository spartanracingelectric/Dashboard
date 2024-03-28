#include "rotary.h"
#define CLICK_THRESHOLD 100
int counter = 0;

unsigned long lastButtonPress = 0;

void displayRotary(int currentStateCLK, bool currentStateSW, int currentStateDT, int& lastStateCLK, int& displayScreen, int& rowCount, int &regenmode)
{
  uint8_t mode;
  
 if(displayScreen == 1){
  
 if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
{
  // Serial.println(1);

  // If the DT state is different than the CLK state then
  // the encoder is rotating CCW so decrement
  
  if (currentStateDT == currentStateCLK)
  {
    mode = 4;
    leds__regenModeSet(regenmode, displayScreen, mode);
    //regenmode++;
    can__send(mode);
    Serial.println("its moving");
  }
  else
  {
    mode = 0;
    leds__regenModeSet(regenmode, displayScreen, mode);
    // Encoder is rotating CW so increment
    can__send(mode);
    regenmode--;
    
    Serial.print("is it really running??");
  }
}
  // Save the current state of the CLK pin for the next iteration
  lastStateCLK = currentStateCLK;
 }
if (currentStateSW == LOW) // LOW is 0V, testing if redefining works
{
  if (millis() - lastButtonPress > 5)
  {
    counter++;
    displayScreen = counter % NUMBER_OF_SCREENS;
  }
 
  // Remember last button press event
  lastButtonPress = millis();
}

delay(1);
}
