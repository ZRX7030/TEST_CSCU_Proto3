#include "compress.h"

int idc_decompress_gzip(const char *pSrc, int iSrcLen, char **pDst)
{
	z_stream stream;
	int iRet, iOutLen;

	if(!pSrc || iSrcLen <= 0)
		return -1;

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = Z_NULL;
	stream.avail_in = 0;

	iRet = inflateInit2(&stream, 47);
	if(iRet != Z_OK)
		return -1;

	stream.next_in = (Byte *)pSrc;
	stream.avail_in = iSrcLen;

	iOutLen = iSrcLen * 4;
	*pDst = malloc(iOutLen);

	if(!*pDst)
	{
		inflateEnd(&stream);
		return -1;
	}

	do
	{
		stream.next_out = (Byte *)*pDst;
		stream.avail_out = iOutLen;
		iRet = inflate(&stream, Z_NO_FLUSH);
		switch(iRet)
		{
			case Z_NEED_DICT:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd(&stream);
				return -1;
		}
	}while(stream.avail_out == 0);

	inflateEnd(&stream);
	return 0;
}


/**
 *压缩
 */
int def(unsigned char *source, int in_len, unsigned char *dest, int *out_len)
{
	int ret;
	z_stream strm;
	
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	ret = deflateInit(&strm, Z_BEST_SPEED);
	if (ret != Z_OK)
		return ret;
	strm.next_in = source;
	strm.avail_in = in_len;
	
	do {
		strm.next_out = dest;
		strm.avail_out = in_len;
		ret = deflate(&strm, Z_FINISH);    /* no bad return value */
		*out_len = in_len - strm.avail_out;
	} while (strm.avail_out == 0);

	(void)deflateEnd(&strm);
	
	return Z_OK;
}

/**
 *解压
 */
int inf(unsigned char *source, int in_len, unsigned char *dest, int max_len, int *out_len)
{
	int ret;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	strm.avail_in = in_len;
	strm.next_in = source;

	do {
		strm.avail_out = max_len;
		strm.next_out = dest;
		ret = inflate(&strm, Z_FINISH);
		switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
		}
		*out_len = max_len - strm.avail_out;
	} while (strm.avail_out == 0);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return Z_OK;                                                                            
}

