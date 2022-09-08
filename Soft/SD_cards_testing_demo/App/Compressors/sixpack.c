#include "compress.h"
#include "compress_io.h"
#include "Sixpack.h"
/* 
 *  Sixpack.h
 *
 *  Header File for the LZSS algorithm (implemented using a Binary Tree).
 *
 *
 *  Description:
 *
 *      See Sixpack.h for a description of the Sixpack Compression utility.
 *
 *
 *      Besides, some slight "C" changes, I added code to place a header on
 *      the file, which contains the size, date, and some other useful info
 *      THAT must not be compressed in final image.
 */

#define NIL   -1              /* End of linked list marker    */

static int          *sp_copymin = 0;
static int          *sp_copymax = 0;

static short        *sp_left    = 0;
static short        *sp_right   = 0;
static short        *sp_up      = 0;
static short        *sp_freq    = 0;

static short        *sp_head    = 0;
static short        *sp_tail    = 0;               /* Hash table                   */
static short        *sp_succ    = 0;
static short        *sp_pred    = 0;               /* Doubly linked lists          */

static unsigned char *sx_buf;        /* Text sx_buf                  */

int    maxdistance;
int    maxsize;
int    g_dist;
int    insert   = MINCOPY;
int    dictfile = 0;
int    binary   = 0;

short  copybits[COPYRANGES] ={ 4, 6, 8, 10, 12}; //, 14  };            // Distance bits

int    input_bit_count;                 /* Input bits sx_bufed          */
int    input_bit_buffer;                /* Input sx_buf                 */
int    output_bit_count;                /* Output bits sx_bufed         */
int    output_bit_buffer;               /* Output sx_buf                */
long   bytes_in;
long   bytes_out;


/* --------------------------------------------------------------------------
 *
 *  initialize
 *
 *      Adaptive Huffman frequency compression
 *
 *          Data structure based partly on "Application of Splay Trees
 *          to Data Compression", Communications of the ACM 8/88
 *
 *      Initialize data for compression or decompression.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing
 *
 *  Notes:
 *      None.
 *
 * --------------------------------------------------------------------------
 */

static void InitializeHuffman(void)
{
  int i, j;


  sp_left    = (short *)cio_malloc((MAXCHAR+1) * sizeof(short));
  sp_right   = (short *)cio_malloc((MAXCHAR+1) * sizeof(short));
  sp_up      = (short *)cio_malloc((TWICEMAX+1) * sizeof(short));
  sp_freq    = (short *)cio_malloc((TWICEMAX+1) * sizeof(short));
  sp_copymin = (int *)cio_malloc(COPYRANGES * sizeof(int));
  sp_copymax = (int *)cio_malloc(COPYRANGES * sizeof(int));


  /*
   *  Reset the Counters
   */

  input_bit_count     = 0;
  input_bit_buffer    = 0;
  output_bit_count    = 0;
  output_bit_buffer   = 0;
  bytes_in            = 0;
  bytes_out           = 0;

  /*
   *  Miscellaneous reset
   */

  insert   = MINCOPY;
  dictfile = 0;
  binary   = 0;

  /*
   *  Initialize Huffman frequency tree
   */

  for (i = 2; i <= TWICEMAX; i++)
  {
    sp_up[i] = i / 2;
    sp_freq[i] = 1;
  }
  for (i = 1; i <= MAXCHAR; i++)
  {
    sp_left[i] = 2 * i;
    sp_right[i] = 2 * i+1;
  }

  /* Initialize copy distance ranges */
  j = 0;
  for (i = 0; i < COPYRANGES; i++)
  {
    sp_copymin[i] = j;
    j += 1 << copybits[i];
    sp_copymax[i] = j - 1;
  }
  maxdistance = j - 1;
  maxsize = maxdistance + MAXCOPY;
}


/*-----------------------------------------------------------------------------------------------------
  
  
  \param void  
-----------------------------------------------------------------------------------------------------*/
static void UnInitializeHuffman(void)
{
  if (sp_left      != NULL) cio_free(sp_left   );
  if (sp_right     != NULL) cio_free(sp_right  );
  if (sp_up        != NULL) cio_free(sp_up     );
  if (sp_freq      != NULL) cio_free(sp_freq   );
  if (sp_copymin   != NULL) cio_free(sp_copymin);
  if (sp_copymax   != NULL) cio_free(sp_copymax);
}

/* --------------------------------------------------------------------------
 *
 *  UpdateFreq
 *
 *      Update frequency counts from leaf to root
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing
 *
 *  Notes:
 *      None.
 *
 * --------------------------------------------------------------------------
 */

static void UpdateFreq(int a, int b)
{
  do
  {
    sp_freq[sp_up[a]] = sp_freq[a] + sp_freq[b];
    a = sp_up[a];
    if (a != ROOT)
    {
      if (sp_left[sp_up[a]] == a) b = sp_right[sp_up[a]];
      else b = sp_left[sp_up[a]];
    }
  }while (a != ROOT);

  // Periodically scale frequencies down by half to avoid overflow
  // This also provides some local adaption and better compression
  if (sp_freq[ROOT] == MAXFREQ)
  {
    for (a = 1; a <= TWICEMAX; a++) sp_freq[a] >>= 1;
  }
}

/* --------------------------------------------------------------------------
 *
 *  UpdateModel
 *
 *  Description:
//      Update Huffman model for each character code
//
//  Cautions:
//      None
//
 *  Notes:
 *      None.
 *
 * --------------------------------------------------------------------------
 */

static void UpdateModel(int code)
{
  int a, b, c, ua, uua;

  a = code + SUCCMAX;
  ++sp_freq[a];
  if (sp_up[a] != ROOT)
  {
    ua = sp_up[a];
    if (sp_left[ua] == a) UpdateFreq(a,sp_right[ua]);
    else UpdateFreq(a,sp_left[ua]);
    do
    {
      uua = sp_up[ua];
      if (sp_left[uua] == ua) b = sp_right[uua];
      else b = sp_left[uua];

      // If high freq lower in tree, swap nodes
      if (sp_freq[a] > sp_freq[b])
      {
        if (sp_left[uua] == ua) sp_right[uua] = a;
        else sp_left[uua] = a;

        if (sp_left[ua] == a)
        {
          sp_left[ua] = b;
          c = sp_right[ua];
        }
        else
        {
          sp_right[ua] = b;
          c = sp_left[ua];
        }
        sp_up[b] = ua;
        sp_up[a] = uua;
        UpdateFreq(b,c);
        a = b;
      }
      a = sp_up[a];
      ua = sp_up[a];
    }while (ua != ROOT);
  }
}

//-----------------------------------------------------------------------------
//  Function:
//      AddNode
//
//  Description:
//      Add node to head of the Hash Table list
//
//  Cautions:
//      None
//
//  Functions Called:
//      None
//
//  Called By:
//
//-----------------------------------------------------------------------------

static void AddNode(int n)
{
  int key;

  key =((sx_buf[n] ^(sx_buf[(n+1)%maxsize]<<4)^(sx_buf[(n+2)%maxsize]<<8)) & HASHMASK);
  if (sp_head[key] == NIL)
  {
    sp_tail[key] = n;
    sp_succ[n] = NIL;
  }
  else
  {
    sp_succ[n] = sp_head[key];
    sp_pred[sp_head[key]] = n;
  }
  sp_head[key] = n;
  sp_pred[n] = NIL;
}

//-----------------------------------------------------------------------------
//  Function:
//      DeleteNode
//
//  Description:
//      Delete node from tail of list
//
//  Cautions:
//      None
//
//  Functions Called:
//      None
//
//  Called By:
//
//-----------------------------------------------------------------------------

void DeleteNode(int n)
{
  int key;

  key =((sx_buf[n] ^(sx_buf[(n+1)%maxsize]<<4)^(sx_buf[(n+2)%maxsize]<<8)) & HASHMASK);
  if (sp_head[key] == sp_tail[key]) sp_head[key] = NIL;
  else
  {
    sp_succ[sp_pred[sp_tail[key]]] = NIL;
    sp_tail[key] = sp_pred[sp_tail[key]];
  }
}

//-----------------------------------------------------------------------------
//  Function:
//      flush_bits
//
//  Description:
//      ???
//
//  Cautions:
//      Converted to a macro to improve speed somewhat...
//
//  Functions Called:
//      putc
//
//  Called By:
//
//-----------------------------------------------------------------------------

static void flush_bits(compress_data_t *pOutput)
{
  if (output_bit_count > 0)
  {
    cio_Put_char(pOutput,(output_bit_buffer <<(8-output_bit_count)));
    bytes_out++;
  }
}
//-----------------------------------------------------------------------------
//  Function:
//      input_bit
//
//  Description:
//      Reads an bit from input file
//
//  Cautions:
//      None
//
//  Functions Called:
//      None
//
//  Called By:
//
//-----------------------------------------------------------------------------

int input_bit(compress_data_t *pInput)
{
  int bit;

  if (input_bit_count-- == 0)
  {
    input_bit_buffer = cio_Get_char(pInput);
    ++bytes_in;
    input_bit_count = 7;
  }

  bit =((input_bit_buffer & 0x80) != 0);
  input_bit_buffer <<= 1;

  return (bit);
}

//-----------------------------------------------------------------------------
//  Function:
//      output_bit
//
//  Description:
//      Writes one bit to output file
//
//  Cautions:
//      None
//
//  Functions Called:
//      putc
//
//  Called By:
//
//-----------------------------------------------------------------------------

static void output_bit(compress_data_t *pOutput, int bit)
{
  output_bit_buffer <<= 1;

  if (bit) output_bit_buffer |= 1;

  if (++output_bit_count == 8)
  {
    cio_Put_char(pOutput,output_bit_buffer);
    output_bit_count = 0;
    ++bytes_out;
  }
}

//-----------------------------------------------------------------------------
//  Function:
//      input_code
//
//  Description:
//      Reads Multibit code from Input File
//
//  Cautions:
//      None
//
//  Functions Called:
//      input_bit
//
//  Called By:
//
//-----------------------------------------------------------------------------

int input_code(compress_data_t *pInput, int bits)
{
  int i, bit = 1, code = 0;

  for (i = 0; i < bits; i++)
  {
    if (input_bit(pInput)) code |= bit;
    bit <<= 1;
  }

  return (code);
}

//-----------------------------------------------------------------------------
//  Function:
//      output_code
//
//  Description:
//      Writes multibit code to output file
//
//  Cautions:
//      None
//
//  Functions Called:
//      output_bit
//
//  Called By:
//
//-----------------------------------------------------------------------------

static void output_code(compress_data_t *pOutput, int code, int bits)
{
  int i;

  for (i = 0; i < bits; i++)
  {
    output_bit(pOutput, code & 0x0001);
    code >>= 1;
  }
}

//-----------------------------------------------------------------------------
//  Function:
//      char_uncompress( unsigned char* )
//
//  Description:
//      Uncompress a character code from input stream
//
//  Cautions:
//      None
//
//  Functions Called:
//      input_bit, UpdateModel
//
//  Called By:
//
//-----------------------------------------------------------------------------

int char_uncompress(compress_data_t *pInput)
{
  int a = ROOT;

  do
  {
    if (input_bit(pInput)) a = sp_right[a];
    else a = sp_left[a];
  } while (a <= MAXCHAR);
  a = a - SUCCMAX;
  UpdateModel(a);
  return (a);
}

//-----------------------------------------------------------------------------
//  Function:
//      compress
//
//  Description:
//      Compress a character code to output stream
//
//  Cautions:
//      None
//
//  Functions Called:
//      output_bit, UpdateModel
//
//  Called By:
//
//-----------------------------------------------------------------------------

static void char_compress(compress_data_t *pOutput, int code)
{
  int a, sp = 0;
  int stk[50];

  a = code + SUCCMAX;
  do
  {
    stk[sp++] =(sp_right[sp_up[a]] == a);
    a = sp_up[a];
  } while (a != ROOT);

  do
  {
    output_bit(pOutput,stk[--sp]);
  } while (sp);

  UpdateModel(code);
}

//-----------------------------------------------------------------------------
//  Function:
//      match
//
//  Description:
//      Find longest string matching lookahead sx_buf string
//
//  Cautions:
//      None
//
//  Functions Called:
//      getkey
//
//  Called By:
//
//-----------------------------------------------------------------------------

int match(int n, int depth)
{
  int i, j, index, key, dst, len, best = 0, cnt = 0;

  if (n == maxsize) n = 0;
  key =((sx_buf[n] ^(sx_buf[(n+1)%maxsize]<<4)^(sx_buf[(n+2)%maxsize]<<8)) & HASHMASK);
  index = sp_head[key];

  while (index != NIL)
  {
    // Did we exceed depth? (Quit)

    if (++cnt > depth) break;

    if (sx_buf[(n+best)%maxsize] == sx_buf[(index+best)%maxsize])
    {
      len = 0;
      i = n;
      j = index;

      while ((sx_buf[i] == sx_buf[j]) && (len < MAXCOPY) && (j != n) && (i != insert))
      {
        ++len;
        if (++i == maxsize) i = 0;
        if (++j == maxsize) j = 0;
      }

      dst = n - index;
      if (dst < 0) dst += maxsize;
      dst -= len;

      // If dict file, quit at shortest distance range

      if (dictfile && dst > sp_copymax[0]) break;

      // Update best match

      if ((len > best) && (dst <= maxdistance))
      {
        if ((len > MINCOPY) || (dst <= sp_copymax[SHORTRANGE+binary]))
        {
          best = len;
          g_dist = dst;
        }
      }
    }
    index = sp_succ[index];
  }

  return (best);
}

//-----------------------------------------------------------------------------
//  Finite Window compression routines

#define IDLE    0      // Not processing a copy
#define COPY    1      // Currently processing copy

//-----------------------------------------------------------------------------
//  Function:
//      void dictionary( void )
//
//  Description:
//      Check first sx_buf for ordered dictionary file. Better compression
//      using short distance copies.
//
//  Cautions:
//      None
//
//  Functions Called:
//
//
//  Called By:
//
//-----------------------------------------------------------------------------

void dictionary(void)
{
  int i = 0, j = 0, k, cnt = 0;

  // Count matching chars at start of adjacent lines
  while (++j < (MINCOPY+MAXCOPY))
  {
    if (sx_buf[j-1] == 10)
    {
      k = j;
      while (sx_buf[i++] == sx_buf[k++]) ++cnt;
      i = j;
    }
  }
  // If matching line prefixes > 25% assume dictionary
  if (cnt > ((MINCOPY+MAXCOPY) / 4)) dictfile = 1;
}

//-----------------------------------------------------------------------------
//  Function:
//      encode( FILE*, FILE* )
//
//  Description:
//      Encode file from input to output
//
//  Cautions:
//      None
//
//  Functions Called:
//      malloc, fseek, fputc, compress, flush_bits, getc, DeleteNode,
//      AddNode, match, output_code
//
//  Called By:
//
//-----------------------------------------------------------------------------

T_compress_size Compress_sixp(compress_data_t *pInput, compress_data_t *pOutput)
{
  int c, i, n=MINCOPY, addpos=0, full=0, state=IDLE, nextlen;
  T_compress_size len;

  InitializeHuffman();

  sp_head = cio_malloc((unsigned long)HASHSIZE * sizeof(short));
  sp_tail = cio_malloc((unsigned long)HASHSIZE * sizeof(short));
  sp_succ = cio_malloc((unsigned long)maxsize * sizeof(short));
  sp_pred = cio_malloc((unsigned long)maxsize * sizeof(short));

  sx_buf = (unsigned char *) cio_malloc(maxsize * sizeof(unsigned char));


  /* 
   * Verify Memory allocation
   */

  if ((sp_head == NULL) || (sp_tail == NULL) || (sp_succ == NULL) || (sp_pred == NULL) || (sx_buf == NULL))
  {
    bytes_out = COMPRESSION_MEMORY;
    goto _exit;
  }

  cio_Rewind(pInput);
  cio_Rewind(pOutput);

  /*
   *  Validate Length
   */

  len = cio_Get_len(pInput);
  if (len == 0)  goto _exit;

  /*
   *  The first FOUR bytes are the file's original size (typically, used
   *  for preallocation of sx_bufs).
   *
   *  This is optional and can be removed.
   */

  for (i = 0; i < 4; ++i)
  {
    cio_Put_char(pOutput,(unsigned char)(len >> i * 8));
  }
  bytes_out = 4;
  // Initialize hash table to empty

  for (i = 0; i < HASHSIZE; i++)
  {
    sp_head[i] = NIL;
  }
  len = 0;

  // OK, now compress first few characters using Pure Huffman

  for (i = 0; i < MINCOPY; i++)
  {
    if ((c = cio_Get_char(pInput)) == EOF)
    {
      char_compress(pOutput,TERMINATE);
      flush_bits(pOutput);

      bytes_out = COMPRESSION_FAIL;
      break;
    }
    char_compress(pOutput,c);
    ++bytes_in;
    sx_buf[i] = c;
  }

  if (bytes_out == COMPRESSION_FAIL)	goto _exit;

  // Preload next few characters into lookahead sx_buf (test for binary file)

  for (i = 0; i < MAXCOPY; i++)
  {
    if ((c = cio_Get_char(pInput)) == EOF) break;
    sx_buf[insert++] = c;
    ++bytes_in;
    if (c > 127) binary = 1;     /* Binary file ? */
  }

  // Check dictionary file

  dictionary();

  while (n != insert)
  {
    // Check compression to insure really a dictionary file
    if (dictfile && ((bytes_in % MAXCOPY) == 0))
    {
      if ((bytes_in / bytes_out) < 2)
      {
        dictfile = 0;               // Oops, not a dictionary file!
      }
    }

    // Update nodes in hash table lists

    if (full) DeleteNode(insert);

    AddNode(addpos);

    // Are we doing copy?
    if (state == COPY)
    {
      if (--len == 1) state = IDLE;
    }
    else // Check for new copy
    {
      // Get match length at next character and current char
      if (binary)
      {
        nextlen = match(n+1,BINNEXT);
        len     = match(n,BINSEARCH);
      }
      else
      {
        nextlen = match(n+1,TEXTNEXT);
        len     = match(n,TEXTSEARCH);
      }

      // If long enough and no better match at next char, start copy

      if ((len >= MINCOPY) && (len >= nextlen))
      {
        state = COPY;
        // Look up minimum bits to encode distance
        for (i = 0; i < COPYRANGES; i++)
        {
          if (g_dist <= sp_copymax[i])
          {
            char_compress(pOutput,FIRSTCODE-MINCOPY+len+i * CODESPERRANGE);
            output_code(pOutput,g_dist-sp_copymin[i],copybits[i]);
            break;
          }
        }
      }
      else // Else output single literal character
      {
        char_compress(pOutput,sx_buf[n]);
      }
    }

    // Advance sx_buf pointers

    if (++n == maxsize)      n      = 0;
    if (++addpos == maxsize) addpos = 0;

    // Add next input character to sx_buf

    if (c != EOF)
    {
      if ((c = cio_Get_char(pInput)) != EOF)
      {
        sx_buf[insert++] = c;
        ++bytes_in;
      }
      else
      {
        full = 0;
      }

      if (insert == maxsize)
      {
        insert = 0;
        full = 1;
      }
    }
  }

  // Output EOF code and free memory

  char_compress(pOutput, TERMINATE);
  flush_bits(pOutput);

_exit:

  //  Release Memory
  if (sp_head   != NULL) cio_free(sp_head);
  if (sp_tail   != NULL) cio_free(sp_tail);
  if (sp_succ   != NULL) cio_free(sp_succ);
  if (sp_pred   != NULL) cio_free(sp_pred);
  if (sx_buf != NULL) cio_free(sx_buf);

  UnInitializeHuffman();
  return (bytes_out);
}

//-----------------------------------------------------------------------------
//  Function:
//      decode( unsigned char*, unsigned char* )
//
//  Description:
//      Decode file from input to output
//
//  Cautions:
//      None
//
//  Functions Called:
//      malloc, uncompress, input_code, free
//
//  Called By:
//
//-----------------------------------------------------------------------------

T_compress_size Uncompress_sixp(compress_data_t *pInput, compress_data_t *pOutput)
{
  int c, n;
  int i, j, k;
  int dst, len, index;

  T_compress_size _nExpectLen = 0;

  InitializeHuffman();

  cio_Rewind(pInput);
  cio_Rewind(pOutput);

  n = 0;
  sx_buf = (unsigned char *)cio_malloc(maxsize);

  // Did we get the sx_buf???

  if (!sx_buf)
  {
    UnInitializeHuffman();
    return (COMPRESSION_MEMORY);
  }
  /*
   *  The first FOUR bytes are the file's original size (typically, used
   *  for preallocation of sx_bufs).
   *
   *  This is optional and can be removed.
   */

  for (i = 0; i < 4; ++i)
  {
    _nExpectLen |=(cio_Get_char(pInput)<< i * 8);
  }

  // OK, begin uncompression.

  while ((c = char_uncompress(pInput)) != TERMINATE)
  {
    // Single literal character?

    if (c < 256)
    {
      cio_Put_char(pOutput,c);
      bytes_out++;
      sx_buf[n++] = c;
      if (n == maxsize) n = 0;
    }
    else  // Else string copy length/distance codes
    {
      index =(c - FIRSTCODE) / CODESPERRANGE;
      len = c - FIRSTCODE + MINCOPY - index * CODESPERRANGE;
      dst = input_code(pInput, copybits[index])+ len + sp_copymin[index];
      j = n;
      k = n - dst;
      if (k < 0) k += maxsize;
      for (i = 0; i < len; i++)
      {
        cio_Put_char(pOutput,sx_buf[k]);
        bytes_out++;
        sx_buf[j++] = sx_buf[k++];
        if (j == maxsize) j = 0;
        if (k == maxsize) k = 0;
      }

      n += len;
      if (n >= maxsize) n -= maxsize;
    }
  }

  cio_free(sx_buf);

  /*
   *  Did we decompress successfully????
   */

  UnInitializeHuffman();

  if (_nExpectLen != cio_Get_pos(pOutput)) return (COMPRESSION_FAIL);

  return (_nExpectLen);
}
