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
    ControlButton(byte buttonPin, byte interval);
    byte check();
    void update();
    void setPin(byte buttonPin);
    void setType(byte buttonType);
    byte read();
    byte getType();
    void setInterval(byte intv);
    byte getInterval();
    void setup(byte buttonPin);
    boolean rose();
    boolean fell();
    void setPTT(boolean val);
    boolean isPTT();
    VirtualButton buttons[2] = { VirtualButton(), VirtualButton() };
  private:
    byte _pin;
    Bounce _button;
    byte _interval = 15;
    byte _value;
    byte _type;
    char _sound[12];
    boolean _isPTT = false;
    void setup();
    
};

#endif
