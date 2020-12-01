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

int parseCreateNodeInstruction (Tzara* tz, char* instr);

int searchNode (Tzara* tz, const char* name);

int searchInput (TzNode* node, const char* name);

int searchOutput (TzNode* node, const char* name);

void parseNodeInputString (Tzara* tz, char* str, int* node, int* input);

void parseNodeOutputString (Tzara* tz, char* str, int* node, int* output);

int parseCreateConstantInstruction (Tzara* tz, char* instr);

int parseConnectInstruction (Tzara* tz, char* instr);

int parseInstruction (Tzara* tz, char*  instr);

int parsePatch (Tzara* tz, FILE* patch);

#endif
