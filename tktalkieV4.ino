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
 *  4.  Added APP_VER so sketch will know what version of the APP is accessing
 *      it.  This is for future use.
 *  
 * V3.1 (5/30/2017)
 *  1.  Added new setting to the SETTINGS.TXT control file to alert the 
 *      mobile app which input/volume controls should be presented.
 *      Valid values are:
 *        MIC  = Show the microphone jack controls only
 *        LINE = Show the line-in controls only 
 *        BOTH = Show mic/line-in controls and input switcher
 *      By default this is set to BOTH for backwards compatibility.  As of 
 *      version 3.1 of the TKTalkie hardware, inputs are wired to mic only
 *      as it will support both wired and wireless microphones.
 *      
 *  2.  Added new settings to the SETTINGS.TXT control file to alert the     
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
  Settings.glove.ControlButtons[App.ptt_button].update();
  if (Settings.glove.ControlButtons[App.ptt_button].fell()) {
    if (STATE == STATE_RUNNING) {
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
 * Read the SETTINGS.TXT file
 */
void startup() 
{

  Serial.println(F("\n----------------------------------------------"));
  Serial.print(F("TKTalkie v"));
  Serial.println(VERSION);
  Serial.println(F("(c) 2017 TK81113/Because...Interwebs!\nwww.TKTalkie.com"));
  Serial.print(F("Debugging is "));
  Serial.println(Config.debug == true ? "ON" : "OFF");
  if (Config.debug == false) {
    Serial.println(F("Type debug=1 [ENTER] to enable debug messages"));
  } else {
    Serial.println(F("Type debug=0 [ENTER] to disable debug messages"));
  }
  Serial.println(F("----------------------------------------------\n"));
  
  // make sure we have a profile to load
  //memset(Settings.file, 0, sizeof(Settings.file));

  File file = SD.open("SETTINGS.TXT");

  const size_t bufferSize = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(6) + 120;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  //const char* json = "{\"profile\":\"PYLEPRO4.TXT\",\"access_code\":\"0525\",\"debug\":1,\"input\":\"mic\",\"echo\":0,\"buttons\":[1,2,3,4,5,6]}";
  
  JsonObject& root = jsonBuffer.parseObject(file);

  if (!root.success()) {
    Serial.println(F("Failed to read file, using default configuration"));
    strlcpy(Config.profile, "DEFAULT.TXT", sizeof(Config.profile)); // "123456789012"
    strlcpy(Config.access_code, "1138", sizeof(Config.access_code)); // "1111111111111111111111111"
    Config.debug = 1;
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
  debug(F("Got startup value PROFILE: %s\n"), Config.profile);
  debug(F("Got startup  value ECHO: %d\n"), Config.echo);
  if (strcasecmp(Config.input, "") == 0) {
    strlcpy(Config.input, "BOTH", sizeof(Config.input));
  }
  debug(F("Got startup value INPUT TYPE: %s\n"), Config.input);

  //strlcpy(Settings.file, Config.profile, sizeof(Settings.file));
  
  if (strcasecmp(Config.profile, "") == 0) {
    // No profile specified, try to find one and load it
    char files[MAX_FILE_COUNT][14];
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

  debug(F("Read access code %s\n"), Config.access_code);
  
  // Load settings from specified file
  //loadSettingsFile(Config.profile);
  
  // Get flange processor ready but keep it off
  flange1.begin(Settings.effects.flanger.buffer,Settings.effects.flanger.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.flanger.offset, Settings.effects.flanger.depth, Settings.effects.flanger.freq);
  flange1.voices(FLANGE_DELAY_PASSTHRU,0,0);
  
  // Parse all of the settings
  //Serial.println("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!! STARTUP CALL PROCESS SETTINGS !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  //processSettings(profile_settings, total);
  //Serial.println("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AFTER STARTUP PROCESS SETTINGS !!!!!!!!!!!!!!!!!!!!!!!!!!");
  
  // apply the settings so we can do stuff
  /*
 /applySettings();

  // set the volume, either by config or volume pot
  
  readVolume();

  // turn on outputs
  Serial.println("--- UNMUTE");
  audioShield.unmuteLineout();
  audioShield.unmuteHeadphone();

  Serial.println("--- GET EFFECTS GAIN");
  float prevVol = Settings.effects.volume;

  // turn on volume for startup sound 
  // if effects volume is at 0
  Serial.println("--- SET EFFECTS GAIN");
  if (prevVol <= 0) {
    effectsMixer.gain(0, 1);
    effectsMixer.gain(1, 1);
  }

  // play startup sound
  Serial.println("--- PLAY STARTUP SOUND");
  long l = playSound(Settings.sounds.start);

  // reset mixer volume if set to 0
  Serial.println("--- RESET EFFECTS GAIN");
  if (prevVol <= 0) {
    effectsMixer.gain(0, Settings.effects.volume);
    effectsMixer.gain(1, Settings.effects.volume);
  }
  
  // add a smidge of delay ;)
  Serial.println("--- DELAY");
  delay(l+100); 

  // play background loop
  Serial.println("--- PLAY LOOP");
  playLoop();

  STATE = STATE_RUNNING;

  */
  Serial.println("----- END OF STARTUP");

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
  
  STATE = STATE_BOOTING;
  
  // You really only need the Serial connection 
  // for output while you are developing, so you 
  // can uncomment this and use Serial.println()
  // to write messages to the console.
  Serial.begin(57600);
  
  Serial1.begin(9600);
  
  delay(500);

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

  // load startup settings
  startup();
  
}

/***
 * Main program loop
 */

void loop() 
{

  switch (STATE) {
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
 
void run() {

  // check loop
  if (STATE == STATE_RUNNING) {

    if (loopLength > 0 && loopMillis > loopLength) {
        playLoop();
    }

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
      char *key, *val, *buf, *buf2, *uid, *app_ver;      
      Serial1.readBytesUntil('\n', received, MAX_DATA_SIZE);
      debug(F("RX: %s\n"), received);
      key = strtok_r(received, "=", &buf);
      val = strtok_r(NULL, "=", &buf);
      uid = strtok_r(val, "|", &buf2);
      val = strtok_r(NULL, "|", &buf2);
      app_ver = strtok_r(NULL, "|", &buf2);
      if (app_ver != NULL && strcasecmp(app_ver, "") != 0) {
        APP_VER = atof(app_ver);
      }
      strcpy(cmd_key, key);
      strcpy(cmd_val, val);
      autoSleepMillis = 0;
      debug(F("BLE Cmd: %s Value: %s Uid: %s App Ver: %s\n"), cmd_key, cmd_val, uid, APP_VER);
      // validate data received from mobile device!
      if (strcasecmp(cmd_key, "connect") == 0) {
          debug(F("Received access code %s\n"), cmd_val);
          if (strcmp(Config.access_code, cmd_val) == 0) {
            connectSound();
            App.ble_connected = true;
            memset(App.device_id, 0, sizeof(App.device_id));
            strcpy(App.device_id, uid);
            debug(F("DEVICE ID %s...Send Access OK\n"), App.device_id);
            sendToApp("access", "1");
          } else {
            App.ble_connected = true;
            sendToApp("access", "0");
            App.ble_connected = false;
          }
          strcpy(cmd_key, "");
      } else {
          if (strcmp(App.device_id, uid) == 0) {
            // Process remote commands
            if (strcasecmp(cmd_key, "disconnect") == 0) {
                App.ble_connected = false;
                disconnectSound();
                memset(App.device_id, 0, sizeof(App.device_id));
                memset(cmd_key, 0, sizeof(cmd_key));
                memset(cmd_val, 0, sizeof(cmd_val));
            } else if (strcasecmp(cmd_key, "config") == 0) {
                sendConfig();
                memset(cmd_key, 0, sizeof(cmd_key));
                memset(cmd_val, 0, sizeof(cmd_val));
            }
          } else {
            // The UUID does not match the one connected, 
            // so clear the command.            
            memset(cmd_key, 0, sizeof(cmd_key));
            memset(cmd_val, 0, sizeof(cmd_val));
          }
      }
    }
    
    if (strcasecmp(cmd_key, "") != 0) {
      Serial.print(">");
      Serial.print(cmd_key);
      if (strcasecmp(cmd_val, "") != 0) {
        Serial.print("=");
        Serial.print(cmd_val);
      }
      Serial.println("<");
      // Check if there is a parameter and process 
      // commands with values first
      if (strcasecmp(cmd_key, "config") == 0) {
        /*
        for (int i = 0; i < 6; i++) {
          Serial.println(STARTUP_SETTINGS[i]);
        }
        */
        Serial.println(Config.profile);
        Serial.println(Config.debug);
      } else if (strcasecmp(cmd_key, "save") == 0) {
        if (strcasecmp(cmd_val, "") != 0) {
            char *ptr, *pfile, *pname;
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
         if (saveSettingsFile(Settings.file) == true) {
          sendToApp("save", "1");
          connectSound();
         } else {
          sendToApp("save", "0");
         }
         if (wasPlaying == true) {
            playLoop();
         }
      } else if (strcasecmp(cmd_key, "access_code") == 0) {
           if (strcasecmp(cmd_val, "") != 0) {
              memset(Config.access_code, 0, sizeof(Config.access_code));
              strlcpy(Config.access_code, cmd_val, sizeof(Config.access_code));
              saveConfig();
          }
      } else if (strcasecmp(cmd_key, "debug") == 0) {
           if (strcasecmp(cmd_val, "") == 0) {
              Serial.print("Config.debug=");
              Serial.println(Config.debug == true ? "1" : "0");
           } else {
              int i = atoi(cmd_val);
              char val[2] = "0";
              if (i == 0) {
                Config.debug = false;
              } else {
                strcpy(val, "1");
                Config.debug = true;
              }
              saveConfig();
          }
      } else if (strcasecmp(cmd_key, "echo") == 0) {
           if (strcasecmp(cmd_val, "") == 0) {
              Serial.print("ECHO=");
              Serial.println(Config.echo == true ? "1" : "0");
           } else {
              int i = atoi(cmd_val);
              Config.echo = i == 1 ? true : false;
              saveConfig();
          }
      } else if (strcasecmp(cmd_key, "default") == 0) {
          if (strcasecmp(cmd_val, "") == 0) {
            strcpy(cmd_val, Settings.file);
          }
          char ret[SETTING_ENTRY_MAX];
          if (setDefaultProfile(cmd_val)) {
            strcpy(ret, "1;");
          } else {
            strcpy(ret, "0;");
          }
          strcat(ret, cmd_val);
          sendToApp("default", ret);
      } else if (strcasecmp(cmd_key, "delete") == 0) {
          if (strcasecmp(cmd_val, "") != 0) {
            char ret[SETTING_ENTRY_MAX];
            if (deleteProfile(cmd_val)) {
              strcpy(ret, "1;");
              strcat(ret, cmd_val);
            } else {
              strcpy(ret, "0;Could not remove profile");
            }
            sendToApp("delete", ret);
          }
      } else if (strcasecmp(cmd_key, "load") == 0) {
          loopPlayer.stop();
          if (strcasecmp(cmd_val, "") != 0) {
            memset(Settings.file, 0, sizeof(Settings.file));
            strcpy(Settings.file, cmd_val);
          } 
          addFileExt(Settings.file);
          char buf[100];
          strcpy(buf, PROFILES_DIR);
          strcat(buf, Settings.file);
          loadSettings(buf);
          //applySettings();
          long l = playSound(Settings.sounds.start);
          delay(l+100);
          playLoop();
          // send to remote if connected
          sendConfig();
      } else if (strcasecmp(cmd_key, "play") == 0) {
          //playSoundFile(EFFECTS_PLAYER, cmd_val);
          effectsPlayer.play(cmd_val);
      } else if (strcasecmp(cmd_key, "play_effect") == 0) {
          playEffect(cmd_val);
      } else if (strcasecmp(cmd_key, "play_sound") == 0) {
          playSound(cmd_val);
      } else if (strcasecmp(cmd_key, "play_loop") == 0) {
          if (strcasecmp(cmd_val, "") != 0) {
            memset(Settings.loop.file, 0, sizeof(Settings.loop.file));
            strcpy(Settings.loop.file, cmd_val);
          }
          playLoop();
      } else if (strcasecmp(cmd_key, "stop_loop") == 0) {
         loopPlayer.stop();
         loopLength = 0;
      } else if (strcasecmp(cmd_key, "beep") == 0) {
          int i = atoi(cmd_val);
          if (i < 1) {
            i = 1;
          }
          beep(i);
      } else if (strcasecmp(cmd_key, "mute") == 0) {
         audioShield.muteHeadphone();
         audioShield.muteLineout();
         App.muted = true;
      } else if (strcasecmp(cmd_key, "unmute") == 0) {
         audioShield.unmuteHeadphone();
         audioShield.unmuteLineout();
         App.muted = false;
      } else if (strcasecmp(cmd_key, "backup") == 0) {
         if (strcasecmp(cmd_val, "") == 0) {
           strcpy(cmd_val, Settings.file);
         }
         addBackupExt(cmd_val);
         saveSettingsFile(cmd_val, false); 
      } else if (strcasecmp(cmd_key, "restore") == 0) {
         loopPlayer.stop();
         if (strcasecmp(cmd_val, "") == 0) {
           strcpy(cmd_val, Settings.file);
         }
         addBackupExt(cmd_val);
         char *ret = strstr(cmd_val, PROFILES_DIR);
         if (ret == NULL) {
           char buf[SETTING_ENTRY_MAX];
           strcpy(buf, PROFILES_DIR);
           strcat(buf, cmd_val);
           memset(cmd_val, 0, sizeof(cmd_val));
           strcpy(cmd_val, buf);
           memset(buf, 0, sizeof(buf));
         }
         loadSettings(cmd_val);    
         //applySettings();
         long l = playSound(Settings.sounds.start);
         delay(l+100);
         playLoop();
      } else if (strcasecmp(cmd_key, "settings") == 0) {
          Serial.println(F(""));
          Serial.println(Settings.file);
          Serial.println(F("--------------------------------------------------------------------------------"));
          char buffer[1024];
          char *p = settingsToString(buffer);
          Serial.println(p);
          Serial.println(F("--------------------------------------------------------------------------------"));
          Serial.println(F(""));
      } else if (strcasecmp(cmd_key, "files") == 0) {
          char temp[MAX_FILE_COUNT][14];
          listFiles("/", temp, MAX_FILE_COUNT, "", true, true);
      } else if (strcasecmp(cmd_key, "show") == 0) {
          showFile(cmd_val);
      } else if (strcasecmp(cmd_key, "sounds") == 0) {
          char temp[MAX_FILE_COUNT][14];
          int count = listFiles(Settings.sounds.dir, temp, MAX_FILE_COUNT, SOUND_EXT, false, true);
          if (strcasecmp(cmd_val, "1") == 0) {
            char buffer[1024];
            char *files = arrayToStringJson(buffer, temp, count);
            sendToApp("sounds", files);
          }
      } else if (strcasecmp(cmd_key, "effects") == 0) {
          char temp[MAX_FILE_COUNT][14];
          int count = listFiles(Settings.effects.dir, temp, MAX_FILE_COUNT, SOUND_EXT, false, true);
          if (strcasecmp(cmd_val, "1") == 0) {
            char buffer[1024];
            char *files = arrayToStringJson(buffer, temp, count);
            sendToApp("effects", files);
          }
      } else if (strcasecmp(cmd_key, "loops") == 0) {
          char temp[MAX_FILE_COUNT][14];
          int count = listFiles(Settings.loop.dir, temp, MAX_FILE_COUNT, SOUND_EXT, false, true);
          if (strcasecmp(cmd_val, "1") == 0) {
            char buffer[1024];
            char *files = arrayToStringJson(buffer, temp, count);
            sendToApp("loops", files);
          }
      } else if (strcasecmp(cmd_key, "profiles") == 0) {
          char temp[MAX_FILE_COUNT][14];
          int count = listFiles(PROFILES_DIR, temp, MAX_FILE_COUNT, FILE_EXT, false, false);
          char buffer[1024];
          char *def = getSettingValue(buffer, "profile");
          for (int i = 0; i < count; i++) {
            Serial.print(temp[i]);
            if (strcasecmp(temp[i], Settings.file) == 0) {
              Serial.print(" (Loaded)");
            }
            if (strcasecmp(temp[i], def) == 0) {
              Serial.print(" (Default)");
            }
            Serial.println("");
          }
      } else if (strcasecmp(cmd_key, "ls") == 0) {
          char paths[MAX_FILE_COUNT][14];
          char buffer[1024];
          // return a list of directories on the card
          int count = listDirectories("/", paths);
          char *dirs = arrayToStringJson(buffer, paths, count);
          sendToApp(cmd_key, dirs);
      } else if (strcasecmp(cmd_key, "help") == 0) {
          showFile("HELP.TXT");
      } else if (strcasecmp(cmd_key, "calibrate") == 0) {
          #ifdef CALIBRATE
            calibrate();
          #endif
      } else if (strcasecmp(cmd_key, "reset") == 0) {
        softreset();
      } else if (strcasecmp(cmd_key, "sleep") == 0) {
        gotoSleep();  
      } else { 
        parseSetting(cmd_key, cmd_val);
        //applySettings();
        if (strcasecmp(cmd_key, "loop") == 0) {
          playLoop();
        }  
      }
      Serial.println("");
      memset(cmd_key, 0, sizeof(cmd_key));
      memset(cmd_val, 0, sizeof(cmd_val));
    }

    // Check sound glove buttons
    for (byte i = 0; i < 6; i++) {

      // The PTT button is a special case, so it is processed separately
      if (!Settings.glove.ControlButtons[i].isPTT()) {

        byte whichButton = Settings.glove.ControlButtons[i].check();
        
          byte btype = Settings.glove.ControlButtons[i].buttons[whichButton-1].getType();

          if (whichButton != 1 && whichButton != 2) {
            continue;
          }
            
          Serial.print("BUTTON PRESSED: ");
          Serial.print(whichButton);
          Serial.print(" TYPE: ");
          Serial.println(btype);
        
          switch(btype) {
            // Sound button
            case 2:
              {
                Serial.println(2);
                if (effectsPlayer.isPlaying()) {
                  effectsPlayer.stop();
                } else {
                  char buffer[14];
                  char *sound = Settings.glove.ControlButtons[i].buttons[whichButton-1].getSound(buffer);
                  Serial.println(sound);
                  playGloveSound(sound);
                }
              }  
              break;
            // Volume Up
            case 3:  
              {
                Settings.volume.master = Settings.volume.master + .01;
                if (Settings.volume.master > 10) {
                  Settings.volume.master = 10;
                }
                Serial.print("VOLUME UP: ");
                Serial.println(Settings.volume.master);
                audioShield.volume(Settings.volume.master);
                boop(440, 1);
              }  
              break;
            // Volume down  
            case 4:
              {
                Settings.volume.master = Settings.volume.master - .01;
                if (Settings.volume.master < 0) {
                  Settings.volume.master = 0;
                }
                Serial.print("VOLUME DOWN: ");
                Serial.println(Settings.volume.master);
                audioShield.volume(Settings.volume.master);
                boop(490, 0);
              }  
              break;
            // mute  
            case 5:
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
              break;
            // sleep/wake  
            case 6:
              {
                Serial.print("CALLING SLEEP WITH BUTTON: ");
                Serial.println(App.wake_button);
                gotoSleep();
              }  
              break;
            // lineout up  
            case 7:
              {
                --Settings.volume.lineout;
                if (Settings.volume.lineout < 13) {
                  Settings.volume.lineout = 13;
                }
                Serial.print("Settings.lineout UP: ");
                Serial.println(Settings.volume.lineout);
                audioShield.lineOutLevel(Settings.volume.lineout);   
                boop(440, 1);  
              }
              break;
            // lineout down  
            case 8:
              {
                ++Settings.volume.lineout;
                if (Settings.volume.lineout > 31) {
                  Settings.volume.lineout = 31;
                }
                Serial.print("Settings.lineout DOWN: ");
                Serial.println(Settings.volume.lineout);
                audioShield.lineOutLevel(Settings.volume.lineout); 
                boop(490, 0);
              }
              break;
            // mic gain up
            case 9:
              {
                ++Settings.volume.microphone;
                if (Settings.volume.microphone > 63) {
                  Settings.volume.microphone = 63;  
                }
                Serial.print("MIC GAIN UP: ");
                Serial.println(Settings.volume.microphone);
                audioShield.micGain(Settings.volume.microphone);  
                boop(540, 1);
              }
              break;
            // mic gain down  
            case 10:
              {
                --Settings.volume.microphone;
                if (Settings.volume.microphone < 0) {
                  Settings.volume.microphone = 0;  
                }
                Serial.print("MIC GAIN DOWN: ");
                Serial.println(Settings.volume.microphone);
                audioShield.micGain(Settings.volume.microphone);  
                boop(540, 0);
              }
              break;
            // Start/Stop Loop
            case 11:
              {
                if (loopPlayer.isPlaying()) {
                  loopPlayer.stop();
                } else {
                  playLoop();
                }
              }
              break; 
            // Loop Gain up
            case 12:
              {
                Settings.loop.volume = Settings.loop.volume + .05;
                if (Settings.loop.volume > 10) {
                  Settings.loop.volume = 10;
                }
                Serial.print("LOOP GAIN UP: ");
                Serial.println(Settings.loop.volume);
                effectsMixer.gain(1, Settings.loop.volume);
                boop(540, 1);
              }
              break;  
            // Loop gain down
            case 13:
              {
                Settings.loop.volume = Settings.loop.volume - .10;
                if (Settings.loop.volume < 0) {
                  Settings.loop.volume = 0;
                }
                Serial.print("LOOP GAIN DOWN: ");
                Serial.println(Settings.loop.volume);
                effectsMixer.gain(1, Settings.loop.volume);
                boop(540, 0);
              }
              break; 
            // Voice gain up
            case 14:
              {
                Settings.voice.volume = Settings.voice.volume + .05;
                if (Settings.voice.volume > 10) {
                  Settings.voice.volume = 10;
                }
                Settings.voice.dry = Settings.voice.dry + .05;
                if (Settings.voice.dry > 10) {
                  Settings.voice.dry = 10;
                }
                Serial.print("VOICE GAIN UP: ");
                Serial.println(Settings.voice.volume);
                Serial.print("DRY GAIN UP: ");
                Serial.println(Settings.voice.dry);
                voiceMixer.gain(0, Settings.voice.volume);
                voiceMixer.gain(1, Settings.voice.volume);
                voiceMixer.gain(2, Settings.voice.dry);  
                boop(540, 0);
              }
              break;
            // Voice gain down
            case 15:
              {
                Settings.voice.volume = Settings.voice.volume - .10;
                if (Settings.voice.volume < 0) {
                  Settings.voice.volume = 0;
                }
                Settings.voice.dry = Settings.voice.dry - .10;
                if (Settings.voice.dry < 0) {
                  Settings.voice.dry = 0;
                }
                Serial.print("VOICE GAIN DOWN: ");
                Serial.println(Settings.voice.volume);
                Serial.print("DRY GAIN DOWN: ");
                Serial.println(Settings.voice.dry);
                voiceMixer.gain(0, Settings.voice.volume);
                voiceMixer.gain(1, Settings.voice.volume);  
                voiceMixer.gain(2, Settings.voice.dry);  
                boop(540, 0);
              }
              break; 
            // Effects gain up
            case 16:
              {
                Settings.effects.volume = Settings.effects.volume + .05;
                if (Settings.effects.volume > 10) {
                  Settings.effects.volume = 10;
                }
                Serial.print("EFFECTS GAIN UP: ");
                Serial.println(Settings.effects.volume);
                effectsMixer.gain(0, Settings.effects.volume);
                effectsMixer.gain(1, Settings.effects.volume);  
                boop(540, 0);
              }
              break;
            // Effects gain down
            case 17:
              {
                Settings.effects.volume = Settings.effects.volume - .10;
                if (Settings.effects.volume < 0) {
                  Settings.effects.volume = 0;
                }
                Serial.print("EFFECTS GAIN DOWN: ");
                Serial.println(Settings.effects.volume);
                effectsMixer.gain(0, Settings.effects.volume);
                effectsMixer.gain(1, Settings.effects.volume);  
                boop(540, 0);
              }
              break;       
               
          }
        }  
    }  

    if (App.ptt_button >= 0 && App.button_initialized == false) {
      App.button_initialized = checkPTTButton();
      if (App.button_initialized) {
        // turn voice on with background noise
        voiceOn();
      }
    } else {
      Settings.glove.ControlButtons[App.ptt_button].update();
    }
    
    if (App.ptt_button >= 0 && App.button_initialized) {

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

              debug(F("Voice val: %4f\n"), val);
              
              if (val < Settings.voice.stop) {
    
                // If the user has stopped talking for at least the required "silence" time and 
                // the mic/line input has fallen below the minimum input threshold, play a random 
                // sound from the card.  NOTE:  You can adjust the delay time to your liking...
                // but it should probably be AT LEAST 1/4 second (250 milliseconds.)
    
                if (stopped >= Settings.voice.wait) {
                  debug(F("Voice stop: %4f\n"), val);
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
    while (duration < msecs) {
        Settings.glove.ControlButtons[App.wake_button].update();
        if (Settings.glove.ControlButtons[App.wake_button].read() != 0) {
            return false;
        }
    }
    return true;
}

void gotoSleep() {
  STATE = STATE_SLEEPING;
  if (loopPlayer.isPlaying()) {
    loopPlayer.stop();
  }
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

// END


