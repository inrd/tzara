#include "tzara.h"

#include <stdio.h>

#include "tzdsp.h"

void init(Tzara *t) {
  int i = 0;
  for (i = 0; i < TZARA_MAX_NODES; ++i) {
    t->nodes[i] = NULL;
  }
  t->numNodes = 0;

  for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
    t->outputs[i] = NULL;
  }

  t->renderDuration = 60;

  tzGenRandomSeed();
}

int addNode(Tzara *tz, TzNode *n, const char *name) {
  if (n != NULL) {
    if (tz->numNodes < (TZARA_MAX_NODES - 1)) {
      strncpy(n->name, name, sizeof(n->name) - 1);
      tz->nodes[tz->numNodes] = n;
      ++tz->numNodes;
      return NO_ERROR;
    }
    return MAX_MEMORY;
  }
  return 2;
}

void connectNodes(Tzara *tz, int inModule, int inOutput, int outModule,
                  int outInput) {
  if ((outInput < tz->nodes[outModule]->numInputs) &&
      (inOutput < tz->nodes[inModule]->numOutputs)) {
    if (tz->nodes[outModule]->inputs[outInput] != NULL) {
      fprintf(stdout, "Warning : node input was already connected. Replacing "
                      "connection.\n");
    }
    tz->nodes[outModule]->inputs[outInput] =
        &(tz->nodes[inModule]->outputs[inOutput]);
  } else {
    /* TODO : should abort */
    fprintf(stderr, "Invalid routing...\n");
  }
}

void connectNodeToOutput(Tzara *tz, int inModule, int inOutput, int outInput) {
  if ((outInput < TZARA_MAX_OUTPUT_CHANS) &&
      (inOutput < tz->nodes[inModule]->numOutputs)) {
    if (tz->outputs[outInput] != NULL) {
      fprintf(stdout,
              "Warning : out channel %d was already connected. Replacing "
              "connection.\n",
              outInput);
    }
    tz->outputs[outInput] = &(tz->nodes[inModule]->outputs[inOutput]);
  } else {
    /* TODO : should abort */
    fprintf(stderr, "Invalid routing...\n");
  }
}

void process(Tzara *tz, float **out, int numChans, int numSamps,
             float samplerate) {
  int i = 0;
  int n = 0;
  int c = 0;

  for (i = 0; i < numSamps; ++i) {

    TzProcessInfo info;
    info.samplerate = samplerate;
    info.duration = (float)(tz->renderDuration) * 1000.f;

    for (n = 0; n < tz->numNodes; ++n) {
      tz->nodes[n]->perform(tz->nodes[n], &info);
    }

    for (c = 0; c < numChans; ++c) {
      out[c][i] = (c < TZARA_MAX_OUTPUT_CHANS) && (tz->outputs[c] != NULL)
                      ? *(tz->outputs[c])
                      : 0.f;
    }
  }
}

void release(Tzara *tz) {
  int i = 0;
  for (i = 0; i < tz->numNodes; ++i) {
    releaseNode(tz->nodes[i]);
    free(tz->nodes[i]);
    tz->nodes[i] = NULL;
  }
  for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
    tz->outputs[i] = NULL;
  }
}
