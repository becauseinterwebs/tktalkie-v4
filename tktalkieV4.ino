/****
 * TK TALKIE by TK-81113 (Brent Williams) <becauseinterwebs@gmail.com>
 * www.tktalkie.com / www.tk81113.com 
 * 
 * Version 4.0 (December 3, 2017)
 *
 * WhiteArmor.net User ID: lerxstrulz
 * 
 * This sketch is meant to use a Teensy 3.2 with a Teensy Audio Shield and 
 * reads sounds from an SD card and plays them after the user stops talking 
 * to simulate comm noise such as clicks and static. This version adds a lot  
 * of new features, including the ability to be controlled via a mobile app 
 * and Bluetooth.  This release also introduces memory optimizations and other 
 * improvements.
 * 
 * You are free to use this code in your own projects, provided they are for 
 * personal, non-commercial use.
 * 
 * The audio components and connections were made using the GUI tool 
 * available at http://www.pjrc.com/teensy/gui.  If you want to modify 
 * the signal path, add more effects, etc., you can copy and paste the 
 * code marked by 'GUITool' into the online editor at the above URL.
 * 
 * WHAT'S NEW:
 * 
 * v4.0 (12/03/17)
 *  1.  Added Chorus and Flange processing for the voice channel.
 *  2.  Added a dry (unprocessed) voice channel.
 * 
 * V3.15 (10/20/17)
 *  1.  Add support for 3 sound-glove buttons.
 *  
 * V3.14 (10/10/17)
 *  1.  Added AUTO SLEEP function to put the TK-Talkie to sleep to conserve
 *      battery if no sound is heard from the microphone for a pre-configured 
 *      amount of time (configurable by the user.)
 * 
 * V3.13 (10/1/17)
 *  1.  Add mute_effects option to stop effects from playing after talkings
 *  
 *  2.  Startup sound, if specified, will always play even if effects level is 0
 *  
 *  3.  Allowed for empty effects.dir option
 *  
 *  4.  Added App.app_ver so sketch will know what version of the APP is accessing
 *      it.  This is for future use.
 *  
 * V3.1 (5/30/2017)
 *  1.  Added new setting to the CONFIG.TXT control file to alert the 
 *      mobile app which input/volume controls should be presented.
 *      Valid values are:
 *        MIC  = Show the microphone jack controls only
 *        LINE = Show the line-in controls only 
 *        BOTH = Show mic/line-in controls and input switcher
 *      By default this is set to BOTH for backwards compatibility.  As of 
 *      version 3.1 of the TKTalkie hardware, inputs are wired to mic only
 *      as it will support both wired and wireless microphones.
 *      
 *  2.  Added new settings to the CONFIG.TXT control file to alert the     
 *      mobile app which output/volume controls should be presented.
 *      Valid values are:
 *         SPKR = Show only headphones/speaker controls 
 *         LINE = Show only line-out controls
 *         BOTH = Show both
 *      By default this is set to BOTH for backwards compatibility
 *      
 *  3.  Add new option (all versions) to allow background loop to continue 
 *      playing while talking (loop is paused by default)
 *      
 *  4.  Fixed bug that prevented PTT button setting up properly.
 *  
 * V3.0 (4/10/2017)
 *  1.  Modified to be able to communicate via Bluetooth Low Energy (BLE) 
 *      serial adapter with mobile app for control of settings and profile 
 *      switching.
 *  2.  Began optimizations on usage of C-type strings to reduce memory usage.
 *  3.  Added new commands specific to BLE adapter usage.
 *  4.  Added new commands to manage multiple configuration profiles.
 *  5.  Reorganized SD card files and folders
 *  
 *  Please visit http://www.tktalkie.com/changelog for prior changes.
 */
 
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Snooze.h>
#include <Bounce2.h>
#include <ArduinoJson.h>
#include "ControlButton.h"
#include "globals.h"
#include "utils.h"
#include "files.h"
#include "sounds.h"
#include "calibrate.h"
#include "settings.h"
#include "ble.h"

/*** 
 * Check if the PTT button was pressed 
 */
boolean checkPTTButton() 
{
  if (!App.ptt_button) {
    return false;
  }
  Settings.glove.ControlButtons[App.ptt_button].update();
  if (Settings.glove.ControlButtons[App.ptt_button].fell()) {
    if (App.state == STATE_RUNNING) {
      if (strcasecmp(Settings.sounds.button, "*") == 0) {
        addSoundEffect();
      } else { 
        playEffect(Settings.sounds.button);
      }
    }
    return true;
  } else {
    return false;
  }
}

/**
 * Read the CONFIG.TXT file
 */
void startup() 
{

  // make sure we have a profile to load
  //memset(Settings.file, 0, sizeof(Settings.file));

  File file = SD.open("CONFIG.TXT");

  const size_t bufferSize = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(7) + 120;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  //const char* json = "{\"profile\":\"PYLEPRO4.TXT\",\"access_code\":\"0525\",\"debug\":1,\"input\":\"mic\",\"echo\":0,\"buttons\":[1,2,3,4,5,6]}";
  
  JsonObject& root = jsonBuffer.parseObject(file);

  if (!root.success()) {
    Serial.println(F("Failed to read file, using default configuration"));
    strlcpy(Config.profile, "DEFAULT.TXT", sizeof(Config.profile)); // "123456789012"
    strlcpy(Config.access_code, "1138", sizeof(Config.access_code)); // "1111111111111111111111111"
    Config.debug = 1;
    Config.baud  = 9600;
    strlcpy(Config.input,"BOTH", sizeof(Config.input)); // "both"
    Config.echo = 1;
    Config.buttons[0] = 2; // 1
    Config.buttons[1] = 3; // 2
    Config.buttons[2] = 4; // 3
    Config.buttons[3] = 0; // 4
    Config.buttons[4] = 0; // 5
    Config.buttons[5] = 0; // 6
  } else {
    strlcpy(Config.profile, (root["profile"] | ""), sizeof(Config.profile)); // "123456789012"
    strlcpy(Config.access_code, (root["access_code"] | "1138"), sizeof(Config.access_code)); // "1111111111111111111111111"
    //Config.debug = ((root["debug"] | 0) == 1) ? true : false; // 1
    Config.debug = root["debug"];
    strlcpy(Config.input, (root["input"] | "BOTH"), sizeof(Config.input)); // "both"
    //Config.echo = ((root["echo"] | 0) == 1) ? true : false; // 0
    Config.echo = root["echo"];
    Config.baud = root["baud"] | 9600;
    JsonArray& buttons = root["buttons"];
    Config.buttons[0] = buttons[0]; // 1
    Config.buttons[1] = buttons[1]; // 2
    Config.buttons[2] = buttons[2]; // 3
    Config.buttons[3] = buttons[3]; // 4
    Config.buttons[4] = buttons[4]; // 5
    Config.buttons[5] = buttons[5]; // 6
  }

  file.close();
    
  debug(F("Got startup value Config.debug: %d\n"), Config.debug);
  debug(F("Got startup value Config.profile: %s\n"), Config.profile);
  debug(F("Got startup  value Config.echo: %d\n"), Config.echo);
  if (strcasecmp(Config.input, "") == 0) {
    strlcpy(Config.input, "BOTH", sizeof(Config.input));
  }
  debug(F("Got startup value Config.input: %s\n"), Config.input);
  debug(F("Got startup value Config.access_code: %s\n"), Config.access_code);
  
  //strlcpy(Settings.file, Config.profile, sizeof(Settings.file));
  
  if (strcasecmp(Config.profile, "") == 0) {
    // No profile specified, try to find one and load it
    char files[MAX_FILE_COUNT][FILENAME_SIZE];
    byte total = listFiles(PROFILES_DIR, files, MAX_FILE_COUNT, FILE_EXT, false, false);
    if (total > 0) {
      memset(Config.profile, 0, sizeof(Config.profile));
      strcpy(Config.profile, files[0]);
    } else {
      debug(F("No profiles found\n"));
    }
  }

  if (strcasecmp(Config.profile, "") == 0) {
    debug(F("NO PROFILE FILE FOUND...USING DEFAULTS!\n"));
    strlcpy(Config.profile, "DEFAULT.TXT", sizeof(Config.profile));
  } else {
    debug(F("PROFILE: %s\n"), Config.profile);
  }

  if (strcasecmp(Config.input, "linein") == 0) {
    audioShield.inputSelect(AUDIO_INPUT_LINEIN);
  } else {
    audioShield.inputSelect(AUDIO_INPUT_MIC);
  }

  debug(F("Read access code %s\n"), Config.access_code);

  Serial.println(F("\n----------------------------------------------"));
  Serial.print(F("TKTalkie v"));
  Serial.println(VERSION);
  Serial.println(F("(c) 2017 TK81113/Because...Interwebs!\nwww.TKTalkie.com"));
  Serial.print(F("Debugging is "));
  Serial.println(Config.debug == 0 ? "OFF" : "ON");
  if (Config.debug == false) {
    Serial.println(F("Type debug=1 [ENTER] to enable debug messages"));
  } else {
    Serial.println(F("Type debug=0 [ENTER] to disable debug messages"));
  }
  Serial.println(F("----------------------------------------------\n"));
  
  // Load settings from specified file
  loadSettings(Config.profile, &Settings, false);
  
  // apply the settings so we can do stuff
  applySettings();

  // set the volume, either by config or volume pot
  
  readVolume();

  // turn on outputs
  audioShield.unmuteLineout();
  audioShield.unmuteHeadphone();

  float prevVol = Settings.effects.volume;

  // turn on volume for startup sound 
  // if effects volume is at 0
  if (prevVol <= 0) {
    effectsMixer.gain(0, 1);
    effectsMixer.gain(1, 1);
  }

  // play startup sound
  long l = playSound(Settings.sounds.start);

  // reset mixer volume if set to 0
  if (prevVol <= 0) {
    effectsMixer.gain(0, Settings.effects.volume);
    effectsMixer.gain(1, Settings.effects.volume);
  }
  
  // add a smidge of delay ;)
  delay(l+100); 

  // play background loop
  playLoop();

  App.state = STATE_RUNNING;

  Serial1.begin(Config.baud);
  delay(250);
  
}

/***
 * Initial setup...runs only once when the board is turned on.
 */
void setup() 
{

  // Not used yet
  //analogReference(DEFAULT);
  //analogReadResolution(12);
  //analogReadAveraging(32);
  
  App.state = STATE_BOOTING;
  
  // You really only need the Serial connection 
  // for output while you are developing, so you 
  // can uncomment this and use Serial.println()
  // to write messages to the console.
  Serial.begin(57600);
  
  delay(250);

  // Always allocate memory for the audio shield!
  AudioMemory(16);
  
  // turn on audio shield
  audioShield.enable();
  
  // disable volume and outputs during setup to avoid pops
  audioShield.muteLineout();
  audioShield.muteHeadphone();
  audioShield.volume(0);

  // turn on post processing
  audioShield.audioPostProcessorEnable();

  // Initialize sound generator for warning tones when we switch modes from PTT back to Voice Activated
  waveform1.begin(WAVEFORM_SINE);
  
  // Check SD card
  SPI.setMOSI(7);  // Set to 7 for Teensy
  SPI.setSCK(14);  // Set to 14 for Teensy
  // CS Pin is 10..do not change for Teensy!
  if (!(SD.begin(10))) {
     Serial.println("Unable to access the SD card");
     beep(2);
  }
  // this just makes sure we get truly random numbers each time
  // when choosing a file to play from the list later on...
  randomSeed(analogRead(0));

  // wait half a sec to give everything time to power up
  delay(500);

  // set volume of effects feed into master mixer
  voiceMixer.gain(3, 1);
  
  // load startup settings
  startup();
  
}

/***
 * Main program loop
 */

void loop() 
{

  switch (App.state) {
    case STATE_NONE:
    case STATE_BOOTING:
    case STATE_SLEEPING:
      // do nothing
      break;
    case STATE_RUNNING:
      // normal program operation
      run();
      break;
  }
  
}

/***
 * Called from main loop
 */
elapsedMillis checkMillis = 0;
byte lastButton = 0;
byte lastControlButton = 0;

void run() {

  // check loop
  if (App.state == STATE_RUNNING) {

    if (loopLength > 0 && loopMillis > loopLength) {
        playLoop();
    }

    // loop and serial command handlers
    char cmd_key[15] = "";
    char cmd_val[MAX_DATA_SIZE] = "";
    char received[SETTING_ENTRY_MAX]    = "";

    if (Serial.available() > 0) { 
      Serial.readBytesUntil('\n', received, MAX_DATA_SIZE);
      memset(cmd_key, 0, sizeof(cmd_key));
      memset(cmd_val, 0, sizeof(cmd_val));
      char *key, *val, *buf;
      key = strtok_r(received, "=", &buf);
      val = strtok_r(NULL, "=", &buf);
      strcpy(cmd_key, key);
      strcpy(cmd_val, val);
      autoSleepMillis = 0;
    } else if (Serial1.available() > 0) {
      char *key, *val, *buf, *buf2, *uid;      
      Serial1.readBytesUntil('\n', received, MAX_DATA_SIZE);
      debug(F("RX: %s\n"), received);
      // get command...will be int the format: cmd=UUID|value
      key = strtok_r(received, "=", &buf);
      // get other side of string after the '='
      val = strtok_r(NULL, "=", &buf);
      // extract device id
      uid = strtok_r(val, "|", &buf2);
      // extract rest of the command string (will be command value)
      val = strtok_r(NULL, "|", &buf2);
      memset(App.device_id, 0, sizeof(App.device_id));
      strcpy(App.device_id, uid);
      strcpy(cmd_key, key);
      strcpy(cmd_val, val);
      autoSleepMillis = 0;
      debug(F("BLE Cmd: %s Value: %s Uid: %s\n"), cmd_key, cmd_val, uid);
    }

    if (strcasecmp(cmd_key, "") != 0) {
        // translate character command into a number for 
        // faster processing
        byte cmdIdx = getCommand(cmd_key);
        switch (cmdIdx) {
          case CMD_CONNECT:
              {
                // validate data received from mobile device!
                char *val, *ver, *buf;
                val = strtok_r(cmd_val, "|", &buf);
                ver = strtok_r(NULL, "|", &buf);
                if (ver == NULL || strcasecmp(ver, "") == 0) {
                  strcpy(ver, "0");
                }
                debug(F("Received access code %s Ver: %s\n"), val, ver);
                if (strcmp(Config.access_code, val) == 0 && atof(ver) >= MIN_APP_VER) {
                  connectSound();
                  App.ble_connected = true;
                  debug(F("DEVICE ID %s...Send Access OK\n"), App.device_id);
                  sendToApp("access", "1");
                } else {
                  memset(App.device_id, 0, sizeof(App.device_id));
                  App.ble_connected = true;
                  if (atof(ver) < MIN_APP_VER) {
                    sendToApp("access", "0|Incorrect app version");
                  } else {
                    sendToApp("access", "0|Incorrect access code");
                  }
                  App.ble_connected = false;
                }
              }
              break;
           case CMD_DISCONNECT:
              App.ble_connected = false;
              disconnectSound();
              memset(App.device_id, 0, sizeof(App.device_id));
              break;
           case CMD_DOWNLOAD:
              sendConfig();
              break;
           case CMD_CONFIG:
              {
                showFile("CONFIG.TXT");
                /*
                Serial.print(F("Profile: "));
                Serial.println(F(Config.profile));
                Serial.print(F("Debug: "));
                Serial.println(Config.debug);
                Serial.print(F("Echo: "));
                Serial.println(Config.echo);
                Serial.print(F("Buttons: "));
                for (byte i = 0; i < 6; i++) {
                  Serial.print(Config.buttons[i]);
                  Serial.print(F(" "));
                }
                Serial.println(F(""));
                Serial.print(F("Input: "));
                Serial.println(F(Config.input));
                Serial.print(F("Access Code: "));
                Serial.println(F(Config.access_code));
                */
              }
              break; 
           case CMD_SAVE:
              {
                char *pfile;
                if (strcasecmp(cmd_val, "") != 0) {
                    char *ptr, *pname;
                    pfile = strtok_r(cmd_val, ";", &ptr);
                    if (strcasecmp(pfile, "") != 0) {
                      memset(Settings.file, 0, sizeof(Settings.file));
                      strcpy(Settings.file, pfile);
                    }
                    pname = strtok_r(NULL, ";", &ptr);
                    if (strcasecmp(pname, "") != 0) {
                      memset(Settings.name, 0, sizeof(Settings.name));
                      strcpy(Settings.name, pname);
                    }
                 }
                 addFileExt(Settings.file);
                 debug(F("Save settings file %s with description %s\n"), Settings.file, Settings.name);
                 boolean wasPlaying = false;
                 if (loopPlayer.isPlaying()) {
                    wasPlaying = true;
                    loopPlayer.stop();
                 }
                 if (saveSettings(Settings.file, true) == true) {
                  sendToApp("save", "1");
                  connectSound();
                 } else {
                  sendToApp("save", "0");
                 }
                 if (wasPlaying == true) {
                    playLoop();
                 } 
              }
              break;
           case CMD_ACCESS_CODE:
              if (strcasecmp(cmd_val, "") != 0) {
                memset(Config.access_code, 0, sizeof(Config.access_code));
                strlcpy(Config.access_code, cmd_val, sizeof(Config.access_code));
                saveConfig();
              }
              break;
           case CMD_DEBUG:
              Config.debug = (atoi(cmd_val) == 0) ? false : true;
              saveConfig();
              break;
           case CMD_ECHO:
              Config.echo = atoi(cmd_val) | 0;
              saveConfig();
              break;
           case CMD_BAUD:
              Config.baud = atol(cmd_val) | 9600;
              saveConfig();
              break;
           case CMD_DEFAULT:
              {
                if (strcasecmp(cmd_val, "") == 0) {
                  strcpy(cmd_val, Settings.file);
                }
                char ret[16];
                if (setDefaultProfile(cmd_val)) {
                  strcpy(ret, "1;");
                } else {
                  strcpy(ret, "0;");
                }
                strcat(ret, cmd_val);
                sendToApp("default", ret);
              }
              break;
           case CMD_DELETE:
              {
                if (strcasecmp(cmd_val, "") != 0) {
                  char ret[FILENAME_SIZE];
                  if (deleteProfile(cmd_val)) {
                    strcpy(ret, "1;");
                    strcat(ret, cmd_val);
                  } else {
                    strcpy(ret, "0;Could not remove profile");
                  }
                  sendToApp("delete", ret);
                }
              }
              break;
           case CMD_LOAD:
              {
                loopPlayer.stop();
                loadSettings(cmd_val, &Settings, false);
                applySettings();
                // send to remote if connected
                if (App.ble_connected) {
                  sendConfig();
                }
                long l = playSound(Settings.sounds.start);
                delay(l+100);
                playLoop();
              }
              break;
           case CMD_PLAY:
              effectsPlayer.play(cmd_val);
              break; 
           case CMD_PLAY_EFFECT:
              playEffect(cmd_val);
              break;
           case CMD_PLAY_SOUND:
              playSound(cmd_val);
              break;
           case CMD_PLAY_GLOVE:
              playGloveSound(cmd_val);
              break;
           case CMD_PLAY_LOOP:
              if (strcasecmp(cmd_val, "") != 0) {
                memset(Settings.loop.file, 0, sizeof(Settings.loop.file));
                strcpy(Settings.loop.file, cmd_val);
                playLoop();
              }
              break;
           case CMD_STOP_LOOP:
              loopPlayer.stop();
              loopLength = 0;
              break;
           case CMD_BEEP:
              {
                int i = atoi(cmd_val);
                if (i < 1) {
                  i = 1;
                }
                beep(i);
              }
              break;
           case CMD_BERP:
              berp();
              break;
           case CMD_MUTE:
              audioShield.muteHeadphone();
              audioShield.muteLineout();
              App.muted = true;
              break;
           case CMD_UNMUTE:
              audioShield.unmuteHeadphone();
              audioShield.unmuteLineout();
              App.muted = false;
              break;
           case CMD_BACKUP:
              if (strcasecmp(cmd_val, "") == 0) {
                strcpy(cmd_val, Settings.file);
              }
              addBackupExt(cmd_val);
              break;
           case CMD_RESTORE:
              {
                 loopPlayer.stop();
                 addBackupExt(cmd_val);
                 loadSettings(cmd_val, &Settings, false);    
                 applySettings();
                 long l = playSound(Settings.sounds.start);
                 delay(l+100);
                 playLoop();
              }
              break;
           case CMD_SETTINGS:
              {
                Serial.println(F(""));
                Serial.println(Settings.file);
                Serial.println(F("--------------------------------------------------------------------------------"));
                char buffer[JSON_BUFFER_SIZE];
                char *p = settingsToString(buffer, true);
                Serial.println(p);
                Serial.println(F("--------------------------------------------------------------------------------"));
                Serial.println(F(""));
              }
              break;
           case CMD_SHOW:
              showFile(cmd_val);
              break;
           case CMD_SOUNDS:   
           case CMD_EFFECTS:
           case CMD_LOOPS:
           case CMD_GLOVE:
           case CMD_PROFILES:
           case CMD_FILES:
              {
                char path[MAX_FILENAME];
                char ext[5];
                boolean recurse = false;
                boolean echo = false;
                strcpy(ext, SOUND_EXT);
                switch (cmdIdx) {
                  case CMD_SOUNDS:
                    strcpy(path, Settings.sounds.dir);
                    break;
                  case CMD_EFFECTS:
                    strcpy(path, Settings.effects.dir);
                    break;
                  case CMD_LOOPS:
                    strcpy(path, Settings.loop.dir);
                    break;
                  case CMD_GLOVE:
                    strcpy(path, Settings.glove.dir);
                    break;
                  case CMD_PROFILES:
                    strcpy(path, PROFILES_DIR);
                    strcpy(ext, FILE_EXT);
                    break;
                  case CMD_FILES:
                    strcpy(path, "/");
                    strcpy(ext, "");
                    recurse = true;
                    echo = true;
                    break;
                }
                char temp[MAX_FILE_COUNT][FILENAME_SIZE];
                int count = listFiles(path, temp, MAX_FILE_COUNT, ext, recurse, echo);
                if (strcasecmp(cmd_val, "1") == 0) {
                  char buffer[1024];
                  char *files = arrayToStringJson(buffer, temp, count);
                  sendToApp(cmd_key, files);
                }
                if (Config.debug && echo == false) {
                  for (int i = 0; i < count; i++) {
                    debug(F(temp[i]));
                  }
                }
              }
              break;
           case CMD_LS:
              {
                char paths[MAX_FILE_COUNT][FILENAME_SIZE];
                char buffer[1025];
                // return a list of directories on the card
                int count = listDirectories("/", paths);
                char *dirs = arrayToStringJson(buffer, paths, count);
                sendToApp(cmd_key, dirs);
              }
              break;
           case CMD_HELP:
              showFile("HELP.TXT");
              break;
           case CMD_CALIBRATE:
              calibrate();
              break;
           case CMD_RESET:
              softreset();
              break;
           case CMD_SLEEP:
              gotoSleep();
              break;
           case CMD_MEM:
              showMemory();
              break;
           default:
              debug(F("Default -> %s = %s\n"), cmd_key, cmd_val);
              parseSetting(cmd_key, cmd_val);
              if (strcasecmp(cmd_key, "loop") == 0) {
                playLoop();
              }
              break;    
        }

    }
    
    // clear command buffers
    memset(cmd_key, 0, sizeof(cmd_key));
    memset(cmd_val, 0, sizeof(cmd_val));         

    // Check sound glove buttons
    for (byte i = 0; i < 6; i++) {

      // The PTT button is a special case, so it is processed separately
      if (!Settings.glove.ControlButtons[i].isPTT()) {

        byte whichButton = Settings.glove.ControlButtons[i].check();
        
          byte btype = Settings.glove.ControlButtons[i].buttons[whichButton-1].getType();

          if (whichButton != 1 && whichButton != 2) {
            continue;
          }
            
          Serial.print(F("BUTTON PRESSED (Physical/Virtual): "));
          Serial.print(i);
          Serial.print(F("/"));
          Serial.print(whichButton);
          Serial.print(F(" TYPE: "));
          Serial.println(btype);
        
          switch(btype) {
            // Sound button
            case BUTTON_SOUND:
              {
                if (effectsPlayer.isPlaying() && lastButton == whichButton && lastControlButton == i) {
                  effectsPlayer.stop();
                } else {
                  char buffer[FILENAME_SIZE];
                  char *sound = Settings.glove.ControlButtons[i].buttons[whichButton-1].getSound(buffer);
                  debug(F("Play glove sound: %s"), sound);
                  if (Settings.loop.mute == true) {
                    loopOff();
                  }
                  long l = playGloveSound(sound);
                  lastButton = whichButton;
                  lastControlButton = i;
                  if (Settings.loop.mute == true) {
                    delay(l+100);
                    loopOn();
                  }
                }
              }  
              break;
            // Volume Up
            case BUTTON_VOL_UP:  
              {
                Settings.volume.master = Settings.volume.master + .01;
                if (Settings.volume.master > MAX_GAIN) {
                  Settings.volume.master = MAX_GAIN;
                  berp();
                } else {
                  debug(F("VOLUME UP: "));
                  Serial.println(Settings.volume.master);
                  audioShield.volume(Settings.volume.master);
                  sendToApp("volume", Settings.volume.master, 3);
                  boopUp();
                }
              }  
              break;
            // Volume down  
            case BUTTON_VOL_DOWN:
              {
                Settings.volume.master = Settings.volume.master - .01;
                if (Settings.volume.master < 0) {
                  Settings.volume.master = 0;
                  berp();
                } else {
                  Serial.print("VOLUME DOWN: ");
                  Serial.println(Settings.volume.master);
                  audioShield.volume(Settings.volume.master);
                  sendToApp("volume", Settings.volume.master, 3);
                  boopDown();
                }
              }  
              break;
            // mute  
            case BUTTON_MUTE:
              {
                if (App.muted) {
                  audioShield.unmuteHeadphone();
                  audioShield.unmuteLineout();
                  App.muted = false;
                } else {
                  audioShield.muteHeadphone();
                  audioShield.muteLineout();
                  App.muted = true;  
                }
              }
              sendToApp("mute", App.muted ? 1 : 0);
              break;
            // sleep/wake  
            case BUTTON_SLEEP:
              {
                Serial.print("CALLING SLEEP WITH BUTTON: ");
                Serial.println(App.wake_button);
                gotoSleep();
              }  
              break;
            // lineout up  
            case BUTTON_LINE_UP:
              {
                --Settings.volume.lineout;
                if (Settings.volume.lineout < 13) {
                  Settings.volume.lineout = 13;
                  berp();
                } else {
                  Serial.print("Settings.lineout UP: ");
                  Serial.println(Settings.volume.lineout);
                  audioShield.lineOutLevel(Settings.volume.lineout);   
                  sendToApp("lineout", Settings.volume.lineout);
                  boopUp();
                }
              }
              break;
            // lineout down  
            case BUTTON_LINE_DOWN:
              {
                ++Settings.volume.lineout;
                if (Settings.volume.lineout > 31) {
                  Settings.volume.lineout = 31;
                  berp();
                } else {
                  Serial.print("Settings.lineout DOWN: ");
                  Serial.println(Settings.volume.lineout);
                  audioShield.lineOutLevel(Settings.volume.lineout); 
                  sendToApp("lineout", Settings.volume.lineout);
                  boopDown();
                }
              }
              break;
            // mic gain up
            case BUTTON_MIC_UP:
              {
                ++Settings.volume.microphone;
                if (Settings.volume.microphone > 63) {
                  Settings.volume.microphone = 63;  
                  berp();
                } else {
                  Serial.print("MIC GAIN UP: ");
                  Serial.println(Settings.volume.microphone);
                  audioShield.micGain(Settings.volume.microphone);  
                  sendToApp("mic", Settings.volume.microphone);
                  boopUp();
                }
              }
              break;
            // mic gain down  
            case BUTTON_MIC_DOWN:
              {
                --Settings.volume.microphone;
                if (Settings.volume.microphone < 0) {
                  Settings.volume.microphone = 0;  
                  berp();
                } else {
                  Serial.print("MIC GAIN DOWN: ");
                  Serial.println(Settings.volume.microphone);
                  audioShield.micGain(Settings.volume.microphone);  
                  sendToApp("mic", Settings.volume.microphone);
                  boopDown();
                }
              }
              break;
            // Start/Stop Loop
            case BUTTON_LOOP:
              {
                if (loopPlayer.isPlaying()) {
                  loopPlayer.stop();
                } else {
                  playLoop();
                }
              }
              break; 
            // Loop Gain up
            case BUTTON_LOOP_UP:
              {
                Settings.loop.volume = Settings.loop.volume + .05;
                if (Settings.loop.volume > MAX_GAIN) {
                  Settings.loop.volume = MAX_GAIN;
                  berp();
                } else {
                  Serial.print("LOOP GAIN UP: ");
                  Serial.println(Settings.loop.volume);
                  effectsMixer.gain(1, Settings.loop.volume);
                  sendToApp("loop_gain", Settings.loop.volume, 3);
                  boopUp();
                }
              }
              break;  
            // Loop gain down
            case BUTTON_LOOP_DOWN:
              {
                Settings.loop.volume = Settings.loop.volume - .10;
                if (Settings.loop.volume < 0) {
                  Settings.loop.volume = 0;
                  berp();
                } else {
                    Serial.print("LOOP GAIN DOWN: ");
                  Serial.println(Settings.loop.volume);
                  effectsMixer.gain(1, Settings.loop.volume);
                  sendToApp("loop_gain", Settings.loop.volume, 3);
                  boopDown();
                }
              }
              break; 
            // Voice gain up
            case BUTTON_VOICE_UP:
              {
                Settings.voice.volume = Settings.voice.volume + .05;
                if (Settings.voice.volume > MAX_GAIN) {
                  Settings.voice.volume = MAX_GAIN;
                  berp();
                } else {
                  Serial.print("VOICE GAIN UP: ");
                  Serial.println(Settings.voice.volume);
                  voiceMixer.gain(0, Settings.voice.volume);
                  voiceMixer.gain(1, Settings.voice.volume);
                  sendToApp("voice_gain", Settings.voice.volume, 3);
                  boopDown();
                }
              }
              break;
            // Voice gain down
            case BUTTON_VOICE_DOWN:
              {
                Settings.voice.volume = Settings.voice.volume - .10;
                if (Settings.voice.volume < 0) {
                  Settings.voice.volume = 0;
                  berp();
                } else {
                  Serial.print("VOICE GAIN DOWN: ");
                  Serial.println(Settings.voice.volume);
                  voiceMixer.gain(0, Settings.voice.volume);
                  voiceMixer.gain(1, Settings.voice.volume);  
                  boopDown();
                  sendToApp("voice_gain", Settings.voice.volume, 3);
                }
              }
              break; 
            // Dry Voice gain up
            case BUTTON_DRY_UP:
              {
                Settings.voice.dry = Settings.voice.dry + .05;
                if (Settings.voice.dry > MAX_GAIN) {
                  Settings.voice.dry = MAX_GAIN;
                  berp();
                } else {
                  Serial.print("DRY GAIN UP: ");
                  Serial.println(Settings.voice.dry);
                  voiceMixer.gain(2, Settings.voice.dry);  
                  sendToApp("dry_gain", Settings.voice.dry, 3);
                  boopDown();
                }
              }
              break;
            // Dry Voice gain down
            case BUTTON_DRY_DOWN:
              {
                Settings.voice.dry = Settings.voice.dry - .10;
                if (Settings.voice.dry < 0) {
                  Settings.voice.dry = 0;
                  berp();
                } else {
                  Serial.print("DRY GAIN DOWN: ");
                  Serial.println(Settings.voice.dry);
                  voiceMixer.gain(2, Settings.voice.dry);  
                  boopDown();
                  sendToApp("dry_gain", Settings.voice.dry, 3);
                }
              }
              break;   
            // Effects gain up
            case BUTTON_EFFECTS_UP:
              {
                Settings.effects.volume = Settings.effects.volume + .05;
                if (Settings.effects.volume > MAX_GAIN) {
                  Settings.effects.volume = MAX_GAIN;
                  berp();
                } else {
                  Serial.print("EFFECTS GAIN UP: ");
                  Serial.println(Settings.effects.volume);
                  effectsMixer.gain(0, Settings.effects.volume);
                  effectsMixer.gain(1, Settings.effects.volume);  
                  boopDown();
                  sendToApp("effects_gain", Settings.effects.volume, 3);
                }
              }
              break;
            // Effects gain down
            case BUTTON_EFFECTS_DOWN:
              {
                Settings.effects.volume = Settings.effects.volume - .10;
                if (Settings.effects.volume < 0) {
                  Settings.effects.volume = 0;
                  berp();
                } else {
                  Serial.print("EFFECTS GAIN DOWN: ");
                  Serial.println(Settings.effects.volume);
                  effectsMixer.gain(0, Settings.effects.volume);
                  effectsMixer.gain(1, Settings.effects.volume);  
                  boopDown();
                  sendToApp("effects_gain", Settings.effects.volume, 3);  
                }  
              }
              break; 
            case BUTTON_SHIFT_RANGE_UP:
              {
                
              }
              break;
            case BUTTON_SHIFT_RANGE_DOWN:
              {
                  
              }
              break;
            case BUTTON_SHIFT_SPEED_UP:
              {
                
              }
              break;
            case BUTTON_SHIFT_SPEED_DOWN:
              {
                  
              }
              break;
          }
        }  
    }  

    if (App.ptt_button && App.button_initialized == false) {
      App.button_initialized = checkPTTButton();
      if (App.button_initialized) {
        // turn voice on with background noise
        voiceOn();
      }
    } else {
      Settings.glove.ControlButtons[App.ptt_button].update();
    }
    
    if (App.ptt_button && App.button_initialized) {

      float val = 0;
      
      // Check if there is silence.  If not, set a flag so that
      // we don't switch back to Voice Activated mode accidentally ;)
      if (App.speaking == true && App.silent == true) {
          if (rms1.available()) {
            val = rms1.read();
            // This check is here to make sure we don't get false readings
            // when the button click noise is played which would prevent 
            // the switch back to Voice Activated mode
            if ((val-Settings.voice.stop) >= .1) {
              App.silent = false;
            }
          }
      }
  
      // Play notification sound if there has been silence for 2 or 5 seconds:
      //    2 seconds: switching back to VA mode
      //    5 seconds: go into sleep mode
      if (App.speaking == true && App.silent == true && (stopped == 2000 || (stopped == 5000 && App.ptt_button == App.wake_button))) {
          connectSound();
      }
  
      // Button press

      if (Settings.glove.ControlButtons[App.ptt_button].fell()) {
        if (strcasecmp(Settings.sounds.button, "*") == 0) {
          addSoundEffect();
        } else {
          playEffect(Settings.sounds.button);
        }
        //ms = 0;
        voiceOn();
      }
      
      // BUTTON RELEASE
      // Switch back to Voice Activated mode if:
      //    1.  PTT button was pushed
      //    2.  There has been silence for at least 2 seconds
      // NOTE:  If you start talking before the 2 second time limit
      //        it will NOT switch back...or if you talk and pause for 
      //        2 seconds or more it will NOT switch back.
      if (Settings.glove.ControlButtons[App.ptt_button].rose()) {
        if (App.silent == true && stopped >= 5000 && App.ptt_button == App.wake_button) {
          gotoSleep();
          return;
        } else if (App.silent == true && stopped >= 2000) {
          voiceOff();
          App.button_initialized = false;
          return;
        } else {
          stopped = 0;
          //while (stopped < Settings.voice.wait) {}
          voiceOff();
          // Random comm sound
          addSoundEffect();
        }
      }
      
    } else {

        // Check if we have audio
        if (rms1.available()) {
          
          // get the input amplitude...will be between 0.0 (no sound) and 1.0
          float val = rms1.read();
    
          // Uncomment this to see what your constant signal is
          //Serial.println(val);
          
          // Set a minimum and maximum level to use or you will trigger it accidentally 
          // by breathing, etc.  Adjust this level as necessary!
          if (val >= Settings.voice.start) {

             debug(F("Voice start: %4f\n"), val);
             
            // If user is not currently App.speaking, then they just started talking :)
            if (App.speaking == false) {
  
              voiceOn();
          
            }
  
          } else if (App.speaking == true) {

              //debug(F("Voice val: %4f\n"), val);
              
              if (val < Settings.voice.stop) {
    
                // If the user has stopped talking for at least the required "silence" time and 
                // the mic/line input has fallen below the minimum input threshold, play a random 
                // sound from the card.  NOTE:  You can adjust the delay time to your liking...
                // but it should probably be AT LEAST 1/4 second (250 milliseconds.)
    
                if (stopped >= Settings.voice.wait) {
                  //debug(F("Voice stop: %4f\n"), val);
                  voiceOff();
                  // play random sound effect
                  addSoundEffect();
                }
    
              } else {
                  
                  // Reset the "silence" counter 
                  stopped = 0;
                  
              }
    
            }
          
         }
  
  
    }
  
    readVolume();

  }

  // Sleep mode check
  if (Settings.sleep.timer > 0 && (autoSleepMillis >= (Settings.sleep.timer * 60000))) {
      gotoSleep();
  }

}


bool buttonHeld(uint16_t msecs) {
    elapsedMillis duration = 0;
    if (!App.wake_button || App.wake_button == 255) {
      return false;
    }
    while (duration < msecs) {
        Settings.glove.ControlButtons[App.wake_button].update();
        if (Settings.glove.ControlButtons[App.wake_button].read() != 0) {
            return false;
        }
    }
    return true;
}

void gotoSleep() {
  App.state = STATE_SLEEPING;
  if (loopPlayer.isPlaying()) {
    loopPlayer.stop();
  }
  Serial.print("Playing Sleep Sound ");
  Serial.println(Settings.sleep.file);
  long l = playSound(Settings.sleep.file);
  delay(l+250);
  SLEEP:
    Serial.print("GOING TO SLEEP NOW...BUTTON IS ");
    Serial.println(App.wake_button);
    Settings.glove.ControlButtons[App.wake_button].update();
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    elapsedMillis timeout = 0;
    while (digitalRead(LED_BUILTIN) > 0 && timeout < 2000) {
      digitalWrite(LED_BUILTIN, LOW);
    }
    Snooze.hibernate( config_teensy3x );
    timeout = 0;
    // debouce set to 15ms, so have to wait and check button status
    while (timeout < 16) Settings.glove.ControlButtons[App.wake_button].update();
    bool awake = buttonHeld(100);
    if (!awake) goto SLEEP;
    softreset();
}
/**
 * Perform a restart without having to cycle power
 */
#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

void softreset() {
 WRITE_RESTART(0x5FA0004);
}

void showMemory() {
    Serial.print("Proc = ");
    Serial.print(AudioProcessorUsage());
    Serial.print(" (");    
    Serial.print(AudioProcessorUsageMax());
    Serial.print("),  Mem = ");
    Serial.print(AudioMemoryUsage());
    Serial.print(" (");    
    Serial.print(AudioMemoryUsageMax());
    Serial.println(")");
    Serial.print("Free Mem: ");
    Serial.println(freeMemory());
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
 
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
// END



