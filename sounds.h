/****
 * Sound utils
 */

/**
 * Emit a warning tone
 */
void beep(const byte times = 1)
{
  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
  for (byte i=0; i<times; i++) {
    waveform1.frequency(720);
    waveform1.amplitude(0.7);
    delay(100);
    delay(50);
    waveform1.amplitude(0);
    delay(350);
  }
}

void boop(int freq, byte dir) {
   waveform1.frequency(freq);
   waveform1.amplitude(.7);
   for (byte i = 0; i < 50; i++) {
      if (dir == 1) {
        freq++;
      } else {
        freq--;
      }
      waveform1.frequency(freq);  
      delay(1);
   }
   waveform1.amplitude(0);
}

/***
 * Read the contents of the SD card and put any files ending with ".WAV" 
 * into the array.  It will recursively search directories.  
 */
void loadSoundEffects() 
{
  if (strcasecmp(Settings.effects_dir, "") == 0) {
    SOUND_EFFECTS_COUNT = 0;
    debug(F("No effects directory specified"));
    return;
  }
  SOUND_EFFECTS_COUNT = listFiles(Settings.effects_dir, Settings.sound_effects, MAX_FILE_COUNT, SOUND_EXT, false, false);
  debug(F("%d Sound effects loaded\n"), SOUND_EFFECTS_COUNT);
}

/***
 * Play the specified sound effect from the SD card
 */
long playSoundFile(byte player, char *filename) 
{
  if (strcasecmp(filename, "") == 0) {
    debug(F("Exit play sound -> blank file name\n"));
    return 0;
  }
  char *ret = strstr(filename, ".");
  if (ret == NULL) {
    char ext[5];
    strcpy(ext, SOUND_EXT);
    strcat(filename, ext);
  }
  debug(F("Play sound file %s on player %d\n"), filename, player);
  unsigned long len = 0;
  switch (player) {
    case LOOP_PLAYER:
      if (loopPlayer.isPlaying()) {
        loopPlayer.stop();
      }
      loopPlayer.play(filename);
      delay(10);
      len = loopPlayer.lengthMillis();
      break;
    default:
      effectsPlayer.play(filename);
      delay(10);
      len = effectsPlayer.lengthMillis();
      break;
  }
  debug(F("Sound File Length: %d\n"), len);
  return len;
}

/**
 * Shortcut to play a sound from the SOUNDS directory
 */
unsigned long playGloveSound(const char *filename)
{
  if (strcasecmp(filename, "") == 0) {
    return 0;
  }
  char buf[25];
  strcpy(buf, Settings.glove_dir);
  strcat(buf, filename);
  return playSoundFile(EFFECTS_PLAYER, buf);
}

/**
 * Shortcut to play a sound from the SOUNDS directory
 */
unsigned long playSound(const char *filename)
{
  if (strcasecmp(filename, "") == 0) {
    return 0;
  }
  char buf[25];
  strcpy(buf, Settings.sounds_dir);
  strcat(buf, filename);
  return playSoundFile(EFFECTS_PLAYER, buf);
}

/**
 * Shortcut to play a sound from the EFFECTS directory
 */
unsigned long playEffect(const char *filename)
{
  if (strcasecmp(filename, "") == 0) {
    return 0;
  }
  char buf[25];
  strcpy(buf, Settings.effects_dir);
  strcat(buf, filename);
  return playSoundFile(EFFECTS_PLAYER, buf);
}

/***
 * Play sound loop and set counters
 */
void playLoop() 
{
  loopLength = 0;
  Serial.println("AT PLAY LOOP");
  if (strcasecmp(Settings.loop_wav, "") != 0) {
    char buf[25];
    strcpy(buf, Settings.loop_dir);
    strcat(buf, Settings.loop_wav);
    Serial.print("PLAYING LOOP: ");
    Serial.println(buf);
    loopLength = playSoundFile(LOOP_PLAYER, buf);
  }
  loopMillis = 0;
}

/***
 * Play a random sound effect from the SD card
 */
void addSoundEffect()
{
  if (App.speaking == true || SOUND_EFFECTS_COUNT < 1 || Settings.mute_effects == true) return;
  // generate a random number between 0 and the number of files read - 1
  byte rnd = 0;
  byte count = 0;
  rnd = lastRnd;
  while (rnd == lastRnd && count < 50) {
   rnd = random(0, SOUND_EFFECTS_COUNT);
   count++;
  }
  lastRnd = rnd;
  // play the file
  playEffect(Settings.sound_effects[rnd]);
}

/***
 * Check the optional volume pot for output level
 */
float readVolume()
{
    float vol = 0;
    if (Settings.volume) {
      audioShield.volume(Settings.volume);
      vol = Settings.volume;
    } else {
      // comment these lines if your audio shield does not have the optional volume pot soldered on
      vol = analogRead(15);
      vol = vol / 1023;
      audioShield.volume(vol);
    }
    if (vol > 1.0) {
      vol = 1.0;
    } else if (vol < 0) {
      vol = 0;
    }
    return vol;
}


/***
 * This is played when switching from PTT back to Voice Activated mode.
 * It is also played when a device connects via Bluetooth. 
 */
void connectSound() 
{
  for (byte i=0; i<3; i++) {
    waveform1.frequency(1440);
    waveform1.amplitude(0.3);
    delay(100);
    waveform1.amplitude(0);
    delay(100);
  }
  waveform1.amplitude(0);
}

/***
 * This is played when a mobile device connects via Bluetooth 
 */
void disconnectSound() 
{
  for (byte i=0; i<3; i++) {
    waveform1.frequency(720);
    waveform1.amplitude(0.3);
    delay(100);
    waveform1.amplitude(0);
    delay(100);
  }
  waveform1.amplitude(0);
}

/***
 * Turns the volume down on the chatter loop
 */
void loopOff() 
{
  effectsMixer.gain(1, 0);
}

/***
 * Turns the volume up on the chatter loop
 */
void loopOn() 
{
  // gradually raise level to avoid pops 
  if (Settings.loop_gain > 1) {
    for (byte i=0; i<=Settings.loop_gain; i++) {
      effectsMixer.gain(1, i);
    }
  }
  effectsMixer.gain(1, Settings.loop_gain);
}

/***
 * Turns off the voice channels on the mixer
 */
void voiceOff() 
{
  autoSleepMillis = 0;
  App.speaking = false;
  App.silent = false;
  stopped = 0;
  pink1.amplitude(0);
  voiceMixer.gain(0, 0);
  voiceMixer.gain(1, 0);
  voiceMixer.gain(2, 0);
  if (Settings.mute_loop == 1) {
    loopOn();
  }
}

/***
 * Turns on the voice channels on the mixer
 */
void voiceOn() 
{
  autoSleepMillis = 0;
  App.speaking = true;
  App.silent = true;
  if (Settings.mute_loop == 1) {
    loopOff();
  }
  // Reset the "user is talking" timer
  stopped = 0;
  // pops are ok here ;)
  pink1.amplitude(Settings.noise_gain);
  voiceMixer.gain(0, Settings.voice_gain);
  voiceMixer.gain(1, Settings.voice_gain);
  voiceMixer.gain(2, Settings.dry_gain);
}
