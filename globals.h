/****
 * Global variables/constants
 */
// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=91.1111068725586,153.88888931274414
AudioAnalyzeRMS          rms1;           //xy=212.11109924316406,189.88888549804688
AudioEffectBitcrusher    bitcrusher1;    //xy=275.111083984375,88.88888549804688
AudioSynthNoisePink      pink1;          //xy=340.6666717529297,314.5555725097656
AudioPlaySdWav           loopPlayer;     //xy=347.111083984375,245.88888549804688
AudioSynthWaveform       waveform1;      //xy=350.1111145019531,279.8888854980469
AudioPlaySdWav           effectsPlayer;  //xy=356.1111068725586,210.88888931274414
AudioEffectFlange        flange1;        //xy=416.1111145019531,89.88888549804688
AudioEffectChorus        chorus1;        //xy=547.1111068725586,88.88888931274414
AudioMixer4              effectsMixer;   //xy=558.111083984375,252.88888549804688
AudioMixer4              voiceMixer;     //xy=712.1111068725586,117.88888931274414
AudioOutputI2S           i2s2;           //xy=877.1111068725586,117.88888931274414
AudioConnection          patchCord1(i2s1, 0, bitcrusher1, 0);
AudioConnection          patchCord2(i2s1, 0, voiceMixer, 2);
AudioConnection          patchCord3(i2s1, 1, rms1, 0);
AudioConnection          patchCord4(bitcrusher1, flange1);
AudioConnection          patchCord5(pink1, 0, effectsMixer, 3);
AudioConnection          patchCord6(loopPlayer, 0, effectsMixer, 1);
AudioConnection          patchCord7(waveform1, 0, effectsMixer, 2);
AudioConnection          patchCord8(effectsPlayer, 0, effectsMixer, 0);
AudioConnection          patchCord9(flange1, chorus1);
AudioConnection          patchCord10(chorus1, 0, voiceMixer, 0);
AudioConnection          patchCord11(effectsMixer, 0, voiceMixer, 3);
AudioConnection          patchCord12(voiceMixer, 0, i2s2, 0);
AudioConnection          patchCord13(voiceMixer, 0, i2s2, 1);
AudioControlSGTL5000     audioShield;    //xy=77.1111068725586,263.88888931274414
// GUItool: end automatically generated code

// version flag
const char VERSION[5] = "4.0";
float APP_VER = 2.0;
      
//elapsedMillis ms;                         // running timer...inputs are checked every 24 milliseconds
elapsedMillis stopped;                      // used to tell how long user has stopped talking

#define MAX_FILE_COUNT 99
#define MAX_SETTINGS_COUNT 40
#define SETTING_ENTRY_MAX  150

byte lastRnd  = -1;                                       // Keeps track of the last file played so that it is different each time

/***************************
 * BUTTON TYPE VALUES
 *   1  = PTT (SLEEP/WAKE unless otherwise configured)
 *   2  = Play/Stop Specified Sound
 *   3  = Spkr Volume Up 
 *   4  = Spkr Volume Down 
 *   5  = Mute Everything
 *   6  = Sleep/Wake (Overrides PTT)
 *   7  = LineOut Volume Up
 *   8  = LineOut Volume Down
 *   9  = MIC_GAIN up
 *   10 = MIC_GAIN down
 *   11 = Start/Stop Loop
 *   12 = Loop gain up
 *   13 = Loop gain down
 *   14 = Voice gain up
 *   15 = Voice gain down
 *   16 = Dry gain up
 *   17 = Dry gain down
 *   18 = Effects gain up
 *   19 = Effects gain down   
 *   
 *   NOTE:  Pin 3 CANNOT wake up...
 *          only digital pins (like 2) work!
 */

#define EFFECTS_PLAYER 1
#define LOOP_PLAYER    2
#define FX_DELAY 16
#define FILENAME_SIZE 14

struct Loop_t {
  char    dir[14]   = "/loops/";
  char    file[14]  = "";
  boolean mute      = true;
  float   volume    = 1;  
};

struct Voice_t {
  float         volume = 1.0000;
  float         dry    = 0.5000;
  float         start  = 0.0300;
  float         stop   = 0.0200;
  unsigned int  wait   = 275;
};

struct Sounds_t {
  char dir[14]    = "/sounds/";
  char start[14]  = "STARTUP.WAV";
  char button[14] = "CLICK3.WAV";
};

struct Flanger_t {
  byte  delay  = FX_DELAY;
  byte  offset = 1;
  byte  depth  = 0;
  float freq   = 0.0625;
  short buffer[FX_DELAY*AUDIO_BLOCK_SAMPLES];
};

struct Chorus_t {
  byte voices  = 1;
  byte delay   = FX_DELAY;
  short buffer[FX_DELAY*AUDIO_BLOCK_SAMPLES];
};

struct Bitcrusher_t {
  byte bits = 16;
  int  rate = 44100;
};

struct Effects_t {
  char          dir[14]   = "/effects/";
  float         volume    = 1.0000;
  byte          highpass  = 1;
  float         noise     = 0.0140;
  boolean       mute      = true;   
  Bitcrusher_t  bitcrusher;
  Chorus_t      chorus;
  Flanger_t     flanger;
  char          files[MAX_FILE_COUNT][14];
  byte          count;
};

struct Eq_t {
  boolean active   = true;
  float   bands[5] = { -1.0,0,1,0,-1.0 };
};

struct Sleep_t {
  unsigned int  timer     = 0;
  char          file[14]  = "SLEEP.WAV";  
};

struct Glove_t {
  char dir[14] = "/glove/";
  char settings[6][30] = { "0","0","0","0","0","0" };
  // This could be turned into a management class, but there is not a lot of stuff to do with it...so....
  ControlButton ControlButtons[6] = { ControlButton(), ControlButton(), ControlButton(), ControlButton(), ControlButton(), ControlButton() }; 
};

struct Volume_t {
  float master     = 0.5;
  byte  lineout    = 29;  // Valid values 13 to 31. Default teensy setting is 29.
  byte  linein     = 5;   // Value values 0 to 15. Default teensy setting is 5;
  byte  microphone = 3;
};

struct Settings_t {
  char      name[25] = "Default Profile";
  char      file[14] = "DEFAULT.TXT";
  Volume_t  volume;
  Loop_t    loop;
  Voice_t   voice;
  Sounds_t  sounds;
  Effects_t effects;
  Eq_t      eq;
  Sleep_t   sleep;
  Glove_t   glove;
} Settings;

struct Config_t {
  char profile[14] = "DEFAULT.TXT";
  // These define the pins the (up to) 6 control buttons can be connected to
// NOTE:  Only digital pins can be used for waking:
//        2,4,6,7,9,10,11,13,16,21,22,26,30,33
  byte buttons[6]       = {0,0,0,0,0,0};
  boolean debug         = false;          // Set to true to have debug messages printed out...useful for testing
  boolean echo          = false;          // Set to true to have BLE TX/RX messages displayed in serial monitor
  char input[5]         = "BOTH";  // Set to "MIC", "LINE" or "BOTH" (default)
  char output[5]        = "BOTH";  // Set to "SPKR", "LINE" or "BOTH" (default)
  char access_code[25]  = "1138";      // the password for remote apps to access this device (specified in global settings file)
  long baud              = 9600;
} Config;

const char PROFILES_DIR[11]         = "/profile4/";

struct App_t {
  boolean silent              = false;          // used for PTT and to switch back to Voice Activated mode
  boolean speaking = false;                   // flag to let us know if the user is speaking or not
  boolean button_initialized  = false;          // flag that lets us know if the PTT has been pushed or not to go into PTT mode
  byte  ptt_button;  
  byte  wake_button = 255;
  // Bluetooth App 
  char device_id[50];                 // the uuid of the device connecting 
  boolean ble_connected = false;       // flag to indicate whether a remote app is connected or not
  boolean muted        = false;       // flag to indicate whether all sounds should be muted
} App;

/**
 * OPERATIONAL STATES - Used for tracking at what stage the app is currently running
 */
byte STATE;                      
const byte STATE_NONE     = 0;   
const byte STATE_BOOTING  = 1;
const byte STATE_RUNNING  = 2;
const byte STATE_SLEEPING = 3;

// Other defaults
const char SOUND_EXT[5]  = ".WAV";
const char FILE_EXT[5]   = ".TXT";
const char BACKUP_EXT[5] = ".BAK";

// Loops
elapsedMillis loopMillis = 0;
unsigned int loopLength;
elapsedMillis autoSleepMillis = 0;

// loop and serial command handlers
char cmd_key[SETTING_ENTRY_MAX] = "";
char cmd_val[SETTING_ENTRY_MAX] = "";
const byte MAX_DATA_SIZE = 100;
char received[MAX_DATA_SIZE] = "";

SnoozeDigital snoozeDigital;
SnoozeAudio   snoozeAudio;
SnoozeBlock config_teensy3x(snoozeDigital, snoozeAudio);

