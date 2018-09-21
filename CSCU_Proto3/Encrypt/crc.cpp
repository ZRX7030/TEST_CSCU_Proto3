/*
 * crc.cpp
 *
 * 描述：计算CRC校验值
 * 创建: LDM 20150401
 * 修改记录:
 * 1.
 */
#include "crc.h"

static T_HWORD crctab[256]={
        (T_HWORD)0 ,        (T_HWORD)32773 ,    (T_HWORD)32783 ,    (T_HWORD)10 ,       (T_HWORD)32795 ,
        (T_HWORD)30 ,       (T_HWORD)20 ,       (T_HWORD)32785 ,    (T_HWORD)32819 ,    (T_HWORD)54 ,
        (T_HWORD)60 ,       (T_HWORD)32825 ,    (T_HWORD)40 ,       (T_HWORD)32813 ,    (T_HWORD)32807 ,
        (T_HWORD)34 ,       (T_HWORD)32867 ,    (T_HWORD)102 ,      (T_HWORD)108 ,      (T_HWORD)32873 ,
        (T_HWORD)120 ,      (T_HWORD)32893 ,    (T_HWORD)32887 ,    (T_HWORD)114 ,      (T_HWORD)80 ,
        (T_HWORD)32853 ,    (T_HWORD)32863 ,    (T_HWORD)90 ,       (T_HWORD)32843 ,    (T_HWORD)78 ,
        (T_HWORD)68 ,       (T_HWORD)32833 ,    (T_HWORD)32963 ,    (T_HWORD)198 ,      (T_HWORD)204 ,
        (T_HWORD)32969 ,    (T_HWORD)216 ,      (T_HWORD)32989 ,    (T_HWORD)32983 ,    (T_HWORD)210 ,
        (T_HWORD)240 ,      (T_HWORD)33013 ,    (T_HWORD)33023 ,    (T_HWORD)250 ,      (T_HWORD)33003 ,
        (T_HWORD)238 ,      (T_HWORD)228 ,      (T_HWORD)32993 ,    (T_HWORD)160 ,      (T_HWORD)32933 ,
        (T_HWORD)32943,     (T_HWORD)170 ,      (T_HWORD)32955 ,    (T_HWORD)190,       (T_HWORD)180 ,
        (T_HWORD)32945 ,    (T_HWORD)32915 ,    (T_HWORD)150 ,      (T_HWORD)156 ,      (T_HWORD)32921 ,
        (T_HWORD)136 ,      (T_HWORD)32909 ,    (T_HWORD)32903 ,    (T_HWORD)130 ,      (T_HWORD)33155,
        (T_HWORD)390 ,      (T_HWORD)396 ,      (T_HWORD)33161 ,    (T_HWORD)408 ,      (T_HWORD)33181 ,
        (T_HWORD)33175 ,    (T_HWORD)402 ,      (T_HWORD)432 ,      (T_HWORD)33205 ,    (T_HWORD)33215 ,
        (T_HWORD)442 ,      (T_HWORD)33195 ,    (T_HWORD)430 ,      (T_HWORD)420 ,      (T_HWORD)33185 ,
        (T_HWORD)480 ,      (T_HWORD)33253 ,    (T_HWORD)33263 ,    (T_HWORD)490 ,      (T_HWORD)33275 ,
        (T_HWORD)510 ,      (T_HWORD)500 ,      (T_HWORD)33265 ,    (T_HWORD)33235 ,    (T_HWORD)470 ,
        (T_HWORD)476 ,      (T_HWORD)33241 ,    (T_HWORD)456 ,      (T_HWORD)33229 ,    (T_HWORD)33223,
        (T_HWORD)450 ,      (T_HWORD)320 ,      (T_HWORD)33093 ,    (T_HWORD)33103 ,    (T_HWORD)330 ,
        (T_HWORD)33115 ,    (T_HWORD)350 ,      (T_HWORD)340 ,      (T_HWORD)33105,     (T_HWORD)33139 ,
        (T_HWORD)374 ,      (T_HWORD)380 ,      (T_HWORD)33145 ,    (T_HWORD)360 ,      (T_HWORD)33133 ,
        (T_HWORD)33127 ,    (T_HWORD)354 ,      (T_HWORD)33059 ,    (T_HWORD)294,       (T_HWORD)300 ,
        (T_HWORD)33065 ,    (T_HWORD)312 ,      (T_HWORD)33085 ,    (T_HWORD)33079 ,    (T_HWORD)306 ,
        (T_HWORD)272 ,      (T_HWORD)33045 ,    (T_HWORD)33055 ,    (T_HWORD)282 ,      (T_HWORD)33035 ,
        (T_HWORD)270 ,      (T_HWORD)260 ,      (T_HWORD)33025 ,    (T_HWORD)33539 ,    (T_HWORD)774 ,
        (T_HWORD)780 ,      (T_HWORD)33545 ,    (T_HWORD)792 ,      (T_HWORD)33565 ,    (T_HWORD)33559 ,
        (T_HWORD)786 ,      (T_HWORD)816 ,      (T_HWORD)33589 ,    (T_HWORD)33599 ,    (T_HWORD)826,
        (T_HWORD)33579 ,    (T_HWORD)814 ,      (T_HWORD)804 ,      (T_HWORD)33569 ,    (T_HWORD)864,
        (T_HWORD)33637 ,    (T_HWORD)33647 ,    (T_HWORD)874,       (T_HWORD)33659 ,    (T_HWORD)894 ,
        (T_HWORD)884 ,      (T_HWORD)33649 ,    (T_HWORD)33619 ,    (T_HWORD)854 ,      (T_HWORD)860 ,
        (T_HWORD)33625 ,    (T_HWORD)840 ,      (T_HWORD)33613 ,    (T_HWORD)33607 ,    (T_HWORD)834,
        (T_HWORD)960,       (T_HWORD)33733 ,    (T_HWORD)33743,     (T_HWORD)970 ,      (T_HWORD)33755 ,
        (T_HWORD)990 ,      (T_HWORD)980 ,      (T_HWORD)33745 ,    (T_HWORD)33779 ,    (T_HWORD)1014 ,
        (T_HWORD)1020 ,     (T_HWORD)33785 ,    (T_HWORD)1000 ,     (T_HWORD)33773 ,    (T_HWORD)33767,
        (T_HWORD)994 ,      (T_HWORD)33699 ,    (T_HWORD)934 ,      (T_HWORD)940 ,      (T_HWORD)33705 ,
        (T_HWORD)952 ,      (T_HWORD)33725 ,    (T_HWORD)33719 ,    (T_HWORD)946 ,      (T_HWORD)912,
        (T_HWORD)33685 ,    (T_HWORD)33695 ,    (T_HWORD)922 ,      (T_HWORD)33675 ,    (T_HWORD)910,
        (T_HWORD)900 ,      (T_HWORD)33665 ,    (T_HWORD)640 ,      (T_HWORD)33413 ,    (T_HWORD)33423 ,
        (T_HWORD)650 ,      (T_HWORD)33435 ,    (T_HWORD)670 ,      (T_HWORD)660 ,      (T_HWORD)33425 ,
        (T_HWORD)33459 ,    (T_HWORD)694,       (T_HWORD)700 ,      (T_HWORD)33465 ,    (T_HWORD)680 ,
        (T_HWORD)33453 ,    (T_HWORD)33447 ,    (T_HWORD)674 ,      (T_HWORD)33507 ,    (T_HWORD)742 ,
        (T_HWORD)748 ,      (T_HWORD)33513 ,    (T_HWORD)760 ,      (T_HWORD)33533 ,    (T_HWORD)33527,
        (T_HWORD)754 ,      (T_HWORD)720 ,      (T_HWORD)33493 ,    (T_HWORD)33503 ,    (T_HWORD)730 ,
        (T_HWORD)33483 ,    (T_HWORD)718 ,      (T_HWORD)708 ,      (T_HWORD)33473 ,    (T_HWORD)33347 ,
        (T_HWORD)582 ,      (T_HWORD)588 ,      (T_HWORD)33353 ,    (T_HWORD)600 ,      (T_HWORD)33373 ,
        (T_HWORD)33367,     (T_HWORD)594 ,      (T_HWORD)624 ,      (T_HWORD)33397 ,    (T_HWORD)33407 ,
        (T_HWORD)634 ,      (T_HWORD)33387 ,    (T_HWORD)622 ,      (T_HWORD)612 ,      (T_HWORD)33377 ,
        (T_HWORD)544 ,      (T_HWORD)33317 ,    (T_HWORD)33327 ,    (T_HWORD)554 ,      (T_HWORD)33339 ,
        (T_HWORD)574 ,      (T_HWORD)564 ,      (T_HWORD)33329 ,    (T_HWORD)33299 ,    (T_HWORD)534 ,
        (T_HWORD)540 ,      (T_HWORD)33305 ,    (T_HWORD)520 ,      (T_HWORD)33293 ,    (T_HWORD)33287 ,
        (T_HWORD)514
};

// 获得CRC校验码
T_HWORD get_CRC16( char *buf, T_WORD count )
{
    T_HWORD crc = 0, t_crc;
    T_UBYTE *p;
    T_WORD i;

    char *ptr = buf;

    for( i=0; i<count; i++,ptr++ )
        crc = (crctab[((crc>>8)&0xff)^(((char)*ptr)&0xff)]^(crc<<8));

    // 调换成字符存储方式
    p = (T_UBYTE *)&t_crc;
    p[0] = (T_UBYTE)((crc & 0xFF00)>>8);
    p[1] = (T_UBYTE)(crc & 0xFF);
    return t_crc;
}
