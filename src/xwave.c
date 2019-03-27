/*
 * xwave - library for working with sound.
 *
 * Copyright (c) 2018 Elektro Yar. Email: git.electroyar@gmail.com
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
#include <stddef.h>

int xwave_init_wave_file(xwave_wave_file* wave_file, const char* file_name, unsigned long sample_rate, unsigned short bits_per_sample, unsigned short num_channels)
{
   memset(wave_file->file_name, '\0', XWAVE_MAX_PATH);
   strcpy(wave_file->file_name, file_name);
   XWaveHeader_t *pWaveHeader = &wave_file->WaveHeader;

   pWaveHeader->chunkId = RIFF_ID;
   pWaveHeader->chunkSize = XWaveChunkSize;
   pWaveHeader->format = RIFF_WAVE;
   pWaveHeader->subchunk1Id = WAVE_FMT;
   pWaveHeader->subchunk1Size = offsetof(XWaveHeader_t, subchunk2Id) - offsetof(XWaveHeader_t, subchunk1Size) - sizeof(((XWaveHeader_t*)(NULL))->subchunk1Size);
   pWaveHeader->audioFormat = WAVE_FORMAT_PCM;
   pWaveHeader->numChannels = num_channels;
   pWaveHeader->sampleRate = sample_rate;
   pWaveHeader->bitsPerSample = bits_per_sample;
   pWaveHeader->byteRate = ((pWaveHeader->bitsPerSample / BITS_IN_BYTE) * pWaveHeader->numChannels * pWaveHeader->sampleRate);
   pWaveHeader->blockAlign = ((pWaveHeader->bitsPerSample / BITS_IN_BYTE) *  pWaveHeader->numChannels);
  
   pWaveHeader->subchunk2Id = WAVE_DATA;
   pWaveHeader->subchunk2Size=0;
  
   
   wave_file->pFile = fopen(wave_file->file_name, "wb");

   if(wave_file->pFile == NULL) return XWAVE_ERROR;

   fwrite(pWaveHeader, sizeof(XWaveHeader_t), 1, wave_file->pFile);

   return XWAVE_OK;
}

void xwave_write_sample_wave_file(xwave_wave_file* wave_file, void* data)
{
   fwrite(data, (wave_file->WaveHeader.bitsPerSample / BITS_IN_BYTE), wave_file->WaveHeader.blockAlign, wave_file->pFile);
   wave_file->WaveHeader.subchunk2Size += ((wave_file->WaveHeader.bitsPerSample / BITS_IN_BYTE) * wave_file->WaveHeader.blockAlign);
}

void xwave_write_data_block_wave_file(xwave_wave_file* wave_file, void* data, unsigned long len)
{
   fwrite(data, (wave_file->WaveHeader.bitsPerSample / BITS_IN_BYTE), len, wave_file->pFile);
   wave_file->WaveHeader.subchunk2Size += (len * (wave_file->WaveHeader.bitsPerSample / BITS_IN_BYTE));
}


// данная функция закрывает аудиофайл и записывает количесвто байт данных.
void xwave_close_wave_file(xwave_wave_file* wave_file)
{
   const unsigned long chunk_id_chunk_size_bits = 8;
   wave_file->WaveHeader.chunkSize = XWaveChunkSize + wave_file->WaveHeader.subchunk2Size;
   fseek(wave_file->pFile, 0, SEEK_SET);
   fwrite(&wave_file->WaveHeader, sizeof(XWaveHeader_t), 1, wave_file->pFile);
   fclose(wave_file->pFile);
}

void xwave_get_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double period, double amplitude, unsigned long len)
{
   unsigned long i;
   unsigned long t_max  = period * (double)sample_rate;
   unsigned long t_div2 = (unsigned long)((period * (double)sample_rate) / 2.0);
   if(amplitude > 1.0) amplitude = 1.0;
   switch(bits_per_sample)
   {
      case 8:
         for(i = 0; i < len; ++i)
         {
            ((unsigned char*)data)[i] = ((i % t_max) >= t_max || (i % t_max) < t_div2) ? ((UCHAR_MAX - 0x80) * amplitude + 0x80) : (0x80 * (1.0 - amplitude));
         }
         break;
      case 16:
         for(i = 0; i < len; ++i)
         {
            ((short*)data)[i] = amplitude*(((i % t_max) >= t_max || (i % t_max) < t_div2) ? SHRT_MAX : SHRT_MIN);
         }
         break;
   };
}

void xwave_get_multiple_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double* period, double* amplitude, unsigned short num_impulses, unsigned long len, bool bDamped)
{
   double mixer;
   double attenuation = 1.0, attenuation_step = 1.0 / (double)len;
   unsigned long t_max, t_div2;
   for(unsigned long i = 0; i < len; ++i)
   {
      mixer = 0.0;
      for(int n = 0; n < num_impulses; ++n)
      {
         t_max = period[n] * sample_rate;
         t_div2 = (unsigned long)(period[n] * (double)sample_rate / 2.0);
         mixer += (amplitude[n] * ((((i % t_max) >= t_max) || ((i % t_max) < t_div2)) ? 1 : -1));
        
      }
      mixer = (bDamped?attenuation:1.0)*((mixer > 1.0) ? 1.0 : (mixer < -1.0) ? -1.0 : mixer);
      switch(bits_per_sample)
      {
         case 8:
            ((unsigned char*)data)[i] = (mixer > 0) ? ((UCHAR_MAX - 0x80) * mixer + 0x80) : (0x80 * (1.0 + mixer));
            break;
         case 16:
            ((short*)data)[i] = mixer*((mixer > 0) ? SHRT_MAX : -SHRT_MIN);
            break;
      };
      if(bDamped&&((attenuation -= attenuation_step) < 0.0)) attenuation = 0.0;
   }
}

void xwave_get_pause_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, unsigned long len)
{
   unsigned long i;
   switch(bits_per_sample)
   {
      case 8:
         for(i = 0; i < len; ++i)
         {
            ((unsigned char*)data)[i] = 0x80;
         }
         break;
      case 16:
         for(i = 0; i < len; ++i)
         {
            ((short*)data)[i] = 0;
         }
         break;
   };
}
