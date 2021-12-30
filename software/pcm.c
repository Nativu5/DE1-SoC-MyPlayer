#include <stdlib.h>
#include <stdio.h>

#include "pcm.h"
#include "audio_control.h"

int play_PCM(const char *filename)
{
    FILE *fp = fopen(filename, "rb+");
    if (fp == NULL)
    {
        fprintf(stderr, "[ERROR] Open %s failed.\n", filename);
        return -1;
    }

    char *sample = (char *)malloc(4);
    while (!feof(fp))
    {
        int try_cnt = 0;
        while (!AUDIO_DacFifoNotFull() && try_cnt < MAX_TRY_CNT)
            try_cnt++;

        if (try_cnt >= MAX_TRY_CNT)
            fprintf(stderr, "[ERROR] Audio chip error...\n");
        
        fread(sample, 1, 4, fp);
        int sample_l = (int)sample[0] | (((int)sample[1]) << 8);
        int sample_r = (int)sample[2] | (((int)sample[3]) << 8);

        AUDIO_DacFifoSetData(sample_l, sample_r);
    }

    free(sample);
    fclose(fp);
    return 0;
}