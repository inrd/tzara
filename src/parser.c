#include "parser.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

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

        case  '@':
            return MODULE_IO_OP;
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
    int i = 0;

    for (i = 1; i < NUM_NODE_TYPES; ++i) {
        if (strncmp(name, nodesDoc[i].name, nameLength) == 0) {
            return i;
        }
    }

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

TzNode* parseAndCreateModule (char* instr) {
    int i = 0;
    int offset = 0;
    char filename[512];

    memset(filename, '\0', 512);

    while (instr[i] != '<' && instr[i] != '\0') {
        ++i;
    }

    if (instr[i] == '\0') {
        fprintf(stderr, "Invalid syntax, cannot parse module file name...\n");
        return NULL;
    }

    /* skip < */
    ++i;
    offset = i;

    while (instr[i] != '>' && instr[i] != '\0') {
        filename[i- offset] = instr[i];
        ++i;
    }

    return createModuleNode(filename);
}

void addEngineNode (void* engine, TzNode* n, char* name, int isModule) {
    if (isModule == 0) {
        addNode((Tzara*)engine, n, name);
    }
    else {
        addModuleNode((TzModule*)engine, n, name);
    }
}

int parseCreateNodeInstruction (void* tz, char* instr, int isModule) {
    char* token;
    int nodeType = INVALID_NODE_TYPE;
    char name [TZNODE_NAME_SIZE];
    char instrCopy[1024];
    int ic = 0;

    memset(instrCopy, '\0', 1024);
    strncpy(instrCopy, instr, 1023);

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
        case MODULE_NODE:
            printf("Creating module : %s\n", name);
            addEngineNode(tz, parseAndCreateModule(instrCopy), name, isModule);
            break;
        
        case VAR_NODE:
            printf("Creating var : %s\n", name);
            addEngineNode(tz, createVarNode(), name, isModule);
            break;
        
        case ADDER_NODE:
            printf("Creating add : %s\n", name);
            addEngineNode(tz, createAdderNode(), name, isModule);
            break;
        
        case SUB_NODE:
            printf("Creating sub : %s\n", name);
            addEngineNode(tz, createSubNode(), name, isModule);
            break;
        
        case MULT_NODE:
            printf("Creating mult : %s\n", name);
            addEngineNode(tz, createMultNode(), name, isModule);
            break;
        
        case DIV_NODE:
            printf("Creating div : %s\n", name);
            addEngineNode(tz, createDivNode(), name, isModule);
            break;
        
        case MODULO_NODE:
            printf("Creating modulo : %s\n", name);
            addEngineNode(tz, createModuloNode(), name, isModule);
            break;
        
        case CLIP_NODE:
            printf("Creating clip : %s\n", name);
            addEngineNode(tz, createClipNode(), name, isModule);
            break;

        case ROUND_NODE:
            printf("Creating round : %s\n", name);
            addEngineNode(tz, createRoundNode(), name, isModule);
            break;
        
        case AND_NODE:
            printf("Creating and : %s\n", name);
            addEngineNode(tz, createAndNode(), name, isModule);
            break;
        
        case OR_NODE:
            printf("Creating or : %s\n", name);
            addEngineNode(tz, createOrNode(), name, isModule);
            break;
        
        case XOR_NODE:
            printf("Creating xor : %s\n", name);
            addEngineNode(tz, createXorNode(), name, isModule);
            break;
        
        case MIX_NODE:
            printf("Creating mix : %s\n", name);
            addEngineNode(tz, createMixNode(), name, isModule);
            break;

        case MAP_NODE:
            printf("Creating mapper : %s\n", name);
            addEngineNode(tz, createMapNode(), name, isModule);
            break;
        
        case MIDITOFREQ_NODE:
            printf("Creating miditofreq : %s\n", name);
            addEngineNode(tz, createMiditofreqNode(), name, isModule);
            break;
        
        case MEM_NODE:
            printf("Creating mem : %s\n", name);
            addEngineNode(tz, createMemNode(), name, isModule);
            break;
        
        case COUNT_NODE:
            printf("Creating count : %s\n", name);
            addEngineNode(tz, createCountNode(), name, isModule);
            break;
        
        case PHASOR_NODE:
            printf("Creating phasor : %s\n", name);
            addEngineNode(tz, createPhasorNode(), name, isModule);
            break;

        case PULSE_NODE:
            printf("Creating pulse : %s\n", name);
            addEngineNode(tz, createPulseNode(), name, isModule);
            break;

        case SINOSC_NODE:
            printf("Creating sinosc : %s\n", name);
            addEngineNode(tz, createSinoscNode(), name, isModule);
            break;

        case SEQ8_NODE:
            printf("Creating seq8 : %s\n", name);
            addEngineNode(tz, createSeq8Node(), name, isModule);
            break;

        case RANDOM_NODE:
            printf("Creating random : %s\n", name);
            addEngineNode(tz, createRandomNode(), name, isModule);
            break;

        case SEGMENT_NODE:
            printf("Creating segment : %s\n", name);
            addEngineNode(tz, createSegmentNode(), name, isModule);
            break;

        case SELECT_NODE:
            printf("Creating select : %s\n", name);
            addEngineNode(tz, createSelectNode(), name, isModule);
            break;

        case DELAY_NODE:
            printf("Creating delay : %s\n", name);
            addEngineNode(tz, createDelayNode(), name, isModule);
            break;

        case FDELAY_NODE:
            printf("Creating fdelay : %s\n", name);
            addEngineNode(tz, createFdelayNode(), name, isModule);
            break;

        default:
            fprintf(stderr, "Could not create node : invalid node type...\n");
            return 1;
            break;
    }

    return 0;
}


int searchNode (void* tz, const char* name, int isModule) {
    int i = 0;
    int nameLength = strlen(name);

    if (isModule == 0) {
        for (i = 0; i < ((Tzara*)tz)->numNodes; ++i) {
            if (nameLength == (int)strlen(((Tzara*)tz)->nodes[i]->name)) {
                if (strncmp(((Tzara*)tz)->nodes[i]->name, name, nameLength) == 0) {
                    return i;
                }
            }
        }
    }
    else {
        for (i = 0; i < ((TzModule*)tz)->numNodes; ++i) {
            if (nameLength == (int)strlen(((TzModule*)tz)->nodes[i]->name)) {
                if (strncmp(((TzModule*)tz)->nodes[i]->name, name, nameLength) == 0) {
                    return i;
                }
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

int searchModuleInput (TzModule* m, const char* name) {
    int i = 0;
    int nameLength = strlen(name);
    for (i = 0; i < m->numInputs; ++i) {
        if (nameLength == (int)strlen(m->inputsNames[i])) {
            if (strncmp(m->inputsNames[i], name, nameLength) == 0) {
                return i;
            }
        }
    }
    return -1;
}

int searchModuleOutput (TzModule* m, const char* name) {
    int i = 0;
    int nameLength = strlen(name);
    for (i = 0; i < m->numOutputs; ++i) {
        if (nameLength == (int)strlen(m->outputsNames[i])) {
            if (strncmp(m->outputsNames[i], name, nameLength) == 0) {
                return i;
            }
        }
    }
    return -1;
}

void parseNodeInputString (void* tz, char* str, int* node, int* input, int isModule) {
    char token[TZNODE_NAME_SIZE];
    int i = 0;
    int offset = 0;

    memset(token, '\0', TZNODE_NAME_SIZE);
    
    if (str[0] == '$') {
        /* var node */
        /* skip '$' */
        while(str[i] != '\0') {
            if (i  > 0) {
                token[i - 1]  = str[i];
            }
            ++i;
        }
        *node = searchNode(tz, token, isModule);
        *input = (*node >= 0) ? 0 : -1;
        return;
    }



    while(str[i] != '@' && str[i] != '\0') {
        token[i]  = str[i];
        ++i;
    }

    if (strncmp(token, "_out_", 5) == 0) {
        if (isModule == 0) {
            *node = TZARA_OUTPUT_NODE_INDEX;
        }
        else {
            *node = MODULE_OUTPUTS_NODE_INDEX;
        }
    }
    else {
        *node = searchNode(tz, token, isModule);
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
    else if (*node == MODULE_OUTPUTS_NODE_INDEX) {
        *input = searchModuleOutput((TzModule*)tz, token);
    }
    else {
        if (isModule == 0) {
            *input = (*node >= 0) ? searchInput(((Tzara*)tz)->nodes[*node], token) : -1;
        }
        else {
            *input = (*node >= 0) ? searchInput(((TzModule*)tz)->nodes[*node], token) : -1;
        }
    }
}

void parseNodeOutputString (void* tz, char* str, int* node, int* output, int isModule) {
    char token[TZNODE_NAME_SIZE];
    int i = 0;
    int offset = 0;

    memset(token, '\0', TZNODE_NAME_SIZE);

    if (str[0] == '$') {
        /* var node */
        /* skip '$' */
        while(str[i] != '\0') {
            if (i  > 0) {
                token[i - 1]  = str[i];
            }
            ++i;
        }
        *node = searchNode(tz, token, isModule);
        *output = (*node >= 0) ? 0 : -1;
        return;
    }


    while(str[i] != '@' && str[i] != '\0') {
        token[i]  = str[i];
        ++i;
    }

    if (strncmp(token, "_in_", 4) == 0) {
        if (isModule != 0) {
            *node = MODULE_INPUTS_NODE_INDEX;
        }
    }
    else {
        *node = searchNode(tz, token, isModule);
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

    if (*node == MODULE_INPUTS_NODE_INDEX) {
        *output = searchModuleInput((TzModule*)tz, token);
    }
    else {
        if (isModule == 0) {
            *output = (*node >= 0) ? searchOutput(((Tzara*)tz)->nodes[*node], token) : -1;
        }
        else {
            *output = (*node >= 0) ? searchOutput(((TzModule*)tz)->nodes[*node], token) : -1;
        }
    }
}

float getConstantValue (char* token) {
    if (strncmp(token, "pi", strlen(token)) == 0) {
        return M_PI;
    }
    else if (strncmp(token, "twopi", strlen(token)) == 0) {
        return 2.f * M_PI;
    }
    else {
        return atof(token);
    }
}



int parseCreateConstantInstruction (void* tz, char* instr, int isModule) {
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
                val = getConstantValue(token);
                break;
            case 2:
                trimNewLine(token);
                parseNodeInputString(tz, token, &node, &input, isModule);
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
        if (isModule == 0) {
            if (input == TZARA_OUTPUT_LEFT_INDEX) {
                printf("Map constant with value %f to out[L]\n", val);
                addNode((Tzara*)tz, createConstantNode(val), "\0");
                connectNodeToOutput((Tzara*)tz, ((Tzara*)tz)->numNodes - 1, 0, 0);
            }
            else if (input == TZARA_OUTPUT_RIGHT_INDEX) {
                printf("Map constant with value %f to out[R]\n", val);
                addNode((Tzara*)tz, createConstantNode(val), "\0");
                connectNodeToOutput((Tzara*)tz, ((Tzara*)tz)->numNodes - 1, 0, 1);
            }
            else {
                fprintf(stderr, "Invalid Input...\n");
                return 1;
            }
            return 0;
        }
        else {
            fprintf(stderr, "Cannot route module node to main out...\n");
            return 1;
        }
    }

    if (node == MODULE_OUTPUTS_NODE_INDEX) {
        printf("Map constant with value %f to module out\n", val);
        addModuleNode((TzModule*)tz, createConstantNode(val), "\0");
        connectModuleOutlet((TzModule*)tz, ((TzModule*)tz)->numNodes - 1, 0, input);
        return 0;
    }

    if (node < 0 || input < 0) {
        fprintf(stderr, "Invalid node input...\n");
        return 1; 
    }

    if (isModule == 0) {
        printf("Map constant with value %f to %s[%s]\n", val, ((Tzara*)tz)->nodes[node]->name, ((Tzara*)tz)->nodes[node]->inputsNames[input]);
        addNode((Tzara*)tz, createConstantNode(val), "\0");
        connectNodes((Tzara*)tz, ((Tzara*)tz)->numNodes - 1, 0, node, input); 
    }
    else {
        printf("Map constant with value %f to %s[%s]\n", val, ((TzModule*)tz)->nodes[node]->name, ((TzModule*)tz)->nodes[node]->inputsNames[input]);
        addModuleNode((TzModule*)tz, createConstantNode(val), "\0");
        connectModuleNodes((TzModule*)tz, ((TzModule*)tz)->numNodes - 1, 0, node, input); 
    }

    return 0;
}



int parseConnectInstruction (void* tz, char* instr, int isModule) {
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
                parseNodeOutputString(tz, token, &srcNode, &srcOutput, isModule);
                break;
            case 2:
                trimNewLine(token);
                parseNodeInputString(tz, token, &destNode, &destInput, isModule);
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
        if (isModule == 0) {
            if (srcNode < 0 || srcOutput < 0) {
                fprintf(stderr, "Invalid connection...\n");
                return 1;
            }
            if (destInput == TZARA_OUTPUT_LEFT_INDEX) {
                printf("Connect %s[%s] to out[L]\n", ((Tzara*)tz)->nodes[srcNode]->name, ((Tzara*)tz)->nodes[srcNode]->outputsNames[srcOutput]);
                connectNodeToOutput((Tzara*)tz, srcNode, srcOutput, 0);
            }
            else if (destInput == TZARA_OUTPUT_RIGHT_INDEX) {
                printf("Connect %s[%s] to out[R]\n", ((Tzara*)tz)->nodes[srcNode]->name, ((Tzara*)tz)->nodes[srcNode]->outputsNames[srcOutput]);
                connectNodeToOutput((Tzara*)tz, srcNode, srcOutput, 1);
            }
            else {
                fprintf(stderr, "Invalid Input...\n");
                return 1;
            }
            return 0;
        }
        else {
            fprintf(stderr, "Cannot route module node to main out...\n");
            return 1;
        }
    }

    if (destNode == MODULE_OUTPUTS_NODE_INDEX && srcNode == MODULE_INPUTS_NODE_INDEX) {
        printf("Cannot connect module input straight to module output...\n");
        return 1;
    }

    if (destNode == MODULE_OUTPUTS_NODE_INDEX) {
        if (srcNode < 0 || srcOutput < 0) {
            fprintf(stderr, "Invalid connection...\n");
            return 1;
        }
        printf("Connect %s[%s] to module out\n", ((TzModule*)tz)->nodes[srcNode]->name, ((TzModule*)tz)->nodes[srcNode]->outputsNames[srcOutput]);
        connectModuleOutlet((TzModule*)tz, srcNode, srcOutput, destInput);
        return 0;
    }

    if (srcNode == MODULE_INPUTS_NODE_INDEX) {
        if (destNode < 0 || destInput < 0) {
            fprintf(stderr, "Invalid connection...\n");
            return 1;
        }
        printf("Connect module in to %s[%s]\n", ((TzModule*)tz)->nodes[destNode]->name, ((TzModule*)tz)->nodes[destNode]->inputsNames[destInput]);
        connectModuleInlet((TzModule*)tz, destNode, destInput, srcOutput);
        return 0;
    }

    if (srcNode < 0 || srcOutput < 0 || destNode < 0 || destInput < 0) {
        fprintf(stderr, "Invalid connection...\n");
        return  1;
    }

    if (isModule == 0) {
        printf("Connect %s[%s] to %s[%s]\n", ((Tzara*)tz)->nodes[srcNode]->name, ((Tzara*)tz)->nodes[srcNode]->outputsNames[srcOutput], ((Tzara*)tz)->nodes[destNode]->name, ((Tzara*)tz)->nodes[destNode]->inputsNames[destInput]);
        connectNodes((Tzara*)tz, srcNode, srcOutput, destNode, destInput); 
    }
    else {
        printf("Connect %s[%s] to %s[%s]\n", ((TzModule*)tz)->nodes[srcNode]->name, ((TzModule*)tz)->nodes[srcNode]->outputsNames[srcOutput], ((TzModule*)tz)->nodes[destNode]->name, ((TzModule*)tz)->nodes[destNode]->inputsNames[destInput]);
        connectModuleNodes((TzModule*)tz, srcNode, srcOutput, destNode, destInput); 
    }

    return 0;
}

int parseModuleIOInstruction (void* tz, char* instr, int isModule) {
    char* token;
    int ic = 0;
    int isIn = 0;
    int isOut = 0;
    char name[TZNODE_NAME_SIZE];

    memset(name, '\0', TZNODE_NAME_SIZE - 1);

    if (isModule == 0) {
        printf("IO connections are only valid in modules!\n");
        return 1;
    }

    token = strtok(instr, " ");

    while (token != NULL) {
        /* drop first token (operator) */
        switch (ic) {
            case 1:
                isIn = strncmp(token, "in", strlen(token)) == 0 ? 1 : 0;
                isOut = strncmp(token, "out", strlen(token)) == 0 ? 1 : 0;
                break;
            case 2:
                trimNewLine(token);
                strcpy(name, token);
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

    if (isIn == 0 && isOut == 0) {
        printf("Invalid module IO instruction...\n");
        return 1;
    }

    if (isIn != 0) {
        printf("Creating module in : %s\n", name);
        createModuleInlet ((TzModule*)tz, name);
        return 0;
    }

    if (isOut != 0) {
        printf("Creating module out : %s\n", name);
        createModuleOutlet ((TzModule*)tz, name);
        return 0;
    }

    return 1;
}



int parseInstruction (void* tz, char*  instr, int isModule) {
    const int op = parseOperator(instr[0]);
    int err = 0;

    switch (op) {
        case COMMENT_OP:
            parseCommentInstruction(instr);
            break;

        case CREATE_NODE_OP:
            err = parseCreateNodeInstruction(tz, instr, isModule);
            break;

        case CREATE_CONSTANT_OP:
            err = parseCreateConstantInstruction(tz, instr, isModule);
            break;

        case CONNECT_OP:
            err = parseConnectInstruction(tz, instr, isModule);
            break;

        case MODULE_IO_OP:
            err = parseModuleIOInstruction(tz, instr, isModule);
            break;

        default:
            break;
    }

    return err;
}



int parsePatch (void* engine, FILE* patch, const char* filename, int isModule) {
    char cache[PARSER_CACHE_SIZE];
    int i;
    int lcount = 1;
    int err  = 0;

    for (i = 0; i < PARSER_CACHE_SIZE; ++i) {
        cache[i] = 0;
    }
    while (fgets(cache, PARSER_CACHE_SIZE, patch) != NULL) {
        err = parseInstruction(engine, cache, isModule);
        if (err != 0) {
            printf("Issue encountered while parsing %s, line %d.\n", filename, lcount);
            break;
        }
        ++lcount;
    }
    return err;
}

