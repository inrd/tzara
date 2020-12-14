/*
MIT License

Copyright (c) 2020 Thomas Hennebert

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next paragraph) shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * A collection of dsp functions used by the Tzara engine
 *
 * Most of these routines are not optimized for realtime processing
 * but they can be used as a basis to code optimized versions.
 *
 */

#ifndef TZDSP_H
#define TZDSP_H

typedef struct TZSvfOutputs TZSvfOutputs;

void tzSwap (float* a, float* b);

float tzClip (float val, float min, float max);

float tzWrap (float val, float min, float max);

float tzMin (float a, float b);

float tzMax (float a, float b);

int tzRoundToInt (float val);

/* returns only the fractional part of val */
float tzFrac (float val);

/* ratio is expected to be in range [0..1] */
float tzLinInterp (float a, float b, float ratio);

/* maps val from the range [min..max] to the range [0..1] */
float tzMapTo0_1 (float val, float min, float max);

/* maps val from the range [0..1] to the range [min..max] */
float tzMapFrom0_1 (float val, float min, float max);

/* maps val from the range [imin..imax] to the range [omin..omax] */
float tzMapToRange (float val, float imin, float imax, float omin, float omax);

float tzMsToSamples (float ms, float samplerate);

/* accepts float value for fine tuning */
float tzMIDIToFreq (float note);

float tzDecibelsToAmp (float db);

float tzMsToHz (float ms);

float tzHzToMs (float hz);

int tzConformNoteToScale (int note, const int scale[12], int root);

void tzFixDenormals (float* x);

void tzFixNaN (float* x);

void tzGenRandomSeed ();
void tzSetRandomSeed (unsigned int seed);

/* returns a random value in range [0..1] */
float tzRandom ();

/* returns a random floating point value in range [min..max] */
float tzRandomFloat (float min, float max);

/* returns a random integer value in range [min..max] */
int tzRandomInt (int min, int max);

float tzWhiteNoise ();

/* smooth value changes requires an external variable to store the value */
void tzSmooth (float* val, float target, float rampTimeMs, float samplerate);

/* requires an external variable to store the phase */
float tzPhasor (float freq, float samplerate, float* phase);

/* requires an external variable to store the phase */
float tzSinewave (float freq, float samplerate, float* phase);

/* polyblep oscillators : based on http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/ */

/* requires an external variable to store the phase */
float tzPolyblepSaw (float freq, float samplerate, float* phase);

/* requires an external variable to store the phase */
/* pw in range [0..1] */
float tzPolyblepSquare (float freq, float pw, float samplerate, float* phase);

/* requires external variables to store the phase and the previous output (z1) */
/* pw in range [0..1] */
float tzPolyblepTriangle (float freq, float pw, float samplerate, float* phase, float* z1);

/* requires an external variable to store z1 */
float tzOnePoleLowpass (float in, float cut, float samplerate, float* z1);

/* requires an external variable to store z1 */
float tzOnePoleHighpass (float in, float cut, float samplerate, float* z1);


struct TZSvfOutputs {
    float lowpass;
    float bandpass;
    float highpass;
    float notch;
};

/* SVF based on Andrew Simper's paper : https://cytomic.com/index.php?q=technical-papers */
/* requires external variables to store ic1eq and ic2eq */
/* returns 4 outputs at once */
/* resonance in range [0..1] */
TZSvfOutputs tzStateVariableFilter (float in, float cut, float res, float samplerate, float* ic1eq, float* ic2eq);

/* requires an externally allocated delay buffer and an external variable to store pos */
/* maxpos = delay buffer length minus 1 */
float tzDelay (float in, float timeMs, float samplerate, float* delayBuf, int maxPos, float* pos);

/* Same as tzDelay but with an internal feedback path */
/* requires an externally allocated delay buffer and an external variable to store pos */
/* maxpos = delay buffer length minus 1 */
/* feedback in range [0..1] */
float tzFeedbackDelay (float in, float timeMs, float feedback, float samplerate, float* delayBuf, int maxPos, float* pos);

/* requires an externally allocated delay buffer and an external variable to store pos */
/* maxpos = delay buffer length minus 1 */
/* gain in range [0..1] */
float tzAllpassDelay (float in, float timeMs, float gain, float samplerate, float* delayBuf, int maxPos, float* pos);

#endif
