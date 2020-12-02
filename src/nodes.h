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
    SUB_NODE,
    MULT_NODE,
    DIV_NODE,
    CLIP_NODE,
    MIX_NODE,
    MAP_NODE,
    MEM_NODE,
    PHASOR_NODE,
    PULSE_NODE,
    SINOSC_NODE,
    SEQ8_NODE,
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
    {"add", "outputs {in1} + {in2}.", "in1, in2", "out"},
    {"sub", "outputs {in1} - {in2}.", "in1, in2", "out"},
    {"mult", "outputs {in1} * {in2}.", "in1, in2", "out"},
    {"div", "outputs {in1} / {in2}.", "in1, in2", "out"},
    {"clip", "clips {in} in range [{min}..{max}].", "in, min, max", "out"},
    {"mix", "interpolates between {in1} and {in2} according to {coeff} in range [0..1].", "in1, in2, coeff", "out"},
    {"map", "maps {in} from the range [{imin}..{imax}] to the range [{omin}..{omax}].", "in, imin, imax, omin, omax", "out"}, 
    {"mem", "1 sample delay.", "in", "out"},
    {"phasor", "generates a ramp in the range [0..1].", "freq(Hz)", "out"},
    {"pulse", "outputs a pulse at a periodic rate.", "rate(Ms)", "out"},
    {"sinosc", "generates a sine wave.", "freq(Hz)", "out"},
    {"seq8", "outputs the values of inputs {step1} to {step8} sequentially when receiving a pulse at {clock}. The sequence length can be changed via input {length}. The output {pos} sends the playhead position.", "clock(pulse), length(1..8), step1, step2, ..., step8", "out, pos"}
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

void performSub (TzNode* n, TzProcessInfo* info);
TzNode* createSubNode ();

void performMult (TzNode* n, TzProcessInfo* info);
TzNode* createMultNode ();

void performDiv (TzNode* n, TzProcessInfo* info);
TzNode* createDivNode ();

void performClip (TzNode* n, TzProcessInfo* info);
TzNode* createClipNode ();

void performMix (TzNode* n, TzProcessInfo* info);
TzNode* createMixNode ();

void performMap (TzNode* n, TzProcessInfo* info);
TzNode* createMapNode ();

void performMem (TzNode* n, TzProcessInfo* info);
TzNode* createMemNode ();


/* =========================================== */

void performConstant (TzNode* n, TzProcessInfo* info);
TzNode* createConstantNode (float val);

/* =========================================== */


void performPhasor (TzNode* n, TzProcessInfo* info);
TzNode* createPhasorNode ();

void performPulse (TzNode* n, TzProcessInfo* info);
TzNode* createPulseNode ();

void performSinosc (TzNode* n, TzProcessInfo* info);
TzNode* createSinoscNode ();

void performSeq8 (TzNode* n, TzProcessInfo* info);
TzNode* createSeq8Node ();

#endif
