#ifndef __LZSS_H
#define __LZSS_H

void lzssCompressFile(FILE *input, BIT_FILE *output);
int  lzssCompressBuffer(char *input, int inputSize, char *output, int outputSize);
void lzssExpandFile(BIT_FILE *input, FILE *output);
int  lzssExpandBuffer(char *input, int inputSize, char *output, int outputSize);
int  lzssExpandFileToBuffer(BIT_FILE *input, char *output, int outputSize);

#endif
