#ifndef TZARA_NODES_H
#define TZARA_NODES_H

#include <stdlib.h>
#include <string.h>

#define TZNODE_MAX_INPUTS 16 
#define TZNODE_MAX_OUTPUTS 16
#define TZNODE_MEMORY_SIZE 32
#define TZNODE_NUM_BUFFERS 16
#define TZNODE_NAME_SIZE 256

#define TZMODULE_MAX_NODES 1024

enum NodeTypes {
    INVALID_NODE_TYPE = 0,
    MODULE_NODE,
    VAR_NODE,
    ADDER_NODE,
    SUB_NODE,
    MULT_NODE,
    DIV_NODE,
    CLIP_NODE,
    ROUND_NODE,
    AND_NODE,
    OR_NODE,
    XOR_NODE,
    MIX_NODE,
    MAP_NODE,
    MIDITOFREQ_NODE,
    MEM_NODE,
    PHASOR_NODE,
    PULSE_NODE,
    SINOSC_NODE,
    SEQ8_NODE,
    RANDOM_NODE,
    SEGMENT_NODE,
    SELECT_NODE,
    DELAY_NODE,
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
    {"module", "special node that processes a patch internally and exposes up to 16 inputs and up to 16 outputs.", "user declared inputs", "user declared outputs"},
    {"var", "holds a variable that can be shared through the patch. Instead of using myvar@val for I/O, you can simply use $myvar.", "val", "val"},
    {"add", "outputs {in1} + {in2}.", "in1, in2", "out"},
    {"sub", "outputs {in1} - {in2}.", "in1, in2", "out"},
    {"mult", "outputs {in1} * {in2}.", "in1, in2", "out"},
    {"div", "outputs {in1} / {in2}.", "in1, in2", "out"},
    {"clip", "clips {in} in range [{min}..{max}].", "in, min, max", "out"},
    {"round", "rounds {in} to the nearest integer value.", "in", "out"},
    {"and", "outputs 1 if both {in1} and {in2} are not 0, outputs 0 otherwise.", "in1 in2", "out"},
    {"or", "outputs 1 if either {in1} or {in2} are not 0, outputs 0 otherwise.", "in1 in2", "out"},
    {"xor", "outputs 1 if one of {in1} and {in2} is not 0, outputs 0 if both are 0 or both are not 0.", "in1 in2", "out"},
    {"mix", "interpolates between {in1} and {in2} according to {coeff} in range [0..1].", "in1, in2, coeff", "out"},
    {"map", "maps {in} from the range [{imin}..{imax}] to the range [{omin}..{omax}].", "in, imin, imax, omin, omax", "out"}, 
    {"miditofreq", "converts a MIDI note [0..127] to a frequency in Hertz.", "in", "out"},
    {"mem", "1 sample delay.", "in", "out"},
    {"phasor", "generates a ramp in the range [0..1]. A pulse at {reset} resets the phase.", "freq(Hz), reset(pulse)", "out"},
    {"pulse", "outputs a pulse at a periodic rate. A pulse at {reset} resets the phase.", "rate(Ms), reset(pulse)", "out"},
    {"sinosc", "generates a sine wave. A pulse at {reset} resets the phase. A signal can be sent to {fm) for frequency modulation with the amount of modulation controled by {fmdepth}.", "freq(Hz), reset(pulse), fm, fmdepth", "out"},
    {"seq8", "outputs the values of inputs {step1} to {step8} sequentially when receiving a pulse at {clock}. The sequence length can be changed via input {length}. The output {pos} sends the playhead position.", "clock(pulse), length(1..8), step1, step2, ..., step8", "out, pos"},
    {"random", "outputs a random value in the range [0..1] when receiving a pulse at {clock}.", "clock", "out"},
    {"segment", "outputs a ramp from {val1} to {val2} in {dur} Ms when receiving a pulse at {clock}.", "clock, val1, val2, dur", "out"},
    {"select", "if {index} is 0, outputs 0 otherwise ouputs the value of the corresponding input.", "index, in1, in2, in3, in4, in5, in6, in7, in8", "out"},
    {"delay", "a basic delay line (up to 2 seconds).", "in time(Ms)", "out"}
};

typedef struct TzModule TzModule;
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
    float* buffers[TZNODE_NUM_BUFFERS];
    char name[TZNODE_NAME_SIZE];
    char inputsNames[TZNODE_MAX_INPUTS][TZNODE_NAME_SIZE];
    char outputsNames[TZNODE_MAX_OUTPUTS][TZNODE_NAME_SIZE];
    TzModule* submodule;
}; 

void flush (TzNode* n);

TzNode* allocateNewNode ();

void releaseNode (TzNode* n);

float getNodeInput (TzNode* n, int inputIndex, float defaultValue);

struct TzModule {
    TzNode* nodes[TZMODULE_MAX_NODES];
    int numNodes;
    float** inputs[TZNODE_MAX_INPUTS];
    int numInputs;
    float* outputs[TZNODE_MAX_OUTPUTS];
    int numOutputs;
    char inputsNames[TZNODE_MAX_INPUTS][TZNODE_NAME_SIZE];
    char outputsNames[TZNODE_MAX_OUTPUTS][TZNODE_NAME_SIZE];
};

int addModuleNode (TzModule* m, TzNode* n, const char* name);

void connectModuleNodes (TzModule* m, int inModule, int inOutput, int outModule, int outInput);

void createModuleInlet (TzModule* m, const char* name);
void createModuleOutlet  (TzModule* m, const char* name);

void connectModuleInlet (TzModule* m, int srcNode, int srcInput, int inletIndex);
void connectModuleOutlet (TzModule* m, int srcNode, int srcOutput, int outletIndex);

/* =========================================== */

void performModule (TzNode* n, TzProcessInfo* info);
TzNode* createModuleNode (const char* filename);

void performVar (TzNode* n, TzProcessInfo* info);
TzNode* createVarNode ();

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

void performRound (TzNode* n, TzProcessInfo* info);
TzNode* createRoundNode ();

void performAnd (TzNode* n, TzProcessInfo* info);
TzNode* createAndNode ();

void performOr (TzNode* n, TzProcessInfo* info);
TzNode* createOrNode ();

void performXor (TzNode* n, TzProcessInfo* info);
TzNode* createXorNode ();

void performMix (TzNode* n, TzProcessInfo* info);
TzNode* createMixNode ();

void performMap (TzNode* n, TzProcessInfo* info);
TzNode* createMapNode ();

void performMiditofreq (TzNode* n, TzProcessInfo* info);
TzNode* createMiditofreqNode ();

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

void performRandom (TzNode* n, TzProcessInfo* info);
TzNode* createRandomNode ();

void performSegment (TzNode* n, TzProcessInfo* info);
TzNode* createSegmentNode ();

void performSelect (TzNode* n, TzProcessInfo* info);
TzNode* createSelectNode ();

void performDelay (TzNode* n, TzProcessInfo* info);
TzNode* createDelayNode ();

#endif
