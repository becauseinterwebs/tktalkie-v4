/****
 * Routines for BLE 
 */

/**
 * Send output to BLE
 */
void btprint(const char *str) {
  if (Config.echo == true) {
    Serial.print("TX: ");
    Serial.println(str);
  }
  if (App.ble_connected == true) {
    Serial1.print(str);
  }
}

/**
 * Send output to BLE
 */
void btprintln(const char *str) {
  if (Config.echo == true) {
    Serial.print("TX: ");
    Serial.println(str);
  }
  if (App.ble_connected == true) {
    Serial1.print(str);
    Serial1.print("\n");
  }
}

void btprint(const __FlashStringHelper *fmt, ... ) {
  char buf[1025]; // resulting string limited to 1M chars
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  if (Config.echo == true) {
    Serial.print(buf);
  }
  // break into chunks
  int l = strlen(buf);
  int a = 0;
  while (a < l) {
    int max = a + 20;
    if (max > l) {
      max = l;
    }
    for (int b = a; b < max; b++) {
      a++;
      Serial1.print(buf[b]);
    }
    delay(30);      
  }
}


/**
 * Shortcut to send output to in JSON format
 */
void sendToApp(const char *cmd, const char *value) 
{
  if (App.ble_connected == false) {
    return;
  }
  if (value[0] == '[') {
    btprint(F("{\"cmd\":\"%s\",\"data\":%s}\n"), cmd, value);
  } else {
    btprint(F("{\"cmd\":\"%s\",\"data\":\"%s\"}\n"), cmd, value);
  }
}

/**
 * Sends config via Bluetooth Serial.  Used for TKTalkie App
 */
void sendConfig() 
{

  debug(F("Sending config\n"));

  btprint(F("{\"cmd\":\"config\", \"data\": { \"ver\":\"%s\","), VERSION);

  char buffer[1024];
  char files[MAX_FILE_COUNT][FILENAME_SIZE];

  // add buttons
  char buttons[20];
  sprintf(buttons, "[%d,%d,%d,%d,%d,%d]", Config.buttons[0], Config.buttons[1], Config.buttons[2], Config.buttons[3], Config.buttons[4], Config.buttons[5]);
  btprint(F("\"buttons\":%s,"), buttons);
  
  byte count = listFiles(Settings.sounds.dir, files, MAX_FILE_COUNT, SOUND_EXT, false, false);

  // Add sound files
  
  char *sounds = arrayToStringJson(buffer, files, count);
  btprint(F("\"sounds\":%s,"), sounds);
  memset(buffer, 0, sizeof(buffer));

  // Clear array
  memset(files, 0, sizeof(files));

  // Add effects 
  // Does this need to be read again?  They should already be in memory
  char *effects = arrayToStringJson(buffer, Settings.effects.files, SOUND_EFFECTS_COUNT);
  btprint(F("\"effects\":%s,"), effects);
  memset(buffer, 0, sizeof(buffer));

  // get loop files 
  count = listFiles(Settings.loop.dir, files, MAX_FILE_COUNT, SOUND_EXT, false, false);
  
  // Add loops 
  char *loops = arrayToStringJson(buffer, files, count);
  btprint(F("\"loops\":%s,"), loops);
  memset(buffer, 0, sizeof(buffer));

  // Clear array
  memset(files, 0, sizeof(files));


  // get glove sound files 
  count = listFiles(Settings.glove.dir, files, MAX_FILE_COUNT, SOUND_EXT, false, false);
  
  // Add glove sounds 
  char *glove_sounds = arrayToStringJson(buffer, files, count);
  btprint(F("\"glove_sounds\":%s,"), loops);
  memset(buffer, 0, sizeof(buffer));

  // Clear array
  memset(files, 0, sizeof(files)); 

  //char *profile = getSettingValue(buffer, "profile");
  btprint(F("\"default\":\"%s\","), Config.profile);
  //memset(buffer, 0, sizeof(buffer));
  
  btprint(F("\"current\":\"%s\","), Settings.file, Settings.name);

  // This is already formatted correctly
  char *json = settingsToString(buffer);
  btprint(F("\"profile\":%s,"), json);
  memset(buffer, 0, sizeof(buffer));

  btprint(F("\"profiles\":["));
  
  // get config profile files 
  count = listFiles(PROFILES_DIR, files, MAX_FILE_COUNT, FILE_EXT, false, false);
  
  for (byte i = 0; i < count; i++) {
     char filename[30];
     strcpy(filename, PROFILES_DIR);
     strcat(filename, files[i]);
     Settings_t settings;
     loadSettings(filename, &settings, false);
     btprint(F("{\"name\":\"%s\",\"desc\":\"%s\"}"), files[i], settings.name);
     if (i < count-1) {
       btprint(F(","));
     }
  }
  
  btprint(F("],\"mute\":%s,"), App.muted == true ? "1" : "0");

  btprint(F("\"bg\":%s,"), loopPlayer.isPlaying() ? "1" : "0");

  // list directories on the card
  memset(buffer, 0, sizeof(buffer));
  // Clear array
  memset(files, 0, sizeof(files));
  count = listDirectories("/", files);
  char *dirs = arrayToStringJson(buffer, files, count);
  btprint(F("\"ls\":%s"), dirs);
            
  // end
  btprint("}}\n");
}

