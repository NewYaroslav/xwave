#include <stdio.h>
#include <stdlib.h>
#include "xwave.h"
#define  data_block_len  32000
int main() {
    printf("Hello world!\n");
    const int bits_per_sample = 16; // Количество бит в сэмпле. Так называемая “глубина” или точность звучания.
    const int sample_rate = 16000;  // Частота дискретизации.
    const int num_channels = 1;     // Количество каналов.

    xwave_wave_file example; // создадим структуру файла wav

    // инициализируем структуру
    xwave_init_wave_file(&example, "example.wav", sample_rate, bits_per_sample, num_channels);
    // создадим файл
    xwave_create_wave_file(&example);
    printf("sample_rate %d\nbits_per_sample: %d\nblock_align: %d\nbyte_rate: %d\n", example.sample_rate, example.bits_per_sample, example.block_align, example.byte_rate);
    // создадим тестовую запись
    
    short data_block[data_block_len];

    // набор амплитуд и периодов
    double ampl[4] = {0.5,0.5,0.5,0.5};
    double period[4] = {0.001,0.005,0.0025,0.001};

    // запишем затухающий сигнал с несколькими генераторами меандра в файл
    for(int i = 1; i < 4; ++i) {
        xwave_get_multiple_damped_impulses_mono(data_block, sample_rate, bits_per_sample, period, ampl, i, data_block_len);
        xwave_write_data_block_wave_file(&example, data_block, data_block_len);
        xwave_get_pause_mono(data_block, sample_rate, bits_per_sample, data_block_len);
        xwave_write_data_block_wave_file(&example, data_block, data_block_len);
    }
    // запишем сигнал с несколькими генераторами меандра в файл
    for(int i = 0; i < 5; ++i) {
        xwave_get_multiple_impulses_mono(data_block, sample_rate, bits_per_sample, period, ampl, 2, data_block_len);
        xwave_write_data_block_wave_file(&example, data_block, data_block_len);
        xwave_get_pause_mono(data_block, sample_rate, bits_per_sample, data_block_len);
        xwave_write_data_block_wave_file(&example, data_block, data_block_len);
    }
    // запишем сигнал с меандром в файл
    for(int i = 0; i < 5; ++i) {
        xwave_get_impulses_mono(data_block, sample_rate, bits_per_sample, 0.001, 0.5, data_block_len);
        xwave_write_data_block_wave_file(&example, data_block, data_block_len);
        xwave_get_pause_mono(data_block, sample_rate, bits_per_sample, data_block_len);
        xwave_write_data_block_wave_file(&example, data_block, data_block_len);
    }
    // сохраним изменения в файле и закроем его
    xwave_close_wave_file(&example);

    return 0;
}
