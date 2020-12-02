#include "parser.h"

#include <stdlib.h>
#include <string.h>


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
    const int nameLength = strlen(name);

    /* TODO: loop around nodesDoc to retrieve the names */
    if (strncmp(name, "add", nameLength) == 0) return ADDER_NODE;
    if (strncmp(name, "mult", nameLength) == 0) return MULT_NODE;
    if (strncmp(name, "clip", nameLength) == 0) return CLIP_NODE;
    if (strncmp(name, "mix", nameLength) == 0) return MIX_NODE;
    if (strncmp(name, "mem", nameLength) == 0) return MEM_NODE;
    if (strncmp(name, "phasor", nameLength) == 0) return PHASOR_NODE;
    if (strncmp(name, "sinosc", nameLength) == 0) return SINOSC_NODE;

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

int parseCreateNodeInstruction (Tzara* tz, char* instr) {
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
                strncpy(name, token, sizeof(name) - 1);
                break;
            default:
                break;
        }
        token = strtok(NULL, " ");
        ++ic;
    }

    if (ic < 3) {
        fprintf(stderr, "Not enough arguments...\n");
        return 1;
    }

    switch(nodeType) {
        case ADDER_NODE:
            printf("Creating adder : %s\n", name);
            addNode(tz, createAdderNode(), name);
            break;
        
        case MULT_NODE:
            printf("Creating multiplier : %s\n", name);
            addNode(tz, createMultNode(), name);
            break;
        
        case CLIP_NODE:
            printf("Creating clipper : %s\n", name);
            addNode(tz, createClipNode(), name);
            break;
        
        case MIX_NODE:
            printf("Creating mixer : %s\n", name);
            addNode(tz, createMixNode(), name);
            break;
        
        case MEM_NODE:
            printf("Creating single sample delay : %s\n", name);
            addNode(tz, createMemNode(), name);
            break;
        
        case PHASOR_NODE:
            printf("Creating phasor : %s\n", name);
            addNode(tz, createPhasorNode(), name);
            break;

        case SINOSC_NODE:
            printf("Creating sinosc : %s\n", name);
            addNode(tz, createSinoscNode(), name);
            break;

        default:
            fprintf(stderr, "Could not create node : invalid node type...\n");
            return 1;
            break;
    }

    return 0;
}


int searchNode (Tzara* tz, const char* name) {
    int i = 0;
    int nameLength = strlen(name);
    for (i = 0; i < tz->numNodes; ++i) {
        if (nameLength == (int)strlen(tz->nodes[i]->name)) {
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
        if (nameLength == (int)strlen(node->inputsNames[i])) {
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
        if (nameLength == (int)strlen(node->outputsNames[i])) {
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

int parseCreateConstantInstruction (Tzara* tz, char* instr) {
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
        return 1;
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
            return 1;
        }
        return 0;
    }

    if (node < 0 || input < 0) {
        fprintf(stderr, "Invalid node input...\n");
        return 1; 
    }

    printf("Map constant with value %f to %s[%s]\n", val, tz->nodes[node]->name, tz->nodes[node]->inputsNames[input]);
    addNode(tz, createConstantNode(val), "\0");
    connectModules(tz, tz->numNodes - 1, 0, node, input); 

    return 0;
}



int parseConnectInstruction (Tzara* tz, char* instr) {
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
        return 1;
    }

    if (destNode == TZARA_OUTPUT_NODE_INDEX) {
        if (srcNode < 0 || srcOutput < 0) {
            fprintf(stderr, "Invalid connection...\n");
            return 1;
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
            return 1;
        }
        return 0;
    }

    if (srcNode < 0 || srcOutput < 0 || destNode < 0 || destInput < 0) {
        fprintf(stderr, "Invalid connection...\n");
        return  1;
    }

    printf("Connect %s[%s] to %s[%s]\n", tz->nodes[srcNode]->name, tz->nodes[srcNode]->outputsNames[srcOutput], tz->nodes[destNode]->name, tz->nodes[destNode]->inputsNames[destInput]);
    connectModules(tz, srcNode, srcOutput, destNode, destInput); 

    return 0;
}



int parseInstruction (Tzara* tz, char*  instr) {
    const int op = parseOperator(instr[0]);
    int err = 0;

    switch (op) {
        case COMMENT_OP:
            parseCommentInstruction(instr);
            break;

        case CREATE_NODE_OP:
            err = parseCreateNodeInstruction(tz, instr);
            break;

        case CREATE_CONSTANT_OP:
            err = parseCreateConstantInstruction(tz, instr);
            break;

        case CONNECT_OP:
            err = parseConnectInstruction(tz, instr);
            break;

        default:
            break;
    }

    return err;
}



int parsePatch (Tzara* tz, FILE* patch) {
    char cache[PARSER_CACHE_SIZE];
    int i;
    int err  = 0;

    for (i = 0; i < PARSER_CACHE_SIZE; ++i) {
        cache[i] = 0;
    }
    while (fgets(cache, PARSER_CACHE_SIZE, patch) != NULL) {
        err = parseInstruction(tz, cache);
        if (err != 0) {
            break;
        }
    }
    return err;
}

