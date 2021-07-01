#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tzara.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define TZARA_FILE_NAME_MAX_LENGTH 512

#define TZARA_RENDER_SR 44100.0
#define TZARA_RENDER_BITDEPTH 32

void normalize(float **buf, const unsigned long int numSamples,
               const int numChannels) {
  float peak = 0.f;
  float ratio = 1.f;
  float in = 0.f;
  unsigned long int i = 0;
  int c = 0;

  for (c = 0; c < numChannels; ++c) {
    for (i = 0; i < numSamples; ++i) {
      in = fabs(buf[c][i]);
      peak = in > peak ? in : peak;
    }
  }

  if (peak > 1.f) {
    printf("Normalizing levels...\n");
    ratio = 1.f / peak;
    printf("Peak : %0.2f\nNormalization factor : %0.2f\n", peak, ratio);

    for (c = 0; c < numChannels; ++c) {
      for (i = 0; i < numSamples; ++i) {
        buf[c][i] *= ratio;
      }
    }
  }
}

void printNodesHelp() {
  int i;
  for (i = 1; i < NUM_NODE_TYPES; ++i) {
    printf("- [%s] : %s\n\t- inputs: %s\n\t- outputs: %s\n", nodesDoc[i].name,
           nodesDoc[i].summary, nodesDoc[i].inputs, nodesDoc[i].outputs);
  }
  printf("\n");
}

void printScalesHelp() {
  int i;
  printf("Available scales for the notescale node :\n");
  for (i = 0; i < NUM_SCALES; ++i) {
    printf("- %s\n", scaleNames[i]);
  }
  printf("\n");
}

void initWavFormat(drwav_data_format *f, int sr) {
  f->container = drwav_container_riff;
  f->format = DR_WAVE_FORMAT_IEEE_FLOAT;
  f->channels = 2;
  f->sampleRate = sr;
  f->bitsPerSample = TZARA_RENDER_BITDEPTH;
}

int main(int argc, char **argv) {
  float *data[TZARA_MAX_OUTPUT_CHANS];
  FILE *patch = NULL;
  Tzara tz;
  const float samplerate = TZARA_RENDER_SR;
  drwav wav;
  drwav_data_format format;
  float *outData;
  unsigned long int i, j = 0;
  unsigned long int numFrames = (unsigned long int)samplerate * 60;
  char patchName[TZARA_FILE_NAME_MAX_LENGTH];
  char wavName[TZARA_FILE_NAME_MAX_LENGTH + 5];
  char *ext = NULL;

  memset(wavName, '\0', TZARA_FILE_NAME_MAX_LENGTH);

  if (argc < 2) {
    fprintf(stderr, "Usage:\ntzara [patch file] [out wav file]\ntzara [patch "
                    "file]\ntzara --nodes\ntzara --scales\n\n");
    return 1;
  }

  if (argc == 2) {
    if (strncmp(argv[1], "--nodes", strlen(argv[1])) == 0) {
      printNodesHelp();
      return 0;
    } else if (strncmp(argv[1], "--scales", strlen(argv[1])) == 0) {
      printScalesHelp();
      return 0;
    } else {
      strncpy(patchName, argv[1], TZARA_FILE_NAME_MAX_LENGTH - 5);
      strcpy(wavName, patchName);
      /* replace extension if any */
      ext = strrchr(wavName, '.');
      if (ext != NULL) {
        strcpy(ext, ".wav");
      } else {
        sprintf(wavName, "%s.wav", patchName);
      }
    }
  } else if (argc == 3) {
    strncpy(patchName, argv[1], TZARA_FILE_NAME_MAX_LENGTH - 1);
    strncpy(wavName, argv[2], TZARA_FILE_NAME_MAX_LENGTH - 1);
  }

  patch = fopen(patchName, "r");
  if (patch == NULL) {
    fprintf(stderr, "Could not open %s...\n", argv[1]);
    return 1;
  }

  init(&tz);

  if (parsePatch(&tz, patch, patchName, 0) != 0) {
    fclose(patch);
    release(&tz);
    fprintf(stderr,
            "Errors encountered while building patch %s...\nAborting.\n\n",
            patchName);
    return 1;
  }

  fclose(patch);

  numFrames = (unsigned long int)samplerate * tz.renderDuration;

  printf("Rendering output...\n");

  for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
    data[i] = (float *)malloc(numFrames * sizeof(float));
  }

  process(&tz, data, TZARA_MAX_OUTPUT_CHANS, numFrames, samplerate);

  normalize(data, numFrames, TZARA_MAX_OUTPUT_CHANS);

  printf("Writing to file...\n");

  initWavFormat(&format, (int)samplerate);

  if (!drwav_init_file_write(&wav, wavName, &format, NULL)) {
    fprintf(stderr, "Error writing %s...\nAborting...\n\n", wavName);
    return 1;
  }

  /* need interleaved data for output */
  outData = (float *)malloc(numFrames * 2 * sizeof(float));

  for (i = 0, j = 0; j < numFrames; i += 2, ++j) {
    outData[i] = data[0][j];
    outData[i + 1] = data[1][j];
  }

  if (drwav_write_pcm_frames(&wav, numFrames, outData) < numFrames) {
    fprintf(stderr, "Error : not all data has been written to %s.\n", wavName);
  }

  drwav_uninit(&wav);

  printf("Rendered to %s\n\n", wavName);

  release(&tz);

  return 0;
}
