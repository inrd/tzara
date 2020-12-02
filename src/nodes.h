#ifndef TZARA_NODES_H
#define TZARA_NODES_H

#include <stdlib.h>
#include <string.h>

#define TZNODE_MAX_INPUTS 16 
#define TZNODE_MAX_OUTPUTS 16
#define TZNODE_MEMORY_SIZE 32
#define TZNODE_NAME_SIZE 256


enum NodeTypes {
    INVALID_NODE_TYPE = 0,
    ADDER_NODE,
    MULT_NODE,
    CLIP_NODE,
    MIX_NODE,
    MEM_NODE,
    PHASOR_NODE,
    SINOSC_NODE,
    NUM_NODE_TYPES
};



typedef struct TzNodeDoc TzNodeDoc;
struct TzNodeDoc {
    const char* name;
    const char* summary;
    const char* inputs;
    const char* outputs;
};

static TzNodeDoc nodesDoc [] = {
    {"-", "-", "-", "-"},
    {"add", "outputs the sum of its inputs.", "in1, in2", "out"},
    {"mult", "outputs the product of its inputs.", "in1 in2", "out"},
    {"clip", "clips {in} in range [{min}..{max}].", "in min max", "out"},
    {"mix", "interpolates between {in1} and {in2} according to {coeff} in range [0..1].", "in1 in2 coeff", "out"},
    {"mem", "1 sample delay.", "in", "out"},
    {"phasor", "generates a ramp in the range [0..1].", "freq(Hz)", "out"},
    {"sinosc", "generates a sine wave.", "freq(Hz)", "out"}
};

typedef struct TzProcessInfo TzProcessInfo;
struct TzProcessInfo {
    float samplerate;
};

typedef struct TzNode TzNode;
struct TzNode {
    float* inputs[TZNODE_MAX_INPUTS];
    int numInputs;
    float outputs[TZNODE_MAX_OUTPUTS];
    int numOutputs;
    void (*perform)(TzNode*, TzProcessInfo*);
    float memory[TZNODE_MEMORY_SIZE];
    char name[TZNODE_NAME_SIZE];
    char inputsNames[TZNODE_MAX_INPUTS][TZNODE_NAME_SIZE];
    char outputsNames[TZNODE_MAX_OUTPUTS][TZNODE_NAME_SIZE];
}; 

void flush (TzNode* n);

TzNode* allocateNewNode ();

float getNodeInput (TzNode* n, int inputIndex, float defaultValue);

/* =========================================== */


void performAdder (TzNode* n, TzProcessInfo* info);
TzNode* createAdderNode ();

void performMult (TzNode* n, TzProcessInfo* info);
TzNode* createMultNode ();

void performClip (TzNode* n, TzProcessInfo* info);
TzNode* createClipNode ();

void performMix (TzNode* n, TzProcessInfo* info);
TzNode* createMixNode ();

void performMem (TzNode* n, TzProcessInfo* info);
TzNode* createMemNode ();


/* =========================================== */

void performConstant (TzNode* n, TzProcessInfo* info);
TzNode* createConstantNode (float val);

/* =========================================== */


void performPhasor (TzNode* n, TzProcessInfo* info);
TzNode* createPhasorNode ();

void performSinosc (TzNode* n, TzProcessInfo* info);
TzNode* createSinoscNode ();

#endif
