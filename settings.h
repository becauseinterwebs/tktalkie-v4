/****
 * Routines to handle reading/handling settings
 */

void ConfigureButton(byte a) {
  
    ControlButtons[a].setPTT(false);
    ControlButtons[a].setPin(0);
    byte buttonNum = 0;
    char buf[25];
    byte pin = Config.buttons[a];
    
    Serial.print("** Configure Button for Pin: ");
    Serial.println(pin);
    if (pin > 0) {

      // setup physical button
      ControlButtons[a].setup(pin);
      
      strcpy(buf, Settings.control_button_settings[a]);
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
              ControlButtons[a].setPTT(true);
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
        
        ControlButtons[a].buttons[buttonNum].setup(button_type);
        
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
                ControlButtons[a].buttons[buttonNum].setSound(button_token);
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
    memset(Settings.profile_name, 0, sizeof(Settings.profile_name));
    strcpy(Settings.profile_name, settingValue);
  } else if (strcasecmp(settingName, "volume") == 0) {
    Settings.volume = atof(settingValue);  
    if (Settings.volume > 1) { 
      Settings.volume = 1;
    } else if (Settings.volume < 0) {
      Settings.volume = 0;
    }
    audioShield.volume(readVolume());
  } else if (strcasecmp(settingName, "lineout") == 0) {
    Settings.lineout = (byte)atoi(settingValue);
    if (Settings.lineout < 13) {
      Settings.lineout = 13;  
    } else if (Settings.lineout > 31) {
      Settings.lineout = 31;
    }
    audioShield.lineOutLevel(Settings.lineout);
  } else if (strcasecmp(settingName, "linein") == 0) {
    Settings.linein = (byte)atoi(settingValue);
    if (Settings.linein < 0) {
      Settings.linein = 0;  
    } else if (Settings.linein > 15) {
      Settings.linein = 15;
    }  
    audioShield.lineInLevel(Settings.linein);
  } else if ((strcasecmp(settingName, "high_pass") == 0) || (strcasecmp(settingName, "highpass") == 0)) {
    Settings.hipass = (byte)atoi(settingValue);
    if (Settings.hipass < 0) { 
      Settings.hipass = 0;
    } else if (Settings.hipass > 1) {
      Settings.hipass = 1;
    }
    if (Settings.hipass == 0) {
      audioShield.adcHighPassFilterDisable();
    } else {
      audioShield.adcHighPassFilterEnable();
    }
  } else if (strcasecmp(settingName, "mic_gain") == 0) {
    Settings.mic_gain = atoi(settingValue);
    audioShield.micGain(Settings.mic_gain);  
  } else if (strcasecmp(settingName, "button_click") == 0) {
    memset(Settings.button_wav, 0, sizeof(Settings.button_wav));
    strcpy(Settings.button_wav, settingValue);
  } else if (strcasecmp(settingName, "startup") == 0) {
    memset(Settings.startup_wav, 0, sizeof(Settings.startup_wav));
    strcpy(Settings.startup_wav, settingValue);
  } else if (strcasecmp(settingName, "startup_sound") == 0) {
    memset(Settings.startup_wav, 0, sizeof(Settings.startup_wav));
    strcpy(Settings.startup_wav, settingValue);  
  } else if (strcasecmp(settingName, "loop") == 0) {
    memset(Settings.loop_wav, 0, sizeof(Settings.loop_wav));
    strcpy(Settings.loop_wav, settingValue);
  } else if (strcasecmp(settingName, "noise_gain") == 0) {
    Settings.noise_gain = atof(settingValue);
    effectsMixer.gain(3, Settings.noise_gain);
  } else if (strcasecmp(settingName, "voice_gain") == 0) {
    Settings.voice_gain = atof(settingValue);
    voiceMixer.gain(0, Settings.voice_gain);
    voiceMixer.gain(1, Settings.voice_gain);
  } else if (strcasecmp(settingName, "dry_gain") == 0) {
    Settings.dry_gain = atof(settingValue);  
    voiceMixer.gain(2, Settings.dry_gain);
  } else if (strcasecmp(settingName, "effects_gain") == 0) {
    Settings.effects_gain = atof(settingValue);
    effectsMixer.gain(0, Settings.effects_gain);
    //effectsMixer.gain(1, Settings.effects_gain);
    // Waveform (BLE) connect sound
    effectsMixer.gain(2, Settings.effects_gain);
  } else if (strcasecmp(settingName, "loop_gain") == 0) {
    Settings.loop_gain = atof(settingValue);
    if (Settings.loop_gain < 0 or Settings.loop_gain > 32767) {
      Settings.loop_gain = 4;
    }
    // chatter loop from SD card
    effectsMixer.gain(1, Settings.loop_gain);
    //loopMixer.gain(0, Settings.loop_gain);
    //loopMixer.gain(1, Settings.loop_gain);
  } else if (strcasecmp(settingName, "silence_time") == 0) {
    Settings.silence_time = atoi(settingValue);
  } else if (strcasecmp(settingName, "voice_start") == 0) {
    Settings.voice_start = atof(settingValue);
  } else if (strcasecmp(settingName, "voice_stop") == 0) {  
    Settings.voice_stop = atof(settingValue);
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
    Settings.eq = (byte)atoi(settingValue);
    if (Settings.eq < 0) {
      Settings.eq = 0;
    } else if (Settings.eq > 1) {
      Settings.eq = 1;
    }
    // Turn on the 5-band graphic equalizer (there is also a 7-band parametric...see the Teensy docs)
    if (Settings.eq == 0) {
      audioShield.eqSelect(FLAT_FREQUENCY);
    } else {
      audioShield.eqSelect(GRAPHIC_EQUALIZER);
    }  
  } else if (strcasecmp(settingName, "eq_bands") == 0) {
    // clear bands and prep for setting
    for (int i = 0; i < 6; i++) {
      Settings.eq_bands[i] = 0;
    }
    char *band, *ptr;
    band = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (band && i < 6) {
      Settings.eq_bands[i] = atof(band);
      i++;
      band = strtok_r(NULL, ",", &ptr);
    }
    // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
    // Valid values are -1 (-11.75dB) to 1 (+12dB)
    // The settings below pull down the lows and highs and push up the mids for 
    // more of a "tin-can" sound.
    audioShield.eqBands(Settings.eq_bands[0], Settings.eq_bands[1], Settings.eq_bands[2], Settings.eq_bands[3], Settings.eq_bands[4]);
  } else if (strcasecmp(settingName, "bitcrushers") == 0 || strcasecmp(settingName, "bitcrusher") == 0) {
    char *token, *ptr;
    token = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (token && i < (3)) {
      Settings.bitcrusher[i] = atoi(token);
      i++;
      token = strtok_r(NULL, ",", &ptr);
    }
    // You can modify these values to process the voice 
    // input.  See the Teensy bitcrusher demo for details.
    bitcrusher1.bits(Settings.bitcrusher[0]);
    bitcrusher1.sampleRate(Settings.bitcrusher[1]);
  } else if (strcasecmp(settingName, "effects_dir") == 0) {
    memset(Settings.effects_dir, 0, sizeof(Settings.effects_dir));
    strcpy(Settings.effects_dir, settingValue);
    fixPath(Settings.effects_dir);
    loadSoundEffects();
  } else if (strcasecmp(settingName, "sounds_dir") == 0) {
    memset(Settings.sounds_dir, 0, sizeof(Settings.sounds_dir));
    strcpy(Settings.sounds_dir, settingValue);
    fixPath(Settings.sounds_dir);
  } else if (strcasecmp(settingName, "loop_dir") == 0) {
    memset(Settings.loop_dir, 0, sizeof(Settings.loop_dir));
    strcpy(Settings.loop_dir, settingValue);
    fixPath(Settings.loop_dir);
  } else if (strcasecmp(settingName, "mute_loop") == 0) {
      Settings.mute_loop = (byte)atoi(settingValue);
      if (Settings.mute_loop > 1) {
        Settings.mute_loop = 1;
      } else if (Settings.mute_loop < 0) {
        Settings.mute_loop = 0;
      }
  } else if (strcasecmp(settingName, "mute_effects") == 0) {
      Settings.mute_effects = (byte)atoi(settingValue);
      if (Settings.mute_effects > 1) {
        Settings.mute_effects = 1;
      } else if (Settings.mute_effects < 0) {
        Settings.mute_effects = 0;
      }
  } else if (strcasecmp(settingName, "sleep_time") == 0) {
      Settings.sleep_time = (byte)atoi(settingValue);
      if (Settings.sleep_time < 0) {
        Settings.sleep_time = 0;
      }
  } else if (strcasecmp(settingName, "sleep_sound") == 0) {
      memset(Settings.sleep_sound, 0, sizeof(Settings.sleep_sound));
      strcpy(Settings.sleep_sound, settingValue);
  } else if (strcasecmp(settingName, "chorus") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        chorus1.voices(0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        chorus1.voices(Settings.chorus_voices);
      } else {
        char *token, *ptr;
        token = strtok_r(settingValue, ",", &ptr);
        byte i = 0;
        while (token && i < 3) {
          switch (i) {
            case 0:
              Settings.chorus_delay = (byte)atoi(token);
              break;
            case 1:
              Settings.chorus_voices = (byte)atoi(token);
              break;
          }
          i++;
          token = strtok_r(NULL, ",", &ptr);
        }  
        if (Settings.chorus_delay > 32) {
          Settings.chorus_delay = 32;
        }
        if (Settings.chorus_delay < 1) {
          Settings.chorus_delay = 1;
        }
        if (Settings.chorus_voices < 0) {
          Settings.chorus_voices = 0;
        }
        if(!chorus1.begin(Settings.chorus_buffer,Settings.chorus_delay*AUDIO_BLOCK_SAMPLES,Settings.chorus_voices)) {
           Serial.println("chorus: startup failed");
        }
      }
  } else if (strcasecmp(settingName, "chorus_delay") == 0) {
      Settings.chorus_delay = (byte)atoi(settingValue);
      if (Settings.chorus_delay > 32) {
        Settings.chorus_delay = 32;
      }
      if (Settings.chorus_delay < 1) {
        Settings.chorus_delay = 1;
      }
      if(!chorus1.begin(Settings.chorus_buffer,Settings.chorus_delay * AUDIO_BLOCK_SAMPLES,Settings.chorus_voices)) {
         Serial.println("chorus_delay: Startup failed");
      }
  } else if (strcasecmp(settingName, "chorus_voices") == 0) {
      Settings.chorus_voices = (byte)atoi(settingValue);
      if (Settings.chorus_voices < 0) {
        Settings.chorus_voices = 0;
      }
      chorus1.voices(Settings.chorus_voices);
  } else if (strcasecmp(settingName, "flange_delay") == 0) {
      Settings.flange_delay = (byte)atoi(settingValue);
      if (Settings.flange_delay > 32) {
        Settings.flange_delay = 32;
      }
      if (Settings.flange_delay < 0) {
        Settings.flange_delay = 0;
      }
      flange1.begin(Settings.flange_buffer,Settings.flange_delay*AUDIO_BLOCK_SAMPLES,Settings.flange_offset,Settings.flange_depth,Settings.flange_freq);    
  } else if (strcasecmp(settingName, "flange_freq") == 0) {
      Settings.flange_freq = atof(settingValue);
      if (Settings.flange_freq < 0) {
        Settings.flange_freq = 0;
      }
      if (Settings.flange_freq > 10) {
        Settings.flange_freq = 10;
      }
      flange1.voices(Settings.flange_offset,Settings.flange_depth,Settings.flange_freq);
  } else if (strcasecmp(settingName, "flange_depth") == 0) {
      Settings.flange_depth = (byte)atoi(settingValue);
      if (Settings.flange_depth < 0) {
        Settings.flange_depth = 0;
      }
      if (Settings.flange_depth > 255) {
        Settings.flange_depth = 255;
      }
      flange1.voices(Settings.flange_offset,Settings.flange_depth,Settings.flange_freq);
  } else if (strcasecmp(settingName, "flange_idx") == 0) {
      Settings.flange_offset = (byte)atoi(settingValue);// * Settings.flange_delay_LENGTH;
      if (Settings.flange_offset < 1) {
        Settings.flange_offset = 1;
      }
      if (Settings.flange_offset > 128) {
        Settings.flange_offset = 128;
      }
      flange1.voices(Settings.flange_offset,Settings.flange_depth,Settings.flange_freq);
  } else if (strcasecmp(settingName, "flange") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        flange1.voices(Settings.FLANGE_DELAY_PASSTHRU,0,0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        flange1.voices(Settings.flange_offset,Settings.flange_depth,Settings.flange_freq);
      }
  } else if (strcasecmp(settingName, "button") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte b = (byte)atoi(token);
      if (b >= 0 && b <= 5) {
        strcpy(Settings.control_button_settings[b], ptr);
        ConfigureButton(b);
      }
    
  } else if (strcasecmp(settingName, "buttons") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, "|", &ptr);
      byte a = 0;
      while (token && a < 6) {
        strcpy(Settings.control_button_settings[a], token);
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

  const char str_template[14] = "\"%s\":\"%s\"";
  const char num_template[14] = "\"%s\":%s";
  char buf[20];
  char tmp[100];

  sprintf(tmp, str_template, "name", Settings.profile_name);
  strcpy(result, tmp);
  strcat(result, ",");

  dtostrf(Settings.volume, 0, 4, buf);
  sprintf(tmp, num_template, "volume", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.mic_gain);
  sprintf(tmp, num_template, "mic_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Settings.linein);
  sprintf(tmp, num_template, "linein", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.lineout);
  sprintf(tmp, num_template, "lineout", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "startup", Settings.startup_wav);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "loop", Settings.loop_wav);
  strcat(result, tmp);
  strcat(result, ",");

  dtostrf(Settings.loop_gain, 0, 4, buf);
  sprintf(tmp, num_template, "loop_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Settings.hipass);
  sprintf(tmp, num_template, "high_pass", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice_gain, 0, 4, buf);
  sprintf(tmp, num_template, "voice_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  dtostrf(Settings.dry_gain, 0, 4, buf);
  sprintf(tmp, num_template, "dry_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.dry_gain, 0, 4, buf);
  sprintf(tmp, num_template, "dry_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice_start, 0, 4, buf);
  sprintf(tmp, num_template, "voice_start", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.voice_stop, 0, 4, buf);
  sprintf(tmp, num_template, "voice_stop", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Settings.silence_time);
  sprintf(tmp, num_template, "silence_time", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  /*
  sprintf(buf, "%d", BUTTON_PIN);
  sprintf(tmp, num_template, "button_pin", buf);
  strcat(result, tmp);
  strcat(result, ",");
  */
  sprintf(tmp, str_template, "button_click", Settings.button_wav);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", Config.input);
  sprintf(tmp, num_template, "input", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(Settings.effects_gain, 0, 4, buf);
  sprintf(tmp, num_template, "effects_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.eq);
  sprintf(tmp, num_template, "eq", buf);
  strcat(result, tmp);
  strcat(result, ",");

  char buffer[SETTING_ENTRY_MAX];
  char *bands = arrayToString(buffer, Settings.eq_bands, 6);
  sprintf(tmp, str_template, "eq_bands", bands);
  strcat(result, tmp);
  strcat(result, ",");
  memset(buffer, 0, sizeof(buffer));
  
  char *bitcrushers = arrayToString(buffer, Settings.bitcrusher, 3);
  sprintf(tmp, str_template, "bitcrushers", bitcrushers);
  strcat(result, tmp);
  strcat(result, ",");
  memset(buffer, 0, sizeof(buffer));
  
  dtostrf(Settings.noise_gain, 0, 4, buf);
  sprintf(tmp, num_template, "noise_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "effects_dir", Settings.effects_dir);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "sounds_dir", Settings.sounds_dir);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "loop_dir", Settings.loop_dir);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", Settings.mute_loop);
  sprintf(tmp, num_template, "mute_loop", buf);
  strcat(result, tmp);

  if (APP_VER > 1.13) {
    sprintf(buf, "%d", Settings.mute_effects);
    sprintf(tmp, num_template, "mute_effects", buf);
    strcat(result, tmp);
  }

  if (APP_VER > 1.14) {
    strcat(result, ",");
    sprintf(buf, "%d", Settings.sleep_time);
    sprintf(tmp, num_template, "sleep_time", buf);
    strcat(result, tmp);
    strcat(result, ",");
    
    sprintf(tmp, str_template, "sleep_sound", Settings.sleep_sound);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.chorus_delay);
    sprintf(tmp, num_template, "chorus_delay", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.chorus_voices);
    sprintf(tmp, num_template, "chorus_voices", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.flange_delay);
    sprintf(tmp, num_template, "flange_delay", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", Settings.flange_offset);
    sprintf(tmp, num_template, "flange_offset", buf);
    strcat(result, tmp);
    strcat(result, ",");
    
    sprintf(buf, "%d", Settings.flange_depth);
    sprintf(tmp, num_template, "flange_depth", buf);
    strcat(result, tmp);
    strcat(result, ",");

    dtostrf(Settings.flange_freq, 0, 4, buf);
    sprintf(tmp, num_template, "flange_freq", buf);
    strcat(result, tmp);
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
      strcat(result, Settings.control_button_settings[i]);
      strcat(result, "\"");
      if (i < 5) {
        strcat(result, ","); 
      }
    }
    strcat(result, "]}");
    */
    //sprintf(tmp, str_template, "buttons", buttons);
    //strcat(result, tmp);
    
  }
  
  return result;
  
}

/***
 * Converts all in-memory settings to string
 */
char *settingsToString(char result[]) 
{
  char buf[SETTING_ENTRY_MAX];

  strcpy(result, "[name=");
  strcat(result, Settings.profile_name);
  strcat(result, "]\n");

  if (Settings.volume > 0) {
    strcat(result, "[volume=");
    dtostrf(Settings.volume, 0, 4, buf);
    strcat(result, buf);
    strcat(result, "]\n");
    memset(buf, 0, sizeof(buf));
  }

  strcat(result, "[linein=");
  sprintf(buf, "%d", Settings.linein);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[lineout=");
  sprintf(buf, "%d", Settings.lineout);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[startup=");
  strcat(result, Settings.startup_wav);
  strcat(result, "]\n");

  strcat(result, "[loop=");
  strcat(result, Settings.loop_wav);
  strcat(result, "]\n");

  strcat(result, "[loop_gain=");
  dtostrf(Settings.loop_gain, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));  

  strcat(result, "[high_pass=");
  sprintf(buf, "%d", Settings.hipass);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[voice_gain=");
  dtostrf(Settings.voice_gain, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[dry_gain=");
  dtostrf(Settings.dry_gain, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
  
  strcat(result, "[voice_start=");
  dtostrf(Settings.voice_start, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[voice_stop=");
  dtostrf(Settings.voice_stop, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");

  memset(buf, 0, sizeof(buf));
  strcat(result, "[silence_time=");
  sprintf(buf, "%d", Settings.silence_time);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

/*
  strcat(result, "[button_pin=");
  sprintf(buf, "%d", BUTTON_PIN);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
*/
  strcat(result, "[button_click=");
  strcat(result, Settings.button_wav);
  strcat(result, "]\n");

  strcat(result, "[input=");
  sprintf(buf, "%d", Config.input);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[mic_gain=");
  sprintf(buf, "%d", Settings.mic_gain);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[effects_gain=");
  dtostrf(Settings.effects_gain, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[eq=");
  sprintf(buf, "%d", Settings.eq);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[eq_bands=");
  char *bands = arrayToString(buf, Settings.eq_bands, 6);
  strcat(result, bands);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[bitcrushers=");
  char *bitcrushers = arrayToString(buf, Settings.bitcrusher, 3); 
  strcat(result, bitcrushers);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[noise_gain=");
  dtostrf(Settings.noise_gain, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[effects_dir=");
  strcat(result, Settings.effects_dir);
  strcat(result, "]\n");

  strcat(result, "[sounds_dir=");
  strcat(result, Settings.sounds_dir);
  strcat(result, "]\n");

  strcat(result, "[loop_dir=");
  strcat(result, Settings.loop_dir);
  strcat(result, "]\n");

  strcat(result, "[mute_loop=");
  sprintf(buf, "%d", Settings.mute_loop);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[mute_effects=");
  sprintf(buf, "%d", Settings.mute_effects);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[sleep_time=");
  sprintf(buf, "%d", Settings.sleep_time);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[sleep_sound=");
  strcat(result, Settings.sleep_sound);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[chorus_delay=");
  sprintf(buf, "%d", Settings.chorus_delay);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[chorus_voices=");
  sprintf(buf, "%d", Settings.chorus_voices);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_delay=");
  sprintf(buf, "%d", Settings.flange_delay);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_offset=");
  sprintf(buf, "%d", Settings.flange_offset);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_depth=");
  sprintf(buf, "%d", Settings.flange_depth);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_freq=");
  dtostrf(Settings.flange_freq, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[buttons=");
  for (byte i=0; i<6; i++) {
    Serial.println(Settings.control_button_settings[i]);
    strcpy(buf, Settings.control_button_settings[i]);
    strcat(result, buf);
    if (i<5) {
      strcat(result, "|");
    }
  }
  strcat(result, "]\n");
  return result;
  
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
    strcpy(filename, Settings.profile_file);
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
  if (strcasecmp(filename, Settings.profile_file) == 0){
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
        result = setDefaultProfile(Settings.profile_file);
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
    strcpy(Settings.control_button_settings[i], "0");
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
  /*
  char settings[MAX_FILE_COUNT][14];
  int total = loadSettingsFile(filename, settings, MAX_SETTINGS_COUNT);
  processSettings(settings, total);
  return total;
  */
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
  if (Settings.eq == 0) {
    audioShield.eqSelect(FLAT_FRSettings.eqUENCY);
  } else {
    audioShield.eqSelect(GRAPHIC_Settings.eqUALIZER);
    // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
    // Valid values are -1 (-11.75dB) to 1 (+12dB)
    // The settings below pull down the lows and highs and push up the mids for 
    // more of a "tin-can" sound.
    audioShield.eqBands(Settings.eq_bands[0], Settings.eq_bands[1], Settings.eq_bands[2], Settings.eq_bands[3], Settings.eq_bands[4]);
  }
  */
  /*
  // tell the audio shield which input to use
  audioShield.inputSelect(Config.input);
  // adjust the gain of the input
  // adjust this as needed
  if (Config.input == 0) {
    audioShield.lineInLevel(Settings.linein);
  } else {
    audioShield.micGain(Settings.mic_gain);
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
  voiceMixer.gain(0, Settings.voice_gain);
  voiceMixer.gain(1, Settings.voice_gain);
  // Dry (unprocessed) voice input
  voiceMixer.gain(2, Settings.dry_gain);
  */
  /*
  // Pink noise channel
  //voiceMixer.gain(1, Settings.noise_gain);
  // Feed from effects mixer
  voiceMixer.gain(3, 1);
  */
  /*
  // stereo channels for SD card...adjust gain as 
  // necessary to match voice level
  effectsMixer.gain(0, Settings.effects_gain);
  effectsMixer.gain(1, Settings.effects_gain);
  // BLE connect sound
  effectsMixer.gain(2, Settings.effects_gain);
  // Feed from loop mixer
  effectsMixer.gain(3, 1);
  */
  /*
  // chatter loop from SD card
  loopMixer.gain(0, Settings.loop_gain);
  loopMixer.gain(1, Settings.loop_gain);
  */
  //audioShield.volume(readVolume());
  //audioShield.lineOutLevel(Settings.lineout);
  /*
  if (Settings.hipass == 0) {
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

  
