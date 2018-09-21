#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#include <zlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int def(unsigned char *source, int in_len, unsigned char *dest, int *out_len);
int inf(unsigned char *source, int in_len, unsigned char *dest, int max_len, int *out_len);
int idc_decompress_gzip(const char *pSrc, int iSrcLen, char **pDst);


#endif
