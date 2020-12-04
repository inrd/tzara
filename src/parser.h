#ifndef TZARA_PARSER_H
#define TZARA_PARSER_H

#include <stdio.h>

#include "tzara.h"
#include "nodes.h"

#define PARSER_CACHE_SIZE 1024

enum Operators {
    NO_OP = 0,
    COMMENT_OP,
    CREATE_NODE_OP,
    CREATE_CONSTANT_OP,
    CONNECT_OP
};

int parseOperator (char op);

void parseCommentInstruction (const char* instr);

int parseNodeType (const char* name);

void trimNewLine (char* str);

void addEngineNode (void* engine, TzNode* n, char* name, int isModule);

int parseCreateNodeInstruction (void* tz, char* instr, int isModule);

int searchNode (void* tz, const char* name, int isModule);

int searchInput (TzNode* node, const char* name);

int searchOutput (TzNode* node, const char* name);

void parseNodeInputString (void* tz, char* str, int* node, int* input, int isModule);

void parseNodeOutputString (void* tz, char* str, int* node, int* output, int isModule);

float getConstantValue (char* token);

int parseCreateConstantInstruction (void* tz, char* instr, int isModule);

int parseConnectInstruction (void* tz, char* instr, int isModule);

int parseInstruction (void* tz, char* instr, int isModule);

int parsePatch (void* engine, FILE* patch, const char* filename, int isModule);

#endif
