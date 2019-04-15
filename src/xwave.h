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

#ifndef XWAVE_H_INCLUDED
#define XWAVE_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

//для работы с файлами
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// Makes a 4-byte "packed ID" int out of 4 characters
#define MAKEID(d,c,b,a)	(((int)(a)<<24)|((int)(b)<<16)|((int)(c)<<8)|((int)(d)))
#define RIFF_ID			MAKEID('R','I','F','F')
#define RIFF_WAVE			MAKEID('W','A','V','E')
#define WAVE_FMT			MAKEID('f','m','t',' ')
#define WAVE_DATA			MAKEID('d','a','t','a')

#define XWAVE_MAX_PATH 260
#define XWAVE_OK 0x01
#define XWAVE_ERROR 0x00
#define XWAVE_SAVE_FILE_ONLINE 0x01
#define WAVE_FORMAT_PCM 0x0001
#define BITS_IN_BYTE 8
#define XWaveChunkSize (sizeof(XWaveHeader_t) - (offsetof(XWaveHeader_t, chunkSize) + sizeof(((XWaveHeader_t*)(NULL))->chunkSize)))

   typedef struct XWaveHeader_s
   {
      uint32_t	chunkId;       /**< Содержит символы “RIFF” в ASCII кодировке (0x52494646 в big-endian представлении). Является началом RIFF-цепочки.*/
      uint32_t	chunkSize;     /**< Это оставшийся размер цепочки, начиная с этой позиции. Иначе говоря, это размер файла – 8, то есть, исключены поля chunkId и chunkSize.*/
      uint32_t	format;        /**< Содержит символы “WAVE” (0x57415645 в big-endian представлении)*/
      uint32_t	subchunk1Id;   /**< Содержит символы “fmt “ (0x666d7420 в big-endian представлении)*/
      uint32_t	subchunk1Size; /**< 16 для формата PCM. Это оставшийся размер подцепочки, начиная с этой позиции.*/
      uint16_t	audioFormat;   /**< Аудио формат, полный список можно получить здесь. Для PCM = 1 (то есть, Линейное квантование). Значения, отличающиеся от 1, обозначают некоторый формат сжатия.*/
      uint16_t	numChannels;   /**< Количество каналов. Моно = 1, Стерео = 2 и т.д.*/
      uint32_t	sampleRate;    /**< Частота дискретизации. 8000 Гц, 44100 Гц и т.д.*/
      uint32_t	byteRate;      /**< Количество байт, переданных за секунду воспроизведения.*/
      uint16_t	blockAlign;    /**< Количество байт для одного сэмпла, включая все каналы.*/
      uint16_t	bitsPerSample; /**< Количество бит в сэмпле. Так называемая “глубина” или точность звучания. 8 бит, 16 бит и т.д.*/
      uint32_t	subchunk2Id;   /**< Содержит символы “data” (0x64617461 в big-endian представлении)*/
      uint32_t	subchunk2Size; /**< Количество байт в области данных.*/
   }XWaveHeader_t;

   typedef struct
   {
      char          file_name[XWAVE_MAX_PATH]; /**< Имя файла */
      FILE*         pFile;                     /**< Сылка на файл */
      uint8_t       isOpen;
      XWaveHeader_t WaveHeader;
   } xwave_wave_file;

   /** \brief Инициализация структуры wav-файла
    *
    * \param wave_file структура файла wav
    * \param file_name имя файла
    * \param sample_rate Частота дискретизации
    * \param bits_per_sample Количество бит в сэмпле
    * \param num_channels Количество каналов
    * \return вернет XWAVE_OK в случае отсутствия ошибок
    *
    */
   int xwave_init_wave_file(xwave_wave_file* wave_file, const char* file_name, unsigned long sample_rate, unsigned short bits_per_sample, unsigned short num_channels);

   /** \brief Открыть wav-файл
    *
    * \param wave_file структура файла wav
    * \param file_name имя файла
    * \return вернет XWAVE_OK в случае отсутствия ошибок
    *
    */
   int xwave_open_wave_file(xwave_wave_file* wave_file, const char* file_name);

   /** \brief Получить количество сэмплов wav-файла
    *
    * \param wave_file структура файла wav
    * \return вернет количество сэмплов
    *
    */
   int xwave_get_num_samples_wave_file(xwave_wave_file* wave_file);

   /** \brief Получить размер буфера wav-файла
    *
    * \param wave_file структура файла wav
    * \return вернет размер буфера с учетом количества каналов и размером данных
    *
    */
   int xwave_get_buffer_size_wave_file(xwave_wave_file* wave_file);

   /** \brief Прочитать wav-файл в буфер
    * Данная функция прочитает в буфер указанное количество сэмплов
    * \param wave_file структура файла wav
    * \param buf буфер
    * \param samples сэмплы, указать меньше 0 чтобы прочитать все сэмплы
    * \return вернет XWAVE_OK в случае отсутствия ошибок
    *
    */
   int xwave_read_wave_file(xwave_wave_file* wave_file, void *buf, int num_samples);

   /** \brief Создать wav файл для онлайн записи
    *
    * \param wave_file структура файла wav
    * \return вернет XWAVE_OK если удалось создать файл
    *
    */
   //int xwave_create_wave_file(xwave_wave_file* wave_file);

   /** \brief Записать сэмпл в wav файл
    *
    * \param wave_file структура файла wav
    * \param data данные для сохранения
    *
    */
   void xwave_write_sample_wave_file(xwave_wave_file* wave_file, void* data);

   /** \brief Записать блок данных в wav файл
    *
    * \param wave_file структура файла wav
    * \param data данные для сохранения
    * \param len длина данных
    *
    */
   void xwave_write_data_block_wave_file(xwave_wave_file* wave_file, void* data, unsigned long len);

   /** \brief Закрыть wav файл
    * \warning Данную функцию необходимо вызывать обязательно в конце записи
    * \param wave_file структура файла wav
    *
    */
   void xwave_close_wave_file(xwave_wave_file* wave_file);

   /** \brief Сгенерировать меандр и записать в массив
    *
    * \param data данные для сохранения
    * \param sample_rate Частота дискретизации
    * \param bits_per_sample Количество бит в сэмпле
    * \param period период меандра
    * \param amplitude амплитуда сигнала (от 0 до 1.0)
    * \param длина массива data
    *
    */
   void xwave_get_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double period, double amplitude, unsigned long len);

   /** \brief Сгенерировать паузу
    *
    * \param data данные для сохранения
    * \param sample_rate Частота дискретизации
    * \param bits_per_sample Количество бит в сэмпле
    * \param длина массива data
    *
    */
   void xwave_get_pause_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, unsigned long len);

   /** \brief Сгенерировать несколько меандров одновременно и записать в массив
    *
    * \param data данные для сохранения
    * \param sample_rate Частота дискретизации
    * \param bits_per_sample Количество бит в сэмпле
    * \param period массив периодов меандра
    * \param amplitude массив амплитуд сигнала (от 0 до 1.0)
    * \param num_impulses количество сигналов в массиве (размер массива period или amplitude)
    * \param длина массива data
    * \param bDamped - затухание меандра
    */
   void xwave_get_multiple_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double* period, double* amplitude, unsigned short num_impulses, unsigned long len, bool bDamped);

   /** \brief Сгенерировать несколько затухающих меандров и записать в массив
    *
    * \param data данные для сохранения
    * \param sample_rate Частота дискретизации
    * \param bits_per_sample Количество бит в сэмпле
    * \param period массив периодов меандра
    * \param amplitude массив амплитуд сигнала (от 0 до 1.0)
    * \param num_impulses количество сигналов в массиве (размер массива period или amplitude)
    * \param длина массива data
    *
    */
  // void xwave_get_multiple_damped_impulses_mono(void* data, unsigned long sample_rate, unsigned short bits_per_sample, double* period, double* amplitude, unsigned short num_impulses, unsigned long len);

#ifdef __cplusplus
}
#endif

#endif // XWAVE_H_INCLUDED
