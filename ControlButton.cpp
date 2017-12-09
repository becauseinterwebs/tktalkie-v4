#include "Arduino.h"
#include "Bounce2.h"
#include "ControlButton.h"

ControlButton::ControlButton()
{
  setup();
  this->_button = Bounce();
}

ControlButton::ControlButton(byte buttonPin)
{
  this->setup();
  this->_button = Bounce();
  this->setPin(buttonPin);
}

ControlButton::ControlButton(byte buttonPin, byte interval)
{
  this->setup();
  this->_interval = interval;
  this->_button = Bounce();
  this->setPin(buttonPin);
}

byte ControlButton::check()
{
  this->_value = 0;
  this->_button.update();
  if (this->_button.fell()) {
      elapsedMillis t = 0;
      while (t < 350) {
        this->_button.update();
        if (this->_button.read() != 0) {
          this->_value = 1;
          return this->_value;
        }
      }
      this->_value = 2;
      return this->_value;
  } else {
    if (this->_button.rose()) {
      this->_value = 3;
      return this->_value;
    }
  }
  return this->_value;
}

void ControlButton::setup()
{
  this->_value = 0;
}

void ControlButton::setup(byte buttonPin)
{
  this->setup();
  this->setPin(buttonPin);
}

void ControlButton::setPin(byte buttonPin)
{
  this->_pin = buttonPin;
  pinMode(this->_pin, INPUT_PULLUP);
  this->_button.attach(this->_pin);
  this->_button.interval(this->_interval);
}

// Most of these are just pass-thrus so that
// we are not accessing the button directly
boolean ControlButton::fell()
{
  return this->_button.fell();
}

boolean ControlButton::rose()
{
  return this->_button.rose();
}

void ControlButton::update() {
  this->_button.update();
}

void ControlButton::setPTT(boolean val) {
  this->_isPTT = val;
}

boolean ControlButton::isPTT() {
  return this->_isPTT;
}

void ControlButton::setInterval(byte intv) {
  this->_interval = intv;
  this->_button.interval(intv);
}

byte ControlButton::getInterval() {
  return this->_interval;
}

byte ControlButton::read() {
  return this->_button.read();
}
