/*
 * 3DES.cpp
 *
 * 类描述：3DES加密算法
 * 创建: LDM 20150401
 * 修改记录:
 * 1.
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "3DES.h"

#ifdef TURBOC
#define code
#define xdata
#define reentrant
#endif
/*#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long   */

/* resultat de codage et decodage */
BYTE res_codage[8];
BYTE res_decodage[8];

static    BYTE Key[64];
static    BYTE input[64], /* data input */
     	  output[64]; /* data output */
static    BYTE Kn[16][48];

/*	------	TABLES DE CHIFFREMENT --------- */
/* table 1 : permutation initiale */
  static BYTE T1[] = {
	57,49,41,33,25,17, 9, 1,
	59,51,43,35,27,19,11, 3,
	61,53,45,37,29,21,13, 5,
	63,55,47,39,31,23,15, 7,
	56,48,40,32,24,16, 8, 0,
	58,50,42,34,26,18,10, 2,
	60,52,44,36,28,20,12, 4,
	62,54,46,38,30,22,14, 6
};

/* table 2 : permutation finale */
  static BYTE T2[] = {
	39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30,
	37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28,
	35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26,
	33, 1,41, 9,49,17,57,25,
	32, 0,40, 8,48,16,56,24
};

/* table 3 : fonction d'expansion E  ( valeur - 1 ) */
  static BYTE T3[] = {
	31, 0, 1, 2, 3, 4,
	 3, 4, 5, 6, 7, 8,
	 7, 8, 9,10,11,12,
	11,12,13,14,15,16,
	15,16,17,18,19,20,
	19,20,21,22,23,24,
	23,24,25,26,27,28,
	27,28,29,30,31, 0
};

/* table 5 : fonction de permutation P */
  static BYTE T5[] = {
	15, 6,19,20,
	28,11,27,16,
	 0,14,22,25,
	 4,17,30, 9,
	 1, 7,23,13,
	31,26, 2, 8,
	18,12,29, 5,
	21,10, 3,24
};

/* table 7 : table de choix 1 */
  static BYTE T7_1_2[56] =
{
	56,48,40,32,24,16, 8,
	 0,57,49,41,33,25,17,
	 9, 1,58,50,42,34,26,
	18,10, 2,59,51,43,35,

	62,54,46,38,30,22,14,
	 6,61,53,45,37,29,21,
	13, 5,60,52,44,36,28,
	20,12, 4,27,19,11, 3
};

/* table 8 : table de d‚calage */
//  static BYTE T8[16] =
  static BYTE T8[] =
{
	 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0
};

/* table 9 : table de choix 2 */
  static BYTE T9[] =
{
	13,16,10,23, 0, 4,
	 2,27,14, 5,20, 9,
	22,18,11, 3,25, 7,
	15, 6,26,19,12, 1,
	40,51,30,36,46,54,
	29,39,50,44,32,47,
	43,48,38,55,33,52,
	45,41,49,35,28,31
};

/* table 6 : s‚lection de fonctions S1 … S8 */
//  code BYTE T6[8][64] =
  static BYTE T6[][64] =
{
/* S1 */
	{
    14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
	 0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	 4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
	15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13
    },

/* S2 */
	{
    15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
	 3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	 0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
	13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9
    },

/* S3 */
	{
    10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
	13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
	 1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12
    },

/* S4 */
	{
	 7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
	13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
	 3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14
    },

/* S5 */
	{
     2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
	14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	 4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
	11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3
    },

/* S6 */
	{
    12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
	10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	 9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
	 4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13
    },

/* S7 */
	{
     4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
	13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	 1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
	 6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12
    },

/* S8 */
	{
    13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
	 1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	 7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
	 2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11
    }
};

/* Table pour eclatement des valeurs precedentes */
//code BYTE TE[16][4] =
static BYTE TE[][4] =
{
   {0,0,0,0},
   {0,0,0,1},
   {0,0,1,0},
   {0,0,1,1},
   {0,1,0,0},
   {0,1,0,1},
   {0,1,1,0},
   {0,1,1,1},
   {1,0,0,0},
   {1,0,0,1},
   {1,0,1,0},
   {1,0,1,1},
   {1,1,0,0},
   {1,1,0,1},
   {1,1,1,0},
   {1,1,1,1}
};
/*void ByteToChr1(BYTE *buf, int len)
{
    int  i;       //zsq for debug
    char buf2[500];

    if (len > 150)
        len = 150;
    su_memset(buf2, 0, sizeof(buf2));
    for (i = 0; i < len; i ++)
    {
        buf2[3*i+1] = hexChar(buf[i] & 0x0f);
        buf2[3*i] = hexChar((buf[i] & 0xf0) >> 4);
        buf2[3*i + 2] = ' ';
    }
    len = su_strlen(buf2);
//    for (i = 0; i < len; i += 26)
//        fprint(buf2+i, 1);
} */

/* ------  CLES ---------- */
/* ----------- Figure 3. Key Schedule Calculation. ------------ */
void Ks(BYTE *Key, BYTE Kn[16][48])
{
     BYTE cd[56];

     BYTE zt[60] ;
     
     int n;
     BYTE tmp11, tmp12, tmp21, tmp22;
     int i;
     BYTE *Knn;
	
	/* choix 1 */
	for (i = 0; i < 56; i++)
	{
 	   	cd[i] =  Key[T7_1_2[i]];
	}
	for (n = 0; n < 16; n++)
	{
		/* rotation … gauche du vecteur en fonction de l'indice */
		if (T8[n] == 0)
		{
			tmp11 = cd[0];
			tmp21 = cd[28];
                        memcpy( zt , &cd[1] , 55 );
                        memcpy( cd , zt     , 55 );
         	     //   memmove(&cd[0], &cd[1], 55); /* ce qui est en 1 va en 0 */
			cd[27] = tmp11;
			cd[55] = tmp21;
		}
		else
		{
			tmp11 = cd[0];
			tmp12 = cd[1];
			tmp21= cd[28];
			tmp22 = cd[29];

			memcpy( zt , &cd[2] , 54 );
			memcpy( cd , zt     , 54 );

  //			memmove(&cd[0], &cd[2], 54); /* ce qui est en 2 va en 0 */

			cd[26] = tmp11;
			cd[27] = tmp12;
			cd[54] = tmp21;
			cd[55] = tmp22;
		}
                /* choix 2 */
		Knn = Kn[n];
		for (i = 0; i < 48; i++)
		{
			Knn[i] = cd[T9[i]];
		}   
//su_delay(500);
/*************/
        if (n == 0)
        {
//        ByteToChr(Kn[n], 48);     //zsq for debug
//        ByteToChr1(cd, 56);
//          su_delay(100000);
        }
/*************/		
	}
}
/* --------- Figure 2. Calculation of f(R, K) ------------------ */
  void fonction(BYTE *Knn, BYTE *r, BYTE *s)
{
	/* n est l'indice de 1 a 16 pour choisir la cle
	   r est R.
	   s est le r‚sultat. */

     BYTE x[32];
     DWORD *px;
     int i, l;
     BYTE c;
     BYTE t;
	/* fonction E */    /*  + OU exclusif */
	/* S‚lection Sn */
    for (i = 0, l = 0, px = (DWORD *) x; i < 8;)
	{
		c = 32 * (r[T3[l]] ^ Knn[l]);
        l++;
		c += 8 * (r[T3[l]] ^ Knn[l]);
        l++;
		c += 4 * (r[T3[l]] ^ Knn[l]);
        l++;
		c += 2 * (r[T3[l]] ^ Knn[l]);
        l++;
		c += 1 * (r[T3[l]] ^ Knn[l]);
        l++;
		c += 16 * (r[T3[l]] ^ Knn[l]);
        l++;
        /* extraction de la valeur */
        t = T6[i][c];
        i++;
		/* Eclatement de la valeur; 4 bits -> 4 bytes */
        *px = *(long *)TE[t];
        px++;
    }
	/* fonction P */
	for (i = 0; i < 32; i++)
	{
		s[i] = x[T5[i]];
	}
}

/* --------- Permutations initiale et finale ------------------- */
  void permutation(BYTE *org, BYTE *tab)
//BYTE *org;
//BYTE *tab;
{
     BYTE tmp[64];
     int i;


	memcpy(tmp, org, 64);
	for (i = 0; i < 64; i++)
	{
		org[i] = tmp[tab[i]];
	}
}
/* ------------ Figure 1. Enciphering Computation. ------------- */
  void chiffrement(BYTE *xi, BYTE *xo, BYTE Kn[16][48])
//BYTE *xi;
//BYTE *xo;
//BYTE Kn[16][48];
{
     BYTE r[32], l[32];
     BYTE rp[32], lp[32];

     int i;
	 int n;

	memcpy(l, &xi[0], 32);
	memcpy(r, &xi[32], 32);

	for (n = 0; n < 16; n++)
	{
		memcpy(lp, r, 32);

		fonction(Kn[n], r, rp);
/*
if (n == 0)              //zsq for debug
{
ByteToChr(Kn[n], 48);
ByteToChr(r, 32);
ByteToChr(rp, 32);
}
*/		
		for (i = 0; i < 32; i++)
		{
			r[i] =( ( l[i]) ^ (rp[i] )  ) ;
		}
		memcpy(l, lp, 32);
/**
if (n == 0)            //zsq for debug
{
ByteToChr(r, 32);
ByteToChr(lp, 32);
}
**/		
	}
	memcpy(&xo[0], r, 32);
	memcpy(&xo[32], l, 32);

}

/* ----------- Deciphering Computation. ----------------------- */
  void dechiffrement(BYTE *xi, BYTE *xo, BYTE Kn[16][48])
//BYTE *xi;
//BYTE *xo;
//BYTE Kn[16][48];
{
     BYTE r[32], l[32], rp[32], lp[32];

	int i;
	int n;

	memcpy(l, &xi[0], 32);
	memcpy(r, &xi[32], 32);

	for (n = 0; n < 16; n++)
	{
		memcpy(lp, r, 32);
		fonction(Kn[15 - n], r, rp);
		for (i = 0; i < 32; i++)
		{
			r[i] =( ( l[i] ) ^ ( rp[i] )) ;
		}
		memcpy(l, lp, 32);
	}

	memcpy(&xo[0], r, 32);
	memcpy(&xo[32], l, 32);
}

/* -------------   Eclater 64 bits en 64 octets ---------------- */
  void eclater(BYTE *buf_bit, BYTE *byte)
//BYTE *buf_bit;
//BYTE *byte;
{
     int i;
     BYTE m;

	for (i = 0; i < 8; i++)
	{
		for (m = 0x80; m != 0;  )  // m >>= 1)
		{
                  if ((buf_bit[i] & m) != 0)
				*byte = 1;
			else
				*byte = 0;
                   byte++;
                  m=m/2 ;
		}
	}

}
/* ------------  Compacter 64 octets en 64 bits ---------------- */
void compacter(BYTE *byte, BYTE *buf_bit)
//BYTE *byte;
//BYTE *buf_bit;
{
	 int i;
	 BYTE m, n;

	for (i = 0; i < 8; i++)
	{
		n = 0;
		for (m = 0x80; m != 0; )  //  m >>= 1)
		{
			if (*byte++)
			n = n | m;
				   m=m/2 ;

		}
		buf_bit[i] = n;
	}
}

/* -------------  D E S ---------------------------------------- */
void des(BYTE *binput, BYTE *boutput, BYTE *bkey)
{

	eclater(binput, input);
	eclater(bkey, Key);
/**************************
ByteToChr(Key, 64);
**************************/
    //su_delay(100);
	Ks(Key, Kn);
/**************************
ByteToChr(Kn[0], 48);
**************************/
   // su_delay(100);
    
	permutation(input, T1);
	chiffrement(input, output, Kn);
/**********
    print_Left("input3:");               //zsq for debug
    ByteToChr(input, sizeof(input));
    print_Left("output1:");
    ByteToChr(output, sizeof(output));
**********/

	permutation(output, T2);
/**********
    print_Left("output2:");              //zsq for debug
    ByteToChr(output, sizeof(output));
**********/
	compacter(output, boutput);
}

void desm1(BYTE *binput, BYTE *boutput, BYTE *bkey)
{
	eclater(binput, input);
	eclater(bkey, Key);
	Ks(Key, Kn);
/****************************
ByteToChr(Kn[0], 48);
****************************/
	permutation(input, T1);
	dechiffrement(input, output, Kn);
/**********
    print_Left("input3:");              //zsq for debug
    ByteToChr(input, sizeof(input));
    print_Left("output1:");
    ByteToChr(output, sizeof(output));
**********/
	permutation(output, T2);
/**********
    print_Left("output2:");
    ByteToChr(output, sizeof(output));
**********/
	compacter(output, boutput);
}

/* --------------------  Fin ----------------------------------- */

/* calcule le code chiffre selon DES :
	- in=pt sur nombre … coder (8 octets =64 bits)
	- kk= pt sur clef d'encodage (8 octets=56 bits + parite)
	(Rq: parite viree pour le calcul donc calcul sur 56 bits et non 64)
	- resultat (nombre code) ds res_codage sur 8 octets */
BYTE * codage_des( BYTE * in , BYTE * kk )
{
	des( in , res_codage , kk);
	return(res_codage);
}



/* calcule le code d‚chiffre selon DES :
	- in=pt sur nombre cod‚ … d‚coder (8 octets =64 bits)
	- kk= pt sur clef d'encodage (8 octets=56 bits + parite)
	(Rq: parite viree pour le calcul donc calcul sur 56 bits et non 64)
	- resultat (nombre d‚code) ds pt res_decodage sur 8 octets */
BYTE * decodage_des( BYTE * in , BYTE * kk )
{
    desm1( in , res_decodage , kk);
	return(res_decodage);
}

/* calcule le code chiffre selon EDE=(DES(Kn)(DES-1(Kn+1)(DES(Kn)))) :
	- in=pt sur nombre cod‚ … d‚coder (8 octets =64 bits)
    - kk= pt sur clef d'encodage (16 octets=Kn(8) + Kn+1(8))
	(Rq: parite viree pour le calcul donc calcul sur 56 bits et non 64)
	- resultat (nombre d‚code) ds pt res_decodage sur 8 octets */
BYTE * codage_ede( BYTE * in , BYTE * kk )
{
    BYTE cleKn1[8],cleKn2[8],res_tmp[8];

    memcpy(cleKn1,kk,8);
    des( in , res_codage , cleKn1);
    memcpy(cleKn2,&kk[8],8);
    desm1(res_codage, res_tmp, cleKn2);
    memset(cleKn2,0,8);
    des( res_tmp , res_codage , cleKn1);
    memset(cleKn1,0,8);
    memset(res_tmp,0,8);
    return(res_codage);
}

WORD minin(WORD a,WORD b)
{
    if (a>=b)
        return(b);
    else
        return(a);
}

void macgen(BYTE *in,WORD len,BYTE *key, BYTE *out)
{
    BYTE lp,thismove;
    WORD  pos;


    memset(out,0,8);
    for(pos =0; pos < len; pos += (WORD)thismove)
    {
        thismove = minin(len-pos,8);
        for(lp=0;lp<thismove;lp++)
            out[lp] ^= in[lp+pos];

        des(out,out,key);
    }

}
/*3desĽÓĂÜ*/
/*·˝·¨Łş16×Ö˝ÚĂÜÔż˛đłÉ×óÓŇÁ˝˛ż·ÖŁ¨×óÓŇ¸÷8¸ö×Ö˝ÚŁ©Ł¬Ę×ĎČÓĂ×óĂÜÔżĽÓĂÜĘýľÝŁ¬˝áąűÔŮÓĂ
ÓŇĂÜÔżĽÓĂÜŁ¬˝áąűÔŮÓĂ×óĂÜÔżĽÓĂÜĽ´µĂ*/
void ThreeDes(BYTE *binput, BYTE *boutput, BYTE *bkey)
{
        BYTE temp_out[9], temp_out_1[9];
	BYTE LeftKey[9],RightKey[9];
	memset(LeftKey,0x00,sizeof(LeftKey));
	memset(RightKey,0x00,sizeof(RightKey));
        memset(temp_out, 0x00, sizeof(temp_out));
        memset(temp_out_1, 0x00, sizeof(temp_out_1));
	memcpy(LeftKey,bkey,8);
	memcpy(RightKey,bkey+8,8);
	des(binput,temp_out,LeftKey);
	desm1(temp_out,temp_out_1,RightKey);
	des(temp_out_1,boutput,LeftKey);
}
/*3des˝âĂÜ*/
/*·˝·¨Łş16×Ö˝ÚĂÜÔż˛đłÉ×óÓŇÁ˝˛ż·ÖŁ¨×óÓŇ¸÷8¸ö×Ö˝ÚŁ©Ł¬Ę×ĎČÓĂ×óĂÜÔż˝âĂÜĘýľÝŁ¬˝áąűÔŮÓĂ
ÓŇĂÜÔż˝âĂÜŁ¬˝áąűÔŮÓĂ×óĂÜÔż˝âĂÜĽ´µĂ*/
void DeThreeDes(BYTE *binput, BYTE *boutput, BYTE *bkey)
{
        BYTE temp_out[9], temp_out_1[9];
        BYTE LeftKey[9],RightKey[9];
	memset(LeftKey,0x00,sizeof(LeftKey));
	memset(RightKey,0x00,sizeof(RightKey));
        memset(temp_out, 0x00, sizeof(temp_out));
        memset(temp_out_1, 0x00, sizeof(temp_out_1));

	memcpy(LeftKey,bkey,8);
	memcpy(RightKey,bkey+8,8);
	desm1(binput,temp_out,LeftKey);
	des(temp_out,temp_out_1,RightKey);
	desm1(temp_out_1,boutput,LeftKey);						
}


/***************Ę®Áů˝řÖĆ×ŞłÉASCIIÂë05-08-11************/
void hex_asc (BYTE *ascii_buf, BYTE *hex_buf, BYTE lg)
{
    BYTE ai,bi,cnt;
    ai=0;bi=0;cnt=0;
    if(lg%2==1) cnt=1;
    while(ai<lg)
    {
        ascii_buf[ai]=((cnt&0x01)?(hex_buf[bi++]&0x0f):(hex_buf[bi]>>4));
        if(ascii_buf[ai]<10) ascii_buf[ai]+='0';
        else ascii_buf[ai]+=('A'-10);
        ai++;cnt++;
    }
}

void int_bcd (BYTE *bcd_buf, BYTE lgBcd, WORD *ptSrc)
/*
ą¦ÄÜËµĂ÷Łş
    ˝«unsigned intĘý×Ö×Ş»»łÉBCDÂë
Ŕý×ÓŁş*ptSrc = 12345
if lgBcd = 2	bcd_bufÎŞ0x23/0x45
if lgBcd = 5	bcd_bufÎŞ0x00/0x00/0x01/0x23/0x45
*/
{
    int i;
    WORD  k;	
    k=*ptSrc;
    for(i=lgBcd-1; i>=0; i--)
    {
        bcd_buf[i]=k%10+(k/10%10)*16;
        k=k/100;
    }
}
/*==========================================================================

//参数说明： flag 1:加密  0:解密
===========================================================================*/
int data_convert(unsigned char *in,int in_len,unsigned char * out,char flag,unsigned char *key)
{
	int count,i;
  	unsigned char tembuf[9];	
	count=in_len/8;

	for(i=0;i<count;i++)
	{
		memset(tembuf,0x00,sizeof(tembuf));
		memcpy(tembuf,in+(i*8),8);
		if(flag==1)
			ThreeDes(tembuf,out+(i*8),key);
		else
			DeThreeDes(tembuf,out+(i*8),key);			
	}
	if(in_len%8!=0)
	{
		memset(tembuf,0x00,sizeof(tembuf));
		memcpy(tembuf,in+(i*8),in_len%8);
		if(flag==1)
			ThreeDes(tembuf,out+(i*8),key);
		else
			DeThreeDes(tembuf,out+(i*8),key);
		i++;
	}
   return i*8;

}
