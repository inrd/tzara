#include "nodes.h"

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

