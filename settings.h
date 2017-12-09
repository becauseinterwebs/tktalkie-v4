/****
 * Routines to handle reading/handling settings
 */

/***
 * Parse and set a configuration setting
 */
void parseSetting(const char *settingName, char *settingValue) 
{

  debug(F("Parse Setting: %s = %s\n"), settingName, settingValue);
  
  if (strcasecmp(settingName, "name") == 0) {
    memset(PROFILE_NAME, 0, sizeof(PROFILE_NAME));
    strcpy(PROFILE_NAME, settingValue);
  } else if (strcasecmp(settingName, "volume") == 0) {
    MASTER_VOLUME = atof(settingValue);  
    if (MASTER_VOLUME > 1) { 
      MASTER_VOLUME = 1;
    } else if (MASTER_VOLUME < 0) {
      MASTER_VOLUME = 0;
    }
  } else if (strcasecmp(settingName, "lineout") == 0) {
    LINEOUT = (byte)atoi(settingValue);
    if (LINEOUT < 13) {
      LINEOUT = 13;  
    } else if (LINEOUT > 31) {
      LINEOUT = 31;
    }
  } else if (strcasecmp(settingName, "linein") == 0) {
    LINEIN = (byte)atoi(settingValue);
    if (LINEIN < 0) {
      LINEIN = 0;  
    } else if (LINEIN > 15) {
      LINEIN = 15;
    }  
  } else if ((strcasecmp(settingName, "high_pass") == 0) || (strcasecmp(settingName, "highpass") == 0)) {
    HIPASS = (byte)atoi(settingValue);
    if (HIPASS < 0) { 
      HIPASS = 0;
    } else if (HIPASS > 1) {
      HIPASS = 1;
    }
  } else if (strcasecmp(settingName, "mic_gain") == 0) {
    MIC_GAIN = atoi(settingValue);  
  } else if (strcasecmp(settingName, "button_click") == 0) {
    memset(BUTTON_WAV, 0, sizeof(BUTTON_WAV));
    strcpy(BUTTON_WAV, settingValue);
  } else if (strcasecmp(settingName, "startup") == 0) {
    memset(STARTUP_WAV, 0, sizeof(STARTUP_WAV));
    strcpy(STARTUP_WAV, settingValue);
  } else if (strcasecmp(settingName, "startup_sound") == 0) {
    memset(STARTUP_WAV, 0, sizeof(STARTUP_WAV));
    strcpy(STARTUP_WAV, settingValue);  
  } else if (strcasecmp(settingName, "loop") == 0) {
    memset(LOOP_WAV, 0, sizeof(LOOP_WAV));
    strcpy(LOOP_WAV, settingValue);
  } else if (strcasecmp(settingName, "noise_gain") == 0) {
    NOISE_GAIN = atof(settingValue);
  } else if (strcasecmp(settingName, "voice_gain") == 0) {
    VOICE_GAIN = atof(settingValue);
  } else if (strcasecmp(settingName, "dry_gain") == 0) {
    DRY_GAIN = atof(settingValue);  
  } else if (strcasecmp(settingName, "effects_gain") == 0) {
    EFFECTS_GAIN = atof(settingValue);
  } else if (strcasecmp(settingName, "loop_gain") == 0) {
    LOOP_GAIN = atof(settingValue);
    if (LOOP_GAIN < 0 or LOOP_GAIN > 32767) {
      LOOP_GAIN = 4;
    }
  } else if (strcasecmp(settingName, "silence_time") == 0) {
    SILENCE_TIME = atoi(settingValue);
  } else if (strcasecmp(settingName, "voice_start") == 0) {
    VOICE_START = atof(settingValue);
  } else if (strcasecmp(settingName, "voice_stop") == 0) {  
    VOICE_STOP = atof(settingValue);
  } else if (strcasecmp(settingName, "input") == 0) {
      AUDIO_INPUT = (byte)atoi(settingValue);
      if (AUDIO_INPUT > 1) {
        AUDIO_INPUT = 1;
      } else if (AUDIO_INPUT < 0) {
        AUDIO_INPUT = 0;
      }
  } else if (strcasecmp(settingName, "eq") == 0) {
    EQ = (byte)atoi(settingValue);
    if (EQ < 0) {
      EQ = 0;
    } else if (EQ > 1) {
      EQ = 1;
    }
  } else if (strcasecmp(settingName, "eq_bands") == 0) {
    // clear bands and prep for setting
    for (int i = 0; i < EQ_BANDS_SIZE; i++) {
      EQ_BANDS[i] = 0;
    }
    char *band, *ptr;
    band = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (band && i < EQ_BANDS_SIZE) {
      EQ_BANDS[i] = atof(band);
      i++;
      band = strtok_r(NULL, ",", &ptr);
    }
  } else if (strcasecmp(settingName, "bitcrushers") == 0) {
    char *token, *ptr;
    token = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (token && i < (BITCRUSHER_SIZE+1)) {
      BITCRUSHER[i] = atoi(token);
      i++;
      token = strtok_r(NULL, ",", &ptr);
    }
  } else if (strcasecmp(settingName, "effects_dir") == 0) {
    memset(EFFECTS_DIR, 0, sizeof(EFFECTS_DIR));
    strcpy(EFFECTS_DIR, settingValue);
    fixPath(EFFECTS_DIR);
    loadSoundEffects();
  } else if (strcasecmp(settingName, "sounds_dir") == 0) {
    memset(SOUNDS_DIR, 0, sizeof(SOUNDS_DIR));
    strcpy(SOUNDS_DIR, settingValue);
    fixPath(SOUNDS_DIR);
  } else if (strcasecmp(settingName, "loop_dir") == 0) {
    memset(LOOP_DIR, 0, sizeof(LOOP_DIR));
    strcpy(LOOP_DIR, settingValue);
    fixPath(LOOP_DIR);
  } else if (strcasecmp(settingName, "mute_loop") == 0) {
      MUTE_LOOP = (byte)atoi(settingValue);
      if (MUTE_LOOP > 1) {
        MUTE_LOOP = 1;
      } else if (MUTE_LOOP < 0) {
        MUTE_LOOP = 0;
      }
  } else if (strcasecmp(settingName, "mute_effects") == 0) {
      MUTE_EFFECTS = (byte)atoi(settingValue);
      if (MUTE_EFFECTS > 1) {
        MUTE_EFFECTS = 1;
      } else if (MUTE_EFFECTS < 0) {
        MUTE_EFFECTS = 0;
      }
  } else if (strcasecmp(settingName, "sleep_time") == 0) {
      SLEEP_TIME = (byte)atoi(settingValue);
      if (SLEEP_TIME < 0) {
        SLEEP_TIME = 0;
      }
  } else if (strcasecmp(settingName, "sleep_sound") == 0) {
      memset(SLEEP_SOUND, 0, sizeof(SLEEP_SOUND));
      strcpy(SLEEP_SOUND, settingValue);
  } else if (strcasecmp(settingName, "chorus") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        chorus1.voices(0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        chorus1.voices(CHORUS_VOICES);
      } else {
        char *token, *ptr;
        token = strtok_r(settingValue, ",", &ptr);
        byte i = 0;
        while (token && i < 3) {
          switch (i) {
            case 0:
              CHORUS_DELAY = (byte)atoi(token);
              break;
            case 1:
              CHORUS_VOICES = (byte)atoi(token);
              break;
          }
          i++;
          token = strtok_r(NULL, ",", &ptr);
        }  
        if (CHORUS_DELAY > 32) {
          CHORUS_DELAY = 32;
        }
        if (CHORUS_DELAY < 1) {
          CHORUS_DELAY = 1;
        }
        if (CHORUS_VOICES < 0) {
          CHORUS_VOICES = 0;
        }
        if(!chorus1.begin(CHORUS_BUFFER,CHORUS_DELAY*AUDIO_BLOCK_SAMPLES,CHORUS_VOICES)) {
           Serial.println("chorus: startup failed");
        }
      }
  } else if (strcasecmp(settingName, "chorus_delay") == 0) {
      CHORUS_DELAY = (byte)atoi(settingValue);
      if (CHORUS_DELAY > 32) {
        CHORUS_DELAY = 32;
      }
      if (CHORUS_DELAY < 1) {
        CHORUS_DELAY = 1;
      }
      if(!chorus1.begin(CHORUS_BUFFER,CHORUS_DELAY * AUDIO_BLOCK_SAMPLES,CHORUS_VOICES)) {
         Serial.println("chorus_delay: Startup failed");
      }
  } else if (strcasecmp(settingName, "chorus_voices") == 0) {
      CHORUS_VOICES = (byte)atoi(settingValue);
      if (CHORUS_VOICES < 0) {
        CHORUS_VOICES = 0;
      }
      chorus1.voices(CHORUS_VOICES);
  } else if (strcasecmp(settingName, "flange_delay") == 0) {
      FLANGE_DELAY = (byte)atoi(settingValue);
      if (FLANGE_DELAY > 32) {
        FLANGE_DELAY = 32;
      }
      if (FLANGE_DELAY < 0) {
        FLANGE_DELAY = 0;
      }
      flange1.begin(FLANGE_BUFFER,FLANGE_DELAY*AUDIO_BLOCK_SAMPLES,FLANGE_OFFSET,FLANGE_DEPTH,FLANGE_FREQ);    
  } else if (strcasecmp(settingName, "flange_freq") == 0) {
      FLANGE_FREQ = atof(settingValue);
      if (FLANGE_FREQ < 0) {
        FLANGE_FREQ = 0;
      }
      if (FLANGE_FREQ > 10) {
        FLANGE_FREQ = 10;
      }
      flange1.voices(FLANGE_OFFSET,FLANGE_DEPTH,FLANGE_FREQ);
  } else if (strcasecmp(settingName, "flange_depth") == 0) {
      FLANGE_DEPTH = (byte)atoi(settingValue);
      if (FLANGE_DEPTH < 0) {
        FLANGE_DEPTH = 0;
      }
      if (FLANGE_DEPTH > 255) {
        FLANGE_DEPTH = 255;
      }
      flange1.voices(FLANGE_OFFSET,FLANGE_DEPTH,FLANGE_FREQ);
  } else if (strcasecmp(settingName, "flange_idx") == 0) {
      FLANGE_OFFSET = (byte)atoi(settingValue);// * FLANGE_DELAY_LENGTH;
      if (FLANGE_OFFSET < 1) {
        FLANGE_OFFSET = 1;
      }
      if (FLANGE_OFFSET > 128) {
        FLANGE_OFFSET = 128;
      }
      flange1.voices(FLANGE_OFFSET,FLANGE_DEPTH,FLANGE_FREQ);
  } else if (strcasecmp(settingName, "flange") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        flange1.voices(FLANGE_DELAY_PASSTHRU,0,0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        flange1.voices(FLANGE_OFFSET,FLANGE_DEPTH,FLANGE_FREQ);
      }
  } else if (strcasecmp(settingName, "button") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte b = (byte)atoi(token);
      if (b >= 0 && b <= 5) {
        strcpy(CONTROL_BUTTON_SETTINGS[b], ptr);
      }
    
  }
  
}

/**
 * Create JSON string of settings 
 * (used for app)
 */
char *settingsToJson(char result[]) 
{

  const char str_template[12] = "\"%s\":\"%s\"";
  const char num_template[12] = "\"%s\":%s";
  char buf[20];
  char tmp[100];

  sprintf(tmp, str_template, "name", PROFILE_NAME);
  strcpy(result, tmp);
  strcat(result, ",");

  dtostrf(MASTER_VOLUME, 0, 4, buf);
  sprintf(tmp, num_template, "volume", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", MIC_GAIN);
  sprintf(tmp, num_template, "mic_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", LINEIN);
  sprintf(tmp, num_template, "linein", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", LINEOUT);
  sprintf(tmp, num_template, "lineout", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "startup", STARTUP_WAV);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "loop", LOOP_WAV);
  strcat(result, tmp);
  strcat(result, ",");

  dtostrf(LOOP_GAIN, 0, 4, buf);
  sprintf(tmp, num_template, "loop_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", HIPASS);
  sprintf(tmp, num_template, "high_pass", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(VOICE_GAIN, 0, 4, buf);
  sprintf(tmp, num_template, "voice_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  dtostrf(DRY_GAIN, 0, 4, buf);
  sprintf(tmp, num_template, "dry_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(DRY_GAIN, 0, 4, buf);
  sprintf(tmp, num_template, "dry_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(VOICE_START, 0, 4, buf);
  sprintf(tmp, num_template, "voice_start", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(VOICE_STOP, 0, 4, buf);
  sprintf(tmp, num_template, "voice_stop", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", SILENCE_TIME);
  sprintf(tmp, num_template, "silence_time", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  /*
  sprintf(buf, "%d", BUTTON_PIN);
  sprintf(tmp, num_template, "button_pin", buf);
  strcat(result, tmp);
  strcat(result, ",");
  */
  sprintf(tmp, str_template, "button_click", BUTTON_WAV);
  strcat(result, tmp);
  strcat(result, ",");
  
  sprintf(buf, "%d", AUDIO_INPUT);
  sprintf(tmp, num_template, "input", buf);
  strcat(result, tmp);
  strcat(result, ",");
  
  dtostrf(EFFECTS_GAIN, 0, 4, buf);
  sprintf(tmp, num_template, "effects_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", EQ);
  sprintf(tmp, num_template, "eq", buf);
  strcat(result, tmp);
  strcat(result, ",");

  char buffer[SETTING_ENTRY_MAX];
  char *bands = arrayToString(buffer, EQ_BANDS, EQ_BANDS_SIZE);
  sprintf(tmp, str_template, "eq_bands", bands);
  strcat(result, tmp);
  strcat(result, ",");
  memset(buffer, 0, sizeof(buffer));
  
  char *bitcrushers = arrayToString(buffer, BITCRUSHER, BITCRUSHER_SIZE);
  sprintf(tmp, str_template, "bitcrushers", bitcrushers);
  strcat(result, tmp);
  strcat(result, ",");
  memset(buffer, 0, sizeof(buffer));
  
  dtostrf(NOISE_GAIN, 0, 4, buf);
  sprintf(tmp, num_template, "noise_gain", buf);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "effects_dir", EFFECTS_DIR);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "sounds_dir", SOUNDS_DIR);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(tmp, str_template, "loop_dir", LOOP_DIR);
  strcat(result, tmp);
  strcat(result, ",");

  sprintf(buf, "%d", MUTE_LOOP);
  sprintf(tmp, num_template, "mute_loop", buf);
  strcat(result, tmp);

  if (APP_VER > 1.13) {
    sprintf(buf, "%d", MUTE_EFFECTS);
    sprintf(tmp, num_template, "mute_effects", buf);
    strcat(result, tmp);
  }

  if (APP_VER > 1.14) {
    strcat(result, ",");
    sprintf(buf, "%d", SLEEP_TIME);
    sprintf(tmp, num_template, "sleep_time", buf);
    strcat(result, tmp);
    strcat(result, ",");
    
    sprintf(tmp, str_template, "sleep_sound", SLEEP_SOUND);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", CHORUS_DELAY);
    sprintf(tmp, num_template, "chorus_delay", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", CHORUS_VOICES);
    sprintf(tmp, num_template, "chorus_voices", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", FLANGE_DELAY);
    sprintf(tmp, num_template, "flange_delay", buf);
    strcat(result, tmp);
    strcat(result, ",");

    sprintf(buf, "%d", FLANGE_OFFSET);
    sprintf(tmp, num_template, "flange_offset", buf);
    strcat(result, tmp);
    strcat(result, ",");
    
    sprintf(buf, "%d", FLANGE_DEPTH);
    sprintf(tmp, num_template, "flange_depth", buf);
    strcat(result, tmp);
    strcat(result, ",");

    dtostrf(FLANGE_FREQ, 0, 4, buf);
    sprintf(tmp, num_template, "flange_freq", buf);
    strcat(result, tmp);
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
  strcat(result, PROFILE_NAME);
  strcat(result, "]\n");

  if (MASTER_VOLUME > 0) {
    strcat(result, "[volume=");
    dtostrf(MASTER_VOLUME, 0, 4, buf);
    strcat(result, buf);
    strcat(result, "]\n");
    memset(buf, 0, sizeof(buf));
  }

  strcat(result, "[linein=");
  sprintf(buf, "%d", LINEIN);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[lineout=");
  sprintf(buf, "%d", LINEOUT);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[startup=");
  strcat(result, STARTUP_WAV);
  strcat(result, "]\n");

  strcat(result, "[loop=");
  strcat(result, LOOP_WAV);
  strcat(result, "]\n");

  strcat(result, "[loop_gain=");
  dtostrf(LOOP_GAIN, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));  

  strcat(result, "[high_pass=");
  sprintf(buf, "%d", HIPASS);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[voice_gain=");
  dtostrf(VOICE_GAIN, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[dry_gain=");
  dtostrf(DRY_GAIN, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
  
  strcat(result, "[voice_start=");
  dtostrf(VOICE_START, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[voice_stop=");
  dtostrf(VOICE_STOP, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");

  memset(buf, 0, sizeof(buf));
  strcat(result, "[silence_time=");
  sprintf(buf, "%d", SILENCE_TIME);
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
  strcat(result, BUTTON_WAV);
  strcat(result, "]\n");

  strcat(result, "[input=");
  sprintf(buf, "%d", AUDIO_INPUT);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[mic_gain=");
  sprintf(buf, "%d", MIC_GAIN);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[effects_gain=");
  dtostrf(EFFECTS_GAIN, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[eq=");
  sprintf(buf, "%d", EQ);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[eq_bands=");
  char *bands = arrayToString(buf, EQ_BANDS, EQ_BANDS_SIZE);
  strcat(result, bands);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[bitcrushers=");
  char *bitcrushers = arrayToString(buf, BITCRUSHER, BITCRUSHER_SIZE); 
  strcat(result, bitcrushers);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[noise_gain=");
  dtostrf(NOISE_GAIN, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[effects_dir=");
  strcat(result, EFFECTS_DIR);
  strcat(result, "]\n");

  strcat(result, "[sounds_dir=");
  strcat(result, SOUNDS_DIR);
  strcat(result, "]\n");

  strcat(result, "[loop_dir=");
  strcat(result, LOOP_DIR);
  strcat(result, "]\n");

  strcat(result, "[mute_loop=");
  sprintf(buf, "%d", MUTE_LOOP);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[mute_effects=");
  sprintf(buf, "%d", MUTE_EFFECTS);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[sleep_time=");
  sprintf(buf, "%d", SLEEP_TIME);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[sleep_sound=");
  strcat(result, SLEEP_SOUND);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[chorus_delay=");
  sprintf(buf, "%d", CHORUS_DELAY);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[chorus_voices=");
  sprintf(buf, "%d", CHORUS_VOICES);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_delay=");
  sprintf(buf, "%d", FLANGE_DELAY);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_offset=");
  sprintf(buf, "%d", FLANGE_OFFSET);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_depth=");
  sprintf(buf, "%d", FLANGE_DEPTH);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));

  strcat(result, "[flange_freq=");
  dtostrf(FLANGE_FREQ, 0, 4, buf);
  strcat(result, buf);
  strcat(result, "]\n");
  memset(buf, 0, sizeof(buf));
  
  return result;
  
}

/**
 * Set a startup setting value
 */
void setSettingValue(const char *key, const char *newValue)
{
   int index = -1;
   char newKey[SETTING_ENTRY_MAX];
   for (int i = 0; i < STARTUP_SETTINGS_COUNT; i++) {
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
  if (index > STARTUP_SETTINGS_COUNT - 1) {
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
  
}

/**
 * Retrieve a startup setting value
 */
char *getSettingValue(char result[], const char *key) 
{
    debug(F("Get setting: %s\n"), key);
    
    for (int i = 0; i < STARTUP_SETTINGS_COUNT; i++) {
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
}

/**
 * Save startup settings
 */
boolean saveSettings() {
  debug(F("Saving config data:\n"));
  File srcFile = openFile(SETTINGS_FILE, FILE_WRITE);
  if (srcFile) {
    for (int i = 0; i < STARTUP_SETTINGS_COUNT; i++) {
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
}

/**
 * Backup settings to specified file
 */
boolean saveSettingsFile(const char *src, const boolean backup = true) 
{
  char filename[SETTING_ENTRY_MAX];
  boolean result = false;
  if (strcasecmp(src, "") == 0) {
    strcpy(filename, PROFILE_FILE);
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
    char profiles[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
    int total = listFiles(PROFILES_DIR, profiles, MAX_FILE_COUNT, FILE_EXT, false, false);
    boolean result = false;
    boolean found = false;
    for (int i = 0; i < total; i++) {
      if (strcasecmp(profiles[i], filename) == 0) {
        setSettingValue("profile", filename);
        found = true;
        break;
      }
    }

    // save results to file if entry was not found
    if (found == true) {
      result = saveSettings();
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
  if (strcasecmp(filename, PROFILE_FILE) == 0){
    debug(F("Cannot delete current profile\n"));
    result = false;
  } else {
    result = deleteFile(path);
    // if the profile filename was the default profile, 
    // set the default profile to the currently loaded profile
    if (result == true) {
      char buffer[SETTING_ENTRY_MAX];
      char *profile = getSettingValue(buffer, "profile");
      if (strcasecmp(filename, profile) == 0) {
        debug(F("Profile was default -> Setting default profile to current profile\n"));
        setSettingValue("profile", PROFILE_FILE);
        saveSettings();
      }
    }

  }
  return result;
}

/***
 * Read settings from specified file
 */
int loadSettingsFile(const char *filename, char results[][SETTING_ENTRY_MAX], int max) 
{
  debug(F("Load Settings File %s\n"), filename);
  char character;
  char settingName[SETTING_ENTRY_MAX] = "";
  char settingValue[SETTING_ENTRY_MAX] = "";
  File myFile;
  int index = 0;
  int c = 0;
  int tries = 0;

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
  return index;
}

/**
 * Process a list of settings values
 */
void processSettings(char settings[][SETTING_ENTRY_MAX], const int max)
{
  for (int i = 0; i < max; i++) {
    char entry[SETTING_ENTRY_MAX];
    strcpy(entry, settings[i]);
    char *key, *value, *ptr;
    key = strtok_r(entry, "=", &ptr);
    value = strtok_r(NULL, "=", &ptr);
    parseSetting(key, value);
  }
}

/**
 * Load specified settings file
 */
int loadSettings(const char *filename) 
{
  char settings[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
  int total = loadSettingsFile(filename, settings, MAX_FILE_COUNT);
  processSettings(settings, total);
  return total;
}

/***
 * Apply settings
 */
void applySettings() 
{

  autoSleepMillis = 0;
  
  // Turn on the 5-band graphic equalizer (there is also a 7-band parametric...see the Teensy docs)
  if (EQ == 0) {
    audioShield.eqSelect(FLAT_FREQUENCY);
  } else {
    audioShield.eqSelect(GRAPHIC_EQUALIZER);
    // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
    // Valid values are -1 (-11.75dB) to 1 (+12dB)
    // The settings below pull down the lows and highs and push up the mids for 
    // more of a "tin-can" sound.
    audioShield.eqBands(EQ_BANDS[0], EQ_BANDS[1], EQ_BANDS[2], EQ_BANDS[3], EQ_BANDS[4]);
  }
  // tell the audio shield which input to use
  audioShield.inputSelect(AUDIO_INPUT);
  // adjust the gain of the input
  // adjust this as needed
  if (AUDIO_INPUT == 0) {
    audioShield.lineInLevel(LINEIN);
  } else {
    audioShield.micGain(MIC_GAIN);
  }  
  // You can modify these values to process the voice 
  // input.  See the Teensy bitcrusher demo for details.
  bitcrusher1.bits(BITCRUSHER[0]);
  bitcrusher1.sampleRate(BITCRUSHER[1]);
  //bitcrusher2.bits(BITCRUSHER[2]);
  //bitcrusher2.sampleRate(BITCRUSHER[3]);
  // Bitcrusher 1 input (fed by mic/line-in)
  voiceMixer.gain(0, VOICE_GAIN);
  voiceMixer.gain(1, VOICE_GAIN);
  // Dry (unprocessed) voice input
  voiceMixer.gain(2, DRY_GAIN);
  // Pink noise channel
  //voiceMixer.gain(1, NOISE_GAIN);
  // Feed from effects mixer
  voiceMixer.gain(3, 1);
  // stereo channels for SD card...adjust gain as 
  // necessary to match voice level
  effectsMixer.gain(0, EFFECTS_GAIN);
  effectsMixer.gain(1, EFFECTS_GAIN);
  // BLE connect sound
  effectsMixer.gain(2, EFFECTS_GAIN);
  // Feed from loop mixer
  effectsMixer.gain(3, 1);
  // chatter loop from SD card
  loopMixer.gain(0, LOOP_GAIN);
  loopMixer.gain(1, LOOP_GAIN);
  // Pink noise channel
  loopMixer.gain(3, NOISE_GAIN);
  audioShield.volume(readVolume());
  audioShield.lineOutLevel(LINEOUT);
  if (HIPASS == 0) {
    audioShield.adcHighPassFilterDisable();
  } else {
    audioShield.adcHighPassFilterEnable();
  }
  // Initialize PTT button
  /*
  if (BUTTON_PIN && BUTTON_PIN > 0) {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    PTT.attach(BUTTON_PIN);
    PTT.interval(15);
    snoozeDigital.pinMode(BUTTON_PIN, INPUT_PULLUP, FALLING);
  }
  */
  voiceOff();

  // Setup control glove buttons
  byte a = 0;
  button_initialized = false;
  PTT_BUTTON = NULL;
  WAKE_BUTTON = NULL;
  while (a < 7) {
    byte pin = CONTROL_BUTTON_PINS[a];
    // need to clear button settings...
    ControlButtons[a].setPTT(false);
    byte buttonNum = 0;
    Serial.print("Pin: ");
    Serial.println(pin);
    if (pin > 0) {

      // setup physical button
      ControlButtons[a].setup(pin);
      
      char *part_token, *part_ptr;
      Serial.print("SETTINGS ");
      Serial.println(CONTROL_BUTTON_SETTINGS[a]);
      part_token = strtok_r(CONTROL_BUTTON_SETTINGS[a], ";", &part_ptr);
      // button_type,data(sound)
      Serial.print("Initial part token: ");
      Serial.println(part_token);
      byte b = 0;

      while (part_token && b < 3) {
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
            debug(F("PTT Button on pin: %d\n"), pin);
            Serial.println(" -> PTT Button");
            PTT_BUTTON = a;
            ControlButtons[a].setPTT(true);
            if (WAKE_BUTTON == NULL) {
              WAKE_BUTTON = a;
              snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
            }
            break;
          // Sound button    
          case 2:
            max = 2;
            debug(F("Sound Button on pin: %d\n"), pin);
            Serial.println(" -> Sound Button");
            break;
          // sleep/wake (overrides PTT)
          case 6:
            WAKE_BUTTON = a;
            snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
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
              Serial.print("Setting sound to: ");
              Serial.println(button_token);
              ControlButtons[a].buttons[buttonNum].setSound(button_token);
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
    a++;
  }


  
}

  
