#include "Arduino.h"
#include "VirtualButton.h"

VirtualButton::VirtualButton()
{
  setup();
}

VirtualButton::VirtualButton(byte buttonType)
{
  this->setup();
  this->setType(buttonType);
}

void VirtualButton::setup()
{
   //this->_value = 0;
}

void VirtualButton::setup(byte buttonType)
{
  this->setup();
  this->setType(buttonType);  
}

void VirtualButton::setType(byte buttonType)
{
  this->_type = buttonType;
}

byte VirtualButton::getType() 
{
  return this->_type;
}

void VirtualButton::setSound(const char *sound)
{
  strcpy(this->_sound, sound);
}

char *VirtualButton::getSound(char sound[])
{
  strcpy(sound, this->_sound);
  return sound;
}

