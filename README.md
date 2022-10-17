# Harmoniqs
 4-voice additive harmonics oscillator with Arduino Nano

 This uses Mozzi library: https://github.com/sensorium/Mozzi
 Remember to change it into HIFI mode in mozzi_config.h and add saw32_uint8.h to tables -folder.

 This synth module got its inspiration from HAGIWO Additive VCO: https://note.com/solder_state/n/n30b3a8737b1e
 I wanted to make a slightly mellower and polyphonic version of it, with internal volume-envelope.
 But Arduino Nano has such miniscule space  and processing power, that it barely has enough power
 to do the output of that HAGIWO design, so I calculate new wavetable during the updateControl() interrupt
 and use that. Problem in that is that there still is very little time to do it, so the sinewave-tables
 are 32 entry long, so there is lot's of distortion and aliasing-type noise.

 TO DO:
 
 When switching to disable-gate-mode with CV, it can lock wrong voice to sound eternally,
 if multiple keys have been pressed before that.

 Does this actually set frequency of the oscillator as an integer? That would cause out-of-tuneness.
 Check how to set it as float or integer-with-digits.

 Switching +-2 octave switch back to neutral sometimes has lag, check if it needs pulldown resistor.

 Output filter for the pwm isn't good, it needs a redesign.

 Sometimes there is clipping-type distortion, check if it's internal or in the output stage.

 Check the schematic generally.

 Write guide for the controls.

 Comment code.

 Remove Finnish names from code.

 Optimize more.

 Make sound better.
