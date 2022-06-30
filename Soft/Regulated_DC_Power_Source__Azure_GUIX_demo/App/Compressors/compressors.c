#include "App.h"
#include "compress.h"
#include "compress_io.h"
#include "Lzss.h"
#include "Sixpack.h"



/*-------------------------------------------------------------------------------------------------------------
   Сжатие входного файла в выходной файл

   Структура блочно сжатых данных

   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]
   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]
-------------------------------------------------------------------------------------------------------------*/
uint32_t Compress_file_to_file(int alg, char *infname, char *outfname)
{
  uint32_t      status = RES_OK;

  uint8_t      *ptr1;
  uint8_t      *ptr2;
  FX_FILE      infid;
  FX_FILE      outfid;
  uint32_t     fsize;
  uint32_t     sz;
  int32_t      res;
  ULONG        actual_size;

  compress_data_t inFP;
  compress_data_t outFP;


  ptr1 = App_malloc_pending((MAX_COMPRESSIBLE_BLOCK_SIZE * 2)+SPARE_AREA_SIZE, 10);
  if (ptr1 == NULL)
  {
    APPLOG("Malloc error");
    return RES_ERROR;
  }
  ptr2 =&ptr1[MAX_COMPRESSIBLE_BLOCK_SIZE+SPARE_AREA_SIZE];

  res = fx_file_open(&fat_fs_media,&infid, infname,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS)
  {
    App_free(ptr1);
    return RES_ERROR;
  }

  res =  Recreate_file_for_write(&outfid, outfname);
  if (res != FX_SUCCESS)
  {
    fx_file_close(&infid);
    App_free(ptr1);
    APPLOG("Error %d", res);
    return RES_ERROR;
  }

  fsize = infid.fx_file_current_file_size;

  while (fsize > 0)
  {
    if (fsize > MAX_COMPRESSIBLE_BLOCK_SIZE)
    {
      sz = MAX_COMPRESSIBLE_BLOCK_SIZE;
    }
    else
    {
      sz = fsize;
    }

    res= fx_file_read(&infid,(void *)ptr2,sz,&actual_size);
    if (res != FX_SUCCESS)
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
      break;
    }

    inFP.eType            = COMPRESS_STRING_TYPE;
    inFP.pData.str.nPos   = 0;
    inFP.pData.str.pData  = ptr2;
    inFP.pData.str.nLen   = sz;

    outFP.eType           = COMPRESS_STRING_TYPE;
    outFP.pData.str.nPos  = 0;
    outFP.pData.str.pData = ptr1;
    outFP.pData.str.nLen  = sz + SPARE_AREA_SIZE;

    if (alg == SIXPACK_ALG)
    {
      res = Compress_sixp(&inFP,&outFP);
    }
    else
    {
      res = Compress_lzss(&inFP,&outFP);
    }

    if (res < 0 )
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
      break;
    }

    // Записываем размер сжатого блока
    res = fx_file_write(&outfid,(void *)&outFP.pData.str.nPos, sizeof(outFP.pData.str.nPos));
    if (res != FX_SUCCESS)
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
      break;
    }

    // Записываем сжатые данные
    res = fx_file_write(&outfid,(void *)outFP.pData.str.pData,outFP.pData.str.nPos);
    if (res != FX_SUCCESS)
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
      break;
    }

    fsize -= sz;
  }

  fx_file_close(&infid);
  fx_file_close(&outfid);
  App_free(ptr1);
  return status;


}


/*-----------------------------------------------------------------------------------------------------
   Сжатие входного файла в выходной файл

   Структура блочно сжатых данных

   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]
   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]


  \param alg
  \param infid
  \param outfid

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Compress_file_to_file_by_handler(int alg, FX_FILE *infid, FX_FILE *outfid)
{
  uint32_t      status;

  uint8_t      *ptr1;
  uint8_t      *ptr2;
  uint32_t     fsize;
  uint32_t     sz;
  int32_t      res;
  ULONG        actual_size;

  compress_data_t inFP;
  compress_data_t outFP;

  ptr1 = App_malloc_pending((MAX_COMPRESSIBLE_BLOCK_SIZE * 2)+SPARE_AREA_SIZE, 10);
  if (ptr1 == NULL)
  {
    APPLOG("Malloc error");
    return RES_ERROR;
  }
  ptr2 =&ptr1[MAX_COMPRESSIBLE_BLOCK_SIZE+SPARE_AREA_SIZE];

  fsize = infid->fx_file_current_file_size;  // Читаем размер входного файла

  while (fsize > 0)
  {
    if (fsize > MAX_COMPRESSIBLE_BLOCK_SIZE)
    {
      sz = MAX_COMPRESSIBLE_BLOCK_SIZE;
    }
    else
    {
      sz = fsize;
    }

    res= fx_file_read(infid,(void *)ptr2,sz,&actual_size);
    if (res != FX_SUCCESS)
    {
      App_free(ptr1);
      APPLOG("Error %d", res);
      return RES_ERROR;
    }

    inFP.eType            = COMPRESS_STRING_TYPE;
    inFP.pData.str.nPos   = 0;
    inFP.pData.str.pData  = ptr2;
    inFP.pData.str.nLen   = sz;

    outFP.eType           = COMPRESS_STRING_TYPE;
    outFP.pData.str.nPos  = 0;
    outFP.pData.str.pData = ptr1;
    outFP.pData.str.nLen  = sz+SPARE_AREA_SIZE;

    if (alg == SIXPACK_ALG)
    {
      res = Compress_sixp(&inFP,&outFP);
    }
    else
    {
      res = Compress_lzss(&inFP,&outFP);
    }

    if (res > 0)
    {
      status = RES_OK;
    }
    else
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
    }

    // Записываем размер сжатого блока
    res = fx_file_write(outfid,(void *)&outFP.pData.str.nPos, sizeof(outFP.pData.str.nPos));
    if (res != FX_SUCCESS)
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
    }

    // Записываем сжатые данные
    res = fx_file_write(outfid,(void *)outFP.pData.str.pData,outFP.pData.str.nPos);
    if (res != FX_SUCCESS)
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
    }

    fsize -= sz;
  }

  res = fx_file_seek(outfid, 0);
  if (res != FX_SUCCESS)
  {
    status = RES_ERROR;
    APPLOG("Error %d", res);
  }
  App_free(ptr1);

  fx_media_flush(&fat_fs_media);
  return status;

}

/*-----------------------------------------------------------------------------------------------------


  \param alg
  \param infid
  \param outfid

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Compress_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int *out_buf_sz)
{
  uint32_t      status;

  int32_t      res;

  compress_data_t inFP;
  compress_data_t outFP;


  inFP.eType            = COMPRESS_STRING_TYPE;
  inFP.pData.str.nPos   = 0;
  inFP.pData.str.pData  = in_buf;
  inFP.pData.str.nLen   = in_buf_sz;

  outFP.eType           = COMPRESS_STRING_TYPE;
  outFP.pData.str.nPos  = 0;
  outFP.pData.str.pData = out_buf;
  outFP.pData.str.nLen  =*out_buf_sz;

  if (alg == SIXPACK_ALG)
  {
    res = Compress_sixp(&inFP,&outFP);
  }
  else
  {
    res = Compress_lzss(&inFP,&outFP);
  }

  if (res > 0)
  {
    *out_buf_sz = res;
    status = RES_OK;
  }
  else
  {
    *out_buf_sz = 0;
    status = RES_ERROR;
  }
  return status;
}


/*-------------------------------------------------------------------------------------------------------------
   Распаковка сжатого файла в файл с распакованными данными

   Структура блочно сжатых данных

   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]
   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]

-------------------------------------------------------------------------------------------------------------*/
uint32_t Decompress_file_to_file(int alg, char *infname, char *outfname)
{
  uint32_t      status = RES_ERROR;
  int32_t       res;
  uint8_t      *ptr1;
  uint8_t      *ptr2;
  FX_FILE       infid;
  FX_FILE       outfid;
  uint32_t      fsize;
  uint32_t      sz_compr;
  uint32_t      sz_decompr;
  ULONG         actual_size;
  compress_data_t inFP;
  compress_data_t outFP;


  ptr1 = App_malloc_pending((MAX_COMPRESSIBLE_BLOCK_SIZE * 2)+SPARE_AREA_SIZE, 10);
  if (ptr1 == NULL)
  {
    APPLOG("Malloc error");
    return RES_ERROR;
  }
  ptr2 =&ptr1[MAX_COMPRESSIBLE_BLOCK_SIZE];

  // Открытие входного сжатия
  res = fx_file_open(&fat_fs_media,&infid, infname,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) return RES_ERROR;

  // Открытие или создание выходного файла
  res =  Recreate_file_for_write(&outfid, outfname);
  if (res != FX_SUCCESS)
  {
    fx_file_close(&infid);
    APPLOG("Error %d", res);
    return RES_ERROR;
  }

  // Читаем размер входного файла
  fsize = infid.fx_file_current_file_size;

  while (fsize > 0)
  {
    // Читаем размер сжатого блока
    res= fx_file_read(&infid,(void *)&sz_compr,sizeof(uint32_t),&actual_size);
    if (res != FX_SUCCESS)
    {
      fx_file_close(&infid);
      fx_file_close(&outfid);
      App_free(ptr1);
      APPLOG("Error %d", res);
      return RES_ERROR;
    }


    // Читаем сжатые данные
    res= fx_file_read(&infid,(void *)ptr1,sz_compr,&actual_size);
    if (res != FX_SUCCESS)
    {
      fx_file_close(&infid);
      fx_file_close(&outfid);
      App_free(ptr1);
      APPLOG("Error %d", res);
      return RES_ERROR;
    }


    inFP.eType            = COMPRESS_STRING_TYPE;
    inFP.pData.str.nPos   = 0;
    inFP.pData.str.pData  = ptr1;
    inFP.pData.str.nLen   = sz_compr;

    outFP.eType           = COMPRESS_STRING_TYPE;
    outFP.pData.str.nPos  = 0;
    outFP.pData.str.pData = ptr2;
    outFP.pData.str.nLen  = MAX_COMPRESSIBLE_BLOCK_SIZE;

    if (alg == SIXPACK_ALG)
    {
      sz_decompr = Uncompress_sixp(&inFP,&outFP);
    }
    else
    {
      sz_decompr = Uncompress_lzss(&inFP,&outFP);
    }

    if (sz_decompr > 0)
    {
      status = RES_OK;
    }
    else
    {
      status = RES_ERROR;
      APPLOG("Error. Size=%d ", sz_decompr);
    }

    res = fx_file_write(&outfid,(void *)outFP.pData.str.pData,outFP.pData.str.nPos);
    if (res != FX_SUCCESS)
    {
      status = RES_ERROR;
      APPLOG("Error %d", res);
    }

    fsize -=(sz_compr + sizeof(uint32_t));
  }

  fx_file_close(&infid);
  fx_file_close(&outfid);
  App_free(ptr1);
  return status;
}

/*-------------------------------------------------------------------------------------------------------------
   Распаковка сжатого файла в память

   Структура блочно сжатых данных

   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]
   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]


-------------------------------------------------------------------------------------------------------------*/
uint32_t Decompress_file_to_mem(int alg, char *input_file_name, void *out_buf, unsigned int out_buf_sz)
{
  FX_FILE        *input_file;
  uint32_t        status = RES_ERROR;
  int32_t         res;
  uint8_t        *compressed_data_buf;
  uint32_t        compressed_data_size;
  compress_data_t inFP;
  compress_data_t outFP;
  ULONG           actual_size;

  // Выделяем память для файловой структуры
  input_file = App_malloc_pending(sizeof(FX_FILE), 10);
  if (input_file == NULL) goto EXIT_ON_ERROR;


  res = fx_file_open(&fat_fs_media, input_file, input_file_name,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;

  // Читаем размер сжатого блока
  res= fx_file_read(input_file,(void *)&compressed_data_size,sizeof(uint32_t),&actual_size);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  if (compressed_data_size > MAX_COMPRESSIBLE_BLOCK_SIZE) goto EXIT_ON_ERROR;


  // Выделяем память для файла с сжатыми данными
  compressed_data_buf = App_malloc_pending(compressed_data_size, 10);
  if (compressed_data_buf == NULL) goto EXIT_ON_ERROR;


  res= fx_file_read(input_file,(void *)compressed_data_buf,compressed_data_size,&actual_size);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;


  inFP.eType            = COMPRESS_STRING_TYPE;
  inFP.pData.str.nPos   = 0;
  inFP.pData.str.pData  = compressed_data_buf;
  inFP.pData.str.nLen   = compressed_data_size;

  outFP.eType           = COMPRESS_STRING_TYPE;
  outFP.pData.str.nPos  = 0;
  outFP.pData.str.pData = out_buf;
  outFP.pData.str.nLen  = out_buf_sz;

  if (alg == SIXPACK_ALG)
  {
    res = Uncompress_sixp(&inFP,&outFP);
  }
  else
  {
    res = Uncompress_lzss(&inFP,&outFP);
  }

  if (res > 0)
  {
    status = RES_OK;
  }
  else goto EXIT_ON_ERROR;

  fx_file_close(input_file);
  App_free(compressed_data_buf);
  App_free(input_file);
  return status;

EXIT_ON_ERROR:

  if (input_file != 0)
  {
    if (input_file->fx_file_id == FX_FILE_ID)
    {
      fx_file_close(input_file);
    }
    App_free(input_file);
  }

  App_free(compressed_data_buf);
  APPLOG("Error %d", res);

  return RES_ERROR;

}

/*-------------------------------------------------------------------------------------------------------------
   Распаковка сжатых данных при передаче по каналу MQTT из в памяти  в память

   Структура блока сжатых данных

   [NNNN] - 4-е байта длины следующего за этим блока сжатых данных
   [....] - блок сжатых данных
   .
   .
   .
   [....]


-------------------------------------------------------------------------------------------------------------*/
int32_t Decompress_mqtt_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int out_buf_sz)
{
  compress_data_t inFP;
  compress_data_t outFP;

  uint32_t        compressed_sz;
  uint8_t         *ptr;

  signed int      res;

  ptr = in_buf;
  memcpy(&compressed_sz, ptr, sizeof(uint32_t));
  ptr += sizeof(uint32_t);

  inFP.eType            = COMPRESS_STRING_TYPE;
  inFP.pData.str.nPos   = 0;
  inFP.pData.str.pData  = ptr;
  inFP.pData.str.nLen   = compressed_sz;

  outFP.eType           = COMPRESS_STRING_TYPE;
  outFP.pData.str.nPos  = 0;
  outFP.pData.str.pData = out_buf;
  outFP.pData.str.nLen  = out_buf_sz;


  if (alg == SIXPACK_ALG)
  {
    res = Uncompress_sixp(&inFP,&outFP);
  }
  else
  {
    res = Uncompress_lzss(&inFP,&outFP);
  }


  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Распаковка обычного формата из памяти в память

  \param alg
  \param in_buf
  \param in_buf_sz
  \param out_buf
  \param out_buf_sz

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Decompress_mem_to_mem(uint8_t alg, void *in_buf, unsigned int in_buf_sz, void *out_buf, unsigned int out_buf_sz)
{
  compress_data_t inFP;
  compress_data_t outFP;

  signed int      res;

  inFP.eType            = COMPRESS_STRING_TYPE;
  inFP.pData.str.nPos   = 0;
  inFP.pData.str.pData  = in_buf;
  inFP.pData.str.nLen   = in_buf_sz;

  outFP.eType           = COMPRESS_STRING_TYPE;
  outFP.pData.str.nPos  = 0;
  outFP.pData.str.pData = out_buf;
  outFP.pData.str.nLen  = out_buf_sz;


  if (alg == SIXPACK_ALG)
  {
    res = Uncompress_sixp(&inFP,&outFP);
  }
  else
  {
    res = Uncompress_lzss(&inFP,&outFP);
  }


  return res;
}

