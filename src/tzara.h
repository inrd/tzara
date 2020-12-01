#ifndef TZARA_CORE_HEADER
#define TZARA_CORE_HEADER

#include "nodes.h"

#define TZARA_MAX_OUTPUT_CHANS 2
#define TZARA_MAX_NODES 4096

#define TZARA_OUTPUT_NODE_INDEX -0xaa
#define TZARA_OUTPUT_LEFT_INDEX -0xbb
#define TZARA_OUTPUT_RIGHT_INDEX -0xcc

enum TzErrors {
    NO_ERROR = 0,
    MAX_MEMORY
};


typedef struct Tzara Tzara;
struct Tzara {
    TzNode* nodes[TZARA_MAX_NODES];
    int numNodes;
    float* outputs[TZARA_MAX_OUTPUT_CHANS];
};

void init(Tzara* t);

int addNode (Tzara* tz, TzNode* n, const char* name);

void connectModules (Tzara* tz, int inModule, int inOutput, int outModule, int outInput);

void connectModuleToOutput (Tzara* tz, int inModule, int inOutput, int outInput);

void process (Tzara* tz, float** out, int numChans, int numSamps, float samplerate);

void release (Tzara* tz);

#endif

