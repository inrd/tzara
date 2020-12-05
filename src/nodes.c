#include "nodes.h"

#include <math.h>
#include <time.h>
#include <stdio.h>

#include "parser.h"

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
    n->submodule = NULL;
}

TzNode* allocateNewNode () {
    TzNode* n = (TzNode*)(malloc(sizeof(TzNode)));
    flush(n);
    return n;
}

void releaseNode (TzNode* n) {
    int i = 0;
    if (n->submodule != NULL) {
        for (i = 0; i < n->submodule->numNodes; ++i) {
            releaseNode(n->submodule->nodes[i]);
            free(n->submodule->nodes[i]);
            n->submodule->nodes[i] = NULL;
        }
        free(n->submodule);
        n->submodule = NULL;
    }
}

float getNodeInput (TzNode* n, int inputIndex, float defaultValue) {
    return n->inputs[inputIndex] != NULL ? *(n->inputs[inputIndex]) : defaultValue;
}


int addModuleNode (TzModule* m, TzNode* n, const char* name) {
    if (m->numNodes < (TZMODULE_MAX_NODES - 1)) {
        strncpy(n->name, name, sizeof(n->name) - 1);
        m->nodes[m->numNodes] = n;
        ++(m->numNodes);
        return 0;
    }
    fprintf(stderr, "Too many nodes added!\n");
    return 1;
}

void connectModuleNodes (TzModule* m, int inModule, int inOutput, int outModule, int outInput) {
    if ((outInput < m->nodes[outModule]->numInputs) && (inOutput < m->nodes[inModule]->numOutputs)) {
        if (m->nodes[outModule]->inputs[outInput] != NULL) {
            fprintf(stdout, "Warning : node input was already connected. Replacing connection.\n");
        }
        m->nodes[outModule]->inputs[outInput] = &(m->nodes[inModule]->outputs[inOutput]);
    }
    else {
        /* TODO : should abort */
        fprintf(stderr, "Invalid routing...\n");
    }
}

void createModuleInlet (TzModule* m, const char* name) {
    if (m->numInputs < TZNODE_MAX_INPUTS) {
        strncpy(m->inputsNames[m->numInputs], name, TZNODE_NAME_SIZE - 1);
        ++(m->numInputs);
    }
    else {
        printf("Too many module inputs!\n");
    }
}

void createModuleOutlet  (TzModule* m, const char* name) {
    if (m->numOutputs < TZNODE_MAX_OUTPUTS) {
        strncpy(m->outputsNames[m->numOutputs], name, TZNODE_NAME_SIZE - 1);
        ++(m->numOutputs);
    }
    else {
        printf("Too many module outputs!\n");
    }
}

void connectModuleInlet (TzModule* m, int srcNode, int srcInput, int inletIndex) {
    m->inputs[inletIndex] = &(m->nodes[srcNode]->inputs[srcInput]);
}

void connectModuleOutlet (TzModule* m, int srcNode, int srcOutput, int outletIndex) {
    m->outputs[outletIndex] = &(m->nodes[srcNode]->outputs[srcOutput]);
}


/* =========================== */

void performModuleNode (TzNode* n, TzProcessInfo* info) {
    int i = 0;
        
    for (i = 0; i < n->numInputs; ++i) {
        *(n->submodule->inputs[i]) = n->inputs[i];
    }

    for (i = 0; i < n->submodule->numNodes; ++i) {
        n->submodule->nodes[i]->perform(n->submodule->nodes[i], info);
    }

    for (i = 0; i < n->numOutputs; ++i) {
        n->outputs[i] = n->submodule->outputs[i] != NULL ? *(n->submodule->outputs[i]) : 0.f;
    }
}

TzNode* createModuleNode (const char* filename) {
    int i = 0;
    FILE* patch = NULL;
    TzNode* n = allocateNewNode();

    fprintf(stderr, "\n== Opening module file : %s ==\n\n", filename);

    patch = fopen(filename, "r");
    if (patch == NULL) {
        fprintf(stderr, "Could not open %s...\n\n", filename);
        return n;
    }

    n->submodule = malloc(sizeof(TzModule));

    if (n->submodule == NULL) {
        fprintf(stderr, "Failed to create module node...\n\n");
        return n;
    }

    for (i = 0; i < TZMODULE_MAX_NODES; ++i) {
        n->submodule->nodes[i] = NULL;
    }
    n->submodule->numNodes = 0;

    for (i = 0; i < TZNODE_MAX_INPUTS; ++i) {
        n->submodule->inputs[i] = NULL;
    }
    n->submodule->numInputs = 0;

    for (i = 0; i < TZNODE_MAX_OUTPUTS; ++i) {
        n->submodule->outputs[i] = NULL;
    }
    n->submodule->numOutputs = 0;

    if (parsePatch (n->submodule, patch, filename , 1) != 0) {
        fclose(patch);
        fprintf(stderr, "Errors encountered while building module patch...\n\n");
        return n;
    }

    fclose(patch);

    printf("\n== Module patch successfully built ==\n\n");

    n->numInputs = n->submodule->numInputs;
    for (i = 0; i < n->numInputs; ++i) {
        strcpy(n->inputsNames[i], n->submodule->inputsNames[i]);
    }
    n->numOutputs = n->submodule->numOutputs;
    for (i = 0; i < n->numOutputs; ++i) {
        strcpy(n->outputsNames[i], n->submodule->outputsNames[i]);
    }
    n->perform = &performModuleNode;
    return n;
}




void performVar (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = in;
}

TzNode* createVarNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "val");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "val");
    n->perform = &performVar;
    return n;
}


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

void performRound (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    float in = getNodeInput(n, 0, 0.f);
    if (in < 0.f) {
        n->outputs[0] = (int)(in - 0.5);
    }
    else {
        n->outputs[0] = (int)(in + 0.5);
    }
}

TzNode* createRoundNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performRound;
    return n;
}

void performAnd (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    float in1 = getNodeInput(n, 0, 0.f);
    float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = (in1 != 0.f && in2 != 0.f) ? 1.f : 0.f;
}

TzNode* createAndNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performAnd;
    return n;
}

void performOr (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    float in1 = getNodeInput(n, 0, 0.f);
    float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = (in1 != 0.f || in2 != 0.f) ? 1.f : 0.f;
}

TzNode* createOrNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performOr;
    return n;
}

void performXor (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    float in1 = getNodeInput(n, 0, 0.f);
    float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = (in1 != 0.f && in2 == 0.f) || (in1 == 0.f && in2 != 0.f) ? 1.f : 0.f;
}

TzNode* createXorNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performXor;
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

    if ((imin >= imax)||(omin >= omax)||(in<imin)||(in>imax)) {
        /* invalid inputs */
        n->outputs[0] = 0.f;
        if ((int)(*errCount) < 10) {
            fprintf(stderr, "Warning : [%s] => invalid inputs, output set to 0...\n%f : [%f..%f] -> [%f..%f]\n", n->name, in, imin, imax, omin, omax); 
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

void performMiditofreq (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float midi = getNodeInput(n, 0, 0.f);
    if (midi < 0) midi = 0;
    if (midi > 127) midi = 127;

    n->outputs[0] = 440.f * pow(2.f, ((float)(midi - 69)) / 12.f);
}

TzNode* createMiditofreqNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMiditofreq;
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
    n->outputs[0] = val;
    n->perform = &performConstant;
    return n;
}

void performPhasor (TzNode* n, TzProcessInfo* info) {
    const float samplerate = info->samplerate;
    const float freq = getNodeInput(n, 0, 440.f);
    const float incr = freq / samplerate;
    const float reset = getNodeInput(n, 1, 0.f);

    float* phase = &(n->memory[0]);

    if (reset > 0) {
        *phase = 0.f;
    }

    n->outputs[0] = *phase;
    *phase += incr;
    while (*phase > 1.f) *phase -= 1.f;
}

TzNode* createPhasorNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "freq");
    strcpy(n->inputsNames[1], "reset");
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
    const float reset = getNodeInput(n, 1, 0.f);

    float* count = &(n->memory[0]);

    if (reset > 0) {
        *count = 0.f;
    }

    n->outputs[0] = (int)(*count) == 0 ? 1.f : 0.f;
    *count += 1.f;
    if (*count >= period) *count = 0.f;
}

TzNode* createPulseNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "rate");
    strcpy(n->inputsNames[1], "reset");
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
    const float reset = getNodeInput(n, 1, 0.f);
    const float fm = getNodeInput(n, 2, 0.f);
    const float fmdepth = getNodeInput(n, 3, 0.f);

    const float incr = (freq + (fm * fmdepth)) * twopi / samplerate;

    float* phase = &(n->memory[0]);

    if (reset > 0) {
        *phase = 0.f;
    }

    n->outputs[0] = sin(*phase);
    *phase += incr;
    while (*phase > twopi) *phase -= twopi;
}

TzNode* createSinoscNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 4;
    strcpy(n->inputsNames[0], "freq");
    strcpy(n->inputsNames[1], "reset");
    strcpy(n->inputsNames[2], "fm");
    strcpy(n->inputsNames[3], "fmdepth");
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


void performSegment (TzNode* n, TzProcessInfo* info) {
    const float samplerate = info->samplerate;
    const float dur = getNodeInput(n, 3, 10.f);
    const float length = (dur > 0 ? dur : 10.f) * 0.001f *samplerate;

    const float v1 = getNodeInput(n, 1, 0.f);
    const float v2 = getNodeInput(n, 2, 0.f);
    const float delta = (v2 - v1) / length;

    const int clock = (int)getNodeInput(n, 0, 0.f);

    float* out = &(n->memory[0]);

    if (clock != 0) {
        *out = v1;
    }

    n->outputs[0] = *out;

    *out += delta;
    if ((delta > 0.f && *out > v2) || (delta < 0.f && *out < v2)) *out = v2;
}

TzNode* createSegmentNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 4;
    strcpy(n->inputsNames[0], "clock");
    strcpy(n->inputsNames[1], "val1");
    strcpy(n->inputsNames[2], "val2");
    strcpy(n->inputsNames[3], "dur");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performSegment;
    return n;
}


void performSelect (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float vals[8];
    int idx = (int)getNodeInput(n, 0, 0.f);
    int i = 0;

    if (idx < 0 || idx > 8) idx = 0;

    for (i = 1; i < 9; ++i) {
        vals[i] = getNodeInput(n, i, 0.f);
    }

    n->outputs[0] = idx == 0 ? 0.f : vals[idx];
}

TzNode* createSelectNode () {
    int i = 0;
    char inName[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 9;
    strcpy(n->inputsNames[0], "index");
    for (i = 1; i < 9; ++i) {
        sprintf(inName, "in%d", i);
        strncpy(n->inputsNames[i], inName, strlen(inName));
    }
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performSelect;
    return n;
}

