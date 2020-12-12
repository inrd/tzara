#include "tzdsp.h"

#include <math.h>


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


float tzMapToRange (float val, float imin, float imax, float omin, float omax) {
    val = tzMapTo0_1(val, imin, imax);
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


void tzFixDenormals (float* x) {
    static const float z = 1e-18;
    *x += z;
    *x -= z;
}


void tzFixNaN (float* x) {
    if (*x != *x) *x = 0.f;
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


float tzPolyblepSaw (float freq, float samplerate, float* phase) {
    const float twopi = 2.f * M_PI;
    float incr = freq * twopi / samplerate;
    float dt = incr / twopi;
    float t = 0.f;
    float pblep = 0.f;
    float wv = 0.f;

    t = *phase / twopi;
    if (t < dt) {
        t = t / dt;
        pblep = (t + t) - (t * t) - 1.f;
    }
    else if (t > (1.f - dt)) {
        t = (t - 1.f) / dt;
        pblep = (t * t) + (t + t) + 1.f;
    }

    wv = (2.f * (*phase) / twopi) - 1.f;
    wv -= pblep;

    *phase += incr;
    *phase = tzWrap(*phase, 0.f, twopi);

    return wv;
}


float tzPolyblepSquare (float freq, float pw, float samplerate, float* phase) {
    const float twopi = 2.f * M_PI;
    float incr = freq + * twopi / samplerate;
    float dt = incr / twopi;
    float t0 = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;
    float pblep1 = 0.f;
    float pblep2 = 0.f;
    float c = 0.f;
    float wv = 0.f;

    t0 = *phase / twopi;
    t1 = t0;

    if (t1 < dt) {
        t1 = t1 / dt;
        pblep1 = (t1 + t1) - (t1 * t1) - 1.f;
    }
    else if (t1 > (1.f - dt)) {
        t1 = (t1 - 1.f) / dt;
        pblep1 = (t1 * t1) + (t1 + t1) + 1.f;
    }

    t2 = fmod((t0 + pw), 1.f);

    if (t2 < dt) {
        t2 = t2 / dt;
        pblep2 = (t2 + t2) - (t2 * t2) - 1.f;
    }
    else if (t2 > (1.f - dt)) {
        t2 = (t2 - 1.f) / dt;
        pblep2 = (t2 * t2) + (t2 + t2) + 1.f;
    }

    c = pw * twopi;
    if (*phase < c) {
        wv = 1.f;
    }
    else {
        wv = -1.f;
    }
    wv += pblep1;
    wv -= pblep2;

    *phase += incr;
    tzWrap(*phase, 0.f, twopi);

    return wv;
}

