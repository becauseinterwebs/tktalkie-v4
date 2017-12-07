/*
  ControlButton.h - Library for TK-Talkie Control Buttons.
  B. Williams Oct 30, 2017
*/

#ifndef ControlButton_h
#define ControlButton_h

#include "Arduino.h"
#include "Bounce2.h"

#include "VirtualButton.h"

//const byte CB_SOUND = 0;
//const byte CB_PTT   = 1;

class ControlButton
{
  public:
    ControlButton();
    ControlButton(byte buttonPin);
    byte check();
    void setPin(byte buttonPin);
    void setType(byte buttonType);
    byte getType();
    void setup(byte buttonPin);
    VirtualButton buttons[2] = { VirtualButton(), VirtualButton() };
  private:
    byte _pin;
    Bounce _button;
    byte _interval;
    byte _value;
    byte _type;
    char _sound[12];
    void setup();
    
};

#endif
