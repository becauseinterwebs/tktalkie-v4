/****
   Global variables/constants
*/

// version flag
#define VERSION     4.0
#define MIN_APP_VER 2.0

/***************************
   BUTTON TYPE VALUES
   NOTE: Only digital pins (like 2) work for Sleep/Wake
*/
#define BUTTON_PTT              1   // PTT (SLEEP/WAKE unless otherwise configured)
#define BUTTON_SOUND            2   // Play/Stop Specified Sound
#define BUTTON_VOL_UP           3   // Spkr Volume Up
#define BUTTON_VOL_DOWN         4   // Spkr Volume Down
#define BUTTON_MUTE             5   // Mute Everything
#define BUTTON_SLEEP            6   // Sleep/Wake (Overrides PTT)
#define BUTTON_LINE_UP          7   // LineOut Volume Up
#define BUTTON_LINE_DOWN        8   // LineOut Volume Down
#define BUTTON_MIC_UP           9   // MIC_GAIN up
#define BUTTON_MIC_DOWN         10  // MIC_GAIN down
#define BUTTON_LOOP             11  // Start/Stop Loop
#define BUTTON_LOOP_UP          12  // Loop gain up
#define BUTTON_LOOP_DOWN        13  // Loop gain down
#define BUTTON_VOICE_UP         14  // Voice gain up
#define BUTTON_VOICE_DOWN       15  // Voice gain down
#define BUTTON_DRY_UP           16  // Dry gain up
#define BUTTON_DRY_DOWN         17  // Dry gain down
#define BUTTON_EFFECTS_UP       18  // Effects gain up
#define BUTTON_EFFECTS_DOWN     19  // Effects gain down
#define BUTTON_SHIFT_RANGE_UP   20  // Voice Shifter range up
#define BUTTON_SHIFT_RANGE_DOWN 21  // Voice Shifter range down
#define BUTTON_SHIFT_SPEED_UP   22  // Voice Shifter speed up
#define BUTTON_SHIFT_SPEED_DOWN 23  // Voice Shifter speed down

/***********************
 * Command Mapping
 */
#define CMD_DEBUG       0
#define CMD_ECHO        1
#define CMD_DEFAULT     2
#define CMD_DELETE      3    
#define CMD_LOAD        4
#define CMD_PLAY        5
#define CMD_PLAY_EFFECT 6
#define CMD_PLAY_SOUND  7
#define CMD_PLAY_GLOVE  8
#define CMD_PLAY_LOOP   9
#define CMD_STOP_LOOP   10
#define CMD_CONFIG      11
#define CMD_MUTE        12
#define CMD_UNMUTE      13
#define CMD_SAVE        14
#define CMD_ACCESS_CODE 15
#define CMD_CONNECT     16
#define CMD_DISCONNECT  17
#define CMD_DOWNLOAD    18
#define CMD_BACKUP      19
#define CMD_RESTORE     20
#define CMD_SETTINGS    21
#define CMD_FILES       22
#define CMD_SOUNDS      23
#define CMD_EFFECTS     24
#define CMD_LOOPS       25
#define CMD_GLOVE       26
#define CMD_PROFILES    27
#define CMD_LS          28
#define CMD_HELP        29
#define CMD_CALIBRATE   30
#define CMD_RESET       31
#define CMD_SLEEP       32
#define CMD_BAUD        33
#define CMD_MEM         34
#define CMD_BEEP        35
#define CMD_BERP        36
#define CMD_SHOW        37

#define MAX_GAIN 10

#define GRANULAR_MEMORY_SIZE 2048  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

elapsedMillis stopped;                      // used to tell how long user has stopped talking

#define MAX_FILE_COUNT 99
#define MAX_SETTINGS_COUNT 40
#define SETTING_ENTRY_MAX  150

#define EFFECTS_PLAYER 1
#define LOOP_PLAYER    2
#define FX_DELAY       16
#define FILENAME_SIZE  14
#define MAX_DATA_SIZE  100

// Files and extensions
#define MAX_FILENAME 14
#define PROFILES_DIR "/profile4/"
#define SOUND_EXT    ".WAV"
#define FILE_EXT     ".TXT"
#define BACKUP_EXT   ".BAK"

/**
   OPERATIONAL STATES - Used for tracking at what stage the app is currently running
*/
#define STATE_NONE     0
#define STATE_BOOTING  1
#define STATE_RUNNING  2
#define STATE_SLEEPING 3

#define JSON_BUFFER_SIZE 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + 2*JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(9) + 830

struct Shifter_t {
  int   length  = 0;
  int   speed   = 512;
  float range   = 5.0;
  byte  enabled = 0;
};

struct Loop_t {
  char    dir[MAX_FILENAME]   = "/loops/";
  char    file[MAX_FILENAME]  = "";
  boolean mute                = true;
  float   volume              = 1;
};

struct Voice_t {
  float         volume = 1.0000;
  float         dry    = 0.5000;
  float         start  = 0.0300;
  float         stop   = 0.0200;
  unsigned int  wait   = 275;
};

struct Sounds_t {
  char dir[MAX_FILENAME]    = "/sounds/";
  char start[MAX_FILENAME]  = "STARTUP.WAV";
  char button[MAX_FILENAME] = "CLICK3.WAV";
};

struct Flanger_t {
  byte  delay  = FX_DELAY;
  byte  offset = 1;
  byte  depth  = 0;
  float freq   = 0.0625;
  short buffer[FX_DELAY * AUDIO_BLOCK_SAMPLES];
  byte enabled = 0;
};

struct Chorus_t {
  byte  voices  = 1;
  byte  delay   = FX_DELAY;
  byte  enabled = 0;
  short buffer[FX_DELAY * AUDIO_BLOCK_SAMPLES];
};

struct Bitcrusher_t {
  byte bits    = 16;
  int  rate    = 44100;
};

struct Effects_t {
  char          dir[MAX_FILENAME]   = "/effects/";
  float         volume              = 1.0000;
  byte          highpass            = 1;
  float         noise               = 0.0140;
  boolean       mute                = true;
  Bitcrusher_t  bitcrusher;
  Chorus_t      chorus;
  Flanger_t     flanger;
  Shifter_t     shifter;
  char          files[MAX_FILE_COUNT][MAX_FILENAME];
  byte          count;
};

struct Eq_t {
  boolean active   = true;
  float   bands[5] = { -1.0, 0, 1, 0, -1.0 };
};

struct Sleep_t {
  unsigned int  timer               = 0;
  char          file[MAX_FILENAME]  = "SLEEP.WAV";
};

struct Glove_t {
  char dir[MAX_FILENAME] = "/glove/";
  char settings[6][30]   = { "0", "0", "0", "0", "0", "0" };
  // This could be turned into a management class, but there is not a lot of stuff to do with it...so....
  ControlButton ControlButtons[6] = { ControlButton(), ControlButton(), ControlButton(), ControlButton(), ControlButton(), ControlButton() };
};

struct Volume_t {
  float master     = 0.5;
  byte  lineout    = 29;  // Valid values 13 to 31. Default teensy setting is 29.
  byte  linein     = 5;   // Value values 0 to 15. Default teensy setting is 5;
  byte  microphone = 3;
};

// Global settings object
struct Settings_t {
  char      name[25]           = "Default Profile";
  char      file[MAX_FILENAME] = "DEFAULT.TXT";
  Volume_t  volume;
  Loop_t    loop;
  Voice_t   voice;
  Sounds_t  sounds;
  Effects_t effects;
  Eq_t      eq;
  Sleep_t   sleep;
  Glove_t   glove;
} Settings;

// NOTE:  Only digital pins can be used for waking:
//        2,4,6,7,9,10,11,13,16,21,22,26,30,33
struct Config_t {
  char    profile[MAX_FILENAME] = "DEFAULT.TXT";
  byte    buttons[6]            = {0, 0, 0, 0, 0, 0}; // These define the pins the (up to) 6 control buttons can be connected to
  boolean debug                 = false;            // Set to true to have debug messages printed out...useful for testing
  boolean echo                  = false;            // Set to true to have BLE TX/RX messages displayed in serial monitor
  char    input[5]              = "BOTH";            // Set to "MIC", "LINE" or "BOTH" (default)
  char    output[5]             = "BOTH";            // Set to "SPKR", "LINE" or "BOTH" (default)
  char    access_code[25]       = "1138";            // the password for remote apps to access this device (specified in global settings file)
  long    baud                  = 9600;              // Serial communication speed between BLE and Teensy
} Config;

struct App_t {
  byte    state               = STATE_NONE;   // tracks the operational state of the application
  boolean silent              = false;        // used for PTT and to switch back to Voice Activated mode
  boolean speaking            = false;        // flag to let us know if the user is speaking or not
  boolean button_initialized  = false;        // flag that lets us know if the PTT has been pushed or not to go into PTT mode
  boolean ble_connected       = false;        // flag to indicate whether a remote app is connected or not
  boolean muted               = false;        // flag to indicate whether all sounds should be muted
  byte    lastRnd             = -1;           // Keeps track of the last file played so that it is different each time
  byte    wake_button         = 255;
  byte    ptt_button;
  char    device_id[50];
} App;

// Loops
unsigned int  loopLength;
elapsedMillis loopMillis      = 0;
elapsedMillis autoSleepMillis = 0;

SnoozeDigital snoozeDigital;
SnoozeAudio   snoozeAudio;
SnoozeBlock   config_teensy3x(snoozeDigital, snoozeAudio);

// GUItool: begin automatically generated code

AudioInputI2S            i2s1;           //xy=59,147
AudioEffectGranular      granular1;      //xy=175,91
AudioAnalyzeRMS          rms1;           //xy=248,201
AudioEffectBitcrusher    bitcrusher1;    //xy=311,100
AudioSynthNoisePink      pink1;          //xy=376,326
AudioPlaySdWav           loopPlayer;     //xy=383,257
AudioSynthWaveform       waveform1;      //xy=386,291
AudioPlaySdWav           effectsPlayer;  //xy=392,222
AudioEffectFlange        flange1;        //xy=452,101
AudioEffectChorus        chorus1;        //xy=583,100
AudioMixer4              effectsMixer;   //xy=594,264
AudioMixer4              voiceMixer;     //xy=748,129
AudioOutputI2S           i2s2;           //xy=913,129
AudioConnection          patchCord1(i2s1, 0, voiceMixer, 2);
AudioConnection          patchCord2(i2s1, 0, granular1, 0);
AudioConnection          patchCord3(i2s1, 1, rms1, 0);
AudioConnection          patchCord4(granular1, bitcrusher1);
AudioConnection          patchCord5(bitcrusher1, flange1);
AudioConnection          patchCord6(pink1, 0, effectsMixer, 3);
AudioConnection          patchCord7(loopPlayer, 0, effectsMixer, 1);
AudioConnection          patchCord8(waveform1, 0, effectsMixer, 2);
AudioConnection          patchCord9(effectsPlayer, 0, effectsMixer, 0);
AudioConnection          patchCord10(flange1, chorus1);
AudioConnection          patchCord11(chorus1, 0, voiceMixer, 0);
AudioConnection          patchCord12(effectsMixer, 0, voiceMixer, 3);
AudioConnection          patchCord13(voiceMixer, 0, i2s2, 0);
AudioConnection          patchCord14(voiceMixer, 0, i2s2, 1);
AudioControlSGTL5000     audioShield;    //xy=113,275
// GUItool: end automatically generated code
/*
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
*/

