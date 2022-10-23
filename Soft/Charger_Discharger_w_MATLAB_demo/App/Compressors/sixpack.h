#ifndef __SIXPACK_H
  #define __SIXPACK_H
/* 
 *  Sixpack.h
 *
 *  Header File for the LZSS algorithm (implemented using a Binary Tree).
 *
 *
 *  Description:
 *
 *      The Sixpack program is a file compression utility using a string copy
 *      algorithm and adaptive Huffman encoding.  The program was written
 *      specifically for the Data Compression contest announced in the February 
 *      1991 issue of Dr. Dobb's Journal, based on earlier compression programs 
 *      that I have developed over the past few years.  The goal was to achieve 
 *      maximum compression on a 640K PC, even if it ran slowly. 
 *
 *      The main disadvantage is slow compression time, since the algorithm
 *      repeatedly searches the last few thousand bytes looking for the longest
 *      string that matches the current text.  Decompression is faster, 
 *      involving no text searching.  The compression speed can be adjusted 
 *      somewhat by changing the search parameters. 
 *
 *      The whimsical name Sixpack was chosen because the program combines six
 *      algorithms into a single data packing method.  The algorithms 
 *      illustrate a variety of data structures, including a binary tree, a 
 *      hash table, doubly linked lists and a circular array.  I must admit 
 *      that integrating all these concepts into a working program was quite 
 *      educational.  A brief description of each algorithm can be found in 
 *      the document "Sixpack.doc". 
 *
 */

  #define TEXTSEARCH              1000    // Max strings to search in text file
  #define BINSEARCH                200    // Max strings to search in binary file
  #define TEXTNEXT                  50    // Max search at next character in text file
  #define BINNEXT                   20    // Max search at next character in binary file
  #define MAXFREQ                 2000    // Max frequency count before table reset
  #define MINCOPY                    3    // Shortest string copy length
  #define MAXCOPY                   32 //64    // Longest string copy length
  #define SHORTRANGE                 3    // Max distance range for shortest length copy
  #define COPYRANGES                 5 //6    // Number of string copy distance bit ranges



  #define CODESPERRANGE           (MAXCOPY - MINCOPY + 1)


// Huffman stuff

  #define HASHSIZE            8192 // 16384           /* No. of entries in hash table */
  #define HASHMASK            (HASHSIZE - 1)  /* Mask for hash key wrap       */


/*
 *  Define hash key function using MINCOPY characters of string prefix
 */

  #define getkey(n) ((buffer[n] ^ (buffer[(n+1)%maxsize]<<4) ^ (buffer[(n+2)%maxsize]<<8)) & HASHMASK)

/*
 *  Adaptive Huffman variables
 */

  #define TERMINATE           256             /* EOF code                     */
  #define FIRSTCODE           257             /* First code for copy lengths  */

  #define MAXCHAR             (FIRSTCODE+COPYRANGES*CODESPERRANGE-1)
  #define SUCCMAX             (MAXCHAR+1)
  #define TWICEMAX            (2*MAXCHAR+1)
  #define ROOT                1



#endif
