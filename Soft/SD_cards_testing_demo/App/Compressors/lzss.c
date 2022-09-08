#include "compress.h"
#include "compress_io.h"
#include "Lzss.h"

/* lzss.c
 *
 *  Limpel-Ziv-Storer-Szymanski Compression Algorithm
 *
 *  This program is a variation of LZSS which implements a binary tree
 *  which improves decompression time with very little affect on compression
 *  effeciency.
 */
  #define N               4096  /* Size of ring buffer              */
  #define F               18	  /* Upper limit for match_length     */

  #define THRESHOLD       2     /* Encode string into position and length if match_length is greater than this */
  #define NIL             N     /* Index for root of binary search  trees.  */


static int s_nMatchPosition;
static int s_nMatchLength;

/* 
 *  Binary Tree and Ring Buffer
 *
 *      Ring buffer is of size N with extra F-1 bytes to facilitate string 
 *      comparison
 */

static int* s_aryLeft;
static int* s_aryRight;
static int* s_aryRoot;

static unsigned char* s_aryRingBuffer;

#define BINARY_TREE_CLEANUP         0
#define BINARY_TREE_INIT            1


/* --------------------------------------------------------------------------
 *
 *  InitBinaryTree
 *
 *      Initializes the Binary Tree for holding strings
 *
 *      For i = 0 to N - 1, s_aryRight[i] and s_aryLeft[i] will be the right 
 *      and left children of node i.  These nodes need not be initialized. 
 *      Also, s_aryRoot[i] is the parent of node i.  These are initialized 
 *      to NIL (= N), which stands for 'not used.' For i = 0 to 255, 
 *      s_aryRight[N + i + 1] is the root of the tree for strings that begin
 *      with character i.  These are initialized to NIL.
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

static int InitBinaryTree( int bInit )
{
  int _nIdx;

  int _nRetval = COMPRESSION_SUCCESS;

  /*
   *  Cleanup?
   */

  if ( bInit == BINARY_TREE_CLEANUP )
  {
    if ( s_aryLeft != NULL )
    {
      cio_free( s_aryLeft );
    }
    if ( s_aryRight !=  NULL )
    {
      cio_free( s_aryRight );
    }
    if ( s_aryRoot != NULL )
    {
      cio_free( s_aryRoot );
    }

    return(_nRetval);
  }

  /*
   *  Initialize?
   */

  do
  {
    /*
     *  Allocate the Tree
     */

    s_aryLeft = (int *)cio_malloc( (N + 1) * sizeof(int) );
    s_aryRight = (int *)cio_malloc( (N + 257) * sizeof(int) );
    s_aryRoot = (int *)cio_malloc( (N + 1) * sizeof(int) );

    /*
     *  If any memory allocation failure, we have to abort
     */

    if ( s_aryLeft == NULL || s_aryRight == NULL || s_aryRoot == NULL )
    {
      _nRetval = COMPRESSION_FAIL;
      break;
    }

    /*
     *  The CRUX of initialization of the tree...
     */

    for ( _nIdx = N + 1; _nIdx <= N + 256; _nIdx++ )
    {
      s_aryRight[_nIdx] = NIL;
    }

    for ( _nIdx = 0; _nIdx < N; _nIdx++ )
    {
      s_aryRoot[_nIdx] = NIL;
    }

  }
  while ( 0 );

  return(_nRetval);

}

/* --------------------------------------------------------------------------
 *
 *  InsertNode
 *
 *      Initializes the Trees for holding strings
 *
 *      Inserts string of length F, s_aryRingBuffer[r..r+F-1], into one of the
 *      trees (s_aryRingBuffer[r]'th tree) and returns the longest-match position
 *      and length via the global variables s_nMatchPosition and s_nMatchLength.
 *      If s_nMatchLength = F, then removes the old node in favor of the new
 *      one, because the old one will be deleted sooner.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing
 *
 *  Notes:
 *      r plays double role, as tree node and position in buffer. 
 *
 * --------------------------------------------------------------------------
 */

static void InsertNode( int r )
{
  int  i, p, cmp;
  unsigned char* pKey;

  cmp = 1;  
  pKey = &s_aryRingBuffer[r];  
  p = N + 1 + pKey[0];

  /*
   *
   */

  s_aryRight[r] = s_aryLeft[r] = NIL;  
  s_nMatchLength = 0;

  /*
   *
   */

  while ( 1 )
  {
    if ( cmp >= 0 )
    {
      if ( s_aryRight[p] != NIL )
      {
        p = s_aryRight[p];
      }
      else
      {
        s_aryRight[p] = r;  
        s_aryRoot[r] = p;  
        return;  
      }
    }
    else
    {
      if ( s_aryLeft[p] != NIL )
      {
        p = s_aryLeft[p];
      }
      else
      {
        s_aryLeft[p] = r;  
        s_aryRoot[r] = p;  
        return;  
      }
    }

    for ( i = 1; i < F; i++ )
    {
      cmp = pKey[i] - s_aryRingBuffer[p + i];
      if ( cmp != 0 )
      {
        break;
      }
    }

    if ( i > s_nMatchLength )
    {
      s_nMatchPosition = p;
      s_nMatchLength = i;

      if ( s_nMatchLength >= F )
      {
        break;
      }
    }
  }

  s_aryRoot[r] = s_aryRoot[p];  
  s_aryLeft[r] = s_aryLeft[p];  
  s_aryRight[r] = s_aryRight[p];

  s_aryRoot[s_aryLeft[p]] = r;  
  s_aryRoot[s_aryRight[p]] = r;

  if ( s_aryRight[s_aryRoot[p]] == p )
  {
    s_aryRight[s_aryRoot[p]] = r;
  }
  else
  {
    s_aryLeft[s_aryRoot[p]] = r;
  }

  s_aryRoot[p] = NIL;

  return;
}

/* --------------------------------------------------------------------------
 *
 *  DeleteNode
 *
 *      Deletes node p from tree
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing
 *
 *  Notes:
 *      None 
 *
 * --------------------------------------------------------------------------
 */

static void DeleteNode( int p )
{
  int  q;

  /*
   *  Is it not in the tree?
   */

  if ( s_aryRoot[p] == NIL )
  {
    return;
  }

  /*
   *
   */

  if ( s_aryRight[p] == NIL )
  {
    q = s_aryLeft[p];
  }

  else if ( s_aryLeft[p] == NIL )
  {
    q = s_aryRight[p];
  }
  else
  {
    q = s_aryLeft[p];
    if ( s_aryRight[q] != NIL )
    {
      do 
      {
        q = s_aryRight[q];  
      } 
      while ( s_aryRight[q] != NIL );

      s_aryRight[s_aryRoot[q]] = s_aryLeft[q];  
      s_aryRoot[s_aryLeft[q]] = s_aryRoot[q];
      s_aryLeft[q] = s_aryLeft[p];  
      s_aryRoot[s_aryLeft[p]] = q;
    }

    s_aryRight[q] = s_aryRight[p];  
    s_aryRoot[s_aryRight[p]] = q;
  }
  s_aryRoot[q] = s_aryRoot[p];
  if ( s_aryRight[s_aryRoot[p]] == p )
  {
    s_aryRight[s_aryRoot[p]] = q;  
  }
  else
  {
    s_aryLeft[s_aryRoot[p]] = q;
  }

  /*
   *  Empty the node
   */

  s_aryRoot[p] = NIL;

  return;
}

/* --------------------------------------------------------------------------
 *
 *  Compress
 *
 *      API to compress data
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The size of the data compressed
 *
 *  Notes:
 *      Both the input and output must be opened (allocated) ahead of this
 *      call.
 *
 * --------------------------------------------------------------------------
 */

T_compress_size Compress_lzss(compress_data_t* pInput, compress_data_t* pOutput )
{
  int _nIdx;
  int _nLen;

  int r, s;
  int _ch;
  unsigned char _aryCodeBuf[17];
  unsigned char mask;

  int _nCodeBufIdx;
  int _nLastMatchLength;

  /*
   *  Allocate Memory on the Heap?
   */


  /*
   *  Initialize the Tree
   */

  if ( InitBinaryTree( BINARY_TREE_INIT ) != COMPRESSION_SUCCESS )
  {
    InitBinaryTree( BINARY_TREE_CLEANUP );
    return(COMPRESSION_MEMORY);
  }

  /*
   *  Allocate the Ring Buffer, if required
   */

  s_aryRingBuffer = (unsigned char*)cio_malloc(N + F - 1);

  if ( s_aryRingBuffer == NULL )
  {
    InitBinaryTree( BINARY_TREE_CLEANUP );
    return(COMPRESSION_MEMORY);
  }


  cio_Rewind(pInput);
  cio_Rewind(pOutput);

  /*
   *  Output Size
   */

  {
    T_compress_size _nSourceLen = cio_Get_len(pInput);

    /*
     *  Validate Length
     */

    if ( _nSourceLen == 0 )
    {
      return(0);
    }

    /*
     *  The first FOUR bytes are the file's original size (typically, used
     *  for preallocation of buffers).
     *
     *  This is optional and can be removed.
     */

    for ( _nIdx = 0; _nIdx < sizeof( T_compress_size ); ++_nIdx )
    {
      cio_Put_char( pOutput,(unsigned char)( _nSourceLen >> _nIdx * 8 ) );
    }
  }

  /* 
   *  code_buf[1..16] saves eight units of code, and code_buf[0] works as 
   *  eight flags, "1" representing that the unit is an unencoded letter 
   *  (1 byte), "0" a position-and-length pair (2 bytes).  Thus, eight 
   *  units require at most 16 bytes of code. 
   */

  _aryCodeBuf[0] = 0;  
  _nCodeBufIdx = mask = 1;
  s = 0;
  r = N - F;

  /* 
   * Clear the buffer with any character that will appear often. 
   */

  for ( _nIdx = s; _nIdx < r; _nIdx++ )
  {
    s_aryRingBuffer[_nIdx] = ' ';  
  }

  /* 
   *  Read F bytes into the last F bytes of the buffer 
   */

  for ( _nLen = 0; (_nLen < F) && ((_ch = cio_Get_char(pInput)) != EOF); _nLen++ )
  {
    s_aryRingBuffer[r + _nLen] = _ch;
  }

  /* 
   *  Insert the F strings, each of which begins with one or more 'space'
   *  characters.  Note the order in which these strings are inserted.  
   *  This way, degenerate trees will be less likely to occur. 
   */

  for ( _nIdx = 1; _nIdx <= F; _nIdx++ )
  {
    InsertNode(r - _nIdx);
  }

  /* 
   *  Finally, insert the whole string just read.  The global variables
   *  s_nMatchLength and s_nMatchPosition are set. 
   */

  InsertNode( r );  
  do 
  {
    /* 
     *  s_nMatchLength may be spuriously long near the end of text. 
     */

    if ( s_nMatchLength > _nLen )
    {
      s_nMatchLength = (int)_nLen;
    }

    /* 
     *  Not long enough match.  Send one byte.
     */

    if ( s_nMatchLength <= THRESHOLD )
    {
      s_nMatchLength = 1; 

      /* 
       * 'send one byte' flag  and send uncoded
       */

      _aryCodeBuf[0] |= mask;  
      _aryCodeBuf[_nCodeBufIdx++] = s_aryRingBuffer[r];
    }

    /*
     *  Send position and length pair.
     *
     *      Note: s_nMatchLength > THRESHOLD.
     */

    else
    {
      _aryCodeBuf[_nCodeBufIdx++] = (unsigned char) s_nMatchPosition;
      _aryCodeBuf[_nCodeBufIdx++] = (unsigned char)(((s_nMatchPosition >> 4) & 0xf0) | (s_nMatchLength - (THRESHOLD+1)));  
    }

    /* 
     *  Shift mask left one bit. 
     */

    mask <<= 1;
    if ( mask == 0 )
    {
      /* 
       *  Send at most 8 units of code together
       */

      for ( _nIdx = 0; _nIdx < _nCodeBufIdx; _nIdx++ )
      {
        cio_Put_char(pOutput,_aryCodeBuf[_nIdx]);
      }

      _aryCodeBuf[0] = 0;  
      _nCodeBufIdx = mask = 1;
    }

    _nLastMatchLength = s_nMatchLength;
    for ( _nIdx = 0; ( _nIdx < _nLastMatchLength ) && 
        ( _ch = cio_Get_char(pInput) ) != EOF; _nIdx++ )
    {
      /*
       *  Delete old strings and read new bytes
       */

      DeleteNode(s);
      s_aryRingBuffer[s] = _ch;

      /*
       *  If the position is near the end of buffer, extend the buffer 
       *  to make string comparison easier.
       */

      if ( s < ( F - 1 ) )
      {
        s_aryRingBuffer[s + N] = _ch;
      }

      /* 
       *  Since this is a ring buffer, increment the position modulo N.
       */

      s = (s + 1) & (N - 1);  
      r = (r + 1) & (N - 1);

      /* 
       *  Register the string in s_aryRingBuffer[r..r+F-1]
       */

      InsertNode(r);  
    }

    while ( _nIdx++ < _nLastMatchLength )
    {
      DeleteNode(s);

      /* 
       *  After the end of text, no need to read, but...
       */

      s = (s + 1) & (N - 1);  
      r = (r + 1) & (N - 1);

      /* 
       * ...buffer may not be empty. 
       */

      if ( --_nLen )
      {
        InsertNode(r);    
      }
    }

    /* 
     *  Continue until length of string to be processed is zero 
     */
  } 
  while ( _nLen > 0 ); 

  /* 
   *  Send remaining code.
   */

  if ( _nCodeBufIdx > 1 )
  {
    for ( _nIdx = 0; _nIdx < _nCodeBufIdx; _nIdx++ )
      cio_Put_char(pOutput,_aryCodeBuf[_nIdx]);
  }


  cio_free(s_aryRingBuffer);
  InitBinaryTree( BINARY_TREE_CLEANUP );


  /*
   *  Return the compressed file size
   */

  return(cio_Get_pos(pOutput));
}

/* --------------------------------------------------------------------------
 *
 *  Uncompress
 *
 *      API to Uncompress a previously compressed file or data
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The size of the data compressed
 *
 *  Notes:
 *      None 
 *
 * --------------------------------------------------------------------------
 */

T_compress_size Uncompress_lzss(compress_data_t* pInput, compress_data_t* pOutput )
{
  int _nIdx;

  int  _rNode;
  int _ch;
  unsigned int flags;

  T_compress_size _nExpectedLen = 0;


  /*
   *  First we must allocate memory for Text Buffer, if required
   */

  s_aryRingBuffer = (unsigned char*)cio_malloc(N + F - 1);

  if ( s_aryRingBuffer == NULL )
  {
    return(COMPRESSION_MEMORY);
  }


  /* 
   * Clear the buffer with any character that will appear often. 
   */

/*    for ( _nIdx = 0; _nIdx < N - F; _nIdx++ ) */

  for ( _nIdx = 0; _nIdx < (N + F - 1); _nIdx++ )
  {
    s_aryRingBuffer[_nIdx] = ' ';
  }

  _rNode = N - F;
  flags = 0;

  /*
   *  The first FOUR bytes are the file's original size (typically, used
   *  for preallocation of buffers).
   */

  cio_Rewind(pInput);
  cio_Rewind(pOutput);

  for ( _nIdx = 0; _nIdx < sizeof(T_compress_size); ++_nIdx )
  {
    _nExpectedLen |= ( cio_Get_char(pInput) << _nIdx * 8 );
  }

  /* 
   *  Forever...conditions internal will force a break
   */

  while ( 1 )
  {
    flags >>= 1;
    if ( ( flags & 256 ) == 0 )
    {
      if ( ( _ch = cio_Get_char(pInput) ) == EOF )
      {
        break;
      }

      /* 
       *  Uses higher byte cleverly to count eight
       */

      flags = _ch | 0xff00;
    }

    if ( flags & 1 )
    {
      if ( ( _ch = cio_Get_char(pInput) ) == EOF )
        break;

      cio_Put_char(pOutput,_ch);
      s_aryRingBuffer[_rNode++] = _ch;  
      _rNode &= (N - 1);
    }
    else
    {
      int _nJdx, _nKdx;

      if ( (( _nIdx = cio_Get_char(pInput) ) == EOF) || 
           (( _nJdx = cio_Get_char(pInput) ) == EOF) )
      {
        break;
      }

      _nIdx |= ((_nJdx & 0xf0) << 4);
      _nJdx = ( _nJdx & 0x0f) + THRESHOLD;

      for ( _nKdx = 0; _nKdx <= _nJdx; _nKdx++ )
      {
        _ch = s_aryRingBuffer[(_nIdx + _nKdx) & (N - 1)];
        cio_Put_char(pOutput,_ch);
        s_aryRingBuffer[_rNode++] = _ch;  
        _rNode &= (N - 1);
      }
    }
  }


  cio_free(s_aryRingBuffer);


  /*
   *  Did we decompress successfully????
   */

  if ( _nExpectedLen != cio_Get_pos(pOutput) )
  {
    return(-1);
  }

  /*
   *  Return the length
   */

  return(_nExpectedLen);
}
