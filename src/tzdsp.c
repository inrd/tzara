#include "tzdsp.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>


void tzSwap (float* a, float* b) {
    float tmp = *a;
    *a = *b;
    *b = tmp;
}


float tzClip (float val, float min, float max) {
    if (min > max) tzSwap(&min, &max);

    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}


float tzWrap (float val, float min, float max) {
    float rng = max - min;
    if (rng == 0.f) return min;

    if (rng < 0.f) tzSwap(&min, &max);

    while (val < min) val += rng;
    while (val > max) val -= rng;
    return val;
}


float tzMin (float a, float b) {
    return a < b ? a : b;
}


float tzMax (float a, float b) {
    return a > b ? a : b;
}


int tzRoundToInt (float val) {
    return val < 0.f ? (int)(val - 0.5) : (int)(val + 0.5);
}

float tzFrac (float val) {
    float av = fabs(val);
    return av - (float)((int)av);
}


float tzLinInterp (float a, float b, float ratio) {
    return a + ratio * (b - a);
}


float tzMapTo0_1 (float val, float min, float max) {
    if (min == max) return 0.f;
    if (min > max) tzSwap(&min, &max);
    val = tzClip(val, min, max);

    return (val - min) / (max - min);
}


float tzMapFrom0_1 (float val, float min, float max) {
    if (min == max) return min;
    if (min > max) tzSwap(&min, &max);

    return min + (val * (max - min));
}


float tzMapToRange (float val, float imin, float imax, float omin, float omax, float curve) {
    val = tzMapTo0_1(val, imin, imax);
    val = curve > 0.f ? pow(val, curve) : 0.f;
    return tzMapFrom0_1(val, omin, omax);
}


float tzMsToSamples (float ms, float samplerate) {
    return ms * 0.001f * samplerate;
}


float tzMIDIToFreq (float note) {
    return 440.f * pow(2.f, (note - 69.f) / 12.f);
}


float tzDecibelsToAmp (float db) {
    return pow(10.f, db / 20.f);
}


float tzMsToHz (float ms) {
    return ms != 0.f ? 1000.f / ms : 0.f;
}


float tzHzToMs (float hz) {
    return hz != 0.f ? 1000.f / hz : 0.f;
}


int tzConformNoteToScale (int note, const int scale[12], int root) {
    const int oct =  note / 12;
    const int base = note % 12;
    const int offset = base + root;
    const int octshift = offset / 12;
    const int conformed = scale[offset % 12] - root;

    return conformed + ((oct + octshift) * 12);
}


void tzFixDenormals (float* x) {
    static const float z = 1e-18;
    *x += z;
    *x -= z;
}


void tzFixNaN (float* x) {
    if (*x != *x) *x = 0.f;
}


void tzGenRandomSeed () {
    srand((unsigned int)time(NULL));
}


void tzSetRandomSeed (unsigned int seed) {
    srand(seed);
}


float tzRandom () {
    return (float)rand()/(float)(RAND_MAX);
}


float tzRandomFloat (float min, float max) {
    return tzMapFrom0_1(tzRandom(), min, max);
}


int tzRandomInt (int min, int max) {
    return tzRoundToInt(tzRandomFloat((float)min, (float)max));
}


float tzWhiteNoise () {
    return tzRandomFloat(-1.f, 1.f);
}


void tzSmooth (float* val, float target, float rampTimeMs, float samplerate) {
    float dur = tzMsToSamples(rampTimeMs, samplerate);
    float delta = (target - *val) / dur;

    if (delta >= 0.f) {
        *val += delta;
        if (*val > target) *val = target;
    }
    else {
        *val += delta;
        if (*val < target) *val = target;
    }
}


float tzPhasor (float freq, float samplerate, float* phase) {
    const float out = *phase;

    *phase += (freq / samplerate);
    *phase = tzWrap(*phase, 0.f, 1.f);

    return out;
}


float tzSinewave (float freq, float samplerate, float* phase) {
    const float out = sin(*phase);
    const float twopi = 2.f * M_PI;

    *phase += freq * twopi / samplerate;
    *phase = tzWrap(*phase, 0.f, twopi);

    return out;
}


float tzCalculatePolyblep (float phaseIncr, float t) {
    float dt = phaseIncr / (2.f * M_PI);

    if (t < dt) {
        t = t / dt;
        return (t + t) - (t * t) - 1.f;
    }
    else if (t > (1.f - dt)) {
        t = (t - 1.f) / dt;
        return (t * t) + (t + t) + 1.f;
    }
    else {
        return 0.f;
    }
}


float tzPolyblepSaw (float freq, float samplerate, float* phase) {
    const float twopi = 2.f * M_PI;
    float incr = freq * twopi / samplerate;
    float t = *phase / twopi;
    float pblep = tzCalculatePolyblep(incr, t);
    float wv = (2.f * (*phase) / twopi) - 1.f;

    wv -= pblep;

    *phase += incr;
    *phase = tzWrap(*phase, 0.f, twopi);

    return wv;
}


float tzPolyblepSquare (float freq, float pw, float samplerate, float* phase) {
    const float twopi = 2.f * M_PI;
    float incr = freq * twopi / samplerate;
    float t1 = *phase / twopi;
    float t2 = fmod((t1 + pw), 1.f);
    float pblep1 = tzCalculatePolyblep(incr, t1);
    float pblep2 = tzCalculatePolyblep(incr, t2);
    float c = pw * twopi;
    float wv = 0.f;

    if (*phase < c) {
        wv = 1.f;
    }
    else {
        wv = -1.f;
    }
    wv += pblep1;
    wv -= pblep2;

    *phase += incr;
    *phase = tzWrap(*phase, 0.f, twopi);

    return wv;
}


float tzPolyblepTriangle (float freq, float pw, float samplerate, float* phase, float* z1) {
    const float twopi = 2.f * M_PI;
    float incr = freq * twopi / samplerate;
    float t1 = *phase / twopi;
    float t2 = fmod((t1 + pw), 1.f);
    float pblep1 = tzCalculatePolyblep(incr, t1);
    float pblep2 = tzCalculatePolyblep(incr, t2);
    float c = pw * twopi;
    float wv = 0.f;

    if (*phase < c) {
        wv = 1.f;
    }
    else {
        wv = -1.f;
    }
    wv += pblep1;
    wv -= pblep2;

    /* make triangle out of square */
    wv = (incr * wv) + ((1.f - incr) * (*z1));

    *z1 = wv;

    *phase += incr;
    *phase = tzWrap(*phase, 0.f, twopi);

    return wv;
}


float tzOnePoleLowpass (float in, float cut, float samplerate, float* z1) {
    const double costh = 2.0 - cos(2.0 * M_PI * (double)cut /  (double)samplerate);
    const double coeff = sqrt(costh * costh - 1.0) - costh;

    const double out = (double)in * (1.0 + coeff) - (double)(*z1) * coeff;
    *z1 = (float)out;

    return (float)out;
}


float tzOnePoleHighpass (float in, float cut, float samplerate, float* z1) {
    const double costh = 2.0 - cos(2.0 * M_PI * (double)cut /  (double)samplerate);
    const double coeff = costh - sqrt(costh * costh - 1.0);

    const double out = (double)in * (1.0 - coeff) - (double)(*z1) * coeff;
    *z1 = (float)out; 

    return out;
}


TZSvfOutputs tzStateVariableFilter (float in, float cut, float res, float samplerate, float* ic1eq, float* ic2eq) {
    const double v0 = (double)in;
    const double g = tan(M_PI * cut / (double)samplerate);
    const double k = 2.0 - 2.0 * (double)res;
    const double a1 = 1.0 / (1.0 + g * (g + k));
    const double a2 = g * a1;
    const double a3 = g * a2;
    const double v3 = v0 - (double)(*ic2eq);
    const double v1 = a1 * (double)(*ic1eq) + a2 * v3;
    const double v2 = (double)(*ic2eq) + a2 * (double)(*ic1eq) + a3 * v3;
    TZSvfOutputs out;

    *ic1eq = (float)(2.0 * v1 - (double)(*ic1eq)); 
    *ic2eq = (float)(2.0 * v2 - (double)(*ic2eq)); 

    out.lowpass = v2;
    out.bandpass = v1;
    out.highpass = v0 - k * v1 - v2;
    out.notch = v0 - k * v1;

    return out;
}


float tzReadFromCirularBuffer (float* buffer, int maxPos, float pos) {
    int ip = (int)pos;
    float frac = pos - (float)ip;
    int ip1 = ip + 1 > maxPos ? 0 : ip + 1;

    return tzLinInterp(buffer[ip], buffer[ip1], frac);
}


float tzDelay (float in, float timeMs, float samplerate, float* delayBuf, int maxPos, float* pos) {
    float stime = tzClip(tzMsToSamples(timeMs, samplerate), 0.f, (float)maxPos);
    float rp = *pos - stime;
    float out = 0.f;

    rp = tzWrap(rp, 0.f, (float)maxPos);
    out = tzReadFromCirularBuffer(delayBuf, maxPos, rp);

    delayBuf[(int)(*pos)] = in;
    ++(*pos);
    *pos = tzWrap(*pos, 0.f, (float)maxPos);

    return out;
}


float tzFeedbackDelay (float in, float timeMs, float feedback, float samplerate, float* delayBuf, int maxPos, float* pos) {
    float stime = tzClip(tzMsToSamples(timeMs, samplerate), 0.f, (float)maxPos);
    float rp = *pos - stime;
    float out = 0.f;

    rp = tzWrap(rp, 0.f, (float)maxPos);
    out = tzReadFromCirularBuffer(delayBuf, maxPos, rp);

    delayBuf[(int)(*pos)] = tanh(in + (out * feedback));
    ++(*pos);
    *pos = tzWrap(*pos, 0.f, (float)maxPos);

    return out;
}


float tzAllpassDelay (float in, float timeMs, float gain, float samplerate, float* delayBuf, int maxPos, float* pos) {
    float stime = tzClip(tzMsToSamples(timeMs, samplerate), 0.f, (float)maxPos);
    float rp = *pos - stime;
    float g = tzClip(gain, 0.f, 1.f);
    float fbck, ffwd = 0.f;
    float v = 0.f;
    float dl = 0.f;
    float out = 0.f;

    rp = tzWrap(rp, 0.f, (float)maxPos);
    dl = tzReadFromCirularBuffer(delayBuf, maxPos, rp);

    fbck = dl * (-g);
    v = in + fbck;
    ffwd = v * g;
    out = dl + ffwd;

    delayBuf[(int)(*pos)] = v;
    ++(*pos);
    *pos = tzWrap(*pos, 0.f, (float)maxPos);

    return out;
}


