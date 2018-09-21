/*
 * crc.h
 *
 * 描述：计算CRC校验值
 * 创建: LDM 20150401
 * 修改记录:
 * 见crc.cpp
 */
#if !defined(T_WORD)
#define T_WORD unsigned int
#endif
#if !defined(T_HWORD)
#define T_HWORD unsigned short
#endif
#if !defined(T_UBYTE)
#define T_UBYTE unsigned char
#endif


// 获得CRC校验码
T_HWORD get_CRC16( char *buf, T_WORD count );
