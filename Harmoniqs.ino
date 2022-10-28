#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/saw32_uint8.h>
#include <ADSR.h>
#include <MIDI.h>
#include <mozzi_midi.h>
#include <Smooth.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define CONTROL_RATE 128 // Hz, powers of 2 are most reliable
#define VOCT_PIN 7
#define GATE_PIN 3
#define OCTAVE_UP_PIN 7
#define OCTAVE_DOWN_PIN 8
#define PORTAMENTO_PIN 2
#define HARMONICS_TYPE_PIN 1
#define HARMONICS_LEVEL_PIN 0
#define ATTACK_PIN 3
#define DECAY_PIN 4
#define SUSTAIN_PIN 5
#define RELEASE_PIN 6
#define ADSR_TO_VOL_MIDI_PIN 6
#define ADSR_TO_VOL_CV_PIN 5
#define ADSR_TO_HARM_LEVEL_PIN 4

const static int8_t oma_wavetable1[32] = {0,25,49,71,90,106,117,125,127,125,117,106,90,71,49,25,0,-25,-49,-71,-90,-106,-117,-125,-127,-125,-117,-106,-90,-71,-49,-25};
const static int8_t oma_wavetable2[32] = {0,49,90,117,127,117,90,49,0,-49,-90,-117,-127,-117,-90,-49,0,49,90,117,127,117,90,49,0,-49,-90,-117,-127,-117,-90,-49};
const static int8_t oma_wavetable3[32] = {0,71,117,125,90,25,-49,-106,-127,-106,-49,25,90,125,117,71,0,-71,-117,-125,-90,-25,49,106,127,106,49,-25,-90,-125,-117,-71};
const static int8_t oma_wavetable4[32] = {0,90,127,90,0,-90,-127,-90,0,90,127,90,0,-90,-127,-90,0,90,127,90,0,-90,-127,-90,0,90,127,90,0,-90,-127,-90};
const static int8_t oma_wavetable5[32] = {0,106,117,25,-90,-125,-49,71,127,71,-49,-125,-90,25,117,106,0,-106,-117,-25,90,125,49,-71,-127,-71,49,125,90,-25,-117,-106};
const static int8_t oma_wavetable6[32] = {0,117,90,-49,-127,-49,90,117,0,-117,-90,49,127,49,-90,-117,0,117,90,-49,-127,-49,90,117,0,-117,-90,49,127,49,-90,-117};
const static int8_t oma_wavetable7[32] = {0,125,49,-106,-90,71,117,-25,-127,-25,117,71,-90,-106,49,125,0,-125,-49,106,90,-71,-117,25,127,25,-117,-71,90,106,-49,-125};
const static int8_t oma_wavetable8[32] = {64,64,-64,-64,64,64,-64,-64,64,64,-64,-64,64,64,-64,-64,64,64,-64,-64,64,64,-64,-64,64,64,-64,-64,64,64,-64,-64};

int8_t t2[32];
int8_t t3[32];
int8_t t4[32];
int8_t t5[32];
int8_t t6[32];
int8_t t7[32];
int8_t t8[32];

Oscil <SAW32_uint_NUM_CELLS, AUDIO_RATE> saw1(SAW32_DATA);
Oscil <SAW32_uint_NUM_CELLS, AUDIO_RATE> saw2(SAW32_DATA);
Oscil <SAW32_uint_NUM_CELLS, AUDIO_RATE> saw3(SAW32_DATA);
Oscil <SAW32_uint_NUM_CELLS, AUDIO_RATE> saw4(SAW32_DATA);

float freq1 = 110;
int voct = 1000;
float freqv1 = 440;

int harm_level_knob = 0;
int harm_type_knob = 0;

int harm_level = 0;
int harm_type = 0;

float volume_gain = 0;

int8_t harmonics_levels[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int8_t harmonics_levels_int[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float h_256_constant = 1.00/256.00;
float h_bigger_constant = 1.00/1024.00;

byte gain = 127;
bool gate = false;
bool gate_prev = false;
long adsr = 0;

bool octplus2 = false;
bool octminus2 = false;

bool adsr_to_vol_midi = false;
bool adsr_to_vol_cv = false;

bool adsr_to_harm_level = false;

int output_table[32];

long output = 0;

int temp = 0;
int temp2 = 0;
int temp3 = 0;

ADSR <CONTROL_RATE, CONTROL_RATE> envelope1;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope2;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope3;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope4;

boolean midi_gate_1 = false;
boolean midi_gate_2 = false;
boolean midi_gate_3 = false;
boolean midi_gate_4 = false;

boolean gates[4];
byte notes[4];
int envelopes[4];
unsigned int ages[4];
long harmlevels[4];
unsigned int volume_gains[4];
boolean midi_gates[4] = { false, false, false, false };


long outputs[4];

int agecounter = 0;

int cv_gate_counter = 0;

float portamento = 0;

int smoothedFreq = 0;

Smooth <unsigned int> aSmooth(portamento);

const static float voctpow[1024] PROGMEM = {
 0,  0.004882, 0.009765, 0.014648, 0.019531, 0.024414, 0.029296, 0.034179, 0.039062, 0.043945, 0.048828, 0.05371,  0.058593, 0.063476, 0.068359, 0.073242, 0.078125, 0.083007, 0.08789,  0.092773, 0.097656, 0.102539, 0.107421, 0.112304, 0.117187, 0.12207,  0.126953, 0.131835, 0.136718, 0.141601, 0.146484, 0.151367, 0.15625,  0.161132, 0.166015, 0.170898, 0.175781, 0.180664, 0.185546, 0.190429, 0.195312, 0.200195, 0.205078, 0.20996,  0.214843, 0.219726, 0.224609, 0.229492, 0.234375, 0.239257, 0.24414,  0.249023, 0.253906, 0.258789, 0.263671, 0.268554, 0.273437, 0.27832,  0.283203, 0.288085, 0.292968, 0.297851, 0.302734, 0.307617, 0.3125, 0.317382, 0.322265, 0.327148, 0.332031, 0.336914, 0.341796, 0.346679, 0.351562, 0.356445, 0.361328, 0.36621,  0.371093, 0.375976, 0.380859, 0.385742, 0.390625, 0.395507, 0.40039,  0.405273, 0.410156, 0.415039, 0.419921, 0.424804, 0.429687, 0.43457,  0.439453, 0.444335, 0.449218, 0.454101, 0.458984, 0.463867, 0.46875,  0.473632, 0.478515, 0.483398, 0.488281, 0.493164, 0.498046, 0.502929, 0.507812, 0.512695, 0.517578, 0.52246,  0.527343, 0.532226, 0.537109, 0.541992, 0.546875, 0.551757, 0.55664,  0.561523, 0.566406, 0.571289, 0.576171, 0.581054, 0.585937, 0.59082,  0.595703, 0.600585, 0.605468, 0.610351, 0.615234, 0.620117, 0.625,  0.629882, 0.634765, 0.639648, 0.644531, 0.649414, 0.654296, 0.659179, 0.664062, 0.668945, 0.673828, 0.67871,  0.683593, 0.688476, 0.693359, 0.698242, 0.703125, 0.708007, 0.71289,  0.717773, 0.722656, 0.727539, 0.732421, 0.737304, 0.742187, 0.74707,  0.751953, 0.756835, 0.761718, 0.766601, 0.771484, 0.776367, 0.78125,  0.786132, 0.791015, 0.795898, 0.800781, 0.805664, 0.810546, 0.815429, 0.820312, 0.825195, 0.830078, 0.83496,  0.839843, 0.844726, 0.849609, 0.854492, 0.859375, 0.864257, 0.86914,  0.874023, 0.878906, 0.883789, 0.888671, 0.893554, 0.898437, 0.90332,  0.908203, 0.913085, 0.917968, 0.922851, 0.927734, 0.932617, 0.9375, 0.942382, 0.947265, 0.952148, 0.957031, 0.961914, 0.966796, 0.971679, 0.976562, 0.981445, 0.986328, 0.99121,  0.996093, 1.000976, 1.005859, 1.010742, 1.015625, 1.020507, 1.02539,  1.030273, 1.035156, 1.040039, 1.044921, 1.049804, 1.054687, 1.05957,  1.064453, 1.069335, 1.074218, 1.079101, 1.083984, 1.088867, 1.09375,  1.098632, 1.103515, 1.108398, 1.113281, 1.118164, 1.123046, 1.127929, 1.132812, 1.137695, 1.142578, 1.14746,  1.152343, 1.157226, 1.162109, 1.166992, 1.171875, 1.176757, 1.18164,  1.186523, 1.191406, 1.196289, 1.201171, 1.206054, 1.210937, 1.21582,  1.220703, 1.225585, 1.230468, 1.235351, 1.240234, 1.245117, 1.25, 1.254882, 1.259765, 1.264648, 1.269531, 1.274414, 1.279296, 1.284179, 1.289062, 1.293945, 1.298828, 1.30371,  1.308593, 1.313476, 1.318359, 1.323242, 1.328125, 1.333007, 1.33789,  1.342773, 1.347656, 1.352539, 1.357421, 1.362304, 1.367187, 1.37207,  1.376953, 1.381835, 1.386718, 1.391601, 1.396484, 1.401367, 1.40625,  1.411132, 1.416015, 1.420898, 1.425781, 1.430664, 1.435546, 1.440429, 1.445312, 1.450195, 1.455078, 1.45996,  1.464843, 1.469726, 1.474609, 1.479492, 1.484375, 1.489257, 1.49414,  1.499023, 1.503906, 1.508789, 1.513671, 1.518554, 1.523437, 1.52832,  1.533203, 1.538085, 1.542968, 1.547851, 1.552734, 1.557617, 1.5625, 1.567382, 1.572265, 1.577148, 1.582031, 1.586914, 1.591796, 1.596679, 1.601562, 1.606445, 1.611328, 1.61621,  1.621093, 1.625976, 1.630859, 1.635742, 1.640625, 1.645507, 1.65039,  1.655273, 1.660156, 1.665039, 1.669921, 1.674804, 1.679687, 1.68457,  1.689453, 1.694335, 1.699218, 1.704101, 1.708984, 1.713867, 1.71875,  1.723632, 1.728515, 1.733398, 1.738281, 1.743164, 1.748046, 1.752929, 1.757812, 1.762695, 1.767578, 1.77246,  1.777343, 1.782226, 1.787109, 1.791992, 1.796875, 1.801757, 1.80664,  1.811523, 1.816406, 1.821289, 1.826171, 1.831054, 1.835937, 1.84082,  1.845703, 1.850585, 1.855468, 1.860351, 1.865234, 1.870117, 1.875,  1.879882, 1.884765, 1.889648, 1.894531, 1.899414, 1.904296, 1.909179, 1.914062, 1.918945, 1.923828, 1.92871,  1.933593, 1.938476, 1.943359, 1.948242, 1.953125, 1.958007, 1.96289,  1.967773, 1.972656, 1.977539, 1.982421, 1.987304, 1.992187, 1.99707,  2.001953, 2.006835, 2.011718, 2.016601, 2.021484, 2.026367, 2.03125,  2.036132, 2.041015, 2.045898, 2.050781, 2.055664, 2.060546, 2.065429, 2.070312, 2.075195, 2.080078, 2.08496,  2.089843, 2.094726, 2.099609, 2.104492, 2.109375, 2.114257, 2.11914,  2.124023, 2.128906, 2.133789, 2.138671, 2.143554, 2.148437, 2.15332,  2.158203, 2.163085, 2.167968, 2.172851, 2.177734, 2.182617, 2.1875, 2.192382, 2.197265, 2.202148, 2.207031, 2.211914, 2.216796, 2.221679, 2.226562, 2.231445, 2.236328, 2.24121,  2.246093, 2.250976, 2.255859, 2.260742, 2.265625, 2.270507, 2.27539,  2.280273, 2.285156, 2.290039, 2.294921, 2.299804, 2.304687, 2.30957,  2.314453, 2.319335, 2.324218, 2.329101, 2.333984, 2.338867, 2.34375,  2.348632, 2.353515, 2.358398, 2.363281, 2.368164, 2.373046, 2.377929, 2.382812, 2.387695, 2.392578, 2.39746,  2.402343, 2.407226, 2.412109, 2.416992, 2.421875, 2.426757, 2.43164,  2.436523, 2.441406, 2.446289, 2.451171, 2.456054, 2.460937, 2.46582,  2.470703, 2.475585, 2.480468, 2.485351, 2.490234, 2.495117, 2.5,  2.504882, 2.509765, 2.514648, 2.519531, 2.524414, 2.529296, 2.534179, 2.539062, 2.543945, 2.548828, 2.55371,  2.558593, 2.563476, 2.568359, 2.573242, 2.578125, 2.583007, 2.58789,  2.592773, 2.597656, 2.602539, 2.607421, 2.612304, 2.617187, 2.62207,  2.626953, 2.631835, 2.636718, 2.641601, 2.646484, 2.651367, 2.65625,  2.661132, 2.666015, 2.670898, 2.675781, 2.680664, 2.685546, 2.690429, 2.695312, 2.700195, 2.705078, 2.70996,  2.714843, 2.719726, 2.724609, 2.729492, 2.734375, 2.739257, 2.74414,  2.749023, 2.753906, 2.758789, 2.763671, 2.768554, 2.773437, 2.77832,  2.783203, 2.788085, 2.792968, 2.797851, 2.802734, 2.807617, 2.8125, 2.817382, 2.822265, 2.827148, 2.832031, 2.836914, 2.841796, 2.846679, 2.851562, 2.856445, 2.861328, 2.86621,  2.871093, 2.875976, 2.880859, 2.885742, 2.890625, 2.895507, 2.90039,  2.905273, 2.910156, 2.915039, 2.919921, 2.924804, 2.929687, 2.93457,  2.939453, 2.944335, 2.949218, 2.954101, 2.958984, 2.963867, 2.96875,  2.973632, 2.978515, 2.983398, 2.988281, 2.993164, 2.998046, 3.002929, 3.007812, 3.012695, 3.017578, 3.02246,  3.027343, 3.032226, 3.037109, 3.041992, 3.046875, 3.051757, 3.05664,  3.061523, 3.066406, 3.071289, 3.076171, 3.081054, 3.085937, 3.09082,  3.095703, 3.100585, 3.105468, 3.110351, 3.115234, 3.120117, 3.125,  3.129882, 3.134765, 3.139648, 3.144531, 3.149414, 3.154296, 3.159179, 3.164062, 3.168945, 3.173828, 3.17871,  3.183593, 3.188476, 3.193359, 3.198242, 3.203125, 3.208007, 3.21289,  3.217773, 3.222656, 3.227539, 3.232421, 3.237304, 3.242187, 3.24707,  3.251953, 3.256835, 3.261718, 3.266601, 3.271484, 3.276367, 3.28125,  3.286132, 3.291015, 3.295898, 3.300781, 3.305664, 3.310546, 3.315429, 3.320312, 3.325195, 3.330078, 3.33496,  3.339843, 3.344726, 3.349609, 3.354492, 3.359375, 3.364257, 3.36914,  3.374023, 3.378906, 3.383789, 3.388671, 3.393554, 3.398437, 3.40332,  3.408203, 3.413085, 3.417968, 3.422851, 3.427734, 3.432617, 3.4375, 3.442382, 3.447265, 3.452148, 3.457031, 3.461914, 3.466796, 3.471679, 3.476562, 3.481445, 3.486328, 3.49121,  3.496093, 3.500976, 3.505859, 3.510742, 3.515625, 3.520507, 3.52539,  3.530273, 3.535156, 3.540039, 3.544921, 3.549804, 3.554687, 3.55957,  3.564453, 3.569335, 3.574218, 3.579101, 3.583984, 3.588867, 3.59375,  3.598632, 3.603515, 3.608398, 3.613281, 3.618164, 3.623046, 3.627929, 3.632812, 3.637695, 3.642578, 3.64746,  3.652343, 3.657226, 3.662109, 3.666992, 3.671875, 3.676757, 3.68164,  3.686523, 3.691406, 3.696289, 3.701171, 3.706054, 3.710937, 3.71582,  3.720703, 3.725585, 3.730468, 3.735351, 3.740234, 3.745117, 3.75, 3.754882, 3.759765, 3.764648, 3.769531, 3.774414, 3.779296, 3.784179, 3.789062, 3.793945, 3.798828, 3.80371,  3.808593, 3.813476, 3.818359, 3.823242, 3.828125, 3.833007, 3.83789,  3.842773, 3.847656, 3.852539, 3.857421, 3.862304, 3.867187, 3.87207,  3.876953, 3.881835, 3.886718, 3.891601, 3.896484, 3.901367, 3.90625,  3.911132, 3.916015, 3.920898, 3.925781, 3.930664, 3.935546, 3.940429, 3.945312, 3.950195, 3.955078, 3.95996,  3.964843, 3.969726, 3.974609, 3.979492, 3.984375, 3.989257, 3.99414,  3.999023, 4.003906, 4.008789, 4.013671, 4.018554, 4.023437, 4.02832,  4.033203, 4.038085, 4.042968, 4.047851, 4.052734, 4.057617, 4.0625, 4.067382, 4.072265, 4.077148, 4.082031, 4.086914, 4.091796, 4.096679, 4.101562, 4.106445, 4.111328, 4.11621,  4.121093, 4.125976, 4.130859, 4.135742, 4.140625, 4.145507, 4.15039,  4.155273, 4.160156, 4.165039, 4.169921, 4.174804, 4.179687, 4.18457,  4.189453, 4.194335, 4.199218, 4.204101, 4.208984, 4.213867, 4.21875,  4.223632, 4.228515, 4.233398, 4.238281, 4.243164, 4.248046, 4.252929, 4.257812, 4.262695, 4.267578, 4.27246,  4.277343, 4.282226, 4.287109, 4.291992, 4.296875, 4.301757, 4.30664,  4.311523, 4.316406, 4.321289, 4.326171, 4.331054, 4.335937, 4.34082,  4.345703, 4.350585, 4.355468, 4.360351, 4.365234, 4.370117, 4.375,  4.379882, 4.384765, 4.389648, 4.394531, 4.399414, 4.404296, 4.409179, 4.414062, 4.418945, 4.423828, 4.42871,  4.433593, 4.438476, 4.443359, 4.448242, 4.453125, 4.458007, 4.46289,  4.467773, 4.472656, 4.477539, 4.482421, 4.487304, 4.492187, 4.49707,  4.501953, 4.506835, 4.511718, 4.516601, 4.521484, 4.526367, 4.53125,  4.536132, 4.541015, 4.545898, 4.550781, 4.555664, 4.560546, 4.565429, 4.570312, 4.575195, 4.580078, 4.58496,  4.589843, 4.594726, 4.599609, 4.604492, 4.609375, 4.614257, 4.61914,  4.624023, 4.628906, 4.633789, 4.638671, 4.643554, 4.648437, 4.65332,  4.658203, 4.663085, 4.667968, 4.672851, 4.677734, 4.682617, 4.6875, 4.692382, 4.697265, 4.702148, 4.707031, 4.711914, 4.716796, 4.721679, 4.726562, 4.731445, 4.736328, 4.74121,  4.746093, 4.750976, 4.755859, 4.760742, 4.765625, 4.770507, 4.77539,  4.780273, 4.785156, 4.790039, 4.794921, 4.799804, 4.804687, 4.80957,  4.814453, 4.819335, 4.824218, 4.829101, 4.833984, 4.838867, 4.84375,  4.848632, 4.853515, 4.858398, 4.863281, 4.868164, 4.873046, 4.877929, 4.882812, 4.887695, 4.892578, 4.89746,  4.902343, 4.907226, 4.912109, 4.916992, 4.921875, 4.926757, 4.93164,  4.936523, 4.941406, 4.946289, 4.951171, 4.956054, 4.960937, 4.96582,  4.970703, 4.975585, 4.980468, 4.985351, 4.990234, 4.995117
};


void setup()
{
  pinMode(GATE_PIN, INPUT);
  pinMode(OCTAVE_UP_PIN, INPUT);
  pinMode(OCTAVE_DOWN_PIN, INPUT);
  pinMode(HARMONICS_TYPE_PIN, INPUT);
  pinMode(HARMONICS_LEVEL_PIN, INPUT);
  pinMode(ADSR_TO_VOL_MIDI_PIN, INPUT);
  pinMode(ADSR_TO_VOL_CV_PIN, INPUT);
  pinMode(ADSR_TO_HARM_LEVEL_PIN, INPUT);

  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);

  MIDI.begin(MIDI_CHANNEL_OMNI);

  startMozzi(CONTROL_RATE);
  
}

unsigned int duration, attack, decay, sustain, release_ms;

void HandleNoteOn(byte channel, byte note, byte velocity) {
  int lowest_age = agecounter;
  int oldest = 0;
  int lowest_age_non_playing = agecounter;
  int oldest_non_playing = -1;
  for (int i = 0; i < 4; i++) {
    if (ages[i] < lowest_age) {
      lowest_age = ages[i];
      oldest = i;
    }
    if (ages[i] < lowest_age_non_playing && !midi_gates[i]) {
      lowest_age_non_playing = ages[i];
      oldest_non_playing = i;
    }
  }

  if (oldest_non_playing != -1) {
    oldest = oldest_non_playing;
  }

  agecounter++;
  if (oldest == 0) {
    saw1.setFreq(mtof(float(note)));
    envelope1.noteOn(true);
    notes[0] = note;
    ages[0] = agecounter;
    midi_gates[0] = true;
  }
  if (oldest == 1) {
    saw2.setFreq(mtof(float(note)));
    envelope2.noteOn(true);
    notes[1] = note;
    ages[1] = agecounter;
    midi_gates[1] = true;
  }
  if (oldest == 2) {
    saw3.setFreq(mtof(float(note)));
    envelope3.noteOn(true);
    notes[2] = note;
    ages[2] = agecounter;
    midi_gates[2] = true;
  }
  if (oldest == 3) {
    saw4.setFreq(mtof(float(note)));
    envelope4.noteOn(true);
    notes[3] = note;
    ages[3] = agecounter;
    midi_gates[3] = true;
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  if (notes[0] == note) {
    envelope1.noteOff();
    //ages[0] = 0;
    midi_gates[0] = false;
  }
  if (notes[1] == note) {
    envelope2.noteOff();
    //ages[1] = 0;
    midi_gates[1] = false;
  }
  if (notes[2] == note) {
    envelope3.noteOff();
    //ages[2] = 0;
    midi_gates[2] = false;
  }
  if (notes[3] == note) {
    envelope4.noteOff();
    //ages[3] = 0;
    midi_gates[3] = false;
  }

  if (agecounter > 30000) {
    agecounter = 0;
  }
}


void updateControl() {
  MIDI.read();
  
  octplus2 = digitalRead(OCTAVE_UP_PIN);
  octminus2 = digitalRead(OCTAVE_DOWN_PIN);
  adsr_to_vol_midi = digitalRead(ADSR_TO_VOL_MIDI_PIN);
  adsr_to_vol_cv = digitalRead(ADSR_TO_VOL_CV_PIN);
  adsr_to_harm_level = digitalRead(ADSR_TO_HARM_LEVEL_PIN);
  harm_type_knob = mozziAnalogRead(HARMONICS_TYPE_PIN);
  harm_level_knob = mozziAnalogRead(HARMONICS_LEVEL_PIN);

  portamento = (float) mozziAnalogRead(PORTAMENTO_PIN);

  portamento *= h_bigger_constant;
  aSmooth.setSmoothness(portamento);

  gate_prev = gate;
  
  if (digitalRead(GATE_PIN)) {
    gate = true;
  } else {
    gate = false;
  }

  harm_level = harm_level_knob >> 3; 
  harm_type = harm_type_knob;

  voct = mozziAnalogRead(VOCT_PIN);
  
  if (octplus2) {
    freq1 = 261.63;
  } else if (octminus2) {
    freq1 = 16.35;
  } else {
    freq1 = 65.41;
  }

  temp = mozziAnalogRead(3);
  if (temp < 500) {
    attack = temp * 2;
  } else {
    attack = map(temp, 500, 1023, 1000, 10000);
  }

  temp = mozziAnalogRead(4);
  if (temp < 500) {
    decay = temp * 2;
  } else {
    decay = map(temp, 500, 1023, 1000, 10000);
  }

  temp = mozziAnalogRead(6);
  if (temp < 500) {
    release_ms = temp * 2;
  } else {
    release_ms = map(temp, 500, 1023, 1000, 10000);
  }

    sustain = mozziAnalogRead(5) >> 2;

    envelope1.setLevels(255,sustain,sustain,0);
    envelope1.setTimes(attack,decay,64000,release_ms);
    envelope1.setSustainLevel(sustain);

    envelope2.setLevels(255,sustain,sustain,0);
    envelope2.setTimes(attack,decay,64000,release_ms);
    envelope2.setSustainLevel(sustain);

    envelope3.setLevels(255,sustain,sustain,0);
    envelope3.setTimes(attack,decay,64000,release_ms);
    envelope3.setSustainLevel(sustain);

    envelope4.setLevels(255,sustain,sustain,0);
    envelope4.setTimes(attack,decay,64000,release_ms);
    envelope4.setSustainLevel(sustain);

    if (gate || adsr_to_vol_cv) {
      
      int tempfreq = freq1 * pow(2, (pgm_read_float(&(voctpow[voct]))));

      if (portamento < 0.1) {
        freqv1 = tempfreq;
      } else {
        freqv1 = aSmooth.next(tempfreq);
      }
    
      switch(cv_gate_counter) {
        case 0:
          saw1.setFreq(freqv1);
          break;
        case 1:
          saw2.setFreq(freqv1);
          break;
        case 2:
          saw3.setFreq(freqv1);
          break;
        case 3:
          saw4.setFreq(freqv1);
          break;
      }
    }

    if (gate && !gate_prev) {

      if (adsr_to_vol_cv) {
        cv_gate_counter = 0;
        envelope2.noteOff();
        envelope3.noteOff();
        envelope4.noteOff();
      }

      switch(cv_gate_counter) {
        case 0:
          envelope1.noteOn();
          break;
        case 1:
          envelope2.noteOn();
          break;
        case 2:
          envelope3.noteOn();
          break;
        case 3:
          envelope4.noteOn();
          break;
      }
    }
  
    if (!gate && gate_prev) {
      switch(cv_gate_counter) {
        case 0:
          envelope1.noteOff();
          cv_gate_counter = 1;
          break;
        case 1:
          envelope2.noteOff();
          cv_gate_counter = 2;
          break;
        case 2:
          envelope3.noteOff();
          cv_gate_counter = 3;
          break;
        case 3:
          envelope4.noteOff();
          cv_gate_counter = 0;
          break;
        default:
          cv_gate_counter = 0;
      }
      if (portamento > 0.1 || adsr_to_vol_cv) {
        cv_gate_counter = 0;
      }
    }

    envelope1.update();
    envelope2.update();
    envelope3.update();
    envelope4.update();

    envelopes[0] = envelope1.next();
    envelopes[1] = envelope2.next();
    envelopes[2] = envelope3.next();
    envelopes[3] = envelope4.next();
    
    volume_gain = adsr * h_256_constant;      

  if (adsr_to_harm_level) {
    harmlevels[0] = ((long) envelopes[0] * harm_level) >> 8;
    harmlevels[1] = ((long) envelopes[1] * harm_level) >> 8;
    harmlevels[2] = ((long) envelopes[2] * harm_level) >> 8;
    harmlevels[3] = ((long) envelopes[3] * harm_level) >> 8;
  } else {
    harmlevels[0] = harm_level;
    harmlevels[1] = harm_level;
    harmlevels[2] = harm_level;
    harmlevels[3] = harm_level;
  }

  if (adsr_to_vol_cv) {
    envelopes[0] = 255;
    envelopes[1] = 0;
    envelopes[2] = 0;
    envelopes[3] = 0;
  }


  if (adsr_to_vol_midi && midi_gates[0]) {
    envelopes[0] = 255;
  }

  if (adsr_to_vol_midi && midi_gates[1]) {
    envelopes[1] = 255;
  }

  if (adsr_to_vol_midi && midi_gates[2]) {
    envelopes[2] = 255;
  }

  if (adsr_to_vol_midi && midi_gates[3]) {
    envelopes[3] = 255;
  }

  harmonics_levels_int[0] = 127;
  harmonics_levels_int[1] = 0;
  harmonics_levels_int[2] = 0;
  harmonics_levels_int[3] = 0;
  harmonics_levels_int[4] = 0;
  harmonics_levels_int[5] = 0;
  harmonics_levels_int[6] = 0;
  harmonics_levels_int[7] = 0;
  
  if (harm_type >= 896) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[2] = harm_type - 896;
    harmonics_levels_int[4] = harm_type - 896;
    harmonics_levels_int[5] = 127 - (harm_type - 896);
    harmonics_levels_int[6] = 127;
    harmonics_levels_int[7] = 127;
  } else if (harm_type >= 768) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[5] = 127;
    harmonics_levels_int[6] = 127;
    harmonics_levels_int[7] = harm_type - 768;
  } else if (harm_type >= 640) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[2] = 127 - (harm_type - 639);
    harmonics_levels_int[3] = 127 - (harm_type - 639);
    harmonics_levels_int[5] = 127;
    harmonics_levels_int[6] = harm_type - 639;
  } else if (harm_type >= 512) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[1] = 127 - (harm_type - 512);
    harmonics_levels_int[2] = 127;
    harmonics_levels_int[3] = 127;
    harmonics_levels_int[4] = 127 - (harm_type - 512);
    harmonics_levels_int[5] = (harm_type - 512);
  } else if (harm_type >= 384) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[1] = 127;
    harmonics_levels_int[2] = 127;
    harmonics_levels_int[3] = 127;
    harmonics_levels_int[4] = (harm_type - 384);
  } else if (harm_type >= 256) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[1] = 127;
    harmonics_levels_int[2] = 127;
    harmonics_levels_int[3] = (harm_type - 256);
  } else if (harm_type >= 128) {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[1] = 127;
    harmonics_levels_int[2] = (harm_type - 128);
  } else {
    //harmonics_levels_int[0] = 127;
    harmonics_levels_int[1] = harm_type;
  }

  for (int i = 0; i < 32 ; i++) {
    t2[i] = (oma_wavetable2[i] * harmonics_levels_int[1]) >> 8;
    t3[i] = (oma_wavetable3[i] * harmonics_levels_int[2]) >> 8;  
    t4[i] = (oma_wavetable4[i] * harmonics_levels_int[3]) >> 8;
    t5[i] = (oma_wavetable5[i] * harmonics_levels_int[4]) >> 8;
    t6[i] = (oma_wavetable6[i] * harmonics_levels_int[5]) >> 8;
    t7[i] = (oma_wavetable7[i] * harmonics_levels_int[6]) >> 8;
    t8[i] = (oma_wavetable8[i] * harmonics_levels_int[7]) >> 8;
    output_table[i] = t2[i] + t3[i] + t4[i] + t5[i] + t6[i] + t7[i] + t8[i]; 
  }

}

int updateAudio() {

  int tempz[4] = { saw1.next(), saw2.next(), saw3.next(), saw4.next() };
/*
  output = (oma_wavetable1[tempz[0]] + ((output_table[tempz[0]] * harmlevels[0]) >> 8)) * envelopes[0];
  output += (oma_wavetable1[tempz[1]] + ((output_table[tempz[1]] * harmlevels[1]) >> 8)) * envelopes[1];
  output += (oma_wavetable1[tempz[2]] + ((output_table[tempz[2]] * harmlevels[2]) >> 8)) * envelopes[2];
  output += (oma_wavetable1[tempz[3]] + ((output_table[tempz[3]] * harmlevels[3]) >> 8)) * envelopes[3];
*/
/*
  int foo = harmlevels[0];
  output = (((oma_wavetable1[tempz[0]] >> 2) + (((output_table[tempz[0]] *foo)>> 10))) * envelopes[0]) >> 8;
  foo = harmlevels[1];
  output += (((oma_wavetable1[tempz[1]] >> 2) + (((output_table[tempz[1]] *foo)>> 10))) * envelopes[1]) >> 8;
  foo = harmlevels[2];
  output += (((oma_wavetable1[tempz[2]] >> 2) + (((output_table[tempz[2]] *foo)>> 10))) * envelopes[2]) >> 8;
  foo = harmlevels[3];
  output += (((oma_wavetable1[tempz[3]] >> 2) + (((output_table[tempz[3]] *foo)>> 10))) * envelopes[3]) >> 8;
*/
  // harmlevels[0] = 0 - 127
  // output_table[tempz[0]] = = -128 - +127

  int outputs[4];

  for (int i = 0; i < 4 ; i++) {
    int temp_harmlevel = harmlevels[i];
    int temp_harmonics = output_table[tempz[i]] >> 1;
    int basefreq = oma_wavetable1[tempz[i]] >> 2;
    int temp_harmonics_leveled = (temp_harmonics * temp_harmlevel) >> 8;
    basefreq = basefreq * envelopes[i];
    temp_harmonics_leveled = temp_harmonics_leveled * envelopes[i];
    int temp_output = ((basefreq + temp_harmonics_leveled)) >> 4;

    outputs[i] = temp_output;
  }

  output = (outputs[0] + outputs[1] + outputs[2] + outputs[3]);
  
  return (int) output;
}
void loop() {

     audioHook();
}
