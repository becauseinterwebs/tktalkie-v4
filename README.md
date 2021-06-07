
# TK-Talkie v4.3

### PUBLIC RELEASE June 7, 2021

This is the latest version of the TK-Talkie firmware. Since I have had plans to vastly improve it for probably 2 years now but can just never find the time to actually do it due to what seems like an ever busier life and career, it's being released in the hopes that these improvements can be made by the community. This is long overdue, anyway. I was wanting to get this to a really good place before doing so, but like I said above, that has just not materialized. So here you go...warts and all!

## About

The TK-Talkie software is part of the TK-Talkie DIY project. This revision has some significant changes designed to making it even easier to configure and use the system through the use of a BLE (Bluetooth Low Energy) device and a mobile app (iOS/Android.) The format of configuration files has changed from a *key=value* text file to using JSON and contains the following improvements:

- Support for BLE (Bluetooth Low Energy) boards used to communicate with TK-Talkie App (iOS/Android) to control TK-Talkie devices allowing you to modify settings, save profiles, change profiles, program control buttons and more.

- Ability to have mulitple profiles (allows you to use TK-Talkie for different costume applications)

- Ability to organize sound files in different directories, allowing different profile to use different sound collections

- Default startup profile can be set (and changed via serial interface or mobile app.)

- Removed support for different equalizer types. Equalizer can be on/off and is a 5-band graphic.

- Warning beeps added for startup and SD card errors (see Warning Beep section below)

- Lots of code optimizations to better manage memory

### From Previous Versions:

- Text config file allows you to customize settings and options without having to recompile the code

- Calibration wizard added to help find optimum settings for your particular use

- Added support for an endless loop (background chatter) file

- Added the ability to use a serial connection (via Arduino IDE Serial Monitor or Terminal software) to change settings, save config files and load config files to enable easier testing and the use of different configs for different applications

- Cleaned up and optimized the code

## Warning Beeps

This version will sound warning beeps when errors are detected (to make is easier to be aware when the unit is in use.) The error is determined by the number of beeps played:

  

- 2 Beeps = Error reading SD card

- 3 Beeps = Error opening a settings (profile) file

- 4 Beeps = Error updating the SETTINGS.TXT (system settings) file

  

## Startup Settings File

  

The file **CONFIG.TXT** is stored at the root of the micro SD card and is used to control the startup functions of TK-Talkie. It has the following entries:
```
{
    "profile": "DEFAULT.TXT",
    "access_code": "xxxx",
    "debug": false,
    "echo": false,
    "input": "mic",
    "baud": 9600,
    "profiles" : "/profiles/",
    "buttons": [
        2,
        3,
        4,
        5,
        8,
        18
    ]
}
```
1.  *profile* - The default profile to load on startup

2.  *access_code* - Alphanumeric access code for the device that is needed to allow the mobile app to connect.

3.  *debug* - Turns debugging on (true) or off (false) for the unit. Previously this was stored in the profile file. Default is off. When on, messages during program operation will be displayed to the serial interface. This can be useful when testing configuration settings.

4.  *echo* - Same as debug, but for BLE operations. This will display the data being sent and received by the BLE.

5.  *input* - Valid options are "mic" or "linein".

6.  *baud* - The serial baud rate to communicate with the BLE. By default, most BLE's use 9600 baud rate. If you reprogram your BLE to use a different baud rate, don't forget to change it here as well. However, 9600 is more than sufficient for the amount of data being sent and received by the device.

7.  *profiles* - The directory on the SD card where the profile files are stored.

8.  *buttons* - TK-Talkie can have up to 6 buttons. By default, the pins to use are 2, 3, 4, 5, 8 and 18. You can change this to fit the number of buttons you wish to connect. For example, if you only have one button, change this to 2,0,0,0,0,0 and so on. *There must always be 6 entries!*
___
**PLEASE NOTE**: *All files must have a 8.3 format, meaning that the file name can have up to 8 characters and the file extension can have up to 3.*
___
## Configuration (Profile) Files

You can now have multiple profile configuration files.  A profile stores the setting to use for a character, such as voice modulation and effects, actions to be used by the control glove(s), etc.  These are stored in the **profiles** folder specified in the *config.txt* file on the SD card, which also holds the default profile to load at system startup. You can easily swap profiles via the serial interface or mobile app.

Profile files have a **.TXT** file extension and have the following format for configuration values (this example is for the default Stormtrooper profile):
```
{
  "name": "Sample TK Profile",
  "volume": {
    "master": 0.55,
    "microphone": 10,
    "linein": 10,
    "lineout": 30
  },
  "sounds": {
    "dir": "/sounds/",
    "start": "SHORT.WAV",
    "button": "CLICK3.WAV",
    "buttonOff": "*",
    "voiceOff": "*"
  },
  "loop": {
    "dir": "/loops/",
    "file": "TKLOOP.WAV",
    "volume": 0.36,
    "mute": true
  },
  "voice": {
    "volume": 3,
    "dry": 0,
    "start": 0.043,
    "stop": 0.02,
    "wait": 275
  },
  "effects": {
    "dir": "/effects/",
    "volume": 1,
    "highpass": 1,
    "bitcrusher": {
      "bits": 10,
      "rate": 10364
    },
    "chorus": {
      "voices": 1,
      "delay": 10,
      "enabled": 1
    },
    "flanger": {
      "delay": 32,
      "offset": 32,
      "depth": 10,
      "freq": 26,
      "enabled": 0
    },
    "shifter": {
      "length": 0,
      "speed": 0,
      "range": 0,
      "enabled": 0
    },
    "noise": 0.14,
    "mute": false
  },
  "eq": {
    "active": true,
    "bands": [
      -0.2,
      -0.14,
      0.3,
      0.07,
      -0.1
    ]
  },
  "sleep": {
    "timer": 0,
    "file": "FSLEEPUK.WAV"
  },
  "glove": {
    "dir": "/tk/",
    "buttons": [
      [
        "2,HOWLONG.WAV",
        "2,SEEID.WAV"
      ],
      [
        "2,NOSEEID.WAV",
        "2,NOTDROID.WAV"
      ],
      [
        "2,MOVEALNG.WAV",
        "2,LOOKDROI.WAV"
      ],
      [
        "2,BLASTER1.WAV",
        "2,BLASTER2.WAV"
      ],
      [
        "2,SETSTUN1.WAV",
        "2,STUN.WAV"
      ],
      [
        "2,ITSTHEM1.WAV",
        "2,LOADWEAP.WAV"
      ]
    ]
  }
}
```

## Sound Collections

Each profile specifies the directories where the sound files to be used for that profile are located. This allows you to either separate or share sounds between profiles. By default, the directories are:
  

#### Sounds Directory

```
"sounds": {
    "dir": "/sounds/",
    ...
```

This directory holds the startup sounds that can be played. It can be shared across profiles since only a single startup sound is played and you can choose which sound to play in your configuration profile.

  #### Loop Directory

```
"loop": {
    "dir": "/loops/",
```

This holds the background loops. Again, this can be shared since only one loop file is played and you can choose which file is used.

  

#### Effects Directory

```
"effects": {
    "dir": "/effects/",
```

This directory holds the sounds (mic pops and clicks, etc.) that are played whenever you stop talking. Although this directory can be shared, if you are using TK-Talkie for other costumes (such as a Clone Trooper, for example, which does not use the mic pops) you can specify a different directory, or leave it blank.

## Configuration Values

 This section under construction!


## Serial Interface

Starting with version 2,  you can use either the Serial Monitor in the Arduino IDE or a termal program to connect to the TK-Talkie and change configuration settings, save configuration files and load configuration files. The Arduino IDE has a built-in serial monitor that can be used for this, or you can connect via a terminal program at 57600 baud.

When connected, the following commands are available:

```
help - Show this help screen.

files - Show a list of files on SD card.

profiles - Show a list of available profiles on SD card.

effects - Show directory listing of effects folder.

sounds - Show directory listing of sounds folder.

loops - Show directory listing of sounds folder.

settings - Show current settings.

config - Displays the startup (system) settings

load - Load settings from default file

save - Save settings to currently loaded profile

access_code=value - Sets BLE access code for mobile app

debug=(1/0) - Turns debugging on/off

beep=n - Play warning beep sound n times

mute - Mute all sounds

unmute - Unmute all sounds

reset - Resets the TK-Talkie unit
```

___
For the following commands, a file extension of .TXT (for settings files) or

.WAV (for sound files) is assumed and will be added if not specified.
___
```
load=filename - Load settings from specified file

save=filename - Save current settings to specified file

default=filename - Set the default settings profile to be loaded
when TK-Talkie starts

delete=filename - Delete a settings file

play=filename - Plays the specified .WAV file (use full path/filename)

play_effect=filename - Play specified file from effects folder

play_sound=filename - Play specified file from sounds folder

play_loop=filename - Play specified file from loops folder

stop_loop - Stop currently playing loop

backup - Quick backup of settings file to SETTINGS.BAK

restore - Quick restore from SETTINGS.BAK

calibrate - Runs microphone calibration wizard
```

## Changing Configuration Settings

 There are now **FOUR** ways to change configuration settings:
  
- Use a BLE and the mobile app

- Change the default values in the source code and recompile and upload the code to your device

- Change configuration values in the *config.txt* and profile files and restart the device

- Make live changes via the serial interface

  

### Use the Mobile App

If you have attached the BLE device (see the V3 tutorial at [http://tktalkie.com/v3/tutorial](http://tktalkie.com/v3/tutorial)), you can use the mobile app to control your device. Please visit [http://tktalkie.com/app](http://tktalkie.com/app) for more information.
  

### Making Configuration Changes via Code
  

If you do not wish to use a config file, you can simply alter the default options located in the **globals.h** file and recompile, then upload, your changes to your device.
  

### Making Configuration Changes via Config Fil
  
Using this option, you can simply edit the config file, save your changes, then reinsert the card into the Teensy device and restart. Please see the **Configuration Values** section for a list of available options.
  

### Making Configuration Changes via Serial Interface


You can make live, real-time changes via the serial interface. Changes take effect immediately. They will be LOST when the device restarts unless you save them using one of the '*save*' commands. When making live changes, use the following format:

setting=value [ENTER]

The new setting will take effect immediately. This is a great way to allow you to tweak and test settings for your particular application of the TK-Talkie system. Please see the **Configuration Values** section for a list of available options.
 

For more information regarding TK-Talkie, please visit [tktalkie.com](http://www.tktalkie.com), or join the support group on [Facebook](https://www.facebook.com/groups/tktalkie).