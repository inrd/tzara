#include "parser.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *midiNotes[128] = {
    "c-1",  "c#-1", "d-1", "d#-1", "e-1", "f-1", "f#-1", "g-1", "g#-1", "a-1",
    "a#-1", "b-1",  "c0",  "c#0",  "d0",  "d#0", "e0",   "f0",  "f#0",  "g0",
    "g#0",  "a0",   "a#0", "b0",   "c1",  "c#1", "d1",   "d#1", "e1",   "f1",
    "f#1",  "g1",   "g#1", "a1",   "a#1", "b1",  "c2",   "c#2", "d2",   "d#2",
    "e2",   "f2",   "f#2", "g2",   "g#2", "a2",  "a#2",  "b2",  "c3",   "c#3",
    "d3",   "d#3",  "e3",  "f3",   "f#3", "g3",  "g#3",  "a3",  "a#3",  "b3",
    "c4",   "c#4",  "d4",  "d#4",  "e4",  "f4",  "f#4",  "g4",  "g#4",  "a4",
    "a#4",  "b4",   "c5",  "c#5",  "d5",  "d#5", "e5",   "f5",  "f#5",  "g5",
    "g#5",  "a5",   "a#5", "b5",   "c6",  "c#6", "d6",   "d#6", "e6",   "f6",
    "f#6",  "g6",   "g#6", "a6",   "a#6", "b6",  "c7",   "c#7", "d7",   "d#7",
    "e7",   "f7",   "f#7", "g7",   "g#7", "a7",  "a#7",  "b7",  "c8",   "c#8",
    "d8",   "d#8",  "e8",  "f8",   "f#8", "g8",  "g#8",  "a8",  "a#8",  "b8",
    "c9",   "c#9",  "d9",  "d#9",  "e9",  "f9",  "f#9",  "g9"};

const char *noteNames[12] = {"c",  "c#", "d",  "d#", "e",  "f",
                             "f#", "g",  "g#", "a",  "a#", "b"};

const char *scaleNames[NUM_SCALES] = {"chromatic",
                                      "major",
                                      "minor",
                                      "harmonic_major",
                                      "harmonic_minor",
                                      "locrian",
                                      "pyramid_hexatonic",
                                      "kung",
                                      "hira_joshi",
                                      "ritsu",
                                      "mela_citrambari",
                                      "raga_bilwadala",
                                      "maqam_hijaz",
                                      "gnossiennes"};

int parseOperator(char op) {
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

  case '@':
    return MODULE_IO_OP;
    break;

  case '!':
    return METADATA_OP;
    break;

  default:
    return NO_OP;
    break;
  }
}

void parseCommentInstruction(char *instr) { printf("%s", instr); }

void parseMetadataInstruction(void *engine, char **tokens, int numTokens,
                              int isModule) {
  int duration = 0;

  if (numTokens >= 3) {
    if (strncmp(tokens[1], "duration", strlen(tokens[1])) == 0) {
      if (isModule != 0) {
        printf("Duration metadata is not valid in a module...\n");
      } else {
        duration = atoi(tokens[2]);
        if (duration <= 0) {
          printf("Invalid duration provided : %s.\nSetting to default value "
                 "(60 seconds).\n",
                 tokens[2]);
        } else {
          ((Tzara *)engine)->renderDuration = duration;
          printf("Render length set to %d seconds.\n", duration);
        }
      }
    } else {
      printf("Unknown metadata : %s\n", tokens[1]);
    }
  }
}

int parseNodeType(const char *name) {
  const int nameLength = strlen(name);
  int i = 0;

  for (i = 1; i < NUM_NODE_TYPES; ++i) {
    if (strncmp(name, nodesDoc[i].name, nameLength) == 0) {
      return i;
    }
  }

  return INVALID_NODE_TYPE;
}

void trimNewLine(char *str) {
  char *nl = NULL;
  nl = strchr(str, '\n');
  if (nl != NULL)
    *nl = '\0';
}

void getParentDir(const char *path, char *out, size_t outSize) {
  const char *slash = NULL;
  size_t len = 0;

  if (out == NULL || outSize == 0)
    return;

  out[0] = '\0';
  if (path == NULL)
    return;

  slash = strrchr(path, '/');
  if (slash == NULL)
    return;

  len = (size_t)(slash - path);
  if (len >= outSize)
    len = outSize - 1;

  memcpy(out, path, len);
  out[len] = '\0';
}

void resolvePatchPath(const char *parentDir, const char *path, char *out,
                      size_t outSize) {
  if (out == NULL || outSize == 0)
    return;

  if (path == NULL || path[0] == '\0' || path[0] == '/' ||
      parentDir == NULL || parentDir[0] == '\0') {
    if (path == NULL) {
      out[0] = '\0';
    } else {
      strncpy(out, path, outSize - 1);
      out[outSize - 1] = '\0';
    }
    return;
  }

  snprintf(out, outSize, "%s/%s", parentDir, path);
}

TzNode *parseAndCreateModule(char **tokens, int numTokens,
                             const char *parentDir) {
  int i = 0;
  int j = 1;
  char filename[PARSER_PATH_SIZE];
  char resolved[PARSER_PATH_SIZE];

  memset(filename, '\0', PARSER_PATH_SIZE);

  for (i = 0; i < numTokens; ++i) {
    if (tokens[i][0] == '<') {

      while (tokens[i][j] != '>' && tokens[i][j] != '\0' &&
             (size_t)(j - 1) < sizeof(filename) - 1) {
        filename[j - 1] = tokens[i][j];
        ++j;
      }

      resolvePatchPath(parentDir, filename, resolved, sizeof(resolved));
      return createModuleNode(resolved);
    }
  }

  printf("Invalid syntax, cannot parse module file name...\n");
  return NULL;
}

TzNode *parseAndCreateMatrix(char **tokens, int numTokens,
                             const char *parentDir) {
  int i = 1;
  char filename[PARSER_PATH_SIZE];
  char resolved[PARSER_PATH_SIZE];
  int numRows, numCols = 0;

  memset(filename, '\0', PARSER_PATH_SIZE);

  if (numTokens < 5) {
    printf("Missing arguments!\nSyntax : + matrix node_name num_rows "
           "num_columns <filename (optional)>\n");
    return NULL;
  }

  numRows = atoi(tokens[3]);
  numCols = atoi(tokens[4]);

  if (numTokens == 5) {
    return createMatrixNode(numRows, numCols, NULL);
  } else {
    if (tokens[5][0] == '<') {
      while (tokens[5][i] != '>' && tokens[5][i] != '\0' &&
             (size_t)(i - 1) < sizeof(filename) - 1) {
        filename[i - 1] = tokens[5][i];
        ++i;
      }
      resolvePatchPath(parentDir, filename, resolved, sizeof(resolved));
      return createMatrixNode(numRows, numCols, resolved);
    } else {
      printf("Invalid syntax.\n");
      return NULL;
    }
  }
}

TzNode *parseAndCreateMget(void *tz, char **tokens, int numTokens,
                           int isModule) {
  TzMatrix *ref = NULL;
  int refIdx = -1;

  if (numTokens != 4) {
    printf("Invalid arguments!\nSyntax : + mget node_name matrix_name\n");
    return NULL;
  }

  trimNewLine(tokens[3]);
  refIdx = searchNode(tz, tokens[3], isModule);

  if (refIdx < 0) {
    printf("Matrix not found : %s\n", tokens[3]);
    return NULL;
  }

  if (isModule != 0) {
    ref = ((TzModule *)tz)->nodes[refIdx]->matrix;
  } else {
    ref = ((Tzara *)tz)->nodes[refIdx]->matrix;
  }

  if (ref == NULL) {
    printf("%s is not a matrix...", tokens[3]);
    return NULL;
  }

  return createMgetNode(ref);
}

TzNode *parseAndCreateMset(void *tz, char **tokens, int numTokens,
                           int isModule) {
  TzMatrix *ref = NULL;
  int refIdx = -1;

  if (numTokens != 4) {
    printf("Invalid arguments!\nSyntax : + mset node_name matrix_name\n");
    return NULL;
  }

  trimNewLine(tokens[3]);
  refIdx = searchNode(tz, tokens[3], isModule);

  if (refIdx < 0) {
    printf("Matrix not found : %s\n", tokens[3]);
    return NULL;
  }

  if (isModule != 0) {
    ref = ((TzModule *)tz)->nodes[refIdx]->matrix;
  } else {
    ref = ((Tzara *)tz)->nodes[refIdx]->matrix;
  }

  if (ref == NULL) {
    printf("%s is not a matrix...", tokens[3]);
    return NULL;
  }

  return createMsetNode(ref);
}

int addEngineNode(void *engine, TzNode *n, char *name, int isModule) {
  if (isModule == 0) {
    return addNode((Tzara *)engine, n, name);
  }
  return addModuleNode((TzModule *)engine, n, name);
}

int parseCreateNodeInstruction(void *tz, char **tokens, int numTokens,
                               int isModule, const char *parentDir) {
  int nodeType = INVALID_NODE_TYPE;
  int err = 0;
  char name[TZNODE_NAME_SIZE];

  if (numTokens < 3) {
    fprintf(stderr, "Not enough arguments...\n");
    return 1;
  }

  nodeType = parseNodeType(tokens[1]);

  strncpy(name, tokens[2], sizeof(name) - 1);
  trimNewLine(name);

  switch (nodeType) {
  case MODULE_NODE:
    printf("Creating module : %s\n", name);
    err = addEngineNode(
        tz, parseAndCreateModule(tokens, numTokens, parentDir), name, isModule);
    break;

  case MATRIX_NODE:
    printf("Creating matrix : %s\n", name);
    err = addEngineNode(
        tz, parseAndCreateMatrix(tokens, numTokens, parentDir), name, isModule);
    break;

  case MGET_NODE:
    printf("Creating mget : %s\n", name);
    err = addEngineNode(tz, parseAndCreateMget(tz, tokens, numTokens, isModule),
                        name, isModule);
    break;

  case MSET_NODE:
    printf("Creating mset : %s\n", name);
    err = addEngineNode(tz, parseAndCreateMset(tz, tokens, numTokens, isModule),
                        name, isModule);
    break;

  case DEFAULTVAL_NODE:
    printf("Creating defaultval : %s\n", name);
    err = addEngineNode(tz, createDefaultvalNode(), name, isModule);
    break;

  case VAR_NODE:
    printf("Creating var : %s\n", name);
    err = addEngineNode(tz, createVarNode(), name, isModule);
    break;

  case ADDER_NODE:
    printf("Creating add : %s\n", name);
    err = addEngineNode(tz, createAdderNode(), name, isModule);
    break;

  case SUB_NODE:
    printf("Creating sub : %s\n", name);
    err = addEngineNode(tz, createSubNode(), name, isModule);
    break;

  case MULT_NODE:
    printf("Creating mult : %s\n", name);
    err = addEngineNode(tz, createMultNode(), name, isModule);
    break;

  case DIV_NODE:
    printf("Creating div : %s\n", name);
    err = addEngineNode(tz, createDivNode(), name, isModule);
    break;

  case MODULO_NODE:
    printf("Creating modulo : %s\n", name);
    err = addEngineNode(tz, createModuloNode(), name, isModule);
    break;

  case POW_NODE:
    printf("Creating pow : %s\n", name);
    err = addEngineNode(tz, createPowNode(), name, isModule);
    break;

  case SQRT_NODE:
    printf("Creating sqrt : %s\n", name);
    err = addEngineNode(tz, createSqrtNode(), name, isModule);
    break;

  case ABS_NODE:
    printf("Creating abs : %s\n", name);
    err = addEngineNode(tz, createAbsNode(), name, isModule);
    break;

  case SIN_NODE:
    printf("Creating sin : %s\n", name);
    err = addEngineNode(tz, createSinNode(), name, isModule);
    break;

  case COS_NODE:
    printf("Creating cos : %s\n", name);
    err = addEngineNode(tz, createCosNode(), name, isModule);
    break;

  case TAN_NODE:
    printf("Creating tan : %s\n", name);
    err = addEngineNode(tz, createTanNode(), name, isModule);
    break;

  case TANH_NODE:
    printf("Creating tanh : %s\n", name);
    err = addEngineNode(tz, createTanhNode(), name, isModule);
    break;

  case CLIP_NODE:
    printf("Creating clip : %s\n", name);
    err = addEngineNode(tz, createClipNode(), name, isModule);
    break;

  case WRAP_NODE:
    printf("Creating wrap : %s\n", name);
    err = addEngineNode(tz, createWrapNode(), name, isModule);
    break;

  case EQUAL_NODE:
    printf("Creating equal : %s\n", name);
    err = addEngineNode(tz, createEqualNode(), name, isModule);
    break;

  case NEQUAL_NODE:
    printf("Creating nequal : %s\n", name);
    err = addEngineNode(tz, createNequalNode(), name, isModule);
    break;

  case LOWER_NODE:
    printf("Creating lower : %s\n", name);
    err = addEngineNode(tz, createLowerNode(), name, isModule);
    break;

  case GREATER_NODE:
    printf("Creating greater : %s\n", name);
    err = addEngineNode(tz, createGreaterNode(), name, isModule);
    break;

  case MIN_NODE:
    printf("Creating min : %s\n", name);
    err = addEngineNode(tz, createMinNode(), name, isModule);
    break;

  case MAX_NODE:
    printf("Creating max : %s\n", name);
    err = addEngineNode(tz, createMaxNode(), name, isModule);
    break;

  case ROUND_NODE:
    printf("Creating round : %s\n", name);
    err = addEngineNode(tz, createRoundNode(), name, isModule);
    break;

  case CEIL_NODE:
    printf("Creating ceil : %s\n", name);
    err = addEngineNode(tz, createCeilNode(), name, isModule);
    break;

  case FLOOR_NODE:
    printf("Creating floor : %s\n", name);
    err = addEngineNode(tz, createFloorNode(), name, isModule);
    break;

  case FRAC_NODE:
    printf("Creating frac : %s\n", name);
    err = addEngineNode(tz, createFracNode(), name, isModule);
    break;

  case AND_NODE:
    printf("Creating and : %s\n", name);
    err = addEngineNode(tz, createAndNode(), name, isModule);
    break;

  case OR_NODE:
    printf("Creating or : %s\n", name);
    err = addEngineNode(tz, createOrNode(), name, isModule);
    break;

  case XOR_NODE:
    printf("Creating xor : %s\n", name);
    err = addEngineNode(tz, createXorNode(), name, isModule);
    break;

  case MIX_NODE:
    printf("Creating mix : %s\n", name);
    err = addEngineNode(tz, createMixNode(), name, isModule);
    break;

  case MERGE_NODE:
    printf("Creating merge : %s\n", name);
    err = addEngineNode(tz, createMergeNode(), name, isModule);
    break;

  case PMERGE_NODE:
    printf("Creating pmerge : %s\n", name);
    err = addEngineNode(tz, createPmergeNode(), name, isModule);
    break;

  case MAP_NODE:
    printf("Creating map : %s\n", name);
    err = addEngineNode(tz, createMapNode(), name, isModule);
    break;

  case FROM01_NODE:
    printf("Creating from0_1 : %s\n", name);
    err = addEngineNode(tz, createFrom0_1Node(), name, isModule);
    break;

  case TO01_NODE:
    printf("Creating to0_1 : %s\n", name);
    err = addEngineNode(tz, createTo0_1Node(), name, isModule);
    break;

  case SMOOTH_NODE:
    printf("Creating smooth : %s\n", name);
    err = addEngineNode(tz, createSmoothNode(), name, isModule);
    break;

  case MIDITOFREQ_NODE:
    printf("Creating miditofreq : %s\n", name);
    err = addEngineNode(tz, createMiditofreqNode(), name, isModule);
    break;

  case DBTOAMP_NODE:
    printf("Creating dbtoamp : %s\n", name);
    err = addEngineNode(tz, createDbtoampNode(), name, isModule);
    break;

  case MSTOHZ_NODE:
    printf("Creating mstohz : %s\n", name);
    err = addEngineNode(tz, createMstohzNode(), name, isModule);
    break;

  case HZTOMS_NODE:
    printf("Creating hztoms : %s\n", name);
    err = addEngineNode(tz, createHztomsNode(), name, isModule);
    break;

  case SAMPLERATE_NODE:
    printf("Creating samplerate : %s\n", name);
    err = addEngineNode(tz, createSamplerateNode(), name, isModule);
    break;

  case DURATION_NODE:
    printf("Creating duration : %s\n", name);
    err = addEngineNode(tz, createDurationNode(), name, isModule);
    break;

  case FIXDENORM_NODE:
    printf("Creating fixdenorm : %s\n", name);
    err = addEngineNode(tz, createFixdenormNode(), name, isModule);
    break;

  case FIXNAN_NODE:
    printf("Creating fixnan : %s\n", name);
    err = addEngineNode(tz, createFixnanNode(), name, isModule);
    break;

  case COUNT_NODE:
    printf("Creating count : %s\n", name);
    err = addEngineNode(tz, createCountNode(), name, isModule);
    break;

  case PHASOR_NODE:
    printf("Creating phasor : %s\n", name);
    err = addEngineNode(tz, createPhasorNode(), name, isModule);
    break;

  case PULSE_NODE:
    printf("Creating pulse : %s\n", name);
    err = addEngineNode(tz, createPulseNode(), name, isModule);
    break;

  case SINOSC_NODE:
    printf("Creating sinosc : %s\n", name);
    err = addEngineNode(tz, createSinoscNode(), name, isModule);
    break;

  case SAWOSC_NODE:
    printf("Creating sawosc : %s\n", name);
    err = addEngineNode(tz, createSawoscNode(), name, isModule);
    break;

  case SQROSC_NODE:
    printf("Creating sqrosc : %s\n", name);
    err = addEngineNode(tz, createSqroscNode(), name, isModule);
    break;

  case TRIOSC_NODE:
    printf("Creating triosc : %s\n", name);
    err = addEngineNode(tz, createTrioscNode(), name, isModule);
    break;

  case NOISE_NODE:
    printf("Creating noise : %s\n", name);
    err = addEngineNode(tz, createNoiseNode(), name, isModule);
    break;

  case SEQ8_NODE:
    printf("Creating seq8 : %s\n", name);
    err = addEngineNode(tz, createSeq8Node(), name, isModule);
    break;

  case RANDOM_NODE:
    printf("Creating random : %s\n", name);
    err = addEngineNode(tz, createRandomNode(), name, isModule);
    break;

  case IRANDOM_NODE:
    printf("Creating irandom : %s\n", name);
    err = addEngineNode(tz, createIrandomNode(), name, isModule);
    break;

  case NOTESCALE_NODE:
    printf("Creating notescale : %s\n", name);
    err = addEngineNode(tz, createNotescaleNode(), name, isModule);
    break;

  case SEGMENT_NODE:
    printf("Creating segment : %s\n", name);
    err = addEngineNode(tz, createSegmentNode(), name, isModule);
    break;

  case ADENV_NODE:
    printf("Creating adenv : %s\n", name);
    err = addEngineNode(tz, createADenvNode(), name, isModule);
    break;

  case ASRENV_NODE:
    printf("Creating asrenv : %s\n", name);
    err = addEngineNode(tz, createASRenvNode(), name, isModule);
    break;

  case SELECT_NODE:
    printf("Creating select : %s\n", name);
    err = addEngineNode(tz, createSelectNode(), name, isModule);
    break;

  case ROUTE_NODE:
    printf("Creating route : %s\n", name);
    err = addEngineNode(tz, createRouteNode(), name, isModule);
    break;

  case SAH_NODE:
    printf("Creating sah : %s\n", name);
    err = addEngineNode(tz, createSahNode(), name, isModule);
    break;

  case GATE_NODE:
    printf("Creating gate : %s\n", name);
    err = addEngineNode(tz, createGateNode(), name, isModule);
    break;

  case TIMEPOINT_NODE:
    printf("Creating timepoint : %s\n", name);
    err = addEngineNode(tz, createTimepointNode(), name, isModule);
    break;

  case LOWPASS_NODE:
    printf("Creating lowpass : %s\n", name);
    err = addEngineNode(tz, createLowpassNode(), name, isModule);
    break;

  case HIGHPASS_NODE:
    printf("Creating highpass : %s\n", name);
    err = addEngineNode(tz, createHighpassNode(), name, isModule);
    break;

  case LOWPASS2_NODE:
    printf("Creating lowpass2 : %s\n", name);
    err = addEngineNode(tz, createLowpass2Node(), name, isModule);
    break;

  case HIGHPASS2_NODE:
    printf("Creating highpass2 : %s\n", name);
    err = addEngineNode(tz, createHighpass2Node(), name, isModule);
    break;

  case BANDPASS_NODE:
    printf("Creating bandpass : %s\n", name);
    err = addEngineNode(tz, createBandpassNode(), name, isModule);
    break;

  case NOTCH_NODE:
    printf("Creating notch : %s\n", name);
    err = addEngineNode(tz, createNotchNode(), name, isModule);
    break;

  case PEAK_NODE:
    printf("Creating peak : %s\n", name);
    err = addEngineNode(tz, createPeakNode(), name, isModule);
    break;

  case LOWSHELF_NODE:
    printf("Creating lowshelf : %s\n", name);
    err = addEngineNode(tz, createLowshelfNode(), name, isModule);
    break;

  case HIGHSHELF_NODE:
    printf("Creating highshelf : %s\n", name);
    err = addEngineNode(tz, createHighshelfNode(), name, isModule);
    break;

  case SVF_NODE:
    printf("Creating svf : %s\n", name);
    err = addEngineNode(tz, createSvfNode(), name, isModule);
    break;

  case DELAY_NODE:
    printf("Creating delay : %s\n", name);
    err = addEngineNode(tz, createDelayNode(), name, isModule);
    break;

  case FDELAY_NODE:
    printf("Creating fdelay : %s\n", name);
    err = addEngineNode(tz, createFdelayNode(), name, isModule);
    break;

  case ALLPASS_NODE:
    printf("Creating allpass : %s\n", name);
    err = addEngineNode(tz, createAllpassNode(), name, isModule);
    break;

  default:
    fprintf(stderr, "Could not create node : invalid node type...\n");
    return 1;
    break;
  }

  if (err != 0) {
    fprintf(stderr, "Could not create node : %s\n", name);
  }

  return err;
}

int searchNode(void *tz, const char *name, int isModule) {
  int i = 0;
  int nameLength = strlen(name);

  if (isModule == 0) {
    for (i = 0; i < ((Tzara *)tz)->numNodes; ++i) {
      if (nameLength == (int)strlen(((Tzara *)tz)->nodes[i]->name)) {
        if (strncmp(((Tzara *)tz)->nodes[i]->name, name, nameLength) == 0) {
          return i;
        }
      }
    }
  } else {
    for (i = 0; i < ((TzModule *)tz)->numNodes; ++i) {
      if (nameLength == (int)strlen(((TzModule *)tz)->nodes[i]->name)) {
        if (strncmp(((TzModule *)tz)->nodes[i]->name, name, nameLength) == 0) {
          return i;
        }
      }
    }
  }
  return -1;
}

int searchInput(TzNode *node, const char *name) {
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

int searchOutput(TzNode *node, const char *name) {
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

int searchModuleInput(TzModule *m, const char *name) {
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

int searchModuleOutput(TzModule *m, const char *name) {
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

void parseNodeInputString(void *tz, char *str, int *node, int *input,
                          int isModule) {
  char token[TZNODE_NAME_SIZE];
  int i = 0;
  int offset = 0;

  memset(token, '\0', TZNODE_NAME_SIZE);

  if (str[0] == '$') {
    /* var node */
    /* skip '$' */
    while (str[i] != '\0') {
      if (i > 0) {
        token[i - 1] = str[i];
      }
      ++i;
    }
    *node = searchNode(tz, token, isModule);
    *input = (*node >= 0) ? 0 : -1;
    return;
  }

  while (str[i] != '@' && str[i] != '\0') {
    token[i] = str[i];
    ++i;
  }

  if (strncmp(token, "_out_", 5) == 0) {
    if (isModule == 0) {
      *node = TZARA_OUTPUT_NODE_INDEX;
    } else {
      *node = MODULE_OUTPUTS_NODE_INDEX;
    }
  } else {
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
    token[i - offset] = str[i];
    ++i;
  }

  if (*node == TZARA_OUTPUT_NODE_INDEX) {
    if (token[0] == 'l') {
      *input = TZARA_OUTPUT_LEFT_INDEX;
    } else if (token[0] == 'r') {
      *input = TZARA_OUTPUT_RIGHT_INDEX;
    } else {
      *input = -1;
    }
  } else if (*node == MODULE_OUTPUTS_NODE_INDEX) {
    *input = searchModuleOutput((TzModule *)tz, token);
  } else {
    if (isModule == 0) {
      *input =
          (*node >= 0) ? searchInput(((Tzara *)tz)->nodes[*node], token) : -1;
    } else {
      *input = (*node >= 0) ? searchInput(((TzModule *)tz)->nodes[*node], token)
                            : -1;
    }
  }
}

void parseNodeOutputString(void *tz, char *str, int *node, int *output,
                           int isModule) {
  char token[TZNODE_NAME_SIZE];
  int i = 0;
  int offset = 0;

  memset(token, '\0', TZNODE_NAME_SIZE);

  if (str[0] == '$') {
    /* var node */
    /* skip '$' */
    while (str[i] != '\0') {
      if (i > 0) {
        token[i - 1] = str[i];
      }
      ++i;
    }
    *node = searchNode(tz, token, isModule);
    *output = (*node >= 0) ? 0 : -1;
    return;
  }

  while (str[i] != '@' && str[i] != '\0') {
    token[i] = str[i];
    ++i;
  }

  if (strncmp(token, "_in_", 4) == 0) {
    if (isModule != 0) {
      *node = MODULE_INPUTS_NODE_INDEX;
    }
  } else {
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
    token[i - offset] = str[i];
    ++i;
  }

  if (*node == MODULE_INPUTS_NODE_INDEX) {
    *output = searchModuleInput((TzModule *)tz, token);
  } else {
    if (isModule == 0) {
      *output =
          (*node >= 0) ? searchOutput(((Tzara *)tz)->nodes[*node], token) : -1;
    } else {
      *output = (*node >= 0)
                    ? searchOutput(((TzModule *)tz)->nodes[*node], token)
                    : -1;
    }
  }
}

float getConstantValue(char *token) {
  int i = 0;
  if (strncmp(token, "pi", strlen(token)) == 0) {
    return M_PI;
  } else if (strncmp(token, "twopi", strlen(token)) == 0) {
    return 2.f * M_PI;
  } else {
    for (i = 0; i < 128; ++i) {
      if (strncmp(token, midiNotes[i], strlen(token)) == 0) {
        return (float)i;
      }
    }

    for (i = 0; i < 12; ++i) {
      if (strncmp(token, noteNames[i], strlen(token)) == 0) {
        return (float)i;
      }
    }

    for (i = 0; i < NUM_SCALES; ++i) {
      if (strncmp(token, scaleNames[i], strlen(token)) == 0) {
        return (float)i;
      }
    }

    return atof(token);
  }
}

int parseCreateConstantInstruction(void *tz, char **tokens, int numTokens,
                                   int isModule) {
  float val = 0.f;
  int node = -1;
  int input = -1;
  char destString[512];

  memset(destString, '\0', 512);

  if (numTokens < 3) {
    printf("Not enough arguments...\n");
    return 1;
  }

  val = getConstantValue(tokens[1]);

  strncpy(destString, tokens[2], 511);
  trimNewLine(destString);
  parseNodeInputString(tz, destString, &node, &input, isModule);

  if (node == TZARA_OUTPUT_NODE_INDEX) {
    if (isModule == 0) {
      if (input == TZARA_OUTPUT_LEFT_INDEX) {
        printf("Map constant with value %f to out[L]\n", val);
        addNode((Tzara *)tz, createConstantNode(val), "\0");
        connectNodeToOutput((Tzara *)tz, ((Tzara *)tz)->numNodes - 1, 0, 0);
      } else if (input == TZARA_OUTPUT_RIGHT_INDEX) {
        printf("Map constant with value %f to out[R]\n", val);
        addNode((Tzara *)tz, createConstantNode(val), "\0");
        connectNodeToOutput((Tzara *)tz, ((Tzara *)tz)->numNodes - 1, 0, 1);
      } else {
        fprintf(stderr, "Invalid Input...\n");
        return 1;
      }
      return 0;
    } else {
      fprintf(stderr, "Cannot route module node to main out...\n");
      return 1;
    }
  }

  if (node == MODULE_OUTPUTS_NODE_INDEX) {
    if (input < 0) {
      fprintf(stderr, "Invalid module output : %s\n", destString);
      return 1;
    }
    printf("Map constant with value %f to module out\n", val);
    addModuleNode((TzModule *)tz, createConstantNode(val), "\0");
    connectModuleOutlet((TzModule *)tz, ((TzModule *)tz)->numNodes - 1, 0,
                        input);
    return 0;
  }

  if (node < 0) {
    printf("Invalid node : %s...\n", destString);
    return 1;
  }

  if (input < 0) {
    printf("Invalid node input : %s...\n", destString);
    return 1;
  }

  if (isModule == 0) {
    printf("Map constant with value %f to %s[%s]\n", val,
           ((Tzara *)tz)->nodes[node]->name,
           ((Tzara *)tz)->nodes[node]->inputsNames[input]);
    addNode((Tzara *)tz, createConstantNode(val), "\0");
    connectNodes((Tzara *)tz, ((Tzara *)tz)->numNodes - 1, 0, node, input);
  } else {
    printf("Map constant with value %f to %s[%s]\n", val,
           ((TzModule *)tz)->nodes[node]->name,
           ((TzModule *)tz)->nodes[node]->inputsNames[input]);
    addModuleNode((TzModule *)tz, createConstantNode(val), "\0");
    connectModuleNodes((TzModule *)tz, ((TzModule *)tz)->numNodes - 1, 0, node,
                       input);
  }

  return 0;
}

int parseConnectInstruction(void *tz, char **tokens, int numTokens,
                            int isModule) {
  int srcNode = -1;
  int srcOutput = -1;
  int destNode = -1;
  int destInput = -1;
  char srcString[512];
  char destString[512];

  memset(srcString, '\0', 512);
  memset(destString, '\0', 512);

  if (numTokens < 3) {
    printf("Not enough arguments...\n");
    return 1;
  }

  strncpy(srcString, tokens[1], 511);
  parseNodeOutputString(tz, srcString, &srcNode, &srcOutput, isModule);

  strncpy(destString, tokens[2], 511);
  trimNewLine(destString);
  parseNodeInputString(tz, destString, &destNode, &destInput, isModule);

  if (destNode == TZARA_OUTPUT_NODE_INDEX) {
    if (isModule == 0) {
      if (srcNode < 0 || srcOutput < 0) {
        fprintf(stderr, "Invalid connection source : %s\n", srcString);
        return 1;
      }
      if (destInput == TZARA_OUTPUT_LEFT_INDEX) {
        printf("Connect %s[%s] to out[L]\n",
               ((Tzara *)tz)->nodes[srcNode]->name,
               ((Tzara *)tz)->nodes[srcNode]->outputsNames[srcOutput]);
        connectNodeToOutput((Tzara *)tz, srcNode, srcOutput, 0);
      } else if (destInput == TZARA_OUTPUT_RIGHT_INDEX) {
        printf("Connect %s[%s] to out[R]\n",
               ((Tzara *)tz)->nodes[srcNode]->name,
               ((Tzara *)tz)->nodes[srcNode]->outputsNames[srcOutput]);
        connectNodeToOutput((Tzara *)tz, srcNode, srcOutput, 1);
      } else {
        fprintf(stderr, "Invalid connection destination : %s\n", destString);
        return 1;
      }
      return 0;
    } else {
      fprintf(stderr, "Cannot route module node to main out...\n");
      return 1;
    }
  }

  if (destNode == MODULE_OUTPUTS_NODE_INDEX &&
      srcNode == MODULE_INPUTS_NODE_INDEX) {
    printf("Cannot connect module input straight to module output...\n");
    return 1;
  }

  if (destNode == MODULE_OUTPUTS_NODE_INDEX) {
    if (srcNode < 0 || srcOutput < 0) {
      fprintf(stderr, "Invalid connection source : %s\n", srcString);
      return 1;
    }
    if (destInput < 0) {
      fprintf(stderr, "Invalid module output : %s\n", destString);
      return 1;
    }
    printf("Connect %s[%s] to module out\n",
           ((TzModule *)tz)->nodes[srcNode]->name,
           ((TzModule *)tz)->nodes[srcNode]->outputsNames[srcOutput]);
    connectModuleOutlet((TzModule *)tz, srcNode, srcOutput, destInput);
    return 0;
  }

  if (srcNode == MODULE_INPUTS_NODE_INDEX) {
    if (destNode < 0 || destInput < 0) {
      fprintf(stderr, "Invalid connection destination : %s\n", destString);
      return 1;
    }
    if (srcOutput < 0) {
      fprintf(stderr, "Invalid module input : %s\n", srcString);
      return 1;
    }
    printf("Connect module in to %s[%s]\n",
           ((TzModule *)tz)->nodes[destNode]->name,
           ((TzModule *)tz)->nodes[destNode]->inputsNames[destInput]);
    connectModuleInlet((TzModule *)tz, destNode, destInput, srcOutput);
    return 0;
  }

  if (srcNode < 0) {
    printf("Invalid node : %s...\n", srcString);
    return 1;
  }

  if (srcOutput < 0) {
    printf("Invalid node output : %s...\n", srcString);
    return 1;
  }

  if (destNode < 0) {
    printf("Invalid node : %s...\n", destString);
    return 1;
  }

  if (destInput < 0) {
    printf("Invalid node input : %s...\n", destString);
    return 1;
  }

  if (isModule == 0) {
    printf("Connect %s[%s] to %s[%s]\n", ((Tzara *)tz)->nodes[srcNode]->name,
           ((Tzara *)tz)->nodes[srcNode]->outputsNames[srcOutput],
           ((Tzara *)tz)->nodes[destNode]->name,
           ((Tzara *)tz)->nodes[destNode]->inputsNames[destInput]);
    connectNodes((Tzara *)tz, srcNode, srcOutput, destNode, destInput);
  } else {
    printf("Connect %s[%s] to %s[%s]\n", ((TzModule *)tz)->nodes[srcNode]->name,
           ((TzModule *)tz)->nodes[srcNode]->outputsNames[srcOutput],
           ((TzModule *)tz)->nodes[destNode]->name,
           ((TzModule *)tz)->nodes[destNode]->inputsNames[destInput]);
    connectModuleNodes((TzModule *)tz, srcNode, srcOutput, destNode, destInput);
  }

  if (destNode < srcNode) {
    printf("! WARNING ! this is a backward connection, it introduces a 1 "
           "sample delay.\n");
  }

  return 0;
}

int parseModuleIOInstruction(void *tz, char **tokens, int numTokens,
                             int isModule) {
  int isIn = 0;
  int isOut = 0;
  char name[TZNODE_NAME_SIZE];

  memset(name, '\0', TZNODE_NAME_SIZE - 1);

  if (isModule == 0) {
    printf("IO connections are only valid in modules!\n");
    return 1;
  }

  if (numTokens < 3) {
    printf("Not enough arguments...\n");
    return 1;
  }

  isIn = strncmp(tokens[1], "in", strlen(tokens[1])) == 0 ? 1 : 0;
  isOut = strncmp(tokens[1], "out", strlen(tokens[1])) == 0 ? 1 : 0;
  strcpy(name, tokens[2]);
  trimNewLine(name);

  if (isIn == 0 && isOut == 0) {
    printf("Invalid module IO instruction...\n");
    return 1;
  }

  if (isIn != 0) {
    printf("Creating module in : %s\n", name);
    createModuleInlet((TzModule *)tz, name);
    return 0;
  }

  if (isOut != 0) {
    printf("Creating module out : %s\n", name);
    createModuleOutlet((TzModule *)tz, name);
    return 0;
  }

  return 1;
}

int parseInstruction(void *tz, char *instr, char **tokens, int numTokens,
                     int isModule, const char *parentDir) {
  const int op = parseOperator(tokens[0][0]);
  int err = 0;

  switch (op) {
  case COMMENT_OP:
    parseCommentInstruction(instr);
    break;

  case CREATE_NODE_OP:
    err = parseCreateNodeInstruction(tz, tokens, numTokens, isModule,
                                     parentDir);
    break;

  case CREATE_CONSTANT_OP:
    err = parseCreateConstantInstruction(tz, tokens, numTokens, isModule);
    break;

  case CONNECT_OP:
    err = parseConnectInstruction(tz, tokens, numTokens, isModule);
    break;

  case MODULE_IO_OP:
    err = parseModuleIOInstruction(tz, tokens, numTokens, isModule);
    break;

  case METADATA_OP:
    parseMetadataInstruction(tz, tokens, numTokens, isModule);
    break;

  default:
    break;
  }

  return err;
}

int parsePatch(void *engine, FILE *patch, const char *filename, int isModule) {
  char cache[PARSER_CACHE_SIZE];
  char instr[PARSER_CACHE_SIZE];
  char parentDir[PARSER_PATH_SIZE];
  int i;
  int lcount = 1;
  int err = 0;
  char *tokens[PARSER_MAX_TOKENS];
  char *tok;
  int numTokens = 0;

  memset(cache, 0, PARSER_CACHE_SIZE);
  memset(instr, 0, PARSER_CACHE_SIZE);
  getParentDir(filename, parentDir, sizeof(parentDir));

  for (i = 0; i < PARSER_MAX_TOKENS; ++i) {
    tokens[i] = malloc(PARSER_TOKEN_LENGTH * sizeof(char));
    memset(tokens[i], '\0', PARSER_TOKEN_LENGTH);
  }

  while (fgets(cache, PARSER_CACHE_SIZE, patch) != NULL) {

    strcpy(instr, cache);

    for (i = 0; i < numTokens; ++i) {
      memset(tokens[i], '\0', PARSER_TOKEN_LENGTH);
    }

    numTokens = 0;
    tok = strtok(cache, " ");

    while (tok != NULL) {
      strncpy(tokens[numTokens], tok, PARSER_TOKEN_LENGTH - 1);
      tok = strtok(NULL, " ");
      ++numTokens;
    }

    if (numTokens > 0) {
      err = parseInstruction(engine, instr, tokens, numTokens, isModule,
                             parentDir);
    }

    if (err != 0) {
      printf("Issue encountered while parsing %s, line %d.\n", filename,
             lcount);
      break;
    }
    ++lcount;
  }

  for (i = 0; i < PARSER_MAX_TOKENS; ++i) {
    free(tokens[i]);
  }

  return err;
}
