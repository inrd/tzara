#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodes.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


#define TZARA_MAX_OUTPUT_CHANS 2
#define TZARA_MAX_NODES 4096

#define PARSER_CACHE_SIZE 1024

#define TZARA_OUTPUT_NODE_INDEX -0xaa
#define TZARA_OUTPUT_LEFT_INDEX -0xbb
#define TZARA_OUTPUT_RIGHT_INDEX -0xcc

#define TZARA_WAV_DURATION_SEC 60
#define TZARA_BUFFER_SIZE 4096

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

void init(Tzara* t, const int numChans) {
    int i = 0;
    for (i = 0; i < TZARA_MAX_NODES; ++i) {
        t->nodes[i] = NULL;
    }
    t->numNodes = 0;

    for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
        t->outputs[i] = NULL;
    }
}

int addNode (Tzara* tz, TzNode* n, const char* name) {
    if (tz->numNodes < (TZARA_MAX_NODES - 1)) {
        strncpy(n->name, name, sizeof(n->name));
        tz->nodes[tz->numNodes] = n;
        ++tz->numNodes;
        return NO_ERROR;
    }
    return MAX_MEMORY;
}

void connectModules (Tzara* tz, int inModule, int inOutput, int outModule, int outInput) {
    if ((outInput < tz->nodes[outModule]->numInputs) && (inOutput < tz->nodes[inModule]->numOutputs)) {
        if (tz->nodes[outModule]->inputs[outInput] != NULL) {
            fprintf(stdout, "Warning : node input was already connected. Replacing connection.\n");
        }
        tz->nodes[outModule]->inputs[outInput] = &(tz->nodes[inModule]->outputs[inOutput]);
    }
    else {
        /* TODO : should abort */
        fprintf(stderr, "Invalid routing...\n");
    }
}

void connectModuleToOutput (Tzara* tz, int inModule, int inOutput, int outInput) {
    if ((outInput < TZARA_MAX_OUTPUT_CHANS) && (inOutput < tz->nodes[inModule]->numOutputs)) {
        if (tz->outputs[outInput] != NULL) {
            fprintf(stdout, "Warning : out channel %d was already connected. Replacing connection.\n", outInput);
        }
        tz->outputs[outInput] = &(tz->nodes[inModule]->outputs[inOutput]);
    }
    else {
        /* TODO : should abort */
        fprintf(stderr, "Invalid routing...\n");
    }
}

void process (Tzara* tz, float** out, int numChans, int numSamps, float samplerate) {
    int i = 0;
    int n = 0;
    int c = 0;

    for (i = 0; i < numSamps; ++i) {
        /* naive implementation */
        for (n = 0; n < tz->numNodes; ++n) {
            tz->nodes[n]->perform(tz->nodes[n], samplerate);
        }

        for (c = 0; c < numChans; ++c) {
            out[c][i] = (c < TZARA_MAX_OUTPUT_CHANS) && (tz->outputs[c] != NULL)? *(tz->outputs[c]) : 0.f;
        }
    }
}

void release (Tzara* tz) {
    int i = 0;
    for (i = 0; i < tz->numNodes; ++i) {
        free(tz->nodes[i]);
        tz->nodes[i] = NULL;
    }
    for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
        tz->outputs[i] = NULL;
    }
}

enum Operators {
    NO_OP = 0,
    COMMENT_OP,
    CREATE_NODE_OP,
    CREATE_CONSTANT_OP,
    CONNECT_OP
};

int parseOperator (char op) {
    switch (op) {
        case '#':
            return COMMENT_OP;
            break;

        case '+':
            return CREATE_NODE_OP;
            break;

        case '=':
            return CREATE_CONSTANT_OP;
            break;

        case '>':
            return CONNECT_OP;
            break;

        default:
            return NO_OP;
            break;
    }
}

void parseCommentInstruction (const char* instr) {
    fprintf(stdout, "%s", instr);
}

int parseNodeType (const char* name) {
    const char* adderID = "add";
    const char* phasorID = "phasor";
    const int nameLength = strlen(name);

    if (strncmp(name, adderID, nameLength) == 0) return ADDER_NODE;
    if (strncmp(name, phasorID, nameLength) == 0) return PHASOR_NODE;

    return INVALID_NODE_TYPE;
}

void trimNewLine (char* str) {
    const int strSize = strlen(str);
    int i = 0;
    for (i = 0; i < strSize; ++i) {
        if (str[i] == '\n') {
            str[i] = '\0';
        }
    }
}

void parseCreateNodeInstruction (Tzara* tz, char* instr) {
    char* token;
    int nodeType = INVALID_NODE_TYPE;
    char name [TZNODE_NAME_SIZE];
    int ic = 0;

    token = strtok(instr, " ");

    while (token != NULL) {
        /* drop first token (operator) */
        switch (ic) {
            case 1:
                nodeType = parseNodeType(token);
                break;
            case 2:
                trimNewLine(token);
                strncpy(name, token, sizeof(name));
                break;
            default:
                break;
        }
        token = strtok(NULL, " ");
        ++ic;
    }

    if (ic < 3) {
        fprintf(stderr, "Not enough arguments...\n");
        /*TODO: abort */
        return;
    }

    switch(nodeType) {
        case ADDER_NODE:
            printf("Creating adder : %s\n", name);
            addNode(tz, createAdderNode(), name);
            break;
        
        case PHASOR_NODE:
            printf("Creating phasor : %s\n", name);
            addNode(tz, createPhasorNode(), name);
            break;
        default:
            /* TODO: abort*/
            fprintf(stderr, "Could not create node : invalid node type...\n");
            break;
    }
}

int searchNode (Tzara* tz, const char* name) {
    int i = 0;
    int nameLength = strlen(name);
    for (i = 0; i < tz->numNodes; ++i) {
        if (nameLength == strlen(tz->nodes[i]->name)) {
            if (strncmp(tz->nodes[i]->name, name, nameLength) == 0) {
                return i;
            }
        }
    }
    return -1;
}

int searchInput (TzNode* node, const char* name) {
    int i = 0;
    int nameLength = strlen(name);
    for (i = 0; i < node->numInputs; ++i) {
        if (nameLength == strlen(node->inputsNames[i])) {
            if (strncmp(node->inputsNames[i], name, nameLength) == 0) {
                return i;
            }
        }
    }
    return -1;
}

int searchOutput (TzNode* node, const char* name) {
    int i = 0;
    int nameLength = strlen(name);
    for (i = 0; i < node->numOutputs; ++i) {
        if (nameLength == strlen(node->outputsNames[i])) {
            if (strncmp(node->outputsNames[i], name, nameLength) == 0) {
                return i;
            }
        }
    }
    return -1;
}

void parseNodeInputString (Tzara* tz, char* str, int* node, int* input) {
    char token[TZNODE_NAME_SIZE];
    int i = 0;
    int offset = 0;

    memset(token, '\0', TZNODE_NAME_SIZE);

    while(str[i] != '@' && str[i] != '\0') {
        token[i]  = str[i];
        ++i;
    }

    if (strncmp(token, "out", 3) == 0) {
        *node = TZARA_OUTPUT_NODE_INDEX;
    }
    else {
        *node = searchNode(tz, token);
    }
    
    if (str[i] == '\0') {
        fprintf(stderr, "Incorrect argument : %s\n", str);
        return;
    }

    ++i;
    offset = i;
    memset(token, '\0', TZNODE_NAME_SIZE);

    while (str[i] != '\0') {
        token[i -  offset] = str[i];
        ++i;
    }

    if (*node == TZARA_OUTPUT_NODE_INDEX) {
        if (token[0] == 'l') {
            *input = TZARA_OUTPUT_LEFT_INDEX;
        }
        else if (token[0] == 'r') {
            *input = TZARA_OUTPUT_RIGHT_INDEX;
        }
        else {
            *input = -1;
        }
    }
    else {
        *input = (*node >= 0) ? searchInput(tz->nodes[*node], token) : -1;
    }
}

void parseNodeOutputString (Tzara* tz, char* str, int* node, int* output) {
    char token[TZNODE_NAME_SIZE];
    int i = 0;
    int offset = 0;

    memset(token, '\0', TZNODE_NAME_SIZE);

    while(str[i] != '@' && str[i] != '\0') {
        token[i]  = str[i];
        ++i;
    }

    *node = searchNode(tz, token);

    if (str[i] == '\0') {
        fprintf(stderr, "Incorrect argument : %s\n", str);
        return;
    }

    ++i;
    offset = i;
    memset(token, '\0', TZNODE_NAME_SIZE);

    while (str[i] != '\0') {
        token[i -  offset] = str[i];
        ++i;
    }

    *output = (*node >= 0) ? searchOutput(tz->nodes[*node], token) : -1;
}



void parseCreateConstantInstruction (Tzara* tz, char* instr) {
    char* token;
    float val = 0.f;
    int node = -1;
    int input = -1;
    int ic = 0;

    token = strtok(instr, " ");

    while (token != NULL) {
        /* drop first token (operator) */
        switch (ic) {
            case 1:
                val = atof(token);
                break;
            case 2:
                trimNewLine(token);
                parseNodeInputString(tz, token, &node, &input);
                break;
            default:
                break;
        }
        token = strtok(NULL, " ");
        ++ic;
    }

    if (ic < 3) {
        fprintf(stderr, "Not enough arguments...\n");
        /*TODO: abort */
        return;
    }

    if (node == TZARA_OUTPUT_NODE_INDEX) {
        if (input == TZARA_OUTPUT_LEFT_INDEX) {
            printf("Map constant with value %f to out[L]\n", val);
            addNode(tz, createConstantNode(val), "\0");
            connectModuleToOutput(tz, tz->numNodes - 1, 0, 0);
        }
        else if (input == TZARA_OUTPUT_RIGHT_INDEX) {
            printf("Map constant with value %f to out[R]\n", val);
            addNode(tz, createConstantNode(val), "\0");
            connectModuleToOutput(tz, tz->numNodes - 1, 0, 1);
        }
        else {
            fprintf(stderr, "Invalid Input...\n");
        }
        return;
    }

    if (node < 0 || input < 0) {
        fprintf(stderr, "Invalid node input...\n");
        /* TODO: abort */
        return;
    }

    printf("Map constant with value %f to %s[%s]\n", val, tz->nodes[node]->name, tz->nodes[node]->inputsNames[input]);
    addNode(tz, createConstantNode(val), "\0");
    connectModules(tz, tz->numNodes - 1, 0, node, input); 
}

void parseConnectInstruction (Tzara* tz, char* instr) {
    char* token;
    int srcNode = -1;
    int srcOutput = -1;
    int destNode = -1;
    int destInput = -1;
    int ic = 0;

    token = strtok(instr, " ");

    while (token != NULL) {
        /* drop first token (operator) */
        switch (ic) {
            case 1:
                parseNodeOutputString(tz, token, &srcNode, &srcOutput);
                break;
            case 2:
                trimNewLine(token);
                parseNodeInputString(tz, token, &destNode, &destInput);
                break;
            default:
                break;
        }
        token = strtok(NULL, " ");
        ++ic;
    }

    if (ic < 2) {
        fprintf(stderr, "Not enough arguments...\n");
        /*TODO: abort */
        return;
    }

    if (destNode == TZARA_OUTPUT_NODE_INDEX) {
        if (srcNode < 0 || srcOutput < 0) {
            fprintf(stderr, "Invalid connection...\n");
            /* TODO: abort */
            return;
        }
        if (destInput == TZARA_OUTPUT_LEFT_INDEX) {
            printf("Connect %s[%s] to out[L]\n", tz->nodes[srcNode]->name, tz->nodes[srcNode]->outputsNames[srcOutput]);
            connectModuleToOutput(tz, srcNode, srcOutput, 0);
        }
        else if (destInput == TZARA_OUTPUT_RIGHT_INDEX) {
            printf("Connect %s[%s] to out[R]\n", tz->nodes[srcNode]->name, tz->nodes[srcNode]->outputsNames[srcOutput]);
            connectModuleToOutput(tz, srcNode, srcOutput, 1);
        }
        else {
            fprintf(stderr, "Invalid Input...\n");
        }
        return;
    }

    if (srcNode < 0 || srcOutput < 0 || destNode < 0 || destInput < 0) {
        fprintf(stderr, "Invalid connection...\n");
        /* TODO: abort */
        return;
    }

    printf("Connect %s[%s] to %s[%s]\n", tz->nodes[srcNode]->name, tz->nodes[srcNode]->outputsNames[srcOutput], tz->nodes[destNode]->name, tz->nodes[destNode]->inputsNames[destInput]);
    connectModules(tz, srcNode, srcOutput, destNode, destInput); 
}

void parseInstruction (Tzara* tz, char*  instr) {
    const int op = parseOperator(instr[0]);
    switch (op) {
        case COMMENT_OP:
            parseCommentInstruction(instr);
            break;

        case CREATE_NODE_OP:
            parseCreateNodeInstruction(tz, instr);
            break;

        case CREATE_CONSTANT_OP:
            parseCreateConstantInstruction(tz, instr);
            break;

        case CONNECT_OP:
            parseConnectInstruction(tz, instr);
            break;

        default:
            break;
    }
}

void parsePatch (Tzara* tz, FILE* patch) {
    char cache[PARSER_CACHE_SIZE];
    int i;
    for (i = 0; i < PARSER_CACHE_SIZE; ++i) {
        cache[i] = 0;
    }
    while (fgets(cache, PARSER_CACHE_SIZE, patch) != NULL) {
        parseInstruction(tz, cache);
    }
}



int main (int argc, char** argv) {
    const int numChans = TZARA_MAX_OUTPUT_CHANS;
    float* data[TZARA_MAX_OUTPUT_CHANS];
    FILE* patch = NULL;
    Tzara tz;
    const float samplerate = 44100.f;
    drwav wav;
    float* outData;
    int error = NO_ERROR;
    int i, j = 0;
    unsigned long int numFrames = (unsigned long int)samplerate * TZARA_WAV_DURATION_SEC;
    unsigned long int framesCount = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage:\ntzara [patchfile]\n\n");
        return 1;
    }

    patch = fopen(argv[1], "r");
    if (patch == NULL) {
        fprintf(stderr, "Could not open %s...\n", argv[1]);
        return 1;
    }

    init(&tz, numChans);

    parsePatch (&tz, patch);

    fclose(patch);

    printf("Rendering output...\n");

    for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
        data[i] = (float*)malloc(TZARA_BUFFER_SIZE * sizeof(float));
    }

    drwav_data_format format;
    format.container = drwav_container_riff;     
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 2;
    format.sampleRate = (int)samplerate;
    format.bitsPerSample = 32;
    drwav_init_file_write(&wav, "out.wav", &format, NULL);

    outData = (float*)malloc(TZARA_BUFFER_SIZE * 2 * sizeof(float));

    while (framesCount < numFrames) {
        process (&tz, data, numChans, TZARA_BUFFER_SIZE, samplerate);

        for (i = 0, j = 0; j < TZARA_BUFFER_SIZE; i += 2, ++j) {
            outData[i] = data[0][j];
            outData[i+1] = data[1][j];
        }
        
        drwav_write_pcm_frames(&wav, TZARA_BUFFER_SIZE, outData);

        framesCount += TZARA_BUFFER_SIZE;
    }

    drwav_uninit(&wav);

    printf("Rendered to out.wav\n\n");

    release(&tz);

    return 0;
}

