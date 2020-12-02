#include "nodes.h"

#include <math.h>

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
    float* phase = n->memory;
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

void performSinosc (TzNode* n, TzProcessInfo* info) {
    const float samplerate = info->samplerate;
    const float twopi = 2.f * M_PI;
    const float freq = getNodeInput(n, 0, 440.f);
    const float incr = freq * twopi / samplerate;
    float* phase = n->memory;
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


