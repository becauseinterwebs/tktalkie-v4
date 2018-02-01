/****
 * Routines to handle reading/handling settings
 */

void ConfigureButton(byte a) {
  
    Settings.glove.ControlButtons[a].setPTT(false);
    Settings.glove.ControlButtons[a].setPin(0);
    byte buttonNum = 0;
    char buf[25];
    byte pin = Config.buttons[a];
    
    Serial.print("** Configure Button for Pin: ");
    Serial.println(pin);
    if (pin > 0) {

      // setup physical button
      Settings.glove.ControlButtons[a].setup(pin);
      
      strcpy(buf, Settings.glove.settings[a]);
      char *part_token, *part_ptr;
      Serial.print("-- SETTINGS: ");
      Serial.println(buf);
      part_token = strtok_r(buf, ";", &part_ptr);
      // button_type,data(sound)
      Serial.print("Initial part token: ");
      Serial.println(part_token);
      byte b = 0;

      while (part_token && b < 2) {
        char *button_token, *button_ptr;
        button_token = strtok_r(part_token, ",", &button_ptr);
        Serial.print("Intitial button token: ");
        Serial.println(button_token);
        byte button_type = (byte)atoi(button_token);
        Serial.print("Button Type: ");
        Serial.println(button_type);
        // by default, do not continue processing
        byte max = 0;
        // Determine how many options we need to read 
        // based on the type of button
        switch (button_type) {
          // PTT/Sleep/Wake Button
          case 1:
            {
              debug(F("PTT Button on pin: %d\n"), pin);
              Serial.println(" -> PTT Button");
              App.ptt_button = a;
              Settings.glove.ControlButtons[a].setPTT(true);
              if (App.wake_button == NULL) {
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
              Serial.print("Sleep Button on pin: ");
              Serial.println(pin);
              App.wake_button = a;
              snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
            }  
            break;   
        }

        // setup virtual button type
        Serial.print("Setting phyical button ");
        Serial.print(a);
        Serial.print(", Virtual button ");
        Serial.print(buttonNum);
        Serial.print(" to ");
        Serial.print(button_type);
        Serial.print(" max: ");
        Serial.println(max);
        
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
                debug(F("Sound Button on pin: %d\n"), pin);
                Serial.println(" -> Sound Button");
                Serial.print("Setting sound to: ");
                Serial.println(button_token);
                Settings.glove.ControlButtons[a].buttons[buttonNum].setSound(button_token);
              }  
              break;
          }

          // NOTE: Each physical button needs to have 
          // to virtual button properties that hold 
          // what to do
          
          Serial.print("token -> ");
          Serial.print(a);
          Serial.print(" -> ");
          Serial.print(b);
          Serial.print(" -> ");
          Serial.print(c);
          Serial.print(" -> ");
          Serial.println(button_token);
          //byte cb_type = atoi(token);
          c++;
          button_token = strtok_r(NULL, ",", &button_ptr);
        }  
        buttonNum++;
        b++;
        part_token = strtok_r(NULL, ";", &part_ptr);
      }
    }
    Serial.println("--------------END OF BUTTON-----------------");
    Serial.println("");
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
    if (Settings.volume.master > 1) { 
      Settings.volume.master = 1;
    } else if (Settings.volume.master < 0) {
      Settings.volume.master = 0;
    }
    audioShield.volume(readVolume());
  } else if (strcasecmp(settingName, "lineout") == 0) {
    Settings.volume.lineout = (byte)atoi(settingValue);
    if (Settings.volume.lineout < 13) {
      Settings.volume.lineout = 13;  
    } else if (Settings.volume.lineout > 31) {
      Settings.volume.lineout = 31;
    }
    audioShield.lineOutLevel(Settings.volume.lineout);
  } else if (strcasecmp(settingName, "linein") == 0) {
    Settings.volume.linein = (byte)atoi(settingValue);
    if (Settings.volume.linein < 0) {
      Settings.volume.linein = 0;  
    } else if (Settings.volume.linein > 15) {
      Settings.volume.linein = 15;
    }  
    audioShield.lineInLevel(Settings.volume.linein);
  } else if ((strcasecmp(settingName, "high_pass") == 0) || (strcasecmp(settingName, "highpass") == 0)) {
    Settings.effects.hipass = (byte)atoi(settingValue);
    if (Settings.effects.hipass < 0) { 
      Settings.effects.hipass = 0;
    } else if (Settings.effects.hipass > 1) {
      Settings.effects.hipass = 1;
    }
    if (Settings.effects.hipass == 0) {
      audioShield.adcHighPassFilterDisable();
    } else {
      audioShield.adcHighPassFilterEnable();
    }
  } else if (strcasecmp(settingName, "microphone") == 0) {
    Settings.volume.microphone = atoi(settingValue);
    audioShield.micGain(Settings.volume.microphone);  
  } else if (strcasecmp(settingName, "button_click") == 0) {
    memset(Settings.sounds.button, 0, sizeof(Settings.sounds.button));
    strcpy(Settings.sounds.button, settingValue);
  } else if (strcasecmp(settingName, "startup") == 0) {
    memset(Settings.sounds.start, 0, sizeof(Settings.sounds.start));
    strcpy(Settings.sounds.start, settingValue);
  } else if (strcasecmp(settingName, "startup_sound") == 0) {
    memset(Settings.sounds.start, 0, sizeof(Settings.sounds.start));
    strcpy(Settings.sounds.start, settingValue);  
  } else if (strcasecmp(settingName, "loop") == 0) {
    memset(Settings.loop.file, 0, sizeof(Settings.loop.file));
    strcpy(Settings.loop.file, settingValue);
  } else if (strcasecmp(settingName, "noise_gain") == 0) {
    Settings.effects.noise = atof(settingValue);
    effectsMixer.gain(3, Settings.effects.noise);
  } else if (strcasecmp(settingName, "voice_gain") == 0) {
    Settings.voice.volume = atof(settingValue);
    voiceMixer.gain(0, Settings.voice.volume);
    voiceMixer.gain(1, Settings.voice.volume);
  } else if (strcasecmp(settingName, "dry_gain") == 0) {
    Settings.voice.dry = atof(settingValue);  
    voiceMixer.gain(2, Settings.voice.dry);
  } else if (strcasecmp(settingName, "effects_gain") == 0) {
    Settings.effects.volume = atof(settingValue);
    effectsMixer.gain(0, Settings.effects.volume);
    //effectsMixer.gain(1, Settings.effects.volume);
    // Waveform (BLE) connect sound
    effectsMixer.gain(2, Settings.effects.volume);
  } else if (strcasecmp(settingName, "loop_gain") == 0) {
    Settings.loop.volume = atof(settingValue);
    if (Settings.loop.volume < 0 or Settings.loop.volume > 32767) {
      Settings.loop.volume = 4;
    }
    // chatter loop from SD card
    effectsMixer.gain(1, Settings.loop.volume);
    //loopMixer.gain(0, Settings.loop.volume);
    //loopMixer.gain(1, Settings.loop.volume);
  } else if (strcasecmp(settingName, "silence_time") == 0) {
    Settings.voice.wait = atoi(settingValue);
  } else if (strcasecmp(settingName, "voice_start") == 0) {
    Settings.voice.start = atof(settingValue);
  } else if (strcasecmp(settingName, "voice_stop") == 0) {  
    Settings.voice.stop = atof(settingValue);
  } else if (strcasecmp(settingName, "input") == 0) {
      // This is actually set in the SETTINGS.TXT file but 
      // is here for older systems where there could be 
      // both a line-in and a mic input
      /*
      Config.input = (byte)atoi(settingValue);
      if (Config.input > 1) {
        Config.input = 1;
      } else if (Config.input < 0) {
        Config.input = 0;
      }
      // tell the audio shield which input to use
      audioShield.inputSelect(Config.input);
      */
  } else if (strcasecmp(settingName, "eq") == 0) {
    Settings.eq.active = (byte)atoi(settingValue);
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
  } else if (strcasecmp(settingName, "eq_bands") == 0) {
    // clear bands and prep for setting
    for (int i = 0; i < 6; i++) {
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
    // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
    // Valid values are -1 (-11.75dB) to 1 (+12dB)
    // The settings below pull down the lows and highs and push up the mids for 
    // more of a "tin-can" sound.
    audioShield.eqBands(Settings.eq.bands[0], Settings.eq.bands[1], Settings.eq.bands[2], Settings.eq.bands[3], Settings.eq.bands[4]);
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
    // You can modify these values to process the voice 
    // input.  See the Teensy bitcrusher demo for details.
    bitcrusher1.bits(Settings.effects.bitcrusher.bits);
    bitcrusher1.sampleRate(Settings.effects.bitcrusher.rate);
  } else if (strcasecmp(settingName, "effects_dir") == 0) {
    memset(Settings.effects.dir, 0, sizeof(Settings.effects.dir));
    strcpy(Settings.effects.dir, settingValue);
    fixPath(Settings.effects.dir);
    loadSoundEffects();
  } else if (strcasecmp(settingName, "sounds_dir") == 0) {
    memset(Settings.sounds.dir, 0, sizeof(Settings.sounds.dir));
    strcpy(Settings.sounds.dir, settingValue);
    fixPath(Settings.sounds.dir);
  } else if (strcasecmp(settingName, "loop_dir") == 0) {
    memset(Settings.loop.dir, 0, sizeof(Settings.loop.dir));
    strcpy(Settings.loop.dir, settingValue);
    fixPath(Settings.loop.dir);
  } else if (strcasecmp(settingName, "mute_loop") == 0) {
      Settings.loop.mute = (byte)atoi(settingValue);
      if (Settings.loop.mute > 1) {
        Settings.loop.mute = 1;
      } else if (Settings.loop.mute < 0) {
        Settings.loop.mute = 0;
      }
  } else if (strcasecmp(settingName, "mute_effects") == 0) {
      Settings.effects.mute = (byte)atoi(settingValue);
      if (Settings.effects.mute > 1) {
        Settings.effects.mute = 1;
      } else if (Settings.effects.mute < 0) {
        Settings.effects.mute = 0;
      }
  } else if (strcasecmp(settingName, "sleep_time") == 0) {
      Settings.sleep.timer = (byte)atoi(settingValue);
      if (Settings.sleep.timer < 0) {
        Settings.sleep.timer = 0;
      }
  } else if (strcasecmp(settingName, "sleep_sound") == 0) {
      memset(Settings.sleep.file, 0, sizeof(Settings.sleep.file));
      strcpy(Settings.sleep.file, settingValue);
  } else if (strcasecmp(settingName, "chorus") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        chorus1.voices(0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        chorus1.voices(Settings.effects.chorus.voices);
      } else {
        char *token, *ptr;
        token = strtok_r(settingValue, ",", &ptr);
        byte i = 0;
        while (token && i < 3) {
          switch (i) {
            case 0:
              Settings.effects.chorus.delay = (byte)atoi(token);
              break;
            case 1:
              Settings.effects.chorus.voices = (byte)atoi(token);
              break;
          }
          i++;
          token = strtok_r(NULL, ",", &ptr);
        }  
        if (Settings.effects.chorus.delay > 32) {
          Settings.effects.chorus.delay = 32;
        }
        if (Settings.effects.chorus.delay < 1) {
          Settings.effects.chorus.delay = 1;
        }
        if (Settings.effects.chorus.voices < 0) {
          Settings.effects.chorus.voices = 0;
        }
        if(!chorus1.begin(Settings.effects.chorus.buffer,Settings.effects.chorus.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.chorus.voices)) {
           Serial.println("chorus: startup failed");
        }
      }
  } else if (strcasecmp(settingName, "chorus_delay") == 0) {
      Settings.effects.chorus.delay = (byte)atoi(settingValue);
      if (Settings.effects.chorus.delay > 32) {
        Settings.effects.chorus.delay = 32;
      }
      if (Settings.effects.chorus.delay < 1) {
        Settings.effects.chorus.delay = 1;
      }
      if(!chorus1.begin(Settings.effects.chorus.buffer,Settings.effects.chorus.delay * AUDIO_BLOCK_SAMPLES,Settings.effects.chorus.voices)) {
         Serial.println("chorus_delay: Startup failed");
      }
  } else if (strcasecmp(settingName, "chorus_voices") == 0) {
      Settings.effects.chorus.voices = (byte)atoi(settingValue);
      if (Settings.effects.chorus.voices < 0) {
        Settings.effects.chorus.voices = 0;
      }
      chorus1.voices(Settings.effects.chorus.voices);
  } else if (strcasecmp(settingName, "flange_delay") == 0) {
      Settings.effects.flanger.delay = (byte)atoi(settingValue);
      if (Settings.effects.flanger.delay > 32) {
        Settings.effects.flanger.delay = 32;
      }
      if (Settings.effects.flanger.delay < 0) {
        Settings.effects.flanger.delay = 0;
      }
      flange1.begin(Settings.effects.flanger.buffer,Settings.effects.flanger.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);    
  } else if (strcasecmp(settingName, "flange_freq") == 0) {
      Settings.effects.flanger.freq = atof(settingValue);
      if (Settings.effects.flanger.freq < 0) {
        Settings.effects.flanger.freq = 0;
      }
      if (Settings.effects.flanger.freq > 10) {
        Settings.effects.flanger.freq = 10;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flange_depth") == 0) {
      Settings.effects.flanger.depth = (byte)atoi(settingValue);
      if (Settings.effects.flanger.depth < 0) {
        Settings.effects.flanger.depth = 0;
      }
      if (Settings.effects.flanger.depth > 255) {
        Settings.effects.flanger.depth = 255;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flange_idx") == 0) {
      Settings.effects.flanger.offset = (byte)atoi(settingValue);// * Settings.effects.flanger.delay_LENGTH;
      if (Settings.effects.flanger.offset < 1) {
        Settings.effects.flanger.offset = 1;
      }
      if (Settings.effects.flanger.offset > 128) {
        Settings.effects.flanger.offset = 128;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flange") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        flange1.voices(Settings.FLANGE_DELAY_PASSTHRU,0,0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
      }
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
 * Create JSON string of settings 
 * (used for app)
 */
char *settingsToJson(char result[]) 
{

/*
  const char str_template[14] = "\"%s\":\"%s\"";
  const char num_template[14] = "\"%s\":%s";
  char buf[20];
  char tmp[100];

  sprintf(tmp, str_template, "name", Settings.name);
  strcpy(result, tmp);
  strcat(result, ",");

  dtostrf(Settings.volume.master, 0, 4, buf);
  sprintf(tmp, num_template, "volume", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.volume.microphone);
  sprintf(tmp, num_template, "microphone", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Settings.volume.linein);
  sprintf(tmp, num_template, "linein", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.volume.lineout);
  sprintf(tmp, num_template, "lineout", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "startup", Settings.sounds.start);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "loop", Settings.loop.file);
  strcat(result, tmp);
  strcat(result, ",");

  dtostrf(Settings.loop.volume, 0, 4, buf);
  sprintf(tmp, num_template, "loop_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Settings.effects.hipass);
  sprintf(tmp, num_template, "high_pass", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice.volume, 0, 4, buf);
  sprintf(tmp, num_template, "voice_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  dtostrf(Settings.voice.dry, 0, 4, buf);
  sprintf(tmp, num_template, "dry_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice.dry, 0, 4, buf);
  sprintf(tmp, num_template, "dry_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice.start, 0, 4, buf);
  sprintf(tmp, num_template, "voice_start", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice.stop, 0, 4, buf);
  sprintf(tmp, num_template, "voice_stop", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Settings.voice.wait);
  sprintf(tmp, num_template, "silence_time", buf);
  strcat(result, tmp);
  strcat(result, ",");
  */
  
  /*
  sprintf(buf, "%d", BUTTON_PIN);
  sprintf(tmp, num_template, "button_pin", buf);
  strcat(result, tmp);
  strcat(result, ",");
  */

  /*
  sprintf(tmp, str_template, "button_click", Settings.sounds.button);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Config.input);
  sprintf(tmp, num_template, "input", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.effects.volume, 0, 4, buf);
  sprintf(tmp, num_template, "effects_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.eq.active);
  sprintf(tmp, num_template, "eq", buf);
  strcat(result, tmp);
  strcat(result, ",");

  char buffer[SETTING_ENTRY_MAX];
  char *bands = arrayToString(buffer, Settings.eq.bands, 6);
  sprintf(tmp, str_template, "eq_bands", bands);
  strcat(result, tmp);
  strcat(result, ",");
  memset(buffer, 0, sizeof(buffer));
  
  char *bitcrushers = arrayToString(buffer, Settings.bitcrusher, 3);
  sprintf(tmp, str_template, "bitcrushers", bitcrushers);
  strcat(result, tmp);
  strcat(result, ",");
  memset(buffer, 0, sizeof(buffer));
  
  dtostrf(Settings.effects.noise, 0, 4, buf);
  sprintf(tmp, num_template, "noise_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "effects_dir", Settings.effects.dir);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "sounds_dir", Settings.sounds.dir);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "loop_dir", Settings.loop.dir);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.loop.mute);
  sprintf(tmp, num_template, "mute_loop", buf);
  strcat(result, tmp);

  if (APP_VER > 1.13) {
    sprintf(buf, "%d", Settings.effects.mute);
    sprintf(tmp, num_template, "mute_effects", buf);
    strcat(result, tmp);
  }

  if (APP_VER > 1.14) {
    strcat(result, ",");
    sprintf(buf, "%d", Settings.sleep.timer);
    sprintf(tmp, num_template, "sleep_time", buf);
    strcat(result, tmp);
    strcat(result, ",");
    
    sprintf(tmp, str_template, "sleep_sound", Settings.sleep.file);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.effects.chorus.delay);
    sprintf(tmp, num_template, "chorus_delay", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.effects.chorus.voices);
    sprintf(tmp, num_template, "chorus_voices", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.effects.flanger.delay);
    sprintf(tmp, num_template, "flange_delay", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.effects.flanger.offset);
    sprintf(tmp, num_template, "flange_offset", buf);
    strcat(result, tmp);
    strcat(result, ",");
    
    sprintf(buf, "%d", Settings.effects.flanger.depth);
    sprintf(tmp, num_template, "flange_depth", buf);
    strcat(result, tmp);
    strcat(result, ",");

    dtostrf(Settings.effects.flanger.freq, 0, 4, buf);
    sprintf(tmp, num_template, "flange_freq", buf);
    strcat(result, tmp);
    */
/*
    strcat(result, "\"buttons\": { \"pins\":[");
    for (byte i = 0; i < 6; i++) {
      strcat(result, Config.buttons[i]);
      if (i < 5) {
        strcat(result, ",");
      }
    }
    strcat(result, "],\"settings\":[");
    for (byte i = 0; i < 6; i++) {
      strcat(result, "\"");
      strcat(result, Settings.glove.settings[i]);
      strcat(result, "\"");
      if (i < 5) {
        strcat(result, ","); 
      }
    }
    strcat(result, "]}");
    */
    //sprintf(tmp, str_template, "buttons", buttons);
    //strcat(result, tmp);
    
  //}
  
  //return result;
  return "";
  
}

/***
 * Converts all in-memory settings to string
 */
char *settingsToString(char result[]) 
{
  /*
  char buf[SETTING_ENTRY_MAX];

  strcpy(result, "[name=");
  strcat(result, Settings.name);
  strcat(result, "]\n");

  if (Settings.volume > 0) {
    strcat(result, "[volume=");
    dtostrf(Settings.volume, 0, 4, buf);
    strcat(result, buf);
    strcat(result, "]\n");
    memset(buf, 0, sizeof(buf));
  }

  strcat(result, "[linein=");
  sprintf(buf, "%d", Settings.volume.linein);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[lineout=");
  sprintf(buf, "%d", Settings.volume.lineout);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[startup=");
  strcat(result, Settings.sounds.start);
  strcat(result, "]\n");

  strcat(result, "[loop=");
  strcat(result, Settings.loop.file);
  strcat(result, "]\n");

  strcat(result, "[loop_gain=");
  dtostrf(Settings.loop.volume, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));  

  strcat(result, "[high_pass=");
  sprintf(buf, "%d", Settings.effects.hipass);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[voice_gain=");
  dtostrf(Settings.voice.volume, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[dry_gain=");
  dtostrf(Settings.voice.dry, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
  
  strcat(result, "[voice_start=");
  dtostrf(Settings.voice.start, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[voice_stop=");
  dtostrf(Settings.voice.stop, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");

  memset(buf, 0, sizeof(buf));
  strcat(result, "[silence_time=");
  sprintf(buf, "%d", Settings.voice.wait);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
*/
/* this was commented out before
  strcat(result, "[button_pin=");
  sprintf(buf, "%d", BUTTON_PIN);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
*/
/*
  strcat(result, "[button_click=");
  strcat(result, Settings.sounds.button);
  strcat(result, "]\n");

  strcat(result, "[input=");
  sprintf(buf, "%d", Config.input);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[microphone=");
  sprintf(buf, "%d", Settings.volume.microphone);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[effects_gain=");
  dtostrf(Settings.effects.volume, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[eq=");
  sprintf(buf, "%d", Settings.eq.active);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[eq_bands=");
  char *bands = arrayToString(buf, Settings.eq.bands, 6);
  strcat(result, bands);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[bitcrushers=");
  char *bitcrushers = arrayToString(buf, Settings.bitcrusher, 3); 
  strcat(result, bitcrushers);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[noise_gain=");
  dtostrf(Settings.effects.noise, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[effects_dir=");
  strcat(result, Settings.effects.dir);
  strcat(result, "]\n");

  strcat(result, "[sounds_dir=");
  strcat(result, Settings.sounds.dir);
  strcat(result, "]\n");

  strcat(result, "[loop_dir=");
  strcat(result, Settings.loop.dir);
  strcat(result, "]\n");

  strcat(result, "[mute_loop=");
  sprintf(buf, "%d", Settings.loop.mute);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[mute_effects=");
  sprintf(buf, "%d", Settings.effects.mute);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[sleep_time=");
  sprintf(buf, "%d", Settings.sleep.timer);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[sleep_sound=");
  strcat(result, Settings.sleep.file);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[chorus_delay=");
  sprintf(buf, "%d", Settings.effects.chorus.delay);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[chorus_voices=");
  sprintf(buf, "%d", Settings.effects.chorus.voices);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_delay=");
  sprintf(buf, "%d", Settings.effects.flanger.delay);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_offset=");
  sprintf(buf, "%d", Settings.effects.flanger.offset);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_depth=");
  sprintf(buf, "%d", Settings.effects.flanger.depth);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_freq=");
  dtostrf(Settings.effects.flanger.freq, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[buttons=");
  for (byte i=0; i<6; i++) {
    Serial.println(Settings.glove.settings[i]);
    strcpy(buf, Settings.glove.settings[i]);
    strcat(result, buf);
    if (i<5) {
      strcat(result, "|");
    }
  }
  strcat(result, "]\n");
  return result;
  */
  return "";
}

/**
 * Set a startup setting value
 */
void setSettingValue(const char *key, const char *newValue)
{
  /*
   int index = -1;
   char newKey[SETTING_ENTRY_MAX];
   for (int i = 0; i < 6; i++) {
    char data[SETTING_ENTRY_MAX];
    strcpy(data, STARTUP_SETTINGS[i]);
    if (strcasecmp(data, "") != 0) {
      char *settingKey, *ptr;
      settingKey = strtok_r(data, "=", &ptr);
      if (strcasecmp(key, settingKey) == 0) {
        index = i;
        break;
      }
    } else {
      // first blank space
      index = i;
      break;
    }
  } 

  // This shouldn't happen, but just in case ;)
  if (index > 6 - 1) {
    Serial.println("Invalid setting index!");
    return;
  }
  
  char buf[SETTING_ENTRY_MAX];
  strcpy(newKey, key);
  strcat(newKey, "=");
  strcpy(buf, newKey);
  strcat(buf, newValue);
  memset(STARTUP_SETTINGS[index], 0, SETTING_ENTRY_MAX);
  strcpy(STARTUP_SETTINGS[index], buf);  
  */
}

/**
 * Retrieve a startup setting value
 */
char *getSettingValue(char result[], const char *key) 
{
    debug(F("Get setting: %s\n"), key);

    /*
    for (int i = 0; i < 6; i++) {
      char setting[SETTING_ENTRY_MAX] = "";
      strcpy(setting, STARTUP_SETTINGS[i]);
      if (strcasecmp(setting, "") != 0) {
        char *name, *value, *ptr;
        name = strtok_r(setting, "=", &ptr);
        value = strtok_r(NULL, "=", &ptr);
        debug(F("Check setting %s = %s\n"), name, value);
        if (strcasecmp(name, key) == 0) {
          result = value;
          break;
        }
      }
    }
    debug(F("Return value %s\n"), result);

    return result;
    */
}

/**
 * Save startup settings
 */
boolean saveConfig() {

  const char filename[13] = "SETTINGS.TXT";
  
  debug(F("Saving Config data\n"));
  
  // Delete existing file, otherwise the Configuration is appended to the file
  SD.remove(filename);

  // Open file for writing
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return false;
  }

  // Allocate the memory pool on the stack
  // Don't forget to change the capacity to match your JSON document.
  // Use https://arduinojson.org/assistant/ to compute the capacity.
  const size_t bufferSize = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(6) + 120;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  // Parse the root object
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["profile"] = Config.profile;
  root["access_code"] = Config.access_code;
  root["debug"] = Config.debug == true ? 1 : 0;
  root["echo"] = Config.echo == true ? 1 : 0;
  root["input"] = Config.input;

  JsonArray& buttons = root.createNestedArray("buttons");
  buttons.add(Config.buttons[0]);
  buttons.add(Config.buttons[1]);
  buttons.add(Config.buttons[2]);
  buttons.add(Config.buttons[3]);
  buttons.add(Config.buttons[4]);
  buttons.add(Config.buttons[5]);

  // Serialize JSON to file
  if (root.printTo(file) == 0) {
    Serial.println(F("Failed to write to file"));
    file.close();
    return false;
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
  return true;
  /*
  File srcFile = openFile("SETTINGS.TXT", FILE_WRITE);
  if (srcFile) {
    for (int i = 0; i < 6; i++) {
      debug(F("%d: %s\n"), i, STARTUP_SETTINGS[i]);
      srcFile.print("[");
      srcFile.print(STARTUP_SETTINGS[i]);
      srcFile.print("]\n");      
    }
    srcFile.close();
    debug(F("Startup Settings Updated\n"));
    return true;
  } else {
    debug(F("**ERROR** Updating Startup Settings!\n"));
    beep(4);
    return false;
  }
  */
}

/**
 * Backup settings to specified file
 */
boolean saveSettingsFile(const char *src, const boolean backup = true) 
{
  char filename[SETTING_ENTRY_MAX];
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
  char srcFileName[25];
  strcpy(srcFileName, PROFILES_DIR);
  strcat(srcFileName, filename);
  if (backup == true) {
    char backupfile[SETTING_ENTRY_MAX];
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
      debug(F("**ERROR** creating backup file!\n"));
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
    char buffer[1024];
    char *p = settingsToString(buffer);
    newFile.println(p);
    newFile.close();
    result = true;
    Serial.print(F("Settings saved to "));
    Serial.println(srcFileName);
  } else {
    Serial.print(F("**ERROR** saving to: "));
    Serial.println(srcFileName);
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
    char profiles[MAX_FILE_COUNT][14];
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
      char buffer[SETTING_ENTRY_MAX];
      if (strcasecmp(filename, Config.profile) == 0) {
        debug(F("Profile was default -> Setting default profile to current profile\n"));
        result = setDefaultProfile(Settings.file);
      }
    }

  }
  return result;
}

/***
 * Read settings from specified file
 */
 /*
int loadSettingsFile(const char *filename, char results[][SETTING_ENTRY_MAX], const byte max) 
{
  debug(F("Load Settings File %s\n"), filename);
  char character;
  char settingName[SETTING_ENTRY_MAX] = "";
  char settingValue[SETTING_ENTRY_MAX] = "";
  File myFile;
  int index = 0;
  int c = 0;
  int tries = 0;
  autoSleepMillis = 0;
  voiceOff();
  // Setup control glove buttons
  button_initialized = false;
  App.ptt_button = NULL;
  App.wake_button = NULL;
  // clear control buttons
  for (byte i = 0; i < 6; i++) {
    ControlButtons[i].reset();
    ControlButtons[i].setPin(0);
    // set to nothing...
    strcpy(Settings.glove.settings[i], "0");
  }
  // Try up to 3 times to read the file
  while (!myFile && tries < 2) {
    myFile = openFile(filename, FILE_READ);
    if (myFile) {
      // set this above the limit so it doesn't keep trying
      tries = 3;
      while (myFile.available() && index < max) {
        character = myFile.read();
        while ((myFile.available()) && (character != '[')) {
          character = myFile.read();
        }
        character = myFile.read();
        settingName[c] = character;
        c += 1 ;
        while ((myFile.available()) && (character != '='))  {
          character = myFile.read();
          if (character != '=') {
            settingName[c] = character;
            c += 1;
          }
        }
        character = myFile.read();
        c = 0;
        while ((myFile.available()) && (character != ']'))  {
          settingValue[c] = character;
          c++;
          character = myFile.read();
        }
        if  (character == ']')  {
          c = 0;
          if (index <= max && strcasecmp(settingName, "") != 0) {
            char buf[SETTING_ENTRY_MAX] = "";
            strcpy(buf, settingName);
            strcat(buf, "=");
            strcat(buf, settingValue);
            strcpy(results[index], buf);
            debug(F("%d of %d lines -> setting %s\n"), index, max, buf);
            index++;
          } else {
            break;
          }
        }
        memset(settingName, 0, sizeof(settingName));
        memset(settingValue, 0, sizeof(settingValue));
      }
      myFile.close();
    } else {
      tries++;
      debug(F("Could not read settings file, attempting try %s\n"), tries+1);
      delay(100);
      if (tries >=2) {
        // if the file didn't open, print an error:
        debug(F("**ERROR** opening settings file %s\n"), filename);
        index = 0;
        beep(3);
      }
    }
  }
  debug(F("File %s loaded"), filename);
  return index;
}
*/
void loadSettingsFile(const char* filename)
{
  char path[100];
  strcpy(path, PROFILES_DIR);
  strcat(path, filename);
  // if it's successful assign values to Settings object
  //total = loadSettingsFile(path, profile_settings, MAX_SETTINGS_COUNT);
  //debug(F("===========> %d lines read"), total);
}
/**
 * Process a list of settings values
 */
void processSettings(char settings[][SETTING_ENTRY_MAX], const int max)
{
  AudioNoInterrupts();
  for (int i = 0; i < max; i++) {
    char entry[SETTING_ENTRY_MAX];
    strcpy(entry, settings[i]);
    char *key, *value, *ptr;
    key = strtok_r(entry, "=", &ptr);
    value = strtok_r(NULL, "=", &ptr);
    parseSetting(key, value);
  }
  Serial.println("----- AFTER PROCESS SETTINGS ");
  AudioInterrupts();
  Serial.println("----- AFTER AUDIO INTERRUPT ");
}

/**
 * Load specified settings file
 */
int loadSettings(const char *filename) 
{
  
  //const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 10*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(12) + 730;
  //const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 10*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(12) + 740;
  const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 10*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9) + 740;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  //const char* json = "{\"name\":\"aaaaaaaaaaaaaaaaaaaa\",\"volume\":0.55,\"microphone\":10,\"linein\":10,\"lineout\":30,\"sounds\":{\"dir\":\"/aaaaaaaa/\",\"start\":\"aaaaaaaa.aaa\",\"button\":\"aaaaaaaa.aaa\"},\"loop\":{\"dir\":\"/aaaaaaaa/\",\"file\":\"aaaaaaaa.aaa\",\"gain\":0.02,\"mute\":1},\"voice\":{\"volume\":3,\"dry\":0,\"start\":0.043,\"stop\":0.02,\"wait\":1000},\"effects\":{\"dir\":\"/aaaaaaaa/\",\"volume\":1,\"high_pass\":1,\"bitcrusher\":{\"bits\":16,\"rate\":44100},\"chorus\":{\"voices\":10,\"delay\":1000},\"flanger\":{\"delay\":32,\"offset\":10,\"depth\":10,\"freq\":0.0625},\"noise\":0.014,\"mute\":1},\"eq\":{\"active\":1,\"bands\":[-0.2,-0.4,-0.35,-0.35,-0.9]},\"sleep\":{\"timer\":60000,\"sound\":\"aaaaaaaa.aaa\"},\"buttons\":[{\"type\":10,\"data\":\"aaaaaaaa.aaa\"},{\"type\":10,\"data\":\"aaaaaaaa.aaa\"},{\"type\":10,\"data\":\"aaaaaaaa.aaa\"},{\"type\":10,\"data\":\"aaaaaaaa.aaa\"},{\"type\":10,\"data\":\"aaaaaaaa.aaa\"},{\"type\":10,\"data\":\"aaaaaaaa.aaa\"}]}";

  char srcFileName[25];
  strcpy(srcFileName, PROFILES_DIR);
  strcat(srcFileName, filename);
  
  File file = SD.open(srcFileName);
  
  JsonObject& root = jsonBuffer.parseObject(file);

  file.close();

  strlcpy(Settings.name, root["name"], sizeof(Settings.name)); // "aaaaaaaaaaaaaaaaaaaa"

  JsonObject& volume = root["volume"];
  Settings.volume.master = volume["master"]; // 0.55
  Settings.volume.microphone = volume["microphone"]; // 10
  Settings.volume.linein = volume["linein"]; // 10
  Settings.volume.lineout = volume["lineout"]; // 30

  JsonObject& sounds = root["sounds"];
  strlcpy(Settings.sounds.dir, sounds["dir"], sizeof(Settings.sounds.dir));
  strlcpy(Settings.sounds.start, sounds["start"], sizeof(Settings.sounds.start));
  strlcpy(Settings.sounds.button, sounds["button"], sizeof(Settings.sounds.button)); // "aaaaaaaa.aaa"
  
  JsonObject& loop = root["loop"];
  strlcpy(Settings.loop.dir, loop["dir"], sizeof(Settings.loop.dir)); // "/aaaaaaaa/"
  const char* loop_file = loop["file"]; // "aaaaaaaa.aaa"
  float loop_gain = loop["gain"]; // 0.02
  int loop_mute = loop["mute"]; // 1
  
  JsonObject& voice = root["voice"];
  int voice_volume = voice["volume"]; // 3
  int voice_dry = voice["dry"]; // 0
  float voice_start = voice["start"]; // 0.043
  float voice_stop = voice["stop"]; // 0.02
  int voice_wait = voice["wait"]; // 1000
  
  JsonObject& effects = root["effects"];
  const char* effects_dir = effects["dir"]; // "/aaaaaaaa/"
  int effects_volume = effects["volume"]; // 1
  int effects_high_pass = effects["high_pass"]; // 1
  
  int effects_bitcrusher_bits = effects["bitcrusher"]["bits"]; // 16
  long effects_bitcrusher_rate = effects["bitcrusher"]["rate"]; // 44100
  
  int effects_chorus_voices = effects["chorus"]["voices"]; // 10
  int effects_chorus_delay = effects["chorus"]["delay"]; // 1000
  
  JsonObject& effects_flanger = effects["flanger"];
  Settings.effects.flanger.delay = effects_flanger["delay"]; // 32
  Settings.effects.flanger.offset = effects_flanger["offset"]; // 10
  Settings.effects.flanger.depth = effects_flanger["depth"]; // 10
  Settings.effects.flanger.freq = effects_flanger["freq"]; // 0.0625
  
  float effects_noise = effects["noise"]; // 0.014
  int effects_mute = effects["mute"]; // 1
  
  int eq_active = root["eq"]["active"]; // 1
  
  JsonArray& eq_bands = root["eq"]["bands"];
  float eq_bands0 = eq_bands[0]; // -0.2
  float eq_bands1 = eq_bands[1]; // -0.4
  float eq_bands2 = eq_bands[2]; // -0.35
  float eq_bands3 = eq_bands[3]; // -0.35
  
  long sleep_timer = root["sleep"]["timer"]; // 60000
  const char* sleep_sound = root["sleep"]["sound"]; // "aaaaaaaa.aaa"
  
  JsonArray& buttons = root["buttons"];
  
  int buttons0_type = buttons[0]["type"]; // 10
  const char* buttons0_data = buttons[0]["data"]; // "aaaaaaaa.aaa"
  
  int buttons1_type = buttons[1]["type"]; // 10
  const char* buttons1_data = buttons[1]["data"]; // "aaaaaaaa.aaa"
  
  int buttons2_type = buttons[2]["type"]; // 10
  const char* buttons2_data = buttons[2]["data"]; // "aaaaaaaa.aaa"
  
  int buttons3_type = buttons[3]["type"]; // 10
  const char* buttons3_data = buttons[3]["data"]; // "aaaaaaaa.aaa"
   
  
  /*
  const char* name = root["name"]; // "aaaaaaaaaaaaaaaaaaaa"
  JsonObject& volume = root["volume"];
float volume_master = volume["master"]; // 0.55
int volume_microphone = volume["microphone"]; // 10
int volume_linein = volume["linein"]; // 10
int volume_lineout = volume["lineout"]; // 30

JsonObject& sounds = root["sounds"];
const char* sounds_dir = sounds["dir"]; // "/aaaaaaaa/"
const char* sounds_start = sounds["start"]; // "aaaaaaaa.aaa"
const char* sounds_button = sounds["button"]; // "aaaaaaaa.aaa"

JsonObject& loop = root["loop"];
const char* loop_dir = loop["dir"]; // "/aaaaaaaa/"
const char* loop_file = loop["file"]; // "aaaaaaaa.aaa"
float loop_volume = loop["volume"]; // 0.02
int loop_mute = loop["mute"]; // 1

JsonObject& voice = root["voice"];
int voice_volume = voice["volume"]; // 3
int voice_dry = voice["dry"]; // 0
float voice_start = voice["start"]; // 0.043
float voice_stop = voice["stop"]; // 0.02
int voice_wait = voice["wait"]; // 1000

JsonObject& effects = root["effects"];
const char* effects_dir = effects["dir"]; // "/aaaaaaaa/"
int effects_volume = effects["volume"]; // 1
int effects_high_pass = effects["high_pass"]; // 1

int effects_bitcrusher_bits = effects["bitcrusher"]["bits"]; // 16
long effects_bitcrusher_rate = effects["bitcrusher"]["rate"]; // 44100

int effects_chorus_voices = effects["chorus"]["voices"]; // 10
int effects_chorus_delay = effects["chorus"]["delay"]; // 1000

JsonObject& effects_flanger = effects["flanger"];
int effects_flanger.delay = effects_flanger["delay"]; // 32
int effects_flanger.offset = effects_flanger["offset"]; // 10
int effects_flanger.depth = effects_flanger["depth"]; // 10
float effects_flanger.freq = effects_flanger["freq"]; // 0.0625

float effects_noise = effects["noise"]; // 0.014
int effects_mute = effects["mute"]; // 1

int eq_active = root["eq"]["active"]; // 1

JsonArray& eq_bands = root["eq"]["bands"];
float eq_bands0 = eq_bands[0]; // -0.2
float eq_bands1 = eq_bands[1]; // -0.4
float eq_bands2 = eq_bands[2]; // -0.35
float eq_bands3 = eq_bands[3]; // -0.35

long sleep_timer = root["sleep"]["timer"]; // 60000
const char* sleep_sound = root["sleep"]["sound"]; // "aaaaaaaa.aaa"

JsonArray& buttons = root["buttons"];

int buttons0_type = buttons[0]["type"]; // 10
const char* buttons0_data = buttons[0]["data"]; // "aaaaaaaa.aaa"

int buttons1_type = buttons[1]["type"]; // 10
const char* buttons1_data = buttons[1]["data"]; // "aaaaaaaa.aaa"

int buttons2_type = buttons[2]["type"]; // 10
const char* buttons2_data = buttons[2]["data"]; // "aaaaaaaa.aaa"

int buttons3_type = buttons[3]["type"]; // 10
const char* buttons3_data = buttons[3]["data"]; // "aaaaaaaa.aaa"
     */
}

void saveSettings() {
    const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 10*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9);
DynamicJsonBuffer jsonBuffer(bufferSize);

JsonObject& root = jsonBuffer.createObject();
root["name"] = "aaaaaaaaaaaaaaaaaaaa";

JsonObject& volume = root.createNestedObject("volume");
volume["master"] = 0.55;
volume["microphone"] = 10;
volume["linein"] = 10;
volume["lineout"] = 30;

JsonObject& sounds = root.createNestedObject("sounds");
sounds["dir"] = "/aaaaaaaa/";
sounds["start"] = "aaaaaaaa.aaa";
sounds["button"] = "aaaaaaaa.aaa";

JsonObject& loop = root.createNestedObject("loop");
loop["dir"] = "/aaaaaaaa/";
loop["file"] = "aaaaaaaa.aaa";
loop["volume"] = 0.02;
loop["mute"] = 1;

JsonObject& voice = root.createNestedObject("voice");
voice["volume"] = 3;
voice["dry"] = 0;
voice["start"] = 0.043;
voice["stop"] = 0.02;
voice["wait"] = 1000;

JsonObject& effects = root.createNestedObject("effects");
effects["dir"] = "/aaaaaaaa/";
effects["volume"] = 1;
effects["high_pass"] = 1;

JsonObject& effects_bitcrusher = effects.createNestedObject("bitcrusher");
effects_bitcrusher["bits"] = 16;
effects_bitcrusher["rate"] = 44100;

JsonObject& effects_chorus = effects.createNestedObject("chorus");
effects_chorus["voices"] = 10;
effects_chorus["delay"] = 1000;

JsonObject& effects_flanger = effects.createNestedObject("flanger");
effects_flanger["delay"] = 32;
effects_flanger["offset"] = 10;
effects_flanger["depth"] = 10;
effects_flanger["freq"] = 0.0625;
effects["noise"] = 0.014;
effects["mute"] = 1;

JsonObject& eq = root.createNestedObject("eq");
eq["active"] = 1;

JsonArray& eq_bands = eq.createNestedArray("bands");
eq_bands.add(-0.2);
eq_bands.add(-0.4);
eq_bands.add(-0.35);
eq_bands.add(-0.35);
eq_bands.add(-0.9);

JsonObject& sleep = root.createNestedObject("sleep");
sleep["timer"] = 60000;
sleep["sound"] = "aaaaaaaa.aaa";

JsonArray& buttons = root.createNestedArray("buttons");

JsonObject& buttons_0 = buttons.createNestedObject();
buttons_0["type"] = 10;
buttons_0["data"] = "aaaaaaaa.aaa";

JsonObject& buttons_1 = buttons.createNestedObject();
buttons_1["type"] = 10;
buttons_1["data"] = "aaaaaaaa.aaa";

JsonObject& buttons_2 = buttons.createNestedObject();
buttons_2["type"] = 10;
buttons_2["data"] = "aaaaaaaa.aaa";

JsonObject& buttons_3 = buttons.createNestedObject();
buttons_3["type"] = 10;
buttons_3["data"] = "aaaaaaaa.aaa";

JsonObject& buttons_4 = buttons.createNestedObject();
buttons_4["type"] = 10;
buttons_4["data"] = "aaaaaaaa.aaa";

JsonObject& buttons_5 = buttons.createNestedObject();
buttons_5["type"] = 10;
buttons_5["data"] = "aaaaaaaa.aaa";

root.printTo(Serial);
  }
  
/***
 * Apply settings
 */
 /*
void applySettings() 
{
*/
  /*
  
  // Turn on the 5-band graphic equalizer (there is also a 7-band parametric...see the Teensy docs)
  if (Settings.eq.active == 0) {
    audioShield.eqSelect(FLAT_FRSettings.eq.activeUENCY);
  } else {
    audioShield.eqSelect(GRAPHIC_Settings.eq.activeUALIZER);
    // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
    // Valid values are -1 (-11.75dB) to 1 (+12dB)
    // The settings below pull down the lows and highs and push up the mids for 
    // more of a "tin-can" sound.
    audioShield.eqBands(Settings.eq.bands[0], Settings.eq.bands[1], Settings.eq.bands[2], Settings.eq.bands[3], Settings.eq.bands[4]);
  }
  */
  /*
  // tell the audio shield which input to use
  audioShield.inputSelect(Config.input);
  // adjust the gain of the input
  // adjust this as needed
  if (Config.input == 0) {
    audioShield.lineInLevel(Settings.volume.linein);
  } else {
    audioShield.micGain(Settings.volume.microphone);
  }  
  */
  /*
  // You can modify these values to process the voice 
  // input.  See the Teensy bitcrusher demo for details.
  bitcrusher1.bits(Settings.bitcrusher[0]);
  bitcrusher1.sampleRate(Settings.bitcrusher[1]);
  //bitcrusher2.bits(Settings.bitcrusher[2]);
  //bitcrusher2.sampleRate(Settings.bitcrusher[3]);
  // Bitcrusher 1 input (fed by mic/line-in)
  */
  /*
  voiceMixer.gain(0, Settings.voice.volume);
  voiceMixer.gain(1, Settings.voice.volume);
  // Dry (unprocessed) voice input
  voiceMixer.gain(2, Settings.voice.dry);
  */
  /*
  // Pink noise channel
  //voiceMixer.gain(1, Settings.effects.noise);
  // Feed from effects mixer
  voiceMixer.gain(3, 1);
  */
  /*
  // stereo channels for SD card...adjust gain as 
  // necessary to match voice level
  effectsMixer.gain(0, Settings.effects.volume);
  effectsMixer.gain(1, Settings.effects.volume);
  // BLE connect sound
  effectsMixer.gain(2, Settings.effects.volume);
  // Feed from loop mixer
  effectsMixer.gain(3, 1);
  */
  /*
  // chatter loop from SD card
  loopMixer.gain(0, Settings.loop.volume);
  loopMixer.gain(1, Settings.loop.volume);
  */
  //audioShield.volume(readVolume());
  //audioShield.lineOutLevel(Settings.volume.lineout);
  /*
  if (Settings.effects.hipass == 0) {
    audioShield.adcHighPassFilterDisable();
  } else {
    audioShield.adcHighPassFilterEnable();
  }
  */
  // Initialize PTT button
  /*
  if (BUTTON_PIN && BUTTON_PIN > 0) {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    PTT.attach(BUTTON_PIN);
    PTT.interval(15);
    snoozeDigital.pinMode(BUTTON_PIN, INPUT_PULLUP, FALLING);
  }
  */
  /*
  autoSleepMillis = 0;
  voiceOff();
  // Setup control glove buttons
  button_initialized = false;
  App.ptt_button = NULL;
  App.wake_button = NULL;
*/
  /*
  for (byte a = 0; a < 6; a++) {
    // need to clear button settings...
    ConfigureButton(a);
  }
  */

//}

  

