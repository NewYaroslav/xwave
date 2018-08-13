/*
* xwave - library for working with sound.
*
* Copyright (c) 2018 Yaroslav Barabanov. Email: elektroyar@yandex.ru
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "xwave.h"
#include <limits.h>
#include <stdio.h>

void xwave_init_wave_file(xwave_wave_file* wave_file, const char* file_name, unsigned long sample_rate, unsigned short bits_per_sample, unsigned short num_channels) {
    memset(wave_file->file_name, '\0', XWAVE_MAX_PATH);
    strcpy(wave_file->file_name, file_name);
    wave_file->sample_rate = sample_rate;
    wave_file->bits_per_sample = bits_per_sample;
    wave_file->num_channels = num_channels;
    wave_file->data = NULL;
}

int xwave_create_wave_file(xwave_wave_file* wave_file) {
    const unsigned short audio_format = 1;  // Аудио формат. Для PCM = 1 (то есть, Линейное квантование). Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
    const unsigned long subchunk1_size = 16; // 16 для формата PCM. Это оставшийся размер подцепочки, начиная с этой позиции.
    const unsigned long chunk_id_chunk_size_bits = 8;
    char temp[8];
    memset(temp, '\0', sizeof(temp));

    wave_file->len_data_type = wave_file->bits_per_sample / 8;
    wave_file->subchunk2_size = 0;
    wave_file->chunk_size = wave_file->subchunk2_size + 44 - chunk_id_chunk_size_bits;
    wave_file->block_align = wave_file->bits_per_sample / (8 * wave_file->num_channels);
    wave_file->byte_rate = wave_file->sample_rate * wave_file->block_align;

    wave_file->fp_save = fopen(wave_file->file_name,"wb");

    if(wave_file->fp_save == NULL) return XWAVE_ERROR;

    strcpy(temp, "RIFF");
    fwrite(&temp, sizeof(char),4, wave_file->fp_save);

    fwrite(&wave_file->chunk_size, sizeof(unsigned long), 1, wave_file->fp_save);

    strcpy(temp, "WAVE");
    fwrite(&temp, sizeof(char),4, wave_file->fp_save);

    strcpy(temp, "fmt ");
    fwrite(&temp, sizeof(char),4, wave_file->fp_save);

    fwrite(&subchunk1_size, sizeof(unsigned long), 1, wave_file->fp_save);
    fwrite(&audio_format, sizeof(unsigned short), 1, wave_file->fp_save);
    fwrite(&wave_file->num_channels, sizeof(unsigned short), 1, wave_file->fp_save);
    fwrite(&wave_file->sample_rate, sizeof(unsigned long), 1, wave_file->fp_save);
    fwrite(&wave_file->byte_rate, sizeof(unsigned long), 1, wave_file->fp_save);
    fwrite(&wave_file->block_align, sizeof(unsigned short), 1, wave_file->fp_save);
    fwrite(&wave_file->bits_per_sample, sizeof(unsigned short), 1, wave_file->fp_save);

    strcpy(temp, "data");
    fwrite(&temp, sizeof(char),4, wave_file->fp_save);

    fwrite(&wave_file->subchunk2_size, sizeof(unsigned long), 1, wave_file->fp_save);

    return XWAVE_OK;
}

void xwave_write_sample_wave_file(xwave_wave_file* wave_file, void* data) {
    fwrite(data, wave_file->len_data_type, wave_file->block_align, wave_file->fp_save);
    wave_file->subchunk2_size += wave_file->len_data_type * wave_file->block_align;
}

void xwave_write_data_block_wave_file(xwave_wave_file* wave_file, void* data, unsigned long len) {
    fwrite(data, wave_file->len_data_type, len, wave_file->fp_save);
    wave_file->subchunk2_size += len * wave_file->len_data_type;
}
// данная функция закрывает аудиофайл и записывает количесвто байт данных.
void xwave_close_wave_file(xwave_wave_file* wave_file) {
    const unsigned long chunk_id_chunk_size_bits = 8;
    wave_file->chunk_size = wave_file->subchunk2_size + 44 - chunk_id_chunk_size_bits;
    fseek(wave_file->fp_save, 4, SEEK_SET);
    fwrite(&wave_file->chunk_size, sizeof(unsigned long), 1, wave_file->fp_save);
    fseek(wave_file->fp_save, 40, SEEK_SET);
    fwrite(&wave_file->subchunk2_size, sizeof(unsigned long), 1, wave_file->fp_save);
    fclose(wave_file->fp_save);
}

void xwave_get_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double period, double amplitude, unsigned long len) {
    unsigned long i;
    unsigned long t_max = period * (double)sample_rate;
    unsigned long t_div2 = (unsigned long)((period * (double)sample_rate) / 2.0);
    if(amplitude > 1.0) amplitude = 1.0;
    switch(bits_per_sample) {
        case 8:
            for(i = 0; i < len; ++i) {
                unsigned long pos = i % t_max;
                if(pos >= t_max) {
                    ((unsigned char*)data)[i] = (UCHAR_MAX - 0x80) * amplitude + 0x80;
                } else
                if(pos >= t_div2) {
                    ((unsigned char*)data)[i] = 0x80 * (1.0 - amplitude);
                } else {
                    ((unsigned char*)data)[i] = (UCHAR_MAX - 0x80) * amplitude + 0x80;
                }
            }
        break;
        case 16:
            for(i = 0; i < len; ++i) {
                unsigned long pos = i % t_max;
                if(pos >= t_max) {
                    ((short*)data)[i] = SHRT_MAX * amplitude;
                    continue;
                } else
                if(pos >= t_div2) {
                    ((short*)data)[i] = SHRT_MIN * amplitude;
                } else {
                    ((short*)data)[i] = SHRT_MAX * amplitude;
                }
            }
        break;
    };
}

void xwave_get_multiple_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double* period, double* amplitude, unsigned short num_impulses, unsigned long len) {
    unsigned long i;
    for(i = 0; i < len; ++i) {
        double mixer = 0.0;
        for(int n = 0; n < num_impulses; ++n) {
            unsigned long t_max = period[n] * sample_rate;
            unsigned long t_div2 = (unsigned long)(period[n] * (double)sample_rate / 2.0);
            unsigned long pos = i % t_max;
            if(pos >= t_max) {
                mixer += amplitude[n];
            } else
            if(pos >= t_div2) {
                mixer -= amplitude[n];
            } else {
                mixer += amplitude[n];
            }
        }
        if(mixer > 1.0) mixer = 1.0;
        if(mixer < -1.0) mixer = -1.0;
        switch(bits_per_sample) {
            case 8:
                if(mixer > 0) {
                    ((unsigned char*)data)[i] = (UCHAR_MAX - 0x80) * mixer + 0x80;
                } else {
                    ((unsigned char*)data)[i] = 0x80 * (1.0 + mixer);
                }
            break;
            case 16:
                if(mixer > 0) {
                    ((short*)data)[i] = SHRT_MAX * mixer;
                } else {
                    ((short*)data)[i] = SHRT_MIN * -mixer;
                }
            break;
        };
    }
}

void xwave_get_multiple_damped_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double* period, double* amplitude, unsigned short num_impulses, unsigned long len) {
    unsigned long i;
    double attenuation = 1.0;
    double attenuation_step = 1.0/(double)len;
    for(i = 0; i < len; ++i) {
        double mixer = 0.0;
        for(int n = 0; n < num_impulses; ++n) {
            unsigned long t_max = period[n] * sample_rate;
            unsigned long t_div2 = (unsigned long)(period[n] * (double)sample_rate / 2.0);
            unsigned long pos = i % t_max;
            if(pos >= t_max) {
                mixer += amplitude[n];
            } else
            if(pos >= t_div2) {
                mixer -= amplitude[n];
            } else {
                mixer += amplitude[n];
            }
        }
        if(mixer > 1.0) mixer = 1.0;
        if(mixer < -1.0) mixer = -1.0;
        mixer *= attenuation;
        switch(bits_per_sample) {
            case 8:
                if(mixer > 0) {
                    ((unsigned char*)data)[i] = (UCHAR_MAX - 0x80) * mixer + 0x80;
                } else {
                    ((unsigned char*)data)[i] = 0x80 * (1.0 + mixer);
                }
            break;
            case 16:
                if(mixer > 0) {
                    ((short*)data)[i] = SHRT_MAX * mixer;
                } else {
                    ((short*)data)[i] = SHRT_MIN * -mixer;
                }
            break;
        };
        attenuation -=  attenuation_step;
        if(attenuation < 0.0) attenuation = 0.0;
    }
}

void xwave_get_pause_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, unsigned long len) {
    unsigned long i;
    switch(bits_per_sample) {
        case 8:
            for(i = 0; i < len; ++i) {
                ((unsigned char*)data)[i] = 0x80;
            }
        break;
        case 16:
            for(i = 0; i < len; ++i) {
                ((short*)data)[i] = 0;
            }
        break;
    };
}
