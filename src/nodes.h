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
    MODULO_NODE,
    SIN_NODE,
    COS_NODE,
    TAN_NODE,
    TANH_NODE,
    CLIP_NODE,
    EQUAL_NODE,
    NEQUAL_NODE,
    LOWER_NODE,
    GREATER_NODE,
    MIN_NODE,
    MAX_NODE,
    ROUND_NODE,
    AND_NODE,
    OR_NODE,
    XOR_NODE,
    MIX_NODE,
    MAP_NODE,
    SMOOTH_NODE,
    MIDITOFREQ_NODE,
    SAMPLERATE_NODE,
    MEM_NODE,
    COUNT_NODE,
    PHASOR_NODE,
    PULSE_NODE,
    SINOSC_NODE,
    SEQ8_NODE,
    RANDOM_NODE,
    SEGMENT_NODE,
    SELECT_NODE,
    DELAY_NODE,
    FDELAY_NODE,
    NUM_NODE_TYPES
};



typedef struct TzNodeDoc TzNodeDoc;
struct TzNodeDoc {
    const char* name;
    const char* summary;
    const char* inputs;
    const char* outputs;
};

extern const TzNodeDoc nodesDoc [NUM_NODE_TYPES];

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

void performModulo (TzNode* n, TzProcessInfo* info);
TzNode* createModuloNode ();

void performSin (TzNode* n, TzProcessInfo* info);
TzNode* createSinNode ();

void performCos (TzNode* n, TzProcessInfo* info);
TzNode* createCosNode ();

void performTan (TzNode* n, TzProcessInfo* info);
TzNode* createTanNode ();

void performTanh (TzNode* n, TzProcessInfo* info);
TzNode* createTanhNode ();

void performClip (TzNode* n, TzProcessInfo* info);
TzNode* createClipNode ();

void performEqual (TzNode* n, TzProcessInfo* info);
TzNode* createEqualNode ();

void performNequal (TzNode* n, TzProcessInfo* info);
TzNode* createNequalNode ();

void performLower (TzNode* n, TzProcessInfo* info);
TzNode* createLowerNode ();

void performGreater (TzNode* n, TzProcessInfo* info);
TzNode* createGreaterNode ();

void performMin (TzNode* n, TzProcessInfo* info);
TzNode* createMinNode ();

void performMax (TzNode* n, TzProcessInfo* info);
TzNode* createMaxNode ();

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

void performSmooth (TzNode* n, TzProcessInfo* info);
TzNode* createSmoothNode ();

void performMiditofreq (TzNode* n, TzProcessInfo* info);
TzNode* createMiditofreqNode ();

void performSamplerate (TzNode* n, TzProcessInfo* info);
TzNode* createSamplerateNode ();

void performMem (TzNode* n, TzProcessInfo* info);
TzNode* createMemNode ();

void performCount (TzNode* n, TzProcessInfo* info);
TzNode* createCountNode ();


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

void performFdelay (TzNode* n, TzProcessInfo* info);
TzNode* createFdelayNode ();

#endif
