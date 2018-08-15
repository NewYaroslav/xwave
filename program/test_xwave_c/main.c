#include <stdio.h>
#include <stdlib.h>
#include <xwave.h>
#define data_block_len 32000

void MakeTestFile(int sample_rate, int bits_per_sample)
{
   char FileName[120];//Надеюсь, нам этого хватит
   sprintf(FileName, "Example_%i_%i.wav", bits_per_sample, sample_rate);
   xwave_wave_file example; // создадим структуру файла wav
   
   // инициализируем структуру
   xwave_init_wave_file(&example, FileName, sample_rate, bits_per_sample, 1);
   // создадим файл
   //xwave_create_wave_file(&example);
   printf("sample_rate %d\nbits_per_sample: %d\nblock_align: %d\nbyte_rate: %d\n\n", example.WaveHeader.sampleRate, example.WaveHeader.bitsPerSample, example.WaveHeader.blockAlign, example.WaveHeader.byteRate);
   // создадим тестовую запись

   short data_block[data_block_len];

   // набор амплитуд и периодов
   double ampl[4] = { 0.5, 0.5, 0.5, 0.5 };
   double period[4] = { 0.001, 0.005, 0.0025, 0.001 };

   // запишем затухающий сигнал с несколькими генераторами меандра в файл
   for (int i = 1; i < 4; ++i)
   {
      xwave_get_multiple_damped_impulses_mono(data_block, sample_rate, bits_per_sample, period, ampl, i, data_block_len);
      xwave_write_data_block_wave_file(&example, data_block, data_block_len);
      xwave_get_pause_mono(data_block, sample_rate, bits_per_sample, data_block_len);
      xwave_write_data_block_wave_file(&example, data_block, data_block_len);
   }
   // запишем сигнал с несколькими генераторами меандра в файл
   for (int i = 0; i < 5; ++i)
   {
      xwave_get_multiple_impulses_mono(data_block, sample_rate, bits_per_sample, period, ampl, 2, data_block_len);
      xwave_write_data_block_wave_file(&example, data_block, data_block_len);
      xwave_get_pause_mono(data_block, sample_rate, bits_per_sample, data_block_len);
      xwave_write_data_block_wave_file(&example, data_block, data_block_len);
   }
   // запишем сигнал с меандром в файл
   for (int i = 0; i < 5; ++i)
   {
      xwave_get_impulses_mono(data_block, sample_rate, bits_per_sample, 0.001, 0.5, data_block_len);
      xwave_write_data_block_wave_file(&example, data_block, data_block_len);
      xwave_get_pause_mono(data_block, sample_rate, bits_per_sample, data_block_len);
      xwave_write_data_block_wave_file(&example, data_block, data_block_len);
   }
   // сохраним изменения в файле и закроем его
   xwave_close_wave_file(&example);
}
int main()
{  
   MakeTestFile(2000, 8);
   MakeTestFile(4000, 8);
   MakeTestFile(8000, 8);
   MakeTestFile(11025, 8);
   MakeTestFile(12000, 8);
   MakeTestFile(16000, 8);
   MakeTestFile(22050, 8);
   MakeTestFile(24000, 8);
   MakeTestFile(32000, 8);
   MakeTestFile(44100, 8);
   MakeTestFile(48000, 8);

   MakeTestFile(2000, 16);
   MakeTestFile(4000, 16);
   MakeTestFile(8000, 16);
   MakeTestFile(11025, 16);
   MakeTestFile(12000, 16);
   MakeTestFile(16000, 16);
   MakeTestFile(22050, 16);
   MakeTestFile(24000, 16);
   MakeTestFile(32000, 16);
   MakeTestFile(44100, 16);
   MakeTestFile(48000, 16);
   return 0;
}
