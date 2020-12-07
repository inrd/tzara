#ifndef TZARA_CORE_HEADER
#define TZARA_CORE_HEADER

#include "nodes.h"

#define TZARA_MAX_OUTPUT_CHANS 2
#define TZARA_MAX_NODES 4096

enum TzErrors {
    NO_ERROR = 0,
    MAX_MEMORY
};


typedef struct Tzara Tzara;
struct Tzara {
    TzNode* nodes[TZARA_MAX_NODES];
    int numNodes;
    float* outputs[TZARA_MAX_OUTPUT_CHANS];
    int renderDuration; /* in seconds */
};

void init(Tzara* t);

int addNode (Tzara* tz, TzNode* n, const char* name);

void connectNodes (Tzara* tz, int inModule, int inOutput, int outModule, int outInput);

void connectNodeToOutput (Tzara* tz, int inModule, int inOutput, int outInput);

void process (Tzara* tz, float** out, int numChans, int numSamps, float samplerate);

void release (Tzara* tz);

#endif

