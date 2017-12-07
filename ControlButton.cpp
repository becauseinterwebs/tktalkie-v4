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
  }
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


