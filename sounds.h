/****
 * Sound utils
 */

/**play
 * Emit a warning tone
 */
void beep(const byte times = 1)
{
  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
  for (byte i=0; i<times; i++) {
    waveform1.frequency(720);
    waveform1.amplitude(0.2);
    delay(150);
    waveform1.amplitude(0);
    delay(350);
  }
}

void boop(int freq, const byte dir=1, const byte distance=75) {
   waveform1.frequency(freq);
   waveform1.amplitude(.2);
   for (byte i = 0; i < distance; i++) {
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

void boopUp() {
  boop(440);
}

void boopDown() {
  boop(480, 0, 100);
}
void berp() {
  for (byte i = 0; i < 2; i++) {
    waveform1.frequency(2440);
    waveform1.amplitude(.2);
    delay(50);
    waveform1.frequency(380);
    delay(75);
    waveform1.amplitude(0);
    delay(30);
  }  
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
  if (Settings.loop.volume > 1) {
    for (byte i=0; i<=Settings.loop.volume; i++) {
      effectsMixer.gain(1, i);
    }
  }
  effectsMixer.gain(1, Settings.loop.volume);
}

/***
 * Read the contents of the SD card and put any files ending with ".WAV" 
 * into the array.  It will recursively search directories.  
 */
void loadSoundEffects() 
{
  if (strcasecmp(Settings.effects.dir, "") == 0) {
    SOUND_EFFECTS_COUNT = 0;
    debug(F("No effects directory specified"));
    return;
  }
  SOUND_EFFECTS_COUNT = listFiles(Settings.effects.dir, Settings.effects.files, MAX_FILE_COUNT, SOUND_EXT, false, false);
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
  char buf[FILENAME_SIZE*2];
  strcpy(buf, Settings.glove.dir);
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
  char buf[FILENAME_SIZE*2];
  strcpy(buf, Settings.sounds.dir);
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
  char buf[FILENAME_SIZE*2];
  strcpy(buf, Settings.effects.dir);
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
  if (strcasecmp(Settings.loop.file, "") != 0) {
    char buf[FILENAME_SIZE*2];
    strcpy(buf, Settings.loop.dir);
    strcat(buf, Settings.loop.file);
    Serial.print("PLAYING LOOP: ");
    Serial.println(buf);
    loopLength = playSoundFile(LOOP_PLAYER, buf);
  } else {
    Serial.println("NO LOOP FILE");
  }
  loopMillis = 0;
}

/***
 * Play a random sound effect from the SD card
 */
void addSoundEffect()
{
  if (App.speaking == true || SOUND_EFFECTS_COUNT < 1 || Settings.effects.mute == true || Settings.effects.mute == 1) return;
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
  playEffect(Settings.effects.files[rnd]);
}

/***
 * Check the optional volume pot for output level
 */
float readVolume()
{
    float vol = 0;
    if (Settings.volume.master) {
      audioShield.volume(Settings.volume.master);
      vol = Settings.volume.master;
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
  if (Settings.loop.mute == true) {
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
  if (Settings.loop.mute == true) {
    loopOff();
  }
  // Reset the "user is talking" timer
  stopped = 0;
  // pops are ok here ;)
  pink1.amplitude(Settings.effects.noise);
  voiceMixer.gain(0, Settings.voice.volume);
  voiceMixer.gain(1, Settings.voice.volume);
  voiceMixer.gain(2, Settings.voice.dry);
}
