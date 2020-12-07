#ifndef TZARA_PARSER_H
#define TZARA_PARSER_H

#include <stdio.h>

#include "tzara.h"
#include "nodes.h"

#define PARSER_CACHE_SIZE 1024

#define PARSER_MAX_TOKENS 256
#define PARSER_TOKEN_LENGTH 256

#define TZARA_OUTPUT_NODE_INDEX -0xaa
#define TZARA_OUTPUT_LEFT_INDEX -0xbb
#define TZARA_OUTPUT_RIGHT_INDEX -0xcc


#define MODULE_INPUTS_NODE_INDEX -0xdd
#define MODULE_OUTPUTS_NODE_INDEX -0xee

extern const char* midiNotes[128];

enum Operators {
    NO_OP = 0,
    COMMENT_OP,
    CREATE_NODE_OP,
    CREATE_CONSTANT_OP,
    CONNECT_OP,
    MODULE_IO_OP
};

int parseOperator (char op);

void parseCommentInstruction (char* instr);

int parseNodeType (const char* name);

void trimNewLine (char* str);

TzNode* parseAndCreateModule (char** tokens, int numTokens);

void addEngineNode (void* engine, TzNode* n, char* name, int isModule);

int parseCreateNodeInstruction (void* tz, char** tokens, int numTokens, int isModule);

int searchNode (void* tz, const char* name, int isModule);

int searchInput (TzNode* node, const char* name);

int searchOutput (TzNode* node, const char* name);

int searchModuleInput (TzModule* m, const char* name);

int searchModuleOutput (TzModule* m, const char* name);

void parseNodeInputString (void* tz, char* str, int* node, int* input, int isModule);

void parseNodeOutputString (void* tz, char* str, int* node, int* output, int isModule);

float getConstantValue (char* token);

int parseCreateConstantInstruction (void* tz, char** tokens, int numTokens, int isModule);

int parseConnectInstruction (void* tz, char** tokens, int numTokens, int isModule);

int parseModuleIOInstruction (void* tz, char** tokens, int numTokens, int isModule);

int parseInstruction (void* tz, char* instr,  char** tokens, int numTokens, int isModule);

int parsePatch (void* engine, FILE* patch, const char* filename, int isModule);

#endif
