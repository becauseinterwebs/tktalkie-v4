/****
 * Routines to handle reading/handling settings
 */

void ConfigureButton(byte a) {
  
    Settings.glove.ControlButtons[a].setPTT(false);
    Settings.glove.ControlButtons[a].setPin(0);
    byte buttonNum = 0;
    char buf[25];
    byte pin = Config.buttons[a];
    
    if (pin > 0) {

      // setup physical button
      Settings.glove.ControlButtons[a].setup(pin);
      
      strcpy(buf, Settings.glove.settings[a]);
      char *part_token, *part_ptr;
      part_token = strtok_r(buf, ";", &part_ptr);
      // button_type,data(sound)
      byte b = 0;

      while (part_token && b < 2) {
        char *button_token, *button_ptr;
        button_token = strtok_r(part_token, ",", &button_ptr);
        byte button_type = (byte)atoi(button_token);
        // by default, do not continue processing
        byte max = 0;
        // Determine how many options we need to read 
        // based on the type of button
        switch (button_type) {
          // PTT/Sleep/Wake Button
          case 1:
            {
              debug(F("PTT Button on pin: %d\n"), pin);
              App.ptt_button = a;
              Settings.glove.ControlButtons[a].setPTT(true);
              if (!App.wake_button || App.wake_button == 255) {
                App.wake_button = a;
                snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
              }
            }  
            break;
          // Sound button    
          case 2:
            {
              max = 2;
            }  
            break;
          // sleep/wake (overrides PTT)
          case 6:
            {
              debug(F("Sleep Button on pin: %d\n"), pin);
              App.wake_button = a;
              snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
            }  
            break;   
        }

        // setup virtual button type
        Settings.glove.ControlButtons[a].buttons[buttonNum].setup(button_type);
        
        // start off with one since we have the first part 
        // and just need to get the second part before 
        // we keep processing the settings
        byte c = 1;
        button_token = strtok_r(NULL, ",", &button_ptr);
        
        while (button_token && c < max) {
          switch (button_type) {
            case 2:
              {
                debug(F("Sound Button on pin %d with sound %s\n"), pin, button_token);
                Settings.glove.ControlButtons[a].buttons[buttonNum].setSound(button_token);
              }  
              break;
          }

          // NOTE: Each physical button needs to have 
          // to virtual button properties that hold 
          // what to do
          c++;
          button_token = strtok_r(NULL, ",", &button_ptr);
        }  
        buttonNum++;
        b++;
        part_token = strtok_r(NULL, ";", &part_ptr);
      }
    }
}

void setVolume() {
  if (Settings.volume.master > 1) { 
    Settings.volume.master = 1;
  } else if (Settings.volume.master < 0) {
    Settings.volume.master = 0;
  }
  //audioShield.volume(readVolume());
  char buf[10];
  dtostrf(Settings.volume.master, 0, 3, buf);
  debug(F("VOLUME: %s\n"), buf);
  audioShield.volume(Settings.volume.master);
}

void setMicGain() {
  audioShield.micGain(Settings.volume.microphone); 
}

void setLineout() {
  if (Settings.volume.lineout < 13) {
    Settings.volume.lineout = 13;  
  } else if (Settings.volume.lineout > 31) {
    Settings.volume.lineout = 31;
  }
  debug(F("LINEOUT: %d\n"), Settings.volume.lineout);
  audioShield.lineOutLevel(Settings.volume.lineout);
}

void setLinein() {
  if (Settings.volume.linein < 0) {
    Settings.volume.linein = 0;  
  } else if (Settings.volume.linein > 15) {
    Settings.volume.linein = 15;
  }  
  debug(F("LINEIN: %d\n"), Settings.volume.linein);
  audioShield.lineInLevel(Settings.volume.linein);
}

void setHipass() {
  if (Settings.effects.highpass < 0) { 
    Settings.effects.highpass = 0;
  } else if (Settings.effects.highpass > 1) {
    Settings.effects.highpass = 1;
  }
  debug(F("HIGHPASS: %d\n"), Settings.effects.highpass);
  if (Settings.effects.highpass == 0) {
    audioShield.adcHighPassFilterDisable();
  } else {
    audioShield.adcHighPassFilterEnable();
  }
}

void setNoiseVolume() {
  char buf[30];
  dtostrf(Settings.effects.noise, 0, 3, buf);
  debug(F("NOISE: %s\n"),  buf);
  effectsMixer.gain(3, Settings.effects.noise);
}

void setVoiceVolume() {
  char buf[30];
  dtostrf(Settings.voice.volume, 0, 3, buf);
  debug(F("VOICE: %s\n"),  buf);
  voiceMixer.gain(0, Settings.voice.volume);
  //voiceMixer.gain(1, Settings.voice.volume);
}

void setDryVolume() {
  char buf[30];
  dtostrf(Settings.voice.dry, 0, 3, buf);
  debug(F("DRY: %s\n"),  buf);
  voiceMixer.gain(2, Settings.voice.dry);
}

void setEffectsVolume() {
  char buf[30];
  dtostrf(Settings.effects.volume, 0, 3, buf);
  debug(F("EFFECTS VOLUME: %s\n"),  buf);
  effectsMixer.gain(0, Settings.effects.volume);
  //effectsMixer.gain(1, Settings.effects.volume);
  // Waveform (BLE) connect sound
  effectsMixer.gain(2, Settings.effects.volume);
}

void setLoopVolume() {
  if (Settings.loop.volume < 0 or Settings.loop.volume > 32767) {
    Settings.loop.volume = 4;
  }
  // chatter loop from SD card
  effectsMixer.gain(1, Settings.loop.volume);
  //loopMixer.gain(0, Settings.loop.volume);
  //loopMixer.gain(1, Settings.loop.volume);
}

void setEq() {
  if (Settings.eq.active < 0) {
    Settings.eq.active = 0;
  } else if (Settings.eq.active > 1) {
    Settings.eq.active = 1;
  }
  // Turn on the 5-band graphic equalizer (there is also a 7-band parametric...see the Teensy docs)
  if (Settings.eq.active == 0) {
    audioShield.eqSelect(FLAT_FREQUENCY);
  } else {
    audioShield.eqSelect(GRAPHIC_EQUALIZER);
  }  
}

void setEqBands() {
  // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
  // Valid values are -1 (-11.75dB) to 1 (+12dB)
  // The settings below pull down the lows and highs and push up the mids for 
  // more of a "tin-can" sound.
  audioShield.eqBands(Settings.eq.bands[0], Settings.eq.bands[1], Settings.eq.bands[2], Settings.eq.bands[3], Settings.eq.bands[4]);
}

void setBitcrusher() {
  // You can modify these values to process the voice 
  // input.  See the Teensy bitcrusher demo for details.
  bitcrusher1.bits(Settings.effects.bitcrusher.bits);
  bitcrusher1.sampleRate(Settings.effects.bitcrusher.rate);
}

void setEffectsDir() {
  fixPath(Settings.effects.dir);
  loadSoundEffects();
}

void setSoundsDir() {
  fixPath(Settings.sounds.dir);
}

void setLoopDir() {
  fixPath(Settings.loop.dir);
}

void setGloveDir() {
  fixPath(Settings.glove.dir);
}

void setLoopMute() {
  if (Settings.loop.mute > 1) {
    Settings.loop.mute = 1;
  } else if (Settings.loop.mute < 0) {
    Settings.loop.mute = 0;
  }
}

void setEffectsMute() {
  if (Settings.effects.mute > 1) {
    Settings.effects.mute = 1;
  } else if (Settings.effects.mute < 0) {
    Settings.effects.mute = 0;
  }
}

void setSleepTimer() {
  if (Settings.sleep.timer < 0) {
    Settings.sleep.timer = 0;
  }
}

void setChorus() {
  if (Settings.effects.chorus.delay > 32) {
      Settings.effects.chorus.delay = 32;
    }
    if (Settings.effects.chorus.delay < 1) {
      Settings.effects.chorus.delay = 1;
    }
    if (Settings.effects.chorus.voices < 0) {
      Settings.effects.chorus.voices = 0;
    }
    debug(F("CHORUS: %d - %d"), Settings.effects.chorus.voices, Settings.effects.chorus.delay);
    if (Settings.effects.chorus.voices < 1 || Settings.effects.chorus.enabled != 1) {
      chorus1.voices(0);
    } else if(!chorus1.begin(Settings.effects.chorus.buffer,Settings.effects.chorus.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.chorus.voices)) {
       Serial.println(F("Chorus: startup failed!"));
    }
}

void setFlanger() {
  if (Settings.effects.flanger.delay > 32) {
    Settings.effects.flanger.delay = 32;
  }
  if (Settings.effects.flanger.delay < 0) {
    Settings.effects.flanger.delay = 0;
  }
  if (Settings.effects.flanger.enabled != 1) {
    flange1.voices(FLANGE_DELAY_PASSTHRU,0,0);
  } else {
    flange1.begin(Settings.effects.flanger.buffer,Settings.effects.flanger.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq); 
  }
}

void setShifter() {
  if (Settings.effects.shifter.enabled == 1) {
    float msec = 25.0 + (((float)Settings.effects.shifter.length/1023.0) * 75.0);
    Serial.print("SPEED BEFORE CALC: ");
    Serial.println(Settings.effects.shifter.speed);
    float speed = ((float)Settings.effects.shifter.speed/1023.0) * 8.0;//powf(2.0, (Settings.effects.shifter.speed/1023) * 2.0 - 1.0); // 0.5 to 2.0
    Serial.print("SPEED AFTER CALC: ");
    Serial.println(speed);
    if (speed < .125) { 
      speed = .125;
    } else if (speed > 8.0) {
      speed = 8.0;
    }
    //ratio = powf(2.0, knobA2 * 6.0 - 3.0); // 0.125 to 8.0 -- uncomment for far too much range!
    debug(F("Shifter set length %d speed %d\n"), Settings.effects.shifter.length, Settings.effects.shifter.speed);
    char buf[10];
    dtostrf(speed, 1, 7, buf);
    debug(F("Shifter speed: %s\n"), buf);
    granular1.beginPitchShift(msec);
    granular1.setSpeed(speed);
  } else {
    debug(F("Shifter STOP"));
    granular1.stop();
  }
}
/***
 * Parse and set a Configuration setting
 */
void parseSetting(const char *settingName, char *settingValue) 
{

  debug(F("Parse Setting: %s = %s\n"), settingName, settingValue);

  if (strcasecmp(settingName, "name") == 0) {
    memset(Settings.name, 0, sizeof(Settings.name));
    strcpy(Settings.name, settingValue);
  } else if (strcasecmp(settingName, "volume") == 0) {
    Settings.volume.master = atof(settingValue);
    setVolume();  
  } else if (strcasecmp(settingName, "lineout") == 0) {
    Settings.volume.lineout = (byte)atoi(settingValue);
    setLineout();
  } else if (strcasecmp(settingName, "linein") == 0) {
    Settings.volume.linein = (byte)atoi(settingValue);
    setLinein();
  } else if ((strcasecmp(settingName, "high_pass") == 0) || (strcasecmp(settingName, "highpass") == 0)) {
    Settings.effects.highpass = (byte)atoi(settingValue);
    setHipass();
  } else if (strcasecmp(settingName, "microphone") == 0 || strcasecmp(settingName, "mic_gain") == 0) {
    Settings.volume.microphone = atoi(settingValue);
    setMicGain();
  } else if (strcasecmp(settingName, "button_click") == 0) {
    memset(Settings.sounds.button, 0, sizeof(Settings.sounds.button));
    strcpy(Settings.sounds.button, settingValue);
  } else if ( (strcasecmp(settingName, "startup") == 0) || (strcasecmp(settingName, "startup_sound") == 0) ) {
    memset(Settings.sounds.start, 0, sizeof(Settings.sounds.start));
    strcpy(Settings.sounds.start, settingValue);
  } else if ( (strcasecmp(settingName, "loop") == 0) || (strcasecmp(settingName, "loop.file") == 0) ) {
    memset(Settings.loop.file, 0, sizeof(Settings.loop.file));
    strcpy(Settings.loop.file, settingValue);
  } else if ( (strcasecmp(settingName, "noise_gain") == 0) || (strcasecmp(settingName, "noise") == 0) ) {
    Settings.effects.noise = atof(settingValue);
    setNoiseVolume();
  } else if ( (strcasecmp(settingName, "voice_gain") == 0) || (strcasecmp(settingName, "voice.volume") == 0) ) {
    Settings.voice.volume = atof(settingValue);
    setVoiceVolume();
  } else if (strcasecmp(settingName, "dry_gain") == 0) {
    Settings.voice.dry = atof(settingValue);  
    setDryVolume();
  } else if (strcasecmp(settingName, "effects_gain") == 0) {
    Settings.effects.volume = atof(settingValue);
    setEffectsVolume();
  } else if (strcasecmp(settingName, "loop_gain") == 0) {
    Settings.loop.volume = atof(settingValue);
    setLoopVolume();
  } else if (strcasecmp(settingName, "silence_time") == 0) {
    Settings.voice.wait = atoi(settingValue);
  } else if (strcasecmp(settingName, "voice_start") == 0) {
    Settings.voice.start = atof(settingValue);
  } else if (strcasecmp(settingName, "voice_stop") == 0) {  
    Settings.voice.stop = atof(settingValue);
  } else if (strcasecmp(settingName, "eq") == 0) {
    Settings.eq.active = (byte)atoi(settingValue);
    setEq();
  } else if (strcasecmp(settingName, "eq_bands") == 0) {
    // clear bands and prep for setting
    for (byte i = 0; i < 5; i++) {
      Settings.eq.bands[i] = 0;
    }
    char *band, *ptr;
    band = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (band && i < 6) {
      Settings.eq.bands[i] = atof(band);
      i++;
      band = strtok_r(NULL, ",", &ptr);
    }
    setEqBands();
  } else if (strcasecmp(settingName, "bitcrushers") == 0 || strcasecmp(settingName, "bitcrusher") == 0) {
    char *token, *ptr;
    token = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (token && i < (3)) {
      switch (i) {
        case 0:
           Settings.effects.bitcrusher.bits = atoi(token);
           break;
        case 1:
           Settings.effects.bitcrusher.rate = atoi(token);
           break;
      }
      i++;
      token = strtok_r(NULL, ",", &ptr);
    }
    setBitcrusher();
  } else if (strcasecmp(settingName, "effects_dir") == 0) {
    memset(Settings.effects.dir, 0, sizeof(Settings.effects.dir));
    strcpy(Settings.effects.dir, settingValue);
    setEffectsDir();
  } else if (strcasecmp(settingName, "sounds_dir") == 0) {
    memset(Settings.sounds.dir, 0, sizeof(Settings.sounds.dir));
    strcpy(Settings.sounds.dir, settingValue);
    setSoundsDir();
  } else if (strcasecmp(settingName, "loop_dir") == 0) {
    memset(Settings.loop.dir, 0, sizeof(Settings.loop.dir));
    strcpy(Settings.loop.dir, settingValue);
    setLoopDir();
  } else if (strcasecmp(settingName, "glove_dir") == 0) {
    memset(Settings.glove.dir, 0, sizeof(Settings.glove.dir));
    strcpy(Settings.glove.dir, settingValue);
    setGloveDir();  
  } else if (strcasecmp(settingName, "mute_loop") == 0) {
    Settings.loop.mute = (byte)atoi(settingValue);
    setLoopMute();
  } else if (strcasecmp(settingName, "mute_effects") == 0) {
    Settings.effects.mute = (byte)atoi(settingValue);
    setEffectsMute();
  } else if (strcasecmp(settingName, "sleep_time") == 0) {
    Settings.sleep.timer = (byte)atoi(settingValue);
    setSleepTimer();
  } else if (strcasecmp(settingName, "sleep_sound") == 0) {
    memset(Settings.sleep.file, 0, sizeof(Settings.sleep.file));
    strcpy(Settings.sleep.file, settingValue);
  } else if (strcasecmp(settingName, "chorus") == 0) {
    if (strcasecmp(settingValue, "0") == 0) {
      //chorus1.voices(0);
      Settings.effects.chorus.enabled = 0;
    } else if (strcasecmp(settingValue, "1") == 0) {
      //chorus1.voices(Settings.effects.chorus.voices);
      Settings.effects.chorus.enabled = 1;
    } else {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte i = 0;
      while (token && i < 3) {
        switch (i) {
          case 0:
            Settings.effects.chorus.voices = (byte)atoi(token);
            break;
          case 1:
            Settings.effects.chorus.delay = (byte)atoi(token);
            break;
        }
        i++;
        token = strtok_r(NULL, ",", &ptr);
      }  
    }
    setChorus();
  } else if (strcasecmp(settingName, "shifter") == 0) {
    if (strcasecmp(settingValue, "0") == 0) {
      //chorus1.voices(0);
      Settings.effects.shifter.enabled = 0;
    } else if (strcasecmp(settingValue, "1") == 0) {
      //chorus1.voices(Settings.effects.chorus.voices);
      Settings.effects.shifter.enabled = 1;
    } else {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte i = 0;
      while (token && i < 3) {
        switch (i) {
          case 0:
            Settings.effects.shifter.length = atoi(token);
            break;
          case 1:
            Settings.effects.shifter.speed = atoi(token);
            break;
        }
        i++;
        token = strtok_r(NULL, ",", &ptr);
      }  
    }
    setShifter();  
  } else if (strcasecmp(settingName, "chorus_delay") == 0) {
      Settings.effects.chorus.delay = (byte)atoi(settingValue);
      setChorus();
  } else if (strcasecmp(settingName, "chorus_voices") == 0) {
      Settings.effects.chorus.voices = (byte)atoi(settingValue);
      if (Settings.effects.chorus.voices < 0) {
        Settings.effects.chorus.voices = 0;
      }
      chorus1.voices(Settings.effects.chorus.voices);
  } else if (strcasecmp(settingName, "flanger_delay") == 0) {
      Settings.effects.flanger.delay = (byte)atoi(settingValue);
      setFlanger();   
  } else if (strcasecmp(settingName, "flanger_freq") == 0) {
      Settings.effects.flanger.freq = atof(settingValue);
      if (Settings.effects.flanger.freq < 0) {
        Settings.effects.flanger.freq = 0;
      }
      if (Settings.effects.flanger.freq > 10) {
        Settings.effects.flanger.freq = 10;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flanger_depth") == 0) {
      Settings.effects.flanger.depth = (byte)atoi(settingValue);
      if (Settings.effects.flanger.depth < 0) {
        Settings.effects.flanger.depth = 0;
      }
      if (Settings.effects.flanger.depth > 255) {
        Settings.effects.flanger.depth = 255;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flanger_offset") == 0) {
      Settings.effects.flanger.offset = (byte)atoi(settingValue);// * Settings.effects.flanger.delay_LENGTH;
      if (Settings.effects.flanger.offset < 1) {
        Settings.effects.flanger.offset = 1;
      }
      if (Settings.effects.flanger.offset > 128) {
        Settings.effects.flanger.offset = 128;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flanger") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        //flange1.voices(FLANGE_DELAY_PASSTHRU,0,0);
        Settings.effects.flanger.enabled = 0;
      } else if (strcasecmp(settingValue, "1") == 0) {
        //flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
        Settings.effects.flanger.enabled = 1;
      } else {
        char *token, *ptr;
        token = strtok_r(settingValue, ",", &ptr);
        byte i = 0;
        while (token && i < 3) {
          switch (i) {
            case 0:
              Settings.effects.flanger.offset = (byte)atoi(token);
              break;
            case 1:
              Settings.effects.flanger.depth = (byte)atoi(token);
              break;
            case 2:
              Settings.effects.flanger.freq = atof(token);
              break;
          }
          i++;
          token = strtok_r(NULL, ",", &ptr);
        } 
        //flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq); 
      }
      setFlanger();
  } else if (strcasecmp(settingName, "button") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte b = (byte)atoi(token);
      if (b >= 0 && b <= 5) {
        strcpy(Settings.glove.settings[b], ptr);
        ConfigureButton(b);
      }
  } else if (strcasecmp(settingName, "buttons") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, "|", &ptr);
      byte a = 0;
      while (token && a < 6) {
        strcpy(Settings.glove.settings[a], token);
        ConfigureButton(a);
        token = strtok_r(NULL, "|", &ptr);
        a++;
      }
  }
  
}

/**
 * Save startup settings
 */
boolean saveConfig() {

  const char filename[13] = "CONFIG.TXT";
  
  debug(F("Saving Config data to %s\n"), filename);
  
  // Open file for writing
  File file = openFile(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return false;
  }

  const size_t bufferSize = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(7) + 120;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  // Parse the root object
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["profile"] = Config.profile;
  root["access_code"] = Config.access_code;
  root["debug"] = Config.debug;
  root["echo"] = Config.echo;
  root["input"] = Config.input;
  root["baud"] = Config.baud | 9600;

  JsonArray& buttons = root.createNestedArray("buttons");
  buttons.add(Config.buttons[0]);
  buttons.add(Config.buttons[1]);
  buttons.add(Config.buttons[2]);
  buttons.add(Config.buttons[3]);
  buttons.add(Config.buttons[4]);
  buttons.add(Config.buttons[5]);

  // Serialize JSON to file
  if (root.prettyPrintTo(file) == 0) {
    Serial.println(F("Failed to write to file"));
    file.close();
    return false;
  } else {
    Serial.println(F("Config file saved."));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
  return true;
  
}

/***
 * Converts all in-memory settings to string
 */
char *settingsToString(char result[], const boolean pretty = false) 
{

  Serial.println("AT SETTINGS TO STRING");
  
  //const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 22*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9);
  // pre-shifter -> const size_t bufferSize = 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9) + 780;
  // with shifter
  //const size_t bufferSize = 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + 2*JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(9);

  DynamicJsonBuffer jsonBuffer(jsonBufferSize());

  Serial.println("AFTER ALLOCATION JSON BUFFER");
  
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = Settings.name;

  Serial.println("AFTER ROOT");
  
  JsonObject& volume = root.createNestedObject("volume");
  volume["master"] = Settings.volume.master;
  volume["microphone"] = Settings.volume.microphone;
  volume["linein"] = Settings.volume.linein;
  volume["lineout"] = Settings.volume.lineout;

  Serial.println("AFTER LEVELS");
  
  JsonObject& sounds = root.createNestedObject("sounds");
  sounds["dir"] = Settings.sounds.dir;
  sounds["start"] = Settings.sounds.start;
  sounds["button"] = Settings.sounds.button;
  
  Serial.println("AFTER SOUNDS");
  
  JsonObject& loop = root.createNestedObject("loop");
  loop["dir"] = Settings.loop.dir;
  loop["file"] = Settings.loop.file;
  loop["volume"] = Settings.loop.volume;
  loop["mute"] = Settings.loop.mute;

  Serial.println("AFTER LOOP");
  
  JsonObject& voice = root.createNestedObject("voice");
  voice["volume"] = Settings.voice.volume;
  voice["dry"] = Settings.voice.dry;
  voice["start"] = Settings.voice.start;
  voice["stop"] = Settings.voice.stop;
  voice["wait"] = Settings.voice.wait;

  Serial.println("AFTER VOICE");
  
  JsonObject& effects = root.createNestedObject("effects");
  effects["dir"] = Settings.effects.dir;
  effects["volume"] = Settings.effects.volume;
  effects["highpass"] = Settings.effects.highpass;

  Serial.println("AFTER EFFECTS");
  
  JsonObject& effects_bitcrusher = effects.createNestedObject("bitcrusher");
  effects_bitcrusher["bits"] = Settings.effects.bitcrusher.bits;
  effects_bitcrusher["rate"] = Settings.effects.bitcrusher.rate;

  Serial.println("AFTER BITCRUSHER");
  
  JsonObject& effects_chorus = effects.createNestedObject("chorus");
  effects_chorus["voices"] = Settings.effects.chorus.voices;
  effects_chorus["delay"] = Settings.effects.chorus.delay;
  effects_chorus["enabled"] = Settings.effects.chorus.enabled;

  Serial.println("AFTER CHORUS");
  
  JsonObject& effects_flanger = effects.createNestedObject("flanger");
  effects_flanger["delay"] = Settings.effects.flanger.delay;
  effects_flanger["offset"] = Settings.effects.flanger.offset;
  effects_flanger["depth"] = Settings.effects.flanger.depth;
  effects_flanger["freq"] = Settings.effects.flanger.freq;
  effects_flanger["enabled"] = Settings.effects.flanger.enabled;

  Serial.println("AFTER FLANGER");

  JsonObject& effects_shifter = effects.createNestedObject("shifter");
  effects_shifter["length"] = Settings.effects.shifter.length;
  effects_shifter["speed"] = Settings.effects.shifter.speed;
  effects_shifter["enabled"] = Settings.effects.shifter.enabled;
  
  effects["noise"] = Settings.effects.noise;
  effects["mute"] = Settings.effects.mute;

  Serial.println("END OF EFFECTS");
  
  JsonObject& eq = root.createNestedObject("eq");
  eq["active"] = Settings.eq.active;

  Serial.println("AFTER EQ");
  
  JsonArray& eq_bands = eq.createNestedArray("bands");
  eq_bands.add(Settings.eq.bands[0]);
  eq_bands.add(Settings.eq.bands[1]);
  eq_bands.add(Settings.eq.bands[2]);
  eq_bands.add(Settings.eq.bands[3]);
  eq_bands.add(Settings.eq.bands[4]);

  Serial.println("AFTER EQ BANDS");
  
  JsonObject& sleep = root.createNestedObject("sleep");
  sleep["timer"] = Settings.sleep.timer;
  sleep["file"] = Settings.sleep.file;

  Serial.println("AFTER SLEEP");

  JsonObject& glove = root.createNestedObject("glove");
  glove["dir"] = Settings.glove.dir;

  Serial.println("AFTER GLOVE");
  
  JsonArray& buttons = glove.createNestedArray("buttons");

  Serial.println("AFTER BUTTONS");
  
  char *button;
  
  JsonArray& buttons_0 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[0].buttons[0].getSettings();
  buttons_0.add(button);
  button = Settings.glove.ControlButtons[0].buttons[1].getSettings();
  buttons_0.add(button);

  Serial.println("AFTER BUTTON 0");
  
  JsonArray& buttons_1 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[1].buttons[0].getSettings();
  buttons_1.add(button);
  button = Settings.glove.ControlButtons[1].buttons[1].getSettings();
  buttons_1.add(button);

  Serial.println("AFTER BUTTON 1");
  
  JsonArray& buttons_2 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[2].buttons[0].getSettings();
  buttons_2.add(button);
  button = Settings.glove.ControlButtons[2].buttons[1].getSettings();
  buttons_2.add(button);

  Serial.println("AFTER BUTTON 2");
  
  JsonArray& buttons_3 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[3].buttons[0].getSettings();
  buttons_3.add(button);
  button = Settings.glove.ControlButtons[3].buttons[1].getSettings();
  buttons_3.add(button);

  Serial.println("AFTER BUTTON 3");
  
  JsonArray& buttons_4 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[4].buttons[0].getSettings();
  buttons_4.add(button);
  button = Settings.glove.ControlButtons[4].buttons[1].getSettings();
  buttons_4.add(button);

  Serial.println("AFTER BUTTON 4");
  
  JsonArray& buttons_5 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[5].buttons[0].getSettings();
  buttons_5.add(button);
  button = Settings.glove.ControlButtons[5].buttons[1].getSettings();
  buttons_5.add(button);

  Serial.println("AFTER BUTTON 5");
  
  //root.prettyPrintTo(Serial);
  
  if (pretty == true) {
    debug(F("ABOUT TO PRETTY PRINT %d"), root.measurePrettyLength() + 1);
    root.prettyPrintTo((char*)result, root.measurePrettyLength() + 1);
    Serial.println("AFTER PRETTY PRINT");
  } else {
    Serial.println("ABOUT TO PRINT");
    root.printTo((char*)result, root.measureLength() + 1);
    Serial.println("AFTER PRINTTO");
  }

  //size_t len = root.measureLength();
  //debug(F("Length: %d\n"), len);

  Serial.println("BEFORE FREE BUTTON");
  free(button);
  Serial.println("AFTER FREE BUTTON");
  
  return result;

}

/**
 * Backup settings to specified file
 */
boolean saveSettings(const char *src, const boolean backup = true) 
{

  char filename[FILENAME_SIZE];
  boolean result = false;
  if (strcasecmp(src, "") == 0) {
    strcpy(filename, Settings.file);
  } else {
    strcpy(filename, src);
  }
  if (backup == true) {
    addFileExt(filename);
  }
  // add profiles path to file name
  char srcFileName[30];
  strcpy(srcFileName, PROFILES_DIR);
  strcat(srcFileName, filename);
  debug(F("Settings file path: %s\n"), srcFileName);
  if (backup == true) {
    char backupfile[30];
    strcpy(backupfile, PROFILES_DIR);
    strcat(backupfile, filename);
    addBackupExt(backupfile);
    debug(F("Backup File: %s\n"), backupfile);
    File bakFile = openFile(backupfile, FILE_WRITE);
    File srcFile = openFile(srcFileName, FILE_READ);
    if (bakFile && srcFile) {
      char c;
      while (srcFile.available()) {
        c = srcFile.read();
        bakFile.write(c);
      }
      bakFile.close();
      srcFile.close();
    } else {
      debug(F("**ERROR** creating backup file %s!\n"), backupfile);
      if (srcFile) {
        srcFile.close();
      }
      if (bakFile) {
        bakFile.close();
      }
    }
  }
  // now save file
  debug(F("Save to: %s\n"), srcFileName);
  
 File newFile = openFile(srcFileName, FILE_WRITE);
  if (newFile) {
    char buffer[1600];
    char* p = settingsToString(buffer, true);
    newFile.print(p);
    newFile.close();
    //free(p);
    result = true;
    debug(F("Settings saved to %s\n"), srcFileName);
  } else {
    debug(F("**ERROR** saving to %s\n"), srcFileName);
  }
  return result;
  
}

/**
 * Set the specified file as the default profile that 
 * is loaded with TKTalkie starts
 */
boolean setDefaultProfile(char *filename) 
{
    addFileExt(filename);
    debug(F("Setting default profile to %s\n"), filename);
    char profiles[MAX_FILE_COUNT][FILENAME_SIZE];
    int total = listFiles(PROFILES_DIR, profiles, MAX_FILE_COUNT, FILE_EXT, false, false);
    boolean result = false;
    boolean found = false;
    for (int i = 0; i < total; i++) {
      if (strcasecmp(profiles[i], filename) == 0) {
        strlcpy(Config.profile, filename, sizeof(Config.profile));
        found = true;
        break;
      }
    }

    // save results to file if entry was not found
    if (found == true) {
      result = saveConfig();
    } else {
      debug(F("Filename was not an existing profile\n"));
    }  

    if (result == true) {
      debug(F("Default profile set\n"));
    } else {
      debug(F("**ERROR** setting default profile\n"));
    }
  
    return result;
}

/**
 * Remove a profile from the list and delete the file
 */
boolean deleteProfile(char *filename) 
{
  boolean result = false;
  addFileExt(filename);
  char path[SETTING_ENTRY_MAX];
  strcpy(path, PROFILES_DIR);
  strcat(path, filename);
  debug(F("Deleting file %s\n"), path);
  // can't delete current profile
  if (strcasecmp(filename, Settings.file) == 0){
    debug(F("Cannot delete current profile\n"));
    result = false;
  } else {
    result = deleteFile(path);
    // if the profile filename was the default profile, 
    // set the default profile to the currently loaded profile
    if (result == true) {
      if (strcasecmp(filename, Config.profile) == 0) {
        debug(F("Profile was default -> Setting default profile to current profile\n"));
        result = setDefaultProfile(Settings.file);
      }
    }

  }
  return result;
}

/**
 * Load specified settings file
 */
void loadSettings(char *filename, Settings_t *settings, const boolean nameOnly) 
{

  char *ret = strstr(filename, ".");  
  if (ret == NULL) {
    addFileExt(filename);
  }
  strlcpy(settings->file, filename, sizeof(settings->file));
  
  // pre-shifter -> const size_t bufferSize = 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9) + 780;
  // with shifter
  //const size_t bufferSize = 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + 2*JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(9) + 830;
  DynamicJsonBuffer jsonBuffer(jsonBufferSize());
  
  char srcFileName[27];
  strcpy(srcFileName, PROFILES_DIR);
  strcat(srcFileName, filename);

  debug(F("Loading settings file: %s\n"), srcFileName);
  
  File file = SD.open(srcFileName);

  if (!file) {
    debug(F("Error reading file %s\n"), srcFileName);
  } else {
    debug(F("Opened %s\n"), srcFileName);
  }
  
  JsonObject& root = jsonBuffer.parseObject(file);

  if (!root.success()) {
    debug(F("ERROR PARSING SETTINGS FILE!\n"));
    return;
  } else {
    debug(F("Parsed settings file OK\n"));
  }
  
  file.close();

  strlcpy(settings->name, root["name"], sizeof(settings->name)); // "aaaaaaaaaaaaaaaaaaaa"

  if (nameOnly) {
    return;
  }
  
  JsonObject& volume = root["volume"];

  char buf[30];
    
  settings->volume.master = volume["master"]; // 0.55
  
  settings->volume.microphone = volume["microphone"]; // 10
  settings->volume.linein = volume["linein"]; // 10
  settings->volume.lineout = volume["lineout"]; // 30

  dtostrf(settings->volume.master, 0, 3, buf);
  debug(F("Volume.master: %s\n"), buf);
  debug(F("Volume.microphone: %d\n"), settings->volume.microphone);
  debug(F("Volume.linein: %d\n"), settings->volume.linein);
  debug(F("Volume.lineout: %d\n"), settings->volume.lineout);
  
  JsonObject& sounds = root["sounds"];
  strlcpy(settings->sounds.dir, sounds["dir"], sizeof(settings->sounds.dir));
  strlcpy(settings->sounds.start, sounds["start"], sizeof(settings->sounds.start));
  strlcpy(settings->sounds.button, sounds["button"], sizeof(settings->sounds.button)); // "aaaaaaaa.aaa"

  debug(F("Sounds.dir: %s\n"), settings->sounds.dir);
  debug(F("Sounds.start: %s\n"), settings->sounds.start);
  debug(F("Sounds.button: %s\n"), settings->sounds.button);
  
  JsonObject& loop = root["loop"];
  strlcpy(settings->loop.dir, loop["dir"], sizeof(settings->loop.dir)); // "/aaaaaaaa/"
  strlcpy(settings->loop.file, loop["file"], sizeof(settings->loop.file)); // "aaaaaaaa.aaa"
  settings->loop.volume = loop["volume"]; // 0.02
  settings->loop.mute = loop["mute"]; // 1

  debug(F("Loop.dir: %s\n"), settings->loop.dir);
  debug(F("Loop.file: %s\n"), settings->loop.file);
  dtostrf(settings->loop.volume, 0, 3, buf);
  debug(F("Loop.volume %s\n"), buf);
  debug(F("Loop.mute: %d\n"), settings->loop.mute);
  
  JsonObject& voice = root["voice"];
  settings->voice.volume = voice["volume"]; // 3
  settings->voice.dry = voice["dry"]; // 0
  settings->voice.start = voice["start"]; // 0.043
  settings->voice.stop = voice["stop"]; // 0.02
  settings->voice.wait = voice["wait"]; // 1000

  dtostrf(settings->voice.volume, 0, 3, buf);
  debug(F("Voice.volume: %s\n"), buf);
  dtostrf(settings->voice.dry, 0, 2, buf);
  debug(F("Voice.dry: %s\n"), buf);
  dtostrf(settings->voice.start, 0, 4, buf);
  debug(F("Voice.start %s\n"), buf);
  dtostrf(settings->voice.stop, 0, 4, buf);
  debug(F("Voice.stop: %s\n"), buf);
  debug(F("Voice.wait: %d\n"), settings->voice.wait);
  
  JsonObject& effects = root["effects"];
  strlcpy(settings->effects.dir, effects["dir"], sizeof(settings->effects.dir)); // "/aaaaaaaa/"
  settings->effects.volume = effects["volume"]; // 1
  settings->effects.highpass = effects["highpass"]; // 1

  dtostrf(settings->effects.volume, 0, 3, buf);
  debug(F("Effects.volume: %s\n"), buf);
  debug(F("Effects.highpass: %d\n"), settings->effects.highpass);
  
  settings->effects.bitcrusher.bits = effects["bitcrusher"]["bits"]; // 16
  settings->effects.bitcrusher.rate = effects["bitcrusher"]["rate"]; // 44100
  debug(F("Bitcrusher.bits: %d\n"), settings->effects.bitcrusher.bits);
  debug(F("Bitcrusher.rate: %d\n"), settings->effects.bitcrusher.rate);
  
  settings->effects.chorus.voices = effects["chorus"]["voices"]; // 10
  settings->effects.chorus.delay = effects["chorus"]["delay"]; // 1000
  //settings->effects.chorus.enabled = (effects["chorus"]["enabled"] | 0);

  debug(F("Chorus.voices: %d\n"), settings->effects.chorus.voices);
  debug(F("Chorus.delay: %d\n"), settings->effects.chorus.delay);
  debug(F("Chorus.enabled: %d\n"), settings->effects.chorus.enabled);
  
  JsonObject& effects_flanger = effects["flanger"];
  settings->effects.flanger.delay = effects_flanger["delay"]; // 32
  settings->effects.flanger.offset = effects_flanger["offset"]; // 10
  settings->effects.flanger.depth = effects_flanger["depth"]; // 10
  settings->effects.flanger.freq = effects_flanger["freq"]; // 0.0625
  settings->effects.flanger.enabled = (effects["flanger"]["enabled"] | 0);
  
  debug(F("Flanger.delay: %d\n"), settings->effects.flanger.delay);
  debug(F("Flanger.offset: %d\n"), settings->effects.flanger.offset);
  debug(F("Flanger.depth: %d\n"), settings->effects.flanger.depth);
  dtostrf(settings->effects.flanger.freq, 0, 4, buf);
  debug(F("Flanger.freq: %s\n"), buf);
  debug(F("Flanger.enabled: %d\n"), settings->effects.flanger.enabled);

  JsonObject& effects_shifter = effects["shifter"];
  settings->effects.shifter.length = effects_shifter["length"]; // 1023
  settings->effects.shifter.speed = effects_shifter["speed"]; // 1023
  settings->effects.shifter.enabled = effects_shifter["enabled"]; // 1

  settings->effects.noise = effects["noise"]; // 0.014
  settings->effects.mute = effects["mute"]; // 1

  dtostrf(settings->effects.noise, 0, 3, buf);
  debug(F("Noise.volume: %s\n"), buf);
  
  settings->eq.active = root["eq"]["active"]; // 1
  
  JsonArray& eq_bands = root["eq"]["bands"];
  settings->eq.bands[0] = eq_bands[0]; // -0.2
  settings->eq.bands[1] = eq_bands[1]; // -0.4
  settings->eq.bands[2] = eq_bands[2]; // -0.35
  settings->eq.bands[3] = eq_bands[3]; // -0.35
  settings->eq.bands[4] = eq_bands[4]; // -0.35

  settings->sleep.timer = root["sleep"]["timer"]; // 60000
  strlcpy(settings->sleep.file, root["sleep"]["file"], sizeof(settings->sleep.file)); // "aaaaaaaa.aaa"

  strlcpy(settings->glove.dir, root["glove"]["dir"], sizeof(settings->glove.dir));
  
  JsonArray& glove_buttons = root["glove"]["buttons"];

  for (byte i = 0; i < 6; i++) {
    strcpy(buf, glove_buttons[i][0]);
    strcat(buf, ";");
    strcat(buf, glove_buttons[i][1]);
    strlcpy(settings->glove.settings[i], buf, sizeof(settings->glove.settings[i]));
    memset(buf, 0, sizeof(buf));
  }
  
}

void applySettings() {
  // Apply settings
  setVolume();
  setMicGain();
  setLineout();
  setLinein();
  setHipass();
  setEffectsVolume();
  setNoiseVolume();
  setVoiceVolume();
  setDryVolume();
  setLoopVolume();
  setEq();
  setEqBands();
  setBitcrusher();
  setEffectsDir();
  setSoundsDir();
  setLoopDir();
  setGloveDir();
  setLoopMute();
  setEffectsMute();
  setSleepTimer();
  setChorus();
  flange1.begin(Settings.effects.flanger.buffer,Settings.effects.flanger.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq); 
  setFlanger(); 
  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);
  setShifter();
  // Configure Buttons
  for (byte i = 0; i < 6; i++) {
    ConfigureButton(i);
  }
}


