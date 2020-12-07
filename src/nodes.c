#include "nodes.h"

#include <math.h>
#include <time.h>
#include <stdio.h>

#include "parser.h"

#define TZ_UNUSED(x) (void)(x)

const TzNodeDoc nodesDoc [NUM_NODE_TYPES] = {
    {"-", "-", "-", "-"},
    {"module", "special node that processes a patch internally and exposes up to 16 inputs and up to 16 outputs.", "user declared inputs", "user declared outputs"},
    {"var", "holds a variable that can be shared through the patch. Instead of using myvar@val for I/O, you can simply use $myvar.", "val", "val"},
    {"add", "outputs {in1} + {in2}.", "in1, in2", "out"},
    {"sub", "outputs {in1} - {in2}.", "in1, in2", "out"},
    {"mult", "outputs {in1} * {in2}.", "in1, in2", "out"},
    {"div", "outputs {in1} / {in2}.", "in1, in2", "out"},
    {"modulo", "outputs {in1} % {in2}.", "in1, in2", "out"},
    {"pow", "outputs {base} raised to the power of {exp}.", "base, exp", "out"},
    {"sqrt", "outputs the square root of {in}.", "in", "out"},
    {"abs", "outputs the absolute value of {in}.", "in", "out"},
    {"sin", "outputs the sine of {in}.", "in", "out"},
    {"cos", "outputs the cosine of {in}.", "in", "out"},
    {"tan", "outputs the tangent of {in}.", "in", "out"},
    {"tanh", "outputs the hyperbolic tangent of {in}.", "in", "out"},
    {"clip", "clips {in} in range [{min}..{max}].", "in, min, max", "out"},
    {"equal", "outputs 1 if {in1} and {in2} are equal, 0 otherwise.", "in1, in2", "out"},
    {"nequal", "outputs 1 if {in1} and {in2} are not equal, 0 otherwise.", "in1, in2", "out"},
    {"lower", "outputs 1 if {in1} < {in2}, 0 otherwise.", "in1, in2", "out"},
    {"greater", "outputs 1 if {in1} > {in2}, 0 otherwise.", "in1, in2", "out"},
    {"min", "outputs the lowest value between {in1} and {in2}.", "in1, in2", "out"},
    {"max", "outputs the highest value between {in1} and {in2}.", "in1, in2", "out"},
    {"round", "rounds {in} to the nearest integer value.", "in", "out"},
    {"and", "outputs 1 if both {in1} and {in2} are not 0, outputs 0 otherwise.", "in1 in2", "out"},
    {"or", "outputs 1 if either {in1} or {in2} are not 0, outputs 0 otherwise.", "in1 in2", "out"},
    {"xor", "outputs 1 if one of {in1} and {in2} is not 0, outputs 0 if both are 0 or both are not 0.", "in1 in2", "out"},
    {"mix", "interpolates between {in1} and {in2} according to {coeff} in range [0..1].", "in1, in2, coeff", "out"},
    {"map", "maps {in} from the range [{imin}..{imax}] to the range [{omin}..{omax}].", "in, imin, imax, omin, omax", "out"}, 
    {"smooth", "smooth value changes at {in} in {dur} milliseconds and outputs the smoothed value.", "in, dur", "out"},
    {"miditofreq", "converts a MIDI note [0..127] to a frequency in Hertz.", "in", "out"},
    {"samplerate", "outputs the current samplerate.", "-", "out"},
    {"count", "outputs the count of non zero signals received at {clock}. Loops back to 0 after reaching {max} (inclusive, defaults to 16).", "clock max", "out"},
    {"phasor", "generates a ramp in the range [0..1]. A pulse at {reset} resets the phase.", "freq(Hz), reset(pulse)", "out"},
    {"pulse", "outputs a pulse at a periodic rate. A pulse at {reset} resets the phase.", "rate(Ms), reset(pulse)", "out"},
    {"sinosc", "generates a sine wave. A pulse at {reset} resets the phase. A signal can be sent to {fm) for frequency modulation with the amount of modulation controled by {fmdepth}.", "freq(Hz), reset(pulse), fm, fmdepth", "out"},
    {"seq8", "outputs the values of inputs {step1} to {step8} sequentially when receiving a pulse at {clock}. The sequence length can be changed via input {length}. The output {pos} sends the playhead position.", "clock(pulse), length(1..8), step1, step2, ..., step8", "out, pos"},
    {"random", "outputs a random value in the range [0..1] when receiving a pulse at {clock}.", "clock", "out"},
    {"segment", "outputs a ramp from {val1} to {val2} in {dur} Ms when receiving a pulse at {clock}. Outputs a pulse at {end} when reaching the end of the segment for chaining segments.", "clock, val1, val2, dur", "out, end(pulse)"},
    {"select", "if {index} is 0, outputs 0 otherwise ouputs the value of the corresponding input.", "index, in1, in2, in3, in4, in5, in6, in7, in8", "out"},
    {"delay", "a basic delay line (up to 2 seconds).", "in, time(Ms)", "out"},
    {"fdelay", "a delay line with feedback (up to 2 seconds).", "in, time(Ms) feed([0..1])", "out"}
};


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
    for (i = 0; i < TZNODE_NUM_BUFFERS; ++i) {
        n->buffers[i] = NULL;
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
    for (i = 0; i < TZNODE_NUM_BUFFERS; ++i) {
        if (n->buffers[i] != NULL) {
            free(n->buffers[i]);
        }
        n->buffers[i] = NULL;
    }
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

void performModulo (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const int in1 = (int)getNodeInput(n, 0, 0.f);
    const int in2 = (int)getNodeInput(n, 1, 0.f);
    n->outputs[0] = in2 != 0 ? (float)(in1 % in2) : 0.f;
}

TzNode* createModuloNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performModulo;
    return n;
}

void performPow (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float base = getNodeInput(n, 0, 0.f);
    const float exp = (float)((int)getNodeInput(n, 1, 0.f));

    if ((base == 0.f && exp == 0.f) || (base == 0.f && exp < 0.f)) {
        n->outputs[0] = 0.f;
    }
    else {
        n->outputs[0] = pow(base, exp);
    }
}

TzNode* createPowNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "base");
    strcpy(n->inputsNames[1], "exp");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performPow;
    return n;
}

void performSqrt (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    if (in < 0.f) {
        n->outputs[0] = 0.f;
    }
    n->outputs[0] = sqrt(in);
}

TzNode* createSqrtNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performSqrt;
    return n;
}

void performAbs (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = fabs(in);
}

TzNode* createAbsNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performAbs;
    return n;
}

void performSin (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = sin(in);
}

TzNode* createSinNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performSin;
    return n;
}

void performCos (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = cos(in);
}

TzNode* createCosNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performCos;
    return n;
}

void performTan (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = tan(in);
}

TzNode* createTanNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performTan;
    return n;
}

void performTanh (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = tanh(in);
}

TzNode* createTanhNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performTanh;
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

void performEqual (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 == in2 ? 1.f : 0.f;
}

TzNode* createEqualNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performEqual;
    return n;
}

void performNequal (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 == in2 ? 0.f : 1.f;
}

TzNode* createNequalNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performNequal;
    return n;
}

void performLower (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 < in2 ? 1.f : 0.f;
}

TzNode* createLowerNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performLower;
    return n;
}

void performGreater (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 > in2 ? 1.f : 0.f;
}

TzNode* createGreaterNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performGreater;
    return n;
}

void performMin (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 < in2 ? in1 : in2;
}

TzNode* createMinNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMin;
    return n;
}

void performMax (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in1 = getNodeInput(n, 0, 0.f);
    const float in2 = getNodeInput(n, 1, 0.f);
    n->outputs[0] = in1 > in2 ? in1 : in2;
}

TzNode* createMaxNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in1");
    strcpy(n->inputsNames[1], "in2");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMax;
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

void performSmooth (TzNode* n, TzProcessInfo* info) {
    const float in = getNodeInput(n, 0, 0.f); 
    float dur = getNodeInput(n, 1, 1.f);
    float* val = &(n->memory[0]);
    float delta = 0.f;
    float* startFlag = &(n->memory[1]);

    if (*startFlag == 0.f) {
        /* jump straight to input value on startup */
        *val = in;
        *startFlag = 1.f;
    }
    else {
        dur = dur * 0.001f * info->samplerate;

        delta = (in - *val) / dur;

        if (delta >= 0.f) {
            *val += delta;
            if (*val > in) {
                *val = in;
            }
        }
        else {
            *val += delta;
            if (*val < in) {
                *val = in;
            }
        }
    }
    
    n->outputs[0] = *val;
}

TzNode* createSmoothNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "dur");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
    n->perform = &performSmooth;
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

void performSamplerate (TzNode* n, TzProcessInfo* info) {
    n->outputs[0] = info->samplerate;
}

TzNode* createSamplerateNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 0;
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performSamplerate;
    return n;
}


void performCount (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float* cnt = &(n->memory[0]);
    const float clock = getNodeInput(n, 0, 0.f);
    const int max = (int)getNodeInput(n, 1, 16.f);

    if (clock != 0.f) {
        *cnt += 1.f;
        if ((int)*cnt > max) *cnt = 1.f;
    }

    n->outputs[0] = *cnt;
}

TzNode* createCountNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "clock");
    strcpy(n->inputsNames[1], "max");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performCount;
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

    n->outputs[0] = steps[(int)(*pos)] >= 0.f ? steps[(int)(*pos)] : 0.f;
    n->outputs[1] = *pos;
}

TzNode* createSeq8Node () {
    int i = 0;
    char stepname[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 10;
    for (i = 0; i < 8; ++i) {
        sprintf(stepname, "step%d", i + 1);
        strcpy(n->inputsNames[i], stepname);
    }
    strcpy(n->inputsNames[8], "clock");
    strcpy(n->inputsNames[9], "length");
    n->numOutputs = 2;
    strcpy(n->outputsNames[0], "out");
    strcpy(n->outputsNames[1], "pos");
    n->memory[0] = -1.f;
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
    n->outputs[1] = 0.f;


    if (clock != 0) {
        *out = v1;
        n->memory[1] = 0.f;
    }

    if ((delta > 0.f && *out >= v2) || (delta < 0.f && *out <= v2)) {
        if ((int)(n->memory[1]) != 1) {
            n->outputs[1] = 1.f;
            n->memory[1] = 1.f;
        }
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
    n->numOutputs = 2;
    strcpy(n->outputsNames[0], "out");
    strcpy(n->outputsNames[1], "end");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
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
        vals[i-1] = getNodeInput(n, i, 0.f);
    }

    n->outputs[0] = idx == 0 ? 0.f : vals[idx - 1];
}

TzNode* createSelectNode () {
    int i = 0;
    char inName[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 9;
    strcpy(n->inputsNames[0], "index");
    for (i = 1; i < 9; ++i) {
        sprintf(inName, "in%d", i);
        strcpy(n->inputsNames[i], inName);
    }
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performSelect;
    return n;
}


void performDelay (TzNode* n, TzProcessInfo* info) {
    const int maxLength = (2 * (int)(info->samplerate)) + 16; /* small padding for safe access */
    const float in  = getNodeInput(n, 0, 0.f);
    float time = getNodeInput(n, 1, 1.f);
    float stime = 1.f;
    float rp = 0;
    int irp = 0;
    int irp1 = 1;
    float frac = 0.f;
    float* wp = &(n->memory[0]);
    float* maxpos = &(n->memory[1]);
    float* startupFlag = &(n->memory[2]);
    
    if (*startupFlag < 1.f) {
        /* init buffer */
        n->buffers[0] = malloc(maxLength * sizeof(float));
        if (n->buffers[0] == NULL) {
            printf("Failed to allocate delay buffer.\n");
        }
        *maxpos = 2.f * (info->samplerate - 1);
        *startupFlag = 1.f;
    }

    if (time < 1.f) time = 1.f;
    stime = time * 0.001 * info->samplerate;
    if (stime > *maxpos) stime = *maxpos;

    rp = *wp - stime;
    while (rp < 0.f) rp += *maxpos;

    irp = (int)rp;
    frac = rp - (float)irp;
    irp1 = irp + 1 > (int)(*maxpos) ? 0 : irp + 1;

    n->outputs[0] = n->buffers[0][irp] + frac * (n->buffers[0][irp1] - n->buffers[0][irp]);

    n->buffers[0][(int)(*wp)] = in;
    ++(*wp);
    *wp = *wp > *maxpos ? 0 : *wp;
}

TzNode* createDelayNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "time");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
    n->memory[2] = 0.f;
    n->perform = &performDelay;
    return n;
}


void performFdelay (TzNode* n, TzProcessInfo* info) {
    const int maxLength = (2 * (int)(info->samplerate)) + 16; /* small padding for safe access */
    const float in  = getNodeInput(n, 0, 0.f);
    float time = getNodeInput(n, 1, 1.f);
    const float feed = getNodeInput(n, 2, 0.f);
    float stime = 1.f;
    float rp = 0;
    int irp = 0;
    int irp1 = 1;
    float frac = 0.f;
    float* wp = &(n->memory[0]);
    float* maxpos = &(n->memory[1]);
    float* startupFlag = &(n->memory[2]);
    
    if (*startupFlag < 1.f) {
        /* init buffer */
        n->buffers[0] = malloc(maxLength * sizeof(float));
        if (n->buffers[0] == NULL) {
            printf("Failed to allocate delay buffer.\n");
        }
        *maxpos = 2.f * (info->samplerate - 1);
        *startupFlag = 1.f;
    }

    if (time < 1.f) time = 1.f;
    stime = time * 0.001 * info->samplerate;
    if (stime > *maxpos) stime = *maxpos;

    rp = *wp - stime;
    while (rp < 0.f) rp += *maxpos;

    irp = (int)rp;
    frac = rp - (float)irp;
    irp1 = irp + 1 > (int)(*maxpos) ? 0 : irp + 1;

    n->outputs[0] = n->buffers[0][irp] + frac * (n->buffers[0][irp1] - n->buffers[0][irp]);

    n->buffers[0][(int)(*wp)] = tanh(in + (n->outputs[0] * feed));
    ++(*wp);
    *wp = *wp > *maxpos ? 0 : *wp;
}

TzNode* createFdelayNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "time");
    strcpy(n->inputsNames[2], "feed");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
    n->memory[2] = 0.f;
    n->perform = &performFdelay;
    return n;
}

