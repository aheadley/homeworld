//
//  Bitfile IO Module
//

//	Darren Stone 1998/10/05
//
//  (file-oriented code based on The Data Compression Book, by Mark Nelson & Jean-Loup Gailly)
//
//

#ifndef _BITIO_H
#define _BITIO_H

#include <stdio.h>

typedef struct bit_file {
    FILE *file;
    unsigned char mask;
    int rack;
    int pacifier_counter;
	int index;  // like a file position, in bytes
} BIT_FILE;

typedef struct bit_buffer {
    char *buffer;
    int index;
    unsigned char mask;
    int rack;
    int pacifier_counter;
} BIT_BUFFER;

int bitioInit(void);
void bitioShutdown(void);

// file oriented bit i/o
BIT_FILE     *bitioFileOpenInput( char *name );
BIT_FILE     *bitioFileOpenOutput( char *name );
void          bitioFileOutputBit( BIT_FILE *bit_file, int bit );
void          bitioFileOutputBits( BIT_FILE *bit_file, unsigned long code, int count );
int           bitioFileInputBit( BIT_FILE *bit_file );
unsigned long bitioFileInputBits( BIT_FILE *bit_file, int bit_count );
int           bitioFileCloseInput( BIT_FILE *bit_file );
int           bitioFileCloseOutput( BIT_FILE *bit_file );
void          bitioFilePrintBinary( FILE *file, unsigned int code, int bits );

// for transitioning between bit & byte mode on open streams
BIT_FILE     *bitioFileAppendStart(FILE *fp);
int			  bitioFileAppendStop(BIT_FILE *bit_file);
BIT_FILE     *bitioFileInputStart(FILE *fp);
int			  bitioFileInputStop(BIT_FILE *bit_file);


// buffer oriented bit i/o
BIT_BUFFER   *bitioBufferOpen(char *buffer);
void          bitioBufferOutputBit(BIT_BUFFER *bitBuffer, int bit);
void          bitioBufferOutputBits(BIT_BUFFER *bitBuffer, unsigned long code, int count);
int           bitioBufferInputBit(BIT_BUFFER *bitBuffer);
unsigned long bitioBufferInputBits(BIT_BUFFER *bitBuffer, int count);
int           bitioBufferCloseOutput(BIT_BUFFER *bitBuffer);
int           bitioBufferCloseInput(BIT_BUFFER *bitBuffer);


#endif


