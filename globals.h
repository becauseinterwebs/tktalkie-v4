/****
 * Global variables/constants
 */
 
// GUItool: begin automatically generated code
/*
AudioInputI2S            i2s1;           //xy=196,125
AudioPlaySdWav           loopPlayer;     //xy=199,402
AudioSynthWaveform       waveform1;      //xy=202,316
AudioPlaySdWav           effectsPlayer;  //xy=208,266
AudioAnalyzeRMS          rms1;           //xy=325,174
AudioSynthNoisePink      pink1;          //xy=473,176
AudioEffectBitcrusher    bitcrusher1;    //xy=487,90
AudioEffectBitcrusher    bitcrusher2;    //xy=487,133
AudioMixer4              loopMixer;      //xy=656,412
AudioMixer4              voiceMixer;     //xy=659,142
AudioMixer4              effectsMixer;   //xy=660,280
AudioOutputI2S           i2s2;           //xy=836,139
AudioConnection          patchCord1(i2s1, 0, rms1, 0);
AudioConnection          patchCord2(i2s1, 0, bitcrusher1, 0);
AudioConnection          patchCord3(i2s1, 0, bitcrusher2, 0);
AudioConnection          patchCord4(loopPlayer, 0, loopMixer, 0);
AudioConnection          patchCord5(loopPlayer, 1, loopMixer, 1);
AudioConnection          patchCord6(waveform1, 0, effectsMixer, 2);
AudioConnection          patchCord7(effectsPlayer, 0, effectsMixer, 0);
AudioConnection          patchCord8(effectsPlayer, 1, effectsMixer, 1);
AudioConnection          patchCord9(pink1, 0, voiceMixer, 2);
AudioConnection          patchCord10(bitcrusher1, 0, voiceMixer, 0);
AudioConnection          patchCord11(bitcrusher2, 0, voiceMixer, 1);
AudioConnection          patchCord12(loopMixer, 0, effectsMixer, 3);
AudioConnection          patchCord13(voiceMixer, 0, i2s2, 0);
AudioConnection          patchCord14(voiceMixer, 0, i2s2, 1);
AudioConnection          patchCord15(effectsMixer, 0, voiceMixer, 3);
AudioControlSGTL5000     audioShield;    //xy=846,428
// GUItool: end automatically generated code
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=91.1111068725586,153.88888931274414
AudioAnalyzeRMS          rms1;           //xy=210.1111068725586,217.88888931274414
AudioSynthNoisePink      pink1;          //xy=266.6666717529297,385.55557441711426
AudioPlaySdWav           loopPlayer;     //xy=275.1111068725586,321.88888931274414
AudioEffectBitcrusher    bitcrusher1;    //xy=283.1111068725586,89.88888931274414
AudioSynthWaveform       waveform1;      //xy=349.1111068725586,253.88888931274414
AudioPlaySdWav           effectsPlayer;  //xy=356.1111068725586,210.88888931274414
AudioEffectFlange        flange1;        //xy=418.1111068725586,88.88888931274414
AudioMixer4              loopMixer;      //xy=451.1111068725586,333.88888931274414
AudioEffectChorus        chorus1;        //xy=547.1111068725586,88.88888931274414
AudioMixer4              effectsMixer;   //xy=591.1111068725586,224.88888931274414
AudioMixer4              voiceMixer;     //xy=712.1111068725586,117.88888931274414
AudioOutputI2S           i2s2;           //xy=877.1111068725586,117.88888931274414
AudioConnection          patchCord1(i2s1, 0, bitcrusher1, 0);
AudioConnection          patchCord2(i2s1, 0, voiceMixer, 2);
AudioConnection          patchCord3(i2s1, 1, rms1, 0);
AudioConnection          patchCord4(pink1, 0, loopMixer, 3);
AudioConnection          patchCord5(loopPlayer, 0, loopMixer, 0);
AudioConnection          patchCord6(loopPlayer, 1, loopMixer, 1);
AudioConnection          patchCord7(bitcrusher1, flange1);
AudioConnection          patchCord8(waveform1, 0, effectsMixer, 2);
AudioConnection          patchCord9(effectsPlayer, 0, effectsMixer, 0);
AudioConnection          patchCord10(effectsPlayer, 1, effectsMixer, 1);
AudioConnection          patchCord11(flange1, chorus1);
AudioConnection          patchCord12(loopMixer, 0, effectsMixer, 3);
AudioConnection          patchCord13(chorus1, 0, voiceMixer, 0);
AudioConnection          patchCord14(chorus1, 0, voiceMixer, 1);
AudioConnection          patchCord15(effectsMixer, 0, voiceMixer, 3);
AudioConnection          patchCord16(voiceMixer, 0, i2s2, 0);
AudioConnection          patchCord17(voiceMixer, 0, i2s2, 1);
AudioControlSGTL5000     audioShield;    //xy=77.1111068725586,263.88888931274414
// GUItool: end automatically generated code



// version flag
const char VERSION[5] = "3.5";
float APP_VER = 1.13;
      
//elapsedMillis ms;                         // running timer...inputs are checked every 24 milliseconds
elapsedMillis stopped;                      // used to tell how long user has stopped talking
boolean speaking = false;                   // flag to let us know if the user is speaking or not

const byte MAX_FILE_COUNT    = 99;
const byte SETTING_ENTRY_MAX = 50;

byte SOUND_EFFECTS_COUNT = 0;                             // This keeps count of how many valid WAV files were found.
char SOUND_EFFECTS[MAX_FILE_COUNT][SETTING_ENTRY_MAX];   // This will hold an array of the WAV files on the SD card.
                                                         // 99 is an arbitrary number.  You can change it as you need to.
byte lastRnd  = -1;                                       // Keeps track of the last file played so that it is different each time

//Bounce PTT = Bounce();                       // Used to read the PTT button (if attached)

byte CONTROL_BUTTON_PINS[6] = {0,0,0,0,0,0};


// this is temp for testing
/***************************
 * BUTTON VALUES
 *   1  = PTT (SLEEP/WAKE unless otherwise configured)
 *   2  = Play Specified Sound
 *   3  = Spkr Volume Up 
 *   4  = Spkr Volume Down 
 *   5  = Mute
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
 *   16 = Effects gain up
 *   17 = Effects gain down
 *   
 *   NOTE:  Pin 3 CANNOT wake up...
 *          only digital pins (like 2) work!
 */
char CONTROL_BUTTON_SETTINGS[6][30] = {
    "0",
    "0",
    "0",
    "0",
    "0",
    "0"  
};


ControlButton ControlButtons[6] = {
  ControlButton(),
  ControlButton(),
  ControlButton(),
  ControlButton(),
  ControlButton(),
  ControlButton()
}; 

boolean silent = false;                      // used for PTT and to switch back to Voice Activated mode
boolean button_initialized = false;          // flag that lets us know if the PTT has been pushed or not to go into PTT mode

const byte EFFECTS_PLAYER = 1;
const byte LOOP_PLAYER = 2;

const char SETTINGS_FILE[SETTING_ENTRY_MAX]    = "SETTINGS.TXT";   // Global settings file

// These are the paths where files are stored for the profile.  This can be changed via the app or
// edited manually so that each profile can have its own files (or you can share them...)
char EFFECTS_DIR[SETTING_ENTRY_MAX] = "/effects/";      // effects sounds (mic pop, clicks, static, etc.)
char SOUNDS_DIR[SETTING_ENTRY_MAX]  = "/sounds/";       // general sound files 
char LOOP_DIR[SETTING_ENTRY_MAX]    = "/loops/";        // sound loops
const char PROFILES_DIR[11]         = "/profiles/";     // Location of config profiles
char GLOVE_DIR[SETTING_ENTRY_MAX]   = "/glove/";

const byte STARTUP_SETTINGS_COUNT    = 6;                                 // counter for number of entries in global settings file
char STARTUP_SETTINGS[STARTUP_SETTINGS_COUNT][SETTING_ENTRY_MAX];        // Holds settings values


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

// Default settings - can be modified through settings file
char     PROFILE_NAME[50] = "Default";
char     PROFILE_FILE[SETTING_ENTRY_MAX] = "TKCONFIG.TXT";
float    MASTER_VOLUME    = 0.5; 
byte     LINEOUT          = 29; // Valid values 13 to 31. Default teensy setting is 29.
byte     LINEIN           = 5;  // Value values 0 to 15. Default teensy setting is 5;
byte     HIPASS           = 0;  // off by default, 1 = on
byte     MIC_GAIN         = 3;
char     STARTUP_WAV[SETTING_ENTRY_MAX];
char     LOOP_WAV[SETTING_ENTRY_MAX];
char     BUTTON_WAV[SETTING_ENTRY_MAX];
byte     AUDIO_INPUT      = AUDIO_INPUT_MIC;
byte     EQ               = 0;
byte     EQ_BANDS_SIZE    = 5;
float    EQ_BANDS[5]      = { -1.0,0,1,0,-1.0 };
byte     BITCRUSHER_SIZE  = 2;
int      BITCRUSHER[2]    = { 16,44100 };
float    LOOP_GAIN        = 2;
float    VOICE_GAIN       = 1;
float    DRY_GAIN         = 1;
float    NOISE_GAIN       = 0;
float    EFFECTS_GAIN     = 1;
uint16_t SILENCE_TIME     = 350;     // The minimum time to wait before playing a sound effect after talking has stopped
float    VOICE_START      = 0.07;    // The amplitude needed to trigger the sound effects process
float    VOICE_STOP       = 0.02;    // The minimum amplitude to use when determining if you've stopped talking.
                                     // Depending upon the microphone you are using, you may get a constant signal
                                     // that is above 0 or even 0.01.  Use the Serial monitor and add output to the 
                                     // loop to see what signal amplitude you are receiving when the mic is "quiet."
boolean  MUTE_LOOP = 1;              // mute loop while talking on/off
boolean  MUTE_EFFECTS = 0;           // flag to mute sound effects after talking has stopped
uint16_t SLEEP_TIME = 0;             // Number of minutes to wait before putting unit to sleep if no sound is heard
                                     // from the microphone (default is 0 = do not sleep.)
byte  PTT_BUTTON;  
byte  WAKE_BUTTON;
                                   
// Sound played when going into sleep mode
char     SLEEP_SOUND[SETTING_ENTRY_MAX];

#define FX_DELAY 16

// Chorus settings
byte  CHORUS_DELAY     = FX_DELAY;
byte  CHORUS_VOICES    = 1;          // off by default
short CHORUS_BUFFER[FX_DELAY*AUDIO_BLOCK_SAMPLES];

byte   FLANGE_DELAY    = FX_DELAY;
byte   FLANGE_OFFSET   = 1;
byte   FLANGE_DEPTH    = 0;
double FLANGE_FREQ     = .0625;
short  FLANGE_BUFFER[FX_DELAY*AUDIO_BLOCK_SAMPLES];


boolean DEBUG     = false;          // Set to true to have debug messages printed out...useful for testing
boolean ECHO      = false;          // Set to true to have BLE TX/RX messages displayed in serial monitor

// Bluetooth App 
char DEVICE_ID[50];                 // the uuid of the device connecting 
char ACCESS_CODE[25] = "1138";      // the password for remote apps to access this device (specified in global settings file)
boolean BT_CONNECTED = false;       // flag to indicate whether a remote app is connected or not
boolean MUTED        = false;       // flag to indicate whether all sounds should be muted

char INPUT_TYPE[5]         = "BOTH";  // Set to "MIC", "LINE" or "BOTH" (default)
char OUTPUT_TYPE[5]        = "BOTH";  // Set to "SPKR", "LINE" or "BOTH" (default)

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

