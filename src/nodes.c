#include "nodes.h"

#include "tzdsp.h"

#include <math.h>
#include <time.h>

#include "parser.h"

#define TZ_UNUSED(x) (void)(x)

# define TZMATRIX_PARSER_CACHE_SIZE  4096


const TzNodeDoc nodesDoc [NUM_NODE_TYPES] = {
    {"-", "-", "-", "-"},
    {"module", "special node that processes a patch internally and exposes up to 16 inputs and up to 16 outputs.", "user declared inputs", "user declared outputs"},
    {"matrix", "special node that stores a matrix (optionally loaded from a file). Use {getrow} and {getcol} to retrieve values at {out}. Set {write} to a non zero value to write the value from {setval} to {setrow} and {setcol}. Get operations have precedence over set operations : if you write a value to a cell and poll that same cell concurrently, the previous value of the cell will be sent to {out}.", "getrow, getcol, setrow, setcol, setval, write", "out"},
    {"mget", "gets a value from a matrix. Useful to access a matrix at different positions during the same processing frame. Create a mget node pointing to a matrix and access the desired value by setting {row} and {col}.", "row col", "out"},
    {"mset", "sets a value in a matrix. Useful to access a matrix at different positions during the same processing frame. Create a mset node pointing to a matrix and set the desired value by setting {val}, {row} and {col}. The value will be set when {write} receives a non zero value.", "val, row, col, write", "-"},
    {"defaultval", "outputs {val} if {in} is not connected, outputs {in} otherwise. Outputs0 if both {in} and {val} are not connected. Use to set a default value for a module input.", "in, val", "out"},
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
    {"wrap", "wraps {in} in range [{min}..{max}].", "in, min, max", "out"},
    {"equal", "outputs 1 if {in1} and {in2} are equal, 0 otherwise.", "in1, in2", "out"},
    {"nequal", "outputs 1 if {in1} and {in2} are not equal, 0 otherwise.", "in1, in2", "out"},
    {"lower", "outputs 1 if {in1} < {in2}, 0 otherwise.", "in1, in2", "out"},
    {"greater", "outputs 1 if {in1} > {in2}, 0 otherwise.", "in1, in2", "out"},
    {"min", "outputs the lowest value between {in1} and {in2}.", "in1, in2", "out"},
    {"max", "outputs the highest value between {in1} and {in2}.", "in1, in2", "out"},
    {"round", "rounds {in} to the nearest integer value.", "in", "out"},
    {"ceil", "rounds {in} up to the nearest higher integer.", "in", "out"},
    {"floor", "rounds {in} down to the nearest lower integer.", "in", "out"},
    {"frac", "outputs the fractional part of {in} (e.g. 0.5 for 2.5).", "in", "out"},
    {"and", "outputs 1 if both {in1} and {in2} are not 0, outputs 0 otherwise.", "in1 in2", "out"},
    {"or", "outputs 1 if either {in1} or {in2} are not 0, outputs 0 otherwise.", "in1 in2", "out"},
    {"xor", "outputs 1 if one of {in1} and {in2} is not 0, outputs 0 if both are 0 or both are not 0.", "in1 in2", "out"},
    {"mix", "interpolates between {in1} and {in2} according to {coeff} in range [0..1].", "in1, in2, coeff", "out"},
    {"merge", "combines all of its inputs into a single signal. To combine pulses, see pmerge.", "in1, in2, ..., in16", "out"},
    {"pmerge", "combines all of its inputs (pulses) into a single pulse output (works like a giant or node). To combine regular signals, see merge.", "in1, in2, ..., in16", "out"},
    {"map", "maps {in} from the range [{imin}..{imax}] to the range [{omin}..{omax}].", "in, imin, imax, omin, omax", "out"}, 
    {"from0_1", "maps {in} from the range [0..1] to the range [{min}..{max}].", "in, min, max", "out"}, 
    {"to0_1", "maps {in} from the range [{min}..{max}] to the range [0..1].", "in, min, max", "out"}, 
    {"smooth", "smooth value changes at {in} in {dur} milliseconds and outputs the smoothed value.", "in, dur", "out"},
    {"miditofreq", "converts a MIDI note [0..127] to a frequency in Hertz.", "in", "out"},
    {"dbtoamp", "converts a deciBel value to a linear amplitude value.", "in(dB)", "out"},
    {"mstohz", "converts a duration  in milliseconds to a frequency in Hertz.", "in", "out"},
    {"hztoms", "converts a frequency in Hertz to a duration  in milliseconds.", "in", "out"},
    {"samplerate", "outputs the current samplerate.", "-", "out"},
    {"fixdenorm", "zeroes denormal numbers in the signal.", "in", "out"},
    {"fixnan", "zeroes NaN in the signal.", "in", "out"},
    {"count", "outputs the count of non zero signals received at {clock}. Loops back to 0 after reaching {max} (inclusive, defaults to 16).", "clock max", "out"},
    {"phasor", "generates a ramp in the range [0..1]. A pulse at {reset} resets the phase.", "freq(Hz), reset(pulse)", "out"},
    {"pulse", "outputs a pulse (1 sample long signal whose value is 1) at a periodic rate. A pulse at {reset} resets the phase.", "rate(Ms), reset(pulse)", "out"},
    {"sinosc", "generates a sine wave. A pulse at {reset} resets the phase. A signal can be sent to {fm) for frequency modulation with the amount of modulation controled by {fmdepth}.", "freq(Hz), reset(pulse), fm, fmdepth", "out"},
    {"sawosc", "a bandlimited sawtooth oscillator. A pulse at {reset} resets the phase. A signal can be sent to {fm) for frequency modulation with the amount of modulation controled by {fmdepth}.", "freq(Hz), reset(pulse), fm, fmdepth", "out"},
    {"sqrosc", "a bandlimited square oscillator. A pulse at {reset} resets the phase. The pulse width can be  controlled via {pw}. A signal can be sent to {fm) for frequency modulation with the amount of modulation controled by {fmdepth}.", "freq(Hz), reset(pulse), pw([0..1]), fm, fmdepth", "out"},
    {"triosc", "a bandlimited triangle oscillator. A pulse at {reset} resets the phase. The pulse width can be  controlled via {pw}. A signal can be sent to {fm) for frequency modulation with the amount of modulation controled by {fmdepth}.", "freq(Hz), reset(pulse), pw([0..1]), fm, fmdepth", "out"},
    {"noise", "generates white noise.", "-", "out"},
    {"seq8", "outputs the values of inputs {step1} to {step8} sequentially when receiving a pulse at {clock}. The sequence length can be changed via input {length}. The output {pos} sends the playhead position.", "clock(pulse), length(1..8), step1, step2, ..., step8", "out, pos"},
    {"random", "outputs a random value in the range [0..1] when receiving a pulse at {clock}.", "clock", "out"},
    {"irandom", "outputs a random integer value in the range [{min}..{max}] (inclusive) when receiving a pulse at {clock}.", "clock, min, max", "out"},
    {"notescale", "conforms a note value ({note}) to a musical {scale} according to a {root} note. Run tzara --scales to get a list of the available scales.", "note, scale, root", "out"},
    {"segment", "outputs a ramp from {val1} to {val2} in {dur} Ms when receiving a pulse at {clock}. Outputs a pulse at {end} when reaching the end of the segment for chaining segments.", "clock, val1, val2, dur", "out, end(pulse)"},
    {"select", "if {index} is 0, outputs 0 otherwise ouputs the value of the corresponding input.", "index, in1, in2, in3, in4, in5, in6, in7, in8", "out"},
    {"route", "if {index} is greater than 0 and lower than 9, outputs {in} to the corresponding {out}.", "in, index", "out1, out2, out3, out4, out5, out6, out7, out8"},
    {"sah", "samples the value at {in} when receiving a non-zero signal (pulse) at {clock}. Outputs the sampled value", "in clock", "out"},
    {"timepoint", "outputs a pulse at a specific timepoint defined by {time} (in milliseconds). Outputs a pulse on startup if {time} is not set.", "time(Ms)", "out"},
    {"lowpass", "a 1 pole lowpass filter.", "in, cut(Hz)", "out"},
    {"highpass", "a 1 pole highpass filter.", "in, cut(Hz)", "out"},
    {"svf", "a state variable filter. Outputs lowpass, bandpass, highpass and notch.", "in, cut, res[0..1]", "lowpass, bandpass, highpass, notch"},
    {"delay", "a basic delay line (up to 2 seconds).", "in, time(Ms)", "out"},
    {"fdelay", "a delay line with feedback (up to 2 seconds).", "in, time(Ms) feed([0..1])", "out"},
    {"allpass", "an allpass filter (up to 2 seconds of delay time).", "in, time(Ms) gain([0..1])", "out"}
};

int initMatrix (TzMatrix* m, const int numRows, const int numCols) {
    int r = 0;
    if (m == NULL) {
        return 1;
    }

    m->numRows = numRows > 0 ? numRows : 1;
    m->numCols = numCols > 0 ? numCols : 1;

    /* need to do more checks for failed allocations */

    m->matrix = malloc(m->numRows * sizeof(float*));
    for (r = 0; r < m->numRows; ++r) {
        m->matrix[r] = malloc(m->numCols * sizeof(float));
    }
    return 0;
}

void releaseMatrix (TzMatrix* m) {
    int r = 0;
    for (r = 0; r < m->numRows; ++r) {
        if (m->matrix[r] != NULL) {
            free(m->matrix[r]);
        }
    }
    free(m->matrix);
    free(m);
}

void populateMatrixFromFile (FILE* f, TzMatrix* m) {
    char cache[TZMATRIX_PARSER_CACHE_SIZE];
    int r, c = 0;
    char* tok;
    char* nl;

    memset(cache, 0, TZMATRIX_PARSER_CACHE_SIZE);

    r = 0;

    while (r < m->numRows) {
        if (fgets(cache, TZMATRIX_PARSER_CACHE_SIZE, f) != NULL) {

            c = 0;
            tok = strtok(cache, " ");

            while (c  < m->numCols) {
                if (tok != NULL) {
                    /* remove newline */
                    nl = strchr(tok, '\n');
                    if (nl != NULL) *nl = '\0';
                    m->matrix[r][c] = getConstantValue(tok);
                    tok = strtok(NULL, " ");
                }
                else {
                    m->matrix[r][c] = 0.f;
                }
                ++c;
            }

        }
        else {
            for (c = 0; c < m->numCols; ++c) {
                m->matrix[r][c] = 0.f;
            }
        }
        ++r;
    }
}


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
    n->matrix = NULL;
    n->matrixRef = NULL;
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

    if (n->matrix != NULL) {
        releaseMatrix(n->matrix);
        n->matrix = NULL;
    }
    n->matrixRef = NULL;
}

float getNodeInput (TzNode* n, int inputIndex, float defaultValue) {
    return n->inputs[inputIndex] != NULL ? *(n->inputs[inputIndex]) : defaultValue;
}

float getNodeInputClipped (TzNode* n, int inputIndex, float defaultValue, float min, float max) {
    float in =  n->inputs[inputIndex] != NULL ? *(n->inputs[inputIndex]) : defaultValue;
    return tzClip(in, min, max);
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

void performMatrix (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const int getrow = getNodeInputClipped(n, 0, 0.f, 0.f, (float)(n->matrix->numRows - 1));
    const int getcol = getNodeInputClipped(n, 1, 0.f, 0.f, (float)(n->matrix->numCols - 1));
    const int setrow = getNodeInputClipped(n, 2, 0.f, 0.f, (float)(n->matrix->numRows - 1));
    const int setcol = getNodeInputClipped(n, 3, 0.f, 0.f, (float)(n->matrix->numCols - 1));
    const float setval = getNodeInput(n, 4, 0.f);
    const int write = getNodeInput(n, 5, 0.f) != 0.f ? 1 : 0;

    n->outputs[0] = n->matrix->matrix[getrow][getcol];

    if (write != 0) {
        n->matrix->matrix[setrow][setcol] = setval;
    }
}

TzNode* createMatrixNode (int numRows, int numCols, char* filename) {
    FILE* f = NULL;
    int r, c = 0;
    TzNode* n = allocateNewNode();

    n->matrix = malloc(sizeof(TzMatrix));

    if ((n->matrix != NULL) && (initMatrix (n->matrix, numRows, numCols) != 0)) {
        fprintf(stderr, "Failed to create matrix node...\n\n");
        return n;
    }


    if (filename != NULL) {
        f = fopen(filename, "r");
        if (f != NULL) {
            printf("\n== Populating matrix from file : %s ==\n\n", filename);
            populateMatrixFromFile(f, n->matrix);

            printf("\n");
            for (r = 0; r < n->matrix->numRows; ++r) {
                for (c = 0; c < n->matrix->numCols; ++c) {
                    printf("[%.2f] ", n->matrix->matrix[r][c]);
                }
                printf("\n");
            }
            printf("\n");
            fclose(f);
            printf("\n== Matrix populated from file ==\n\n");
        }
        else {
            printf("Unable to open matrix file : %s...\n", filename);
        }
    }

    else {
        for (r = 0; r < n->matrix->numRows; ++r) {
            for (c = 0; c < n->matrix->numCols; ++c) {
                n->matrix->matrix[r][c] = 0.f;
            }
        }
        printf("Initialized empty matrix of %d * %d.\n", n->matrix->numRows, n->matrix->numCols);
    }

    n->numInputs = 6;
    strcpy(n->inputsNames[0], "getrow");
    strcpy(n->inputsNames[1], "getcol");
    strcpy(n->inputsNames[2], "setrow");
    strcpy(n->inputsNames[3], "setcol");
    strcpy(n->inputsNames[4], "setval");
    strcpy(n->inputsNames[5], "write");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");

    n->perform = &performMatrix;
    return n;
}

void performMget (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const int getrow = getNodeInputClipped(n, 0, 0.f, 0.f, (float)(n->matrixRef->numRows - 1));
    const int getcol = getNodeInputClipped(n, 1, 0.f, 0.f, (float)(n->matrixRef->numCols - 1));

    n->outputs[0] = n->matrixRef->matrix[getrow][getcol];
}

TzNode* createMgetNode (TzMatrix* parentMatrix) {
    if (parentMatrix == NULL) {
        printf("Invalid matrix reference!\n");
        return NULL;
    }
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "row");
    strcpy(n->inputsNames[1], "col");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->matrixRef = parentMatrix;
    n->perform = &performMget;
    return n;
}

void performMset (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float setval = getNodeInput(n, 0, 0.f);
    const int setrow = getNodeInputClipped(n, 1, 0.f, 0.f, (float)(n->matrixRef->numRows - 1));
    const int setcol = getNodeInputClipped(n, 2, 0.f, 0.f, (float)(n->matrixRef->numCols - 1));
    const int write = getNodeInput(n, 3, 0.f) != 0.f ? 1 : 0;

    if (write != 0) {
        n->matrixRef->matrix[setrow][setcol] = setval;
    }
}

TzNode* createMsetNode (TzMatrix* parentMatrix) {
    if (parentMatrix == NULL) {
        printf("Invalid matrix reference!\n");
        return NULL;
    }
    TzNode* n = allocateNewNode();
    n->numInputs = 4;
    strcpy(n->inputsNames[0], "val");
    strcpy(n->inputsNames[1], "row");
    strcpy(n->inputsNames[2], "col");
    strcpy(n->inputsNames[3], "write");
    n->numOutputs = 0;
    n->matrixRef = parentMatrix;
    n->perform = &performMset;
    return n;
}



void performDefaultval (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float* in = n->inputs[0];
    if (in != NULL) {
        n->outputs[0] = *in;
    }
    else {
        n->outputs[0] = getNodeInput(n, 1, 0.f);
    }
}

TzNode* createDefaultvalNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "val");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performDefaultval;
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
    const float exp = getNodeInput(n, 1, 1.f);

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
    float min = getNodeInput(n, 1, -1.f);
    float max = getNodeInput(n, 2, 1.f);

    n->outputs[0] = tzClip(in, min, max);
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

void performWrap (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float in = getNodeInput(n, 0, 0.f);
    float min = getNodeInput(n, 1, -1.f);
    float max = getNodeInput(n, 2, 1.f);

    n->outputs[0] = tzWrap(in, min, max);
}

TzNode* createWrapNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "min");
    strcpy(n->inputsNames[2], "max");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performWrap;
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
    n->outputs[0] = tzMin(in1, in2);
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
    n->outputs[0] = tzMax(in1, in2);
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
    n->outputs[0] = (float)tzRoundToInt(in);
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

void performCeil (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = ceil(in);
}

TzNode* createCeilNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performCeil;
    return n;
}

void performFloor (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = floor(in);
}

TzNode* createFloorNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performFloor;
    return n;
}

void performFrac (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    n->outputs[0] = tzFrac(in);
}

TzNode* createFracNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performFrac;
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
    float coeff = getNodeInputClipped(n, 2, 0.f, 0.f, 1.f);
    
    n->outputs[0] = tzLinInterp(a, b, coeff);
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

void performMerge (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float out = 0.f;
    int i = 0;

    for (i = 0; i < 16; ++i) {
        out += getNodeInput(n, i, 0.f);
    }

    n->outputs[0] = out;
}

TzNode* createMergeNode () {
    int i = 0;
    char inName[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 16;
    for (i = 0; i < 16; ++i) {
        sprintf(inName, "in%d", i);
        strcpy(n->inputsNames[i], inName);
    }
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMerge;
    return n;
}

void performPmerge (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float out = 0.f;
    int i = 0;

    for (i = 0; i < 16; ++i) {
        if (getNodeInput(n, i, 0.f) != 0.f) out = 1.f;
    }

    n->outputs[0] = out;
}

TzNode* createPmergeNode () {
    int i = 0;
    char inName[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 16;
    for (i = 0; i < 16; ++i) {
        sprintf(inName, "in%d", i);
        strcpy(n->inputsNames[i], inName);
    }
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performPmerge;
    return n;
}

void performMap (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float in = getNodeInput(n, 0, 0.f);
    float imin = getNodeInput(n, 1, 0.f);
    float imax = getNodeInput(n, 2, 1.f);
    float omin = getNodeInput(n, 3, 0.f);
    float omax = getNodeInput(n, 4, 1.f);

    n->outputs[0] = tzMapToRange(in, imin, imax, omin, omax);
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
    n->perform = &performMap;
    return n;
}

void performFrom0_1 (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float in = getNodeInputClipped(n, 0, 0.f, 0.f, 1.f);
    float min = getNodeInput(n, 1, 0.f);
    float max = getNodeInput(n, 2, 1.f);

    n->outputs[0] = tzMapFrom0_1(in, min, max);
}

TzNode* createFrom0_1Node () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "min");
    strcpy(n->inputsNames[2], "max");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performFrom0_1;
    return n;
}

void performTo0_1 (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    float in = getNodeInput(n, 0, 0.f);
    float min = getNodeInput(n, 1, 0.f);
    float max = getNodeInput(n, 2, 1.f);

    n->outputs[0] = tzMapTo0_1(in, min, max);
}

TzNode* createTo0_1Node () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "min");
    strcpy(n->inputsNames[2], "max");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performTo0_1;
    return n;
}

void performSmooth (TzNode* n, TzProcessInfo* info) {
    float in = getNodeInput(n, 0, 0.f); 
    float dur = getNodeInput(n, 1, 1.f);
    float* val = &(n->memory[0]);
    float* startFlag = &(n->memory[1]);

    if (*startFlag == 0.f) {
        /* jump straight to input value on startup */
        *val = in;
        *startFlag = 1.f;
    }
    else if (dur <= 0.f) {
        *val = in;
    }
    else {
        tzSmooth(val, in, dur, info->samplerate);
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

    float midi = getNodeInputClipped(n, 0, 0.f, 0.f, 127.f);

    n->outputs[0] = tzMIDIToFreq(midi);
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

void performDbtoamp (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float db = getNodeInput(n, 0, 0.f);

    n->outputs[0] = tzDecibelsToAmp(db);
}

TzNode* createDbtoampNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performDbtoamp;
    return n;
}

void performMstohz (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float ms = getNodeInput(n, 0, 0.f);

    n->outputs[0] = tzMsToHz(ms);
}

TzNode* createMstohzNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performMstohz;
    return n;
}

void performHztoms (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float hz = getNodeInput(n, 0, 0.f);

    n->outputs[0] = tzHzToMs(hz); 
}

TzNode* createHztomsNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performHztoms;
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


void performFixdenorm (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    if (n->inputs[0] != NULL) {
        tzFixDenormals(n->inputs[0]);
        n->outputs[0] = *(n->inputs[0]);
    }
}

TzNode* createFixdenormNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performFixdenorm;
    return n;
}

void performFixnan (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    if (n->inputs[0] != NULL) {
        tzFixNaN(n->inputs[0]);
        n->outputs[0] = *(n->inputs[0]);
    }
}

TzNode* createFixnanNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performFixnan;
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
    float freq = getNodeInput(n, 0, 440.f);
    float reset = getNodeInput(n, 1, 0.f);
    float* phase = &(n->memory[0]);

    if (reset > 0) *phase = 0.f;

    n->outputs[0] = tzPhasor(freq, info->samplerate, phase);
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
    const float rate = getNodeInput(n, 0, 100.f);
    const float period = tzMsToSamples(rate, info->samplerate);
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
    const float freq = getNodeInput(n, 0, 440.f);
    const float reset = getNodeInput(n, 1, 0.f);
    const float fm = getNodeInput(n, 2, 0.f);
    const float fmdepth = getNodeInput(n, 3, 0.f);
    float* phase = &(n->memory[0]);

    if (reset > 0) {
        *phase = 0.f;
    }

    n->outputs[0] = tzSinewave(freq + (fm * fmdepth), info->samplerate, phase);
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

void performSawosc (TzNode* n, TzProcessInfo* info) {
    const float freq = getNodeInput(n, 0, 440.f);
    const float reset = getNodeInput(n, 1, 0.f);
    const float fm = getNodeInput(n, 2, 0.f);
    const float fmdepth = getNodeInput(n, 3, 0.f);
    float* phase = &(n->memory[0]);

    if (reset > 0) {
        *phase = 0.f;
    }

    n->outputs[0] = tzPolyblepSaw(freq + (fm * fmdepth), info->samplerate, phase);
}

TzNode* createSawoscNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 4;
    strcpy(n->inputsNames[0], "freq");
    strcpy(n->inputsNames[1], "reset");
    strcpy(n->inputsNames[2], "fm");
    strcpy(n->inputsNames[3], "fmdepth");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performSawosc;
    return n;
}

void performSqrosc (TzNode* n, TzProcessInfo* info) {
    const float freq = getNodeInput(n, 0, 440.f);
    const float reset = getNodeInput(n, 1, 0.f);
    const float pw = getNodeInputClipped(n, 2, 0.5f, 0.f, 1.f);
    const float fm = getNodeInput(n, 3, 0.f);
    const float fmdepth = getNodeInput(n, 4, 0.f);
    float* phase = &(n->memory[0]);

    if (reset > 0) {
        *phase = 0.f;
    }

    n->outputs[0] = tzPolyblepSquare(freq + (fm * fmdepth), pw, info->samplerate, phase);
}

TzNode* createSqroscNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 5;
    strcpy(n->inputsNames[0], "freq");
    strcpy(n->inputsNames[1], "reset");
    strcpy(n->inputsNames[2], "pw");
    strcpy(n->inputsNames[3], "fm");
    strcpy(n->inputsNames[4], "fmdepth");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performSqrosc;
    return n;
}

void performTriosc (TzNode* n, TzProcessInfo* info) {
    const float freq = getNodeInput(n, 0, 440.f);
    const float reset = getNodeInput(n, 1, 0.f);
    const float pw = getNodeInputClipped(n, 2, 0.5f, 0.f, 1.f);
    const float fm = getNodeInput(n, 3, 0.f);
    const float fmdepth = getNodeInput(n, 4, 0.f);
    float* phase = &(n->memory[0]);
    float* mem = &(n->memory[1]);

    if (reset > 0) {
        *phase = 0.f;
    }

    n->outputs[0] = tzPolyblepTriangle(freq + (fm * fmdepth), pw, info->samplerate, phase, mem);
}

TzNode* createTrioscNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 5;
    strcpy(n->inputsNames[0], "freq");
    strcpy(n->inputsNames[1], "reset");
    strcpy(n->inputsNames[2], "pw");
    strcpy(n->inputsNames[3], "fm");
    strcpy(n->inputsNames[4], "fmdepth");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
    n->perform = &performTriosc;
    return n;
}

void performNoise (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    n->outputs[0] = tzWhiteNoise();
}

TzNode* createNoiseNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 0;
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performNoise;
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
        *out = tzRandom();
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

void performIrandom (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const int clock = (int)getNodeInput(n, 0, 0.f);
    int min = (int)getNodeInput(n, 1, 0.f);
    int max = (int)getNodeInput(n, 2, 1.f);
    float* out = &(n->memory[0]);

    if (clock != 0) {
        *out = (float)tzRandomInt((int)min, (int)max);
    }

    n->outputs[0] = *out;
}

TzNode* createIrandomNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "clock");
    strcpy(n->inputsNames[1], "min");
    strcpy(n->inputsNames[2], "max");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performIrandom;
    return n;
}

const int musicalScales [NUM_SCALES][12] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11},
    {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10},
    {0, 0, 2, 2, 4, 5, 5, 7, 8, 8, 8, 11},
    {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 8, 11},
    {0, 1, 1, 3, 3, 5, 6, 6, 8, 8, 10, 10},
    {0, 0, 2, 3, 3, 5, 6, 6, 6, 9, 9, 9},
    {0, 0, 2, 2, 4, 4, 6, 6, 6, 9, 9, 9},
    {0, 0, 2, 3, 3, 3, 3, 7, 8, 8, 8, 8},
    {0, 1, 1, 3, 3, 5, 5, 5, 8, 8, 10, 10},
    {0, 0, 2, 2, 4, 4, 6, 7, 7, 7, 10, 11},
    {0, 0, 0, 0, 4, 4, 4, 4, 4, 9, 9, 9},
    {0, 1, 1, 1, 4, 5, 5, 7, 8, 8, 10, 11},
    {0, 0, 2, 3, 3, 3, 6, 7, 7, 9, 10, 10}
};

void performNotescale (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    int note = (int)getNodeInputClipped(n, 0, 0.f, 0.f, 127.f);
    int scale = (int)getNodeInputClipped(n, 1, 0.f, 0.f, (float)(NUM_SCALES - 1));
    int root = (int)getNodeInputClipped(n, 2, 0.f, 0.f, 11.f);

    n->outputs[0] = (float)tzConformNoteToScale(note, musicalScales[scale], root);
}

TzNode* createNotescaleNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "note");
    strcpy(n->inputsNames[1], "scale");
    strcpy(n->inputsNames[2], "root");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->perform = &performNotescale;
    return n;
}


void performSegment (TzNode* n, TzProcessInfo* info) {
    const float dur = getNodeInput(n, 3, 10.f);
    const float v1 = getNodeInput(n, 1, 0.f);
    const float v2 = getNodeInput(n, 2, 0.f);

    const float length = tzMsToSamples((dur > 0 ? dur : 10.f), info->samplerate);
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

void performRoute (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);

    const float in = getNodeInput(n, 0, 0.f);
    int idx = (int)getNodeInput(n, 1, 0.f);
    int i = 0;

    for (i = 0; i < 8; ++i) {
        n->outputs[i] = (idx == (i + 1)) ? in : 0.f;
    }
}

TzNode* createRouteNode () {
    int i = 0;
    char outName[32];

    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "index");
    n->numOutputs = 8;
    for (i = 0; i < 8; ++i) {
        sprintf(outName, "out%d", i + 1);
        strcpy(n->outputsNames[i], outName);
    }
    n->perform = &performRoute;
    return n;
}

void performSah (TzNode* n, TzProcessInfo* info) {
    TZ_UNUSED(info);
    const float in = getNodeInput(n, 0, 0.f);
    const float clock = getNodeInput(n, 1, 0.f);
    float* smp = &(n->memory[0]);

    if (clock != 0.f) {
        *smp = in;
    }

    n->outputs[0] = *smp;
}

TzNode* createSahNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "clock");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performSah;
    return n;
}

void performTimepoint (TzNode* n, TzProcessInfo* info) {
    float time = getNodeInput(n, 0, 0.f);
    float* pos = &(n->memory[0]);

    time = tzMsToSamples(time, info->samplerate);
    time = (float)tzRoundToInt(time); /* round to int for comparison */

    n->outputs[0] = *pos == time ? 1.f : 0.f;

    *pos += 1;
}

TzNode* createTimepointNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "time");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performTimepoint;
    return n;
}

void performLowpass (TzNode* n, TzProcessInfo* info) {
    float in = getNodeInput(n, 0, 0.f);
    float cut = getNodeInput(n, 1, 11000.f);
    float* z1 = &(n->memory[0]);

    n->outputs[0] = tzOnePoleLowpass(in, cut, info->samplerate, z1);
}

TzNode* createLowpassNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "cut");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performLowpass;
    return n;
}

void performHighpass (TzNode* n, TzProcessInfo* info) {
    float in = getNodeInput(n, 0, 0.f);
    float cut = getNodeInput(n, 1, 100.f);
    float* z1 = &(n->memory[0]);

    n->outputs[0] = tzOnePoleHighpass(in, cut, info->samplerate, z1);
}

TzNode* createHighpassNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 2;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "cut");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->perform = &performHighpass;
    return n;
}


void performSvf (TzNode* n, TzProcessInfo* info) {
    float in = getNodeInput(n, 0, 0.f);
    float cut = getNodeInput(n, 1, 1100.f);
    float res = getNodeInputClipped(n, 2, 0.5f, 0.f, 1.f);
    float* ic1eq = &(n->memory[0]);
    float* ic2eq = &(n->memory[1]);
    float* low = &(n->outputs[0]);
    float* band = &(n->outputs[1]);
    float* high = &(n->outputs[2]);
    float* notch = &(n->outputs[3]);
    TZSvfOutputs svfOut;

    svfOut = tzStateVariableFilter(in, cut, res, info->samplerate, ic1eq, ic2eq);
    *low = svfOut.lowpass;
    *band = svfOut.bandpass;
    *high = svfOut.highpass;
    *notch = svfOut.notch;
}

TzNode* createSvfNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "cut");
    strcpy(n->inputsNames[2], "res");
    n->numOutputs = 4;
    strcpy(n->outputsNames[0], "lowpass");
    strcpy(n->outputsNames[1], "bandpass");
    strcpy(n->outputsNames[2], "highpass");
    strcpy(n->outputsNames[3], "notch");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
    n->perform = &performSvf;
    return n;
}


void performDelay (TzNode* n, TzProcessInfo* info) {
    const int maxLength = (2 * (int)(info->samplerate)) + 16; /* small padding for safe access */
    float in  = getNodeInput(n, 0, 0.f);
    float time = getNodeInput(n, 1, 1.f);
    float* pos = &(n->memory[0]);
    float* maxpos = &(n->memory[1]);
    float* startupFlag = &(n->memory[2]);
    
    if (*startupFlag < 1.f) {
        /* init buffer */
        n->buffers[0] = malloc(maxLength * sizeof(float));
        if (n->buffers[0] == NULL) {
            printf("Failed to allocate delay buffer.\n");
        }
        memset(n->buffers[0], 0, maxLength);
        *maxpos = (2.f * info->samplerate) - 1;
        *startupFlag = 1.f;
    }

    n->outputs[0] = tzDelay(in, time, info->samplerate, n->buffers[0], (int)(*maxpos), pos);
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
    float in  = getNodeInput(n, 0, 0.f);
    float time = getNodeInput(n, 1, 1.f);
    float feed = getNodeInput(n, 2, 0.f);
    float* pos = &(n->memory[0]);
    float* maxpos = &(n->memory[1]);
    float* startupFlag = &(n->memory[2]);
    
    if (*startupFlag < 1.f) {
        /* init buffer */
        n->buffers[0] = malloc(maxLength * sizeof(float));
        if (n->buffers[0] == NULL) {
            printf("Failed to allocate delay buffer.\n");
        }
        memset(n->buffers[0], 0, maxLength);
        *maxpos = (2.f * info->samplerate) - 1;
        *startupFlag = 1.f;
    }

    n->outputs[0] = tzFeedbackDelay(in, time, feed, info->samplerate, n->buffers[0], (int)(*maxpos), pos);
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

void performAllpass (TzNode* n, TzProcessInfo* info) {
    const int maxLength = (2 * (int)(info->samplerate)) + 16; /* small padding for safe access */
    float in  = getNodeInput(n, 0, 0.f);
    float time = getNodeInput(n, 1, 1.f);
    float gain = getNodeInput(n, 2, 0.f);
    float* pos = &(n->memory[0]);
    float* maxpos = &(n->memory[1]);
    float* startupFlag = &(n->memory[2]);
    
    if (*startupFlag < 1.f) {
        /* init buffer */
        n->buffers[0] = malloc(maxLength * sizeof(float));
        if (n->buffers[0] == NULL) {
            printf("Failed to allocate delay buffer.\n");
        }
        memset(n->buffers[0], 0, maxLength);
        *maxpos = (2.f * info->samplerate) - 1;
        *startupFlag = 1.f;
    }

    n->outputs[0] = tzAllpassDelay(in, time, gain, info->samplerate, n->buffers[0], (int)(*maxpos), pos);
}

TzNode* createAllpassNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 3;
    strcpy(n->inputsNames[0], "in");
    strcpy(n->inputsNames[1], "time");
    strcpy(n->inputsNames[2], "gain");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out");
    n->memory[0] = 0.f;
    n->memory[1] = 0.f;
    n->memory[2] = 0.f;
    n->perform = &performAllpass;
    return n;
}

