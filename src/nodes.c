#include "nodes.h"

#include <math.h>
#include <time.h>
#include <stdio.h>

#define TZ_UNUSED(x) (void)(x)


void flush (TzNode* n) {
    int i = 0;
    for (i  = 0; i < TZNODE_MAX_INPUTS; ++i) {
        n->inputs[i] = NULL;
    }
    for (i  = 0; i < TZNODE_MAX_OUTPUTS; ++i) {
        n->outputs[i] = 0.f;
    }
    for (i = 0; i < TZNODE_MEMORY_SIZE; ++i) {
        n->memory[i] = 0.f;
    }
    memset(n->name, '\0', TZNODE_NAME_SIZE);
    for (i = 0; i < TZNODE_MAX_INPUTS; ++i) {
        memset(n->inputsNames[i], '\0', TZNODE_NAME_SIZE);
    }
    for (i = 0; i < TZNODE_MAX_OUTPUTS; ++i) {
        memset(n->outputsNames[i], '\0', TZNODE_NAME_SIZE);
    }
}

TzNode* allocateNewNode () {
    TzNode* n = (TzNode*)(malloc(sizeof(TzNode)));
    flush(n);
    return n;
}

float getNodeInput (TzNode* n, int inputIndex, float defaultValue) {
    return n->inputs[inputIndex] != NULL ? *(n->inputs[inputIndex]) : defaultValue;
}


/* =========================== */

void performAdder (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 + in2;
}

TzNode* createAdderNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performAdder;
    return n;
}

void performSub (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 - in2;
}

TzNode* createSubNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performSub;
    return n;
}

void performMult (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 * in2;
}

TzNode* createMultNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMult;
    return n;
}

void performDiv (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in2 != 0.f ? in1 / in2 : 0.f;
}

TzNode* createDivNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performDiv;
    return n;
}

void performClip (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float in = getNodeInput(n, 0, 0.f);
    const float min = getNodeInput(n, 1, -1.f);
    const float max = getNodeInput(n, 2, 1.f);
    
    if (in < min) in = min;
    if (in > max) in = max;
    n->outputs[0] = in;
}

TzNode* createClipNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "min");
    strcpy(n->inputsNames[2], "max");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performClip;
    return n;
}

void performMix (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float a = getNodeInput(n, 0, 0.f);
    const float b = getNodeInput(n, 1, 0.f);
    float coeff = getNodeInput(n, 2, 0.f);
    
    if (coeff < 0.f) coeff = 0.f;
    if (coeff > 1.f) coeff = 1.f;
    n->outputs[0] = a + coeff * (b - a);
}

TzNode* createMixNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    strcpy(n->inputsNames[2], "coeff");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMix;
    return n;
}

void performMap (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in = getNodeInput(n, 0, 0.f);
    const float imin = getNodeInput(n, 1, 0.f);
    const float imax = getNodeInput(n, 2, 1.f);
    const float omin = getNodeInput(n, 3, 0.f);
    const float omax = getNodeInput(n, 4, 1.f);
    float tmp = 0.f;

    float* errCount = &(n->memory[0]);

    if ((imin >= imax)||(omin >= omax)||(in<imin)||(in>omax)) {
        /* invalid inputs */
        n->outputs[0] = 0.f;
        if ((int)(*errCount) < 10) {
            fprintf(stderr, "Warning : [%s] => invalid inputs, output set to 0...\n", n->name); 
            *errCount += 1;
        }
        else if ((int)(*errCount) == 10) {
            fprintf(stderr, "[%s] => too many warnings, no longer printing them!\n", n->name); 
            *errCount += 1;
        }
    }
    else {
        tmp = (in - imin) / (imax - imin);
        n->outputs[0] = omin + (tmp * (omax - omin));
    }
}

TzNode* createMapNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 5;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "imin");
    strcpy(n->inputsNames[2], "imax");
    strcpy(n->inputsNames[3], "omin");
    strcpy(n->inputsNames[4], "omax");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performMap;
    return n;
}


void performMem (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float z1 = n->memory[0];
    n->memory[0] = getNodeInput(n, 0, 0.f);

    n->outputs[0] = z1;
}

TzNode* createMemNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performMem;
    return n;
}

void performConstant (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    n->outputs[0] = n->memory[0];
}

TzNode* createConstantNode (float val) {
    TzNode* n = allocateNewNode();
    n->numInputs = 0;
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = val;
    n->perform = &performConstant;
    return n;
}

void performPhasor (TzNode* n, TzProcessInfo* info) {
    const float samplerate = info->samplerate;
    const float freq = getNodeInput(n, 0, 440.f);
    const float incr = freq / samplerate;
    float* phase = &(n->memory[0]);
    n->outputs[0] = *phase;
    *phase += incr;
    while (*phase > 1.f) *phase -= 1.f;
}

TzNode* createPhasorNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "freq");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performPhasor;
    return n;
}

void performPulse (TzNode* n, TzProcessInfo* info) {
    const float samplerate = info->samplerate;
    const float rate = getNodeInput(n, 0, 100.f);
    const float period = rate * 0.001f *samplerate;
    float* count = &(n->memory[0]);
    n->outputs[0] = (int)(*count) == 0 ? 1.f : 0.f;
    *count += 1.f;
    if (*count >= period) *count = 0.f;
}

TzNode* createPulseNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "rate");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performPulse;
    return n;
}

void performSinosc (TzNode* n, TzProcessInfo* info) {
    const float samplerate = info->samplerate;
    const float twopi = 2.f * M_PI;
    const float freq = getNodeInput(n, 0, 440.f);
    const float incr = freq * twopi / samplerate;
    float* phase = &(n->memory[0]);
    n->outputs[0] = sin(*phase);
    *phase += incr;
    while (*phase > twopi) *phase -= twopi;
}

TzNode* createSinoscNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "freq");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performSinosc;
    return n;
}

void performSeq8 (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float steps[8];
    const int clock = (int)getNodeInput(n, 8, 0.f);
    int length = (int)getNodeInput(n, 9, 8.f);
    int i = 0;

    for (i = 0; i < 8; ++i) {
        steps[i] = getNodeInput(n, i, 0.f);
    }
    if (length < 1) length = 1;
    if (length > 8) length = 8;

    float* pos = &(n->memory[0]);

    if (clock != 0) {
        *pos += 1.f;
        if ((int)(*pos) >= length) {
            *pos = 0.f;
        }
    }

    n->outputs[0] = steps[(int)(*pos)];
    n->outputs[1] = *pos;
}

TzNode* createSeq8Node () {
    int i = 0;
    char stepname[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 10;
    for (i = 0; i < 8; ++i) {
        sprintf(stepname, "step%d", i + 1);
        strncpy(n->inputsNames[i], stepname, strlen(stepname));
    }
    strcpy(n->inputsNames[8], "clock");
    strcpy(n->inputsNames[9], "length");
    n->numOutputs = 2;
    strcpy(n->outputsNames[0], "out");
    strcpy(n->outputsNames[1], "pos");
    n->memory[0] = 0.f;
    n->perform = &performSeq8;
    return n;
}

void performRandom (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const int clock = (int)getNodeInput(n, 0, 0.f);
    float* out = &(n->memory[0]);

    if (clock != 0) {
        *out = (float)rand()/(float)(RAND_MAX);
    }

    n->outputs[0] = *out;
}

TzNode* createRandomNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "clock");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performRandom;
    return n;
}



