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
    DEFAULTVAL_NODE,
    VAR_NODE,
    ADDER_NODE,
    SUB_NODE,
    MULT_NODE,
    DIV_NODE,
    MODULO_NODE,
    POW_NODE,
    SQRT_NODE,
    ABS_NODE,
    SIN_NODE,
    COS_NODE,
    TAN_NODE,
    TANH_NODE,
    CLIP_NODE,
    WRAP_NODE,
    EQUAL_NODE,
    NEQUAL_NODE,
    LOWER_NODE,
    GREATER_NODE,
    MIN_NODE,
    MAX_NODE,
    ROUND_NODE,
    CEIL_NODE,
    FLOOR_NODE,
    FRAC_NODE,
    AND_NODE,
    OR_NODE,
    XOR_NODE,
    MIX_NODE,
    MERGE_NODE,
    PMERGE_NODE,
    MAP_NODE,
    SMOOTH_NODE,
    MIDITOFREQ_NODE,
    DBTOAMP_NODE,
    MSTOHZ_NODE,
    HZTOMS_NODE,
    SAMPLERATE_NODE,
    FIXDENORM_NODE,
    FIXNAN_NODE,
    COUNT_NODE,
    PHASOR_NODE,
    PULSE_NODE,
    SINOSC_NODE,
    NOISE_NODE,
    SEQ8_NODE,
    RANDOM_NODE,
    NOTESCALE_NODE,
    SEGMENT_NODE,
    SELECT_NODE,
    ROUTE_NODE,
    SAH_NODE,
    TIMEPOINT_NODE,
    LOWPASS_NODE,
    HIGHPASS_NODE,
    SVF_NODE,
    DELAY_NODE,
    FDELAY_NODE,
    NUM_NODE_TYPES
};


enum NoteScales {
    CHROMATIC_SCALE = 0,
    MAJOR_SCALE,
    MINOR_SCALE,
    HARMONIC_MAJOR_SCALE,
    HARMONIC_MINOR_SCALE,
    LOCRIAN_SCALE,
    PYRAMID_HEXATONIC_SCALE,
    KUNG_SCALE,
    HIRA_JOSHI_SCALE,
    RITSU_SCALE,
    MELA_CITRAMBARI_SCALE,
    RAGA_BILWADALA_SCALE,
    MAQAM_HIJAZ_SCALE,
    GNOSSIENNES_SCALE,
    NUM_SCALES
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
float getNodeInputClipped (TzNode* n, int inputIndex, float defaultValue, float min, float max);

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

void performDefaultval (TzNode* n, TzProcessInfo* info);
TzNode* createDefaultvalNode ();

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

void performPow (TzNode* n, TzProcessInfo* info);
TzNode* createPowNode ();

void performSqrt (TzNode* n, TzProcessInfo* info);
TzNode* createSqrtNode ();

void performAbs (TzNode* n, TzProcessInfo* info);
TzNode* createAbsNode ();

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

void performWrap (TzNode* n, TzProcessInfo* info);
TzNode* createWrapNode ();

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

void performCeil (TzNode* n, TzProcessInfo* info);
TzNode* createCeilNode ();

void performFloor (TzNode* n, TzProcessInfo* info);
TzNode* createFloorNode ();

void performFrac (TzNode* n, TzProcessInfo* info);
TzNode* createFracNode ();

void performAnd (TzNode* n, TzProcessInfo* info);
TzNode* createAndNode ();

void performOr (TzNode* n, TzProcessInfo* info);
TzNode* createOrNode ();

void performXor (TzNode* n, TzProcessInfo* info);
TzNode* createXorNode ();

void performMix (TzNode* n, TzProcessInfo* info);
TzNode* createMixNode ();

void performMerge (TzNode* n, TzProcessInfo* info);
TzNode* createMergeNode ();

void performPmerge (TzNode* n, TzProcessInfo* info);
TzNode* createPmergeNode ();

void performMap (TzNode* n, TzProcessInfo* info);
TzNode* createMapNode ();

void performSmooth (TzNode* n, TzProcessInfo* info);
TzNode* createSmoothNode ();

void performMiditofreq (TzNode* n, TzProcessInfo* info);
TzNode* createMiditofreqNode ();

void performDbtoamp (TzNode* n, TzProcessInfo* info);
TzNode* createDbtoampNode ();

void performMstohz (TzNode* n, TzProcessInfo* info);
TzNode* createMstohzNode ();

void performHztoms (TzNode* n, TzProcessInfo* info);
TzNode* createHztomsNode ();

void performSamplerate (TzNode* n, TzProcessInfo* info);
TzNode* createSamplerateNode ();

void performFixdenorm (TzNode* n, TzProcessInfo* info);
TzNode* createFixdenormNode ();

void performFixnan (TzNode* n, TzProcessInfo* info);
TzNode* createFixnanNode ();

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

void performNoise (TzNode* n, TzProcessInfo* info);
TzNode* createNoiseNode ();

void performSeq8 (TzNode* n, TzProcessInfo* info);
TzNode* createSeq8Node ();

void performRandom (TzNode* n, TzProcessInfo* info);
TzNode* createRandomNode ();

extern const int musicalScales [NUM_SCALES][12];
void performNotescale (TzNode* n, TzProcessInfo* info);
TzNode* createNotescaleNode ();

void performSegment (TzNode* n, TzProcessInfo* info);
TzNode* createSegmentNode ();

void performSelect (TzNode* n, TzProcessInfo* info);
TzNode* createSelectNode ();

void performRoute (TzNode* n, TzProcessInfo* info);
TzNode* createRouteNode ();

void performSah (TzNode* n, TzProcessInfo* info);
TzNode* createSahNode ();

void performTimepoint (TzNode* n, TzProcessInfo* info);
TzNode* createTimepointNode ();




void performLowpass (TzNode* n, TzProcessInfo* info);
TzNode* createLowpassNode ();

void performHighpass (TzNode* n, TzProcessInfo* info);
TzNode* createHighpassNode ();

void performSvf (TzNode* n, TzProcessInfo* info);
TzNode* createSvfNode ();

void performDelay (TzNode* n, TzProcessInfo* info);
TzNode* createDelayNode ();

void performFdelay (TzNode* n, TzProcessInfo* info);
TzNode* createFdelayNode ();

#endif
