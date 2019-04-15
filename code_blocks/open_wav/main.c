#include <stdio.h>
#include <stdlib.h>
#include <xwave.h>

int main() {
    printf("Hello world!\n");
    xwave_wave_file wav_file;
    xwave_open_wave_file(&wav_file,"test.wav");

    printf("data len = %d\n", wav_file.WaveHeader.subchunk2Size);
    printf("bits per sample = %d\n", wav_file.WaveHeader.bitsPerSample);
    printf("sample rate = %d\n", wav_file.WaveHeader.sampleRate);
    printf("num channels = %d\n", wav_file.WaveHeader.numChannels);

    printf("samples = %d\n", xwave_get_num_samples_wave_file(&wav_file));
    printf("sec = %f\n", (float)xwave_get_num_samples_wave_file(&wav_file)/(float)wav_file.WaveHeader.sampleRate);
    short *data = NULL;

    int num_samples = xwave_get_buffer_size_wave_file(&wav_file);
    data = (short*)malloc(sizeof(short) * num_samples);

    xwave_read_wave_file(&wav_file, data, -1);

    int num_example = 10;
    for(int i = 0; i < num_example; ++i) {
        for(int k = 0; k < wav_file.WaveHeader.numChannels; ++k) {
            printf("channel %d data %d\n", k, data[i + k]);
        }
    }

    xwave_close_wave_file(&wav_file);
    free(data);
    return 0;
}
