#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tzara.h"
#include "parser.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


#define TZARA_WAV_DURATION_SEC 60
#define TZARA_BUFFER_SIZE 4096

void normalize (float** buf, const unsigned long int numSamples, const int numChannels) {
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

    if (peak != 0.f) {
        ratio = 1.f / peak;
    }

    printf("Peak : %0.2f\nNormalization factor : %0.2f\n", peak, ratio);

    for (c = 0; c < numChannels; ++c) {
        for (i = 0; i < numSamples; ++i) {
            buf[c][i] *=  ratio;
        }
    }

}

int main (int argc, char** argv) {
    float* data[TZARA_MAX_OUTPUT_CHANS];
    FILE* patch = NULL;
    Tzara tz;
    const float samplerate = 44100.f;
    drwav wav;
    float* outData;
    int i, j = 0;
    unsigned long int numFrames = (unsigned long int)samplerate * TZARA_WAV_DURATION_SEC;
    unsigned long int framesCount = 0;

    srand((unsigned int)time(NULL));

    if (argc < 2) {
        fprintf(stderr, "Usage:\ntzara [patchfile]\nor\ntzara --nodes\n\n");
        return 1;
    }

    if (strncmp(argv[1], "--nodes", strlen(argv[1])) == 0) {
        for (i = 1; i < NUM_NODE_TYPES; ++i) {
            printf("- [%s] : %s\n\t- inputs: %s\n\t- outputs: %s\n", nodesDoc[i].name, nodesDoc[i].summary, nodesDoc[i].inputs, nodesDoc[i].outputs);
        }
        printf("\n");
        return 0;
    }

    patch = fopen(argv[1], "r");
    if (patch == NULL) {
        fprintf(stderr, "Could not open %s...\n", argv[1]);
        return 1;
    }

    init(&tz);

    if (parsePatch (&tz, patch, argv[1], 0) != 0) {
        fclose(patch);
        release(&tz);
        fprintf(stderr, "Errors encountered while building patch...\nAborting.\n\n");
        return 1;
    }

    fclose(patch);

    printf("Rendering output...\n");

    for (i = 0; i < TZARA_MAX_OUTPUT_CHANS; ++i) {
        data[i] = (float*)malloc(numFrames * sizeof(float));
    }

    process (&tz, data, TZARA_MAX_OUTPUT_CHANS, numFrames, samplerate);

    printf("Normalizing levels...\n");

    normalize (data, numFrames, TZARA_MAX_OUTPUT_CHANS);

    printf("Writing to file...\n");

    drwav_data_format format;
    format.container = drwav_container_riff;     
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 2;
    format.sampleRate = (int)samplerate;
    format.bitsPerSample = 32;
    drwav_init_file_write(&wav, "out.wav", &format, NULL);

    outData = (float*)malloc(TZARA_BUFFER_SIZE * 2 * sizeof(float));

    framesCount = 0;

    while (framesCount < numFrames) {
        for (i = 0, j = 0; j < TZARA_BUFFER_SIZE; i += 2, ++j) {
            outData[i] = data[0][j + framesCount];
            outData[i+1] = data[1][j + framesCount];
        }
        
        drwav_write_pcm_frames(&wav, TZARA_BUFFER_SIZE, outData);

        framesCount += TZARA_BUFFER_SIZE;
    }

    drwav_uninit(&wav);

    printf("Rendered to out.wav\n\n");

    release(&tz);

    return 0;
}

