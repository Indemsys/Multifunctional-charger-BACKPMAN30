#ifndef __COMPRESS_H
  #define __COMPRESS_H


  #include <stdlib.h>
  #include <stdio.h>
  #include <stdint.h>
  #include "tx_api.h"
  #include "fx_api.h"

/* --------------------------------------------------------------------------
 *  Common types within the Compression Library
 *
 *      pData   - Pointer to the Data Buffer
 *      nPos    - Current Read/Write Position in the Buffer
 *      nLen    - Length of the Buffer
 */

typedef long T_compress_size;

struct T_compress_string
{
  unsigned char* pData;
  T_compress_size nPos;
  T_compress_size nLen;
};

typedef struct T_compress_string compress_string_t;




typedef enum
{
  COMPRESS_FILE_TYPE,
  COMPRESS_STRING_TYPE

} T_DATA_TYPE;

/*
 *  Compress Object type with Discriminating Union
 *
 *      eType   - Type of Data
 *      pData   - Pointer to where Data is sourced
 */

struct T_compress_data
{
  T_DATA_TYPE  eType;
  union
  {
    FX_FILE  *file;
    compress_string_t str;
  } pData;
};

typedef struct T_compress_data compress_data_t;

/*
 *  Defining a Compression Data Type (can be string or file)
 */

  #define DECLARE_COMPRESS_DATA_STRING(nam)      compress_data_t nam = { COMPRESS_STRING_TYPE, 0, 0, 0 }
  #define DECLARE_COMPRESS_DATA_FILE(nam)        compress_data_t nam = { COMPRESS_FILE_TYPE, (FILE*)0 }
  #define INIT_COMPRESS_DATA_STRING(nam,ptr,len) nam##.pData.str.pData = ptr; if ( ptr != 0 ) nam##->pData.str.nLen = len
  //#define INIT_COMPRESS_DATA_FILE(nam,ptr)       nam##.pData.file = ptr
  //#define GET_COMPRESS_DATA_FILE(nam)            (FX_FILE*)nam##.pData.file
  #define GET_COMPRESS_DATA_STRING(nam)          (compress_string_t*)&(nam##.pData.str)

/*
 *  Error Codes
 */

  #define COMPRESSION_SUCCESS          0
  #define COMPRESSION_FAIL            -1
  #define COMPRESSION_MEMORY          -2



#define SIXPACK_ALG   0
#define LZSS_ALG      1

#define MAX_COMPRESSIBLE_BLOCK_SIZE       (1024*30ul)  // Максимальный размер сжимаемого блока данных.
                                                       // Если размер данных превышает эту величину то данные разбиваются на блоки размером равным или меньшим данной величине.
                                                       // Ограничение размера вызвано нехваткой памяти
#define SPARE_AREA_SIZE                   (1024)


uint32_t Compress_file_to_file(int alg, char* infname, char* outfname);
uint32_t Compress_file_to_file_by_handler(int alg, FX_FILE *infid, FX_FILE *outfid);
uint32_t Compress_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int *out_buf_sz);
uint32_t Decompress_file_to_file(int alg, char* infname, char* outfname);
uint32_t Decompress_file_to_mem(int alg, char *input_file_name, void *out_buf, unsigned int out_buf_sz);
int32_t  Decompress_mqtt_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int out_buf_sz);
int32_t  Decompress_mem_to_mem(uint8_t alg, void* in_buf, unsigned int in_buf_sz, void*  out_buf, unsigned int out_buf_sz );

T_compress_size Compress_lzss( compress_data_t* pInput, compress_data_t* pOutput );
T_compress_size Uncompress_lzss( compress_data_t* pInput, compress_data_t* pOutput );
T_compress_size Compress_sixp( compress_data_t* pInput, compress_data_t* pOutput );
T_compress_size Uncompress_sixp( compress_data_t* pInput, compress_data_t* pOutput );

#endif
