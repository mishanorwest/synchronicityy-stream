// Searches.cpp: lookup functions for FindCity DLL
#include "FindCity.h"
#if defined __unix__ || defined __APPLE__
#include <string.h>
#include <stdlib.h> //for atoi
#include <ctype.h> // for isdigit
#include <iconv.h> // for iconv - replacement for CharToOem
#endif

#define MAX_CHANGES_TABLE_LENGTH 300

extern int m_nVersion;
extern int m_nStates, m_nEntries, m_nAlternate;
extern int m_nCurrentCountry;
extern int m_returnAbbreviations;
extern int m_illinoisTreatment;
extern int m_bEnableHalfDST;
extern int m_bAlwaysReturnDupFlag;
extern int m_nChangeCount;
extern FILE * m_fpData[4];
int m_nNumberOfChanges;
int m_nChangesTable[MAX_CHANGES_TABLE_LENGTH][3];

extern char m_Names[MAXENTRIES][61];
extern char m_Abbrevs[MAXENTRIES][7];
extern int m_nCounts[MAXENTRIES];	// city counts
extern unsigned char m_nFileIndex[MAXENTRIES];
extern unsigned int m_nCityPtrIndex[MAXENTRIES];
extern unsigned short int * m_pTimeIndex;
extern unsigned int * m_pTableDates;
extern short int * m_pTableValues;
extern unsigned int * m_CityPointers;
extern struct SoundexInfo * m_pSoundexList;

extern char * m_pUpcase;
extern struct CityInfo ** m_pPersonal;
extern int RecordLength[4];
extern int CityNameLen[4];

const char soundexnum[] = "01230120022455012623010202";
static int daysofar[10] = {30,61,91,122,152,183,214,244,275,305};
static int MonthLength[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
char upcase1[256] =
     {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
      '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
      '`','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}','~','',
      'C','U','E','A','A','A','A','C','E','E','E','I','I','I','A','A',
      'E','A','A','O','O','O','U','U','Y','O','U',0x9b,0x009c,'?',0x9e,0x9f,
      'A','I','O','U','N','N','A','O',0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
      0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
      'A','A','A','A','A','A','A','C','E','E','E','E','I','I','I','I',
      'D','N','O','O','O','O','O',0xd7,'O','U','U','U','U','Y',0xde,0xdf,
      'A','A','A','A','A','A','A','C','E','E','E','E','I','I','I','I',
      'O','N','O','O','O','O','O',0xf7,'O','U','U','U','U','Y',0xfe,'Y'};
char upcase2[256] =
     {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
      '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
      '`','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}','~','',
      0x9e,'Y',' ','F',' ',' ',' ',' ',' ',' ','S',0x8b,'O',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','S',0x9b,'O','?',0x9e,'Y',
      ' ',' ','C','L',' ','Y',' ',' ',0xa8,' ',0xaa,0xab,0xac,0xad,' ',0xaf,
      0xb0,0xb1,0xb2,0xb3,0xb4,'U',0xb6,'*',0xb8,0xb9,0xba,0xbb,' ',' ',' ',0xbf,
      'A','A','A','A','A','A','A','C','E','E','E','E','I','I','I','I',
      'D','N','O','O','O','O','O',0xd7,'O','U','U','U','U','Y',0xde,0xdf,
      'A','A','A','A','A','A','A','C','E','E','E','E','I','I','I','I',
      'O','N','O','O','O','O','O',0xf7,'O','U','U','U','U','Y',0xfe,'Y'};
static int doskip1[256] = {
      0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,
      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
		0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0};
static int doskip2[256] = {
      0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,1,
      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
      1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
      1,0,1,0,1,1,1,1,1,1,0,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,3,1,1,0,
		1,1,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,
      0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0};
short int wuni[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,  0x1a,  0x1b,  0x1c,  0x1d,  0x1e,  0x1f,
  0x20,  0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x27,  0x28,  0x29,  0x2a,  0x2b,  0x2c,  0x2d,  0x2e,  0x2f,
  0x30,  0x31,  0x32,  0x33,  0x34,  0x35,  0x36,  0x37,  0x38,  0x39,  0x3a,  0x3b,  0x3c,  0x3d,  0x3e,  0x3f,
  0x40,  0x41,  0x42,  0x43,  0x44,  0x45,  0x46,  0x47,  0x48,  0x49,  0x4a,  0x4b,  0x4c,  0x4d,  0x4e,  0x4f,
  0x50,  0x51,  0x52,  0x53,  0x54,  0x55,  0x56,  0x57,  0x58,  0x59,  0x5a,  0x5b,  0x5c,  0x5d,  0x5e,  0x5f,
  0x60,  0x61,  0x62,  0x63,  0x64,  0x65,  0x66,  0x67,  0x68,  0x69,  0x6a,  0x6b,  0x6c,  0x6d,  0x6e,  0x6f,
  0x70,  0x71,  0x72,  0x73,  0x74,  0x75,  0x76,  0x77,  0x78,  0x79,  0x7a,  0x7b,  0x7c,  0x7d,  0x7e,  0x7f,
0x20ac,  0x20,  0x2c, 0x192,0x201e,0x2026,0x2020,0x2021, 0x2c6,0x2030, 0x160,0x2039, 0x152,  0x20, 0x17d,  0x20,
  0x20,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014, 0x2dc,0x2122, 0x161,0x203a, 0x153,  0x20, 0x17e, 0x178,
  0xa0,  0xa1,  0xa2,  0xa3,  0xa4,  0xa5,  0xa6,  0xa7,  0xa8,  0xa9,  0xaa,  0xab,  0xac,  0xad,  0xae,  0xaf,
  0xb0,  0xb1,  0xb2,  0xb3,  0xb4,  0xb5,  0xb6,  0xb7,  0xb8,  0xb9,  0xba,  0xbb,  0xbc,  0xbd,  0xbe,  0xbf,
  0xc0,  0xc1,  0xc2,  0xc3,  0xc4,  0xc5,  0xc6,  0xc7,  0xc8,  0xc9,  0xca,  0xcb,  0xcc,  0xcd,  0xce,  0xcf,
  0xd0,  0xd1,  0xd2,  0xd3,  0xd4,  0xd5,  0xd6,  0xd7,  0xd8,  0xd9,  0xda,  0xdb,  0xdc,  0xdd,  0xde,  0xdf,
  0xe0,  0xe1,  0xe2,  0xe3,  0xe4,  0xe5,  0xe6,  0xe7,  0xe8,  0xe9,  0xea,  0xeb,  0xec,  0xed,  0xee,  0xef,
  0xf0,  0xf1,  0xf2,  0xf3,  0xf4,  0xf5,  0xf6,  0xf7,  0xf8,  0xf9,  0xfa,  0xfb,  0xfc,  0xfd,  0xfe,  0xff};
char ununi[1024] = {
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
      '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
      '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
      'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      ' ',0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
      0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
		0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
		0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
		0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
		0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
		'A','a','A','a','A','a','C','c','C','c','C','c','C','c','D','d',
		0xd0,'d','E','e','E','e','E','e','E','e','E','e','G','g','G','g',
		'G','g','G','g','H','h','H','h','I','i','I','i','I','i','I','i',
		'I','i','I','i','J','j','K','k','k','L','l','L','l','L','l','L',
		'l','L','l','N','n','N','n','N','n','n','N','n','O','o','O','o',
		'O','o',0x8c,0x9c,'R','r','R','r','R','r','S','s','S','s','S','s',
		0x8a,0x9a,'T','t','T','t','T','t','U','u','U','u','U','u','U','u',
		'U','u','U','u','W','w','Y','y',0x9f,'Z','z','Z','z','Z','z','f',
		'b','B','B','b','B','b','C','C','c','D','D','D','d','?','E','e',
		'E','F',0x83,'G','?','h','i','I','K','k','l','l','m','N','n','O',
		'O','o','P','p','P','p','R','S','s','S','?','t','T','t','T','U',
		'u','U','U','Y','y','Z','z','Z','Z','z','z','z','5','5','t','?',
		'|','?','?','!','J','J','j','L','L','l','N','N','n','A','a','I',
		'i','O','o','U','u','U','u','U','u','U','u','U','u','e','A','a',
		'A','a','A','a','G','g','G','g','K','k','O','o','O','o','Z','z',
		'j','J','J','j','G','g','H','?','N','n','A','a','A','a','O','o',
		'A','a','A','a','E','e','E','e','I','i','I','i','O','o','O','o',
		'R','r','R','r','U','u','U','u','S','s','T','t','Z','z','H','h',
		'N','?','?','?','Z','Z','A','a','E','e','O','o','O','o','O','o',
		'O','o','Y','y','?','?','?','?','?','?','?','?','?','?','?','?',
		'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
		'a','a','a','b','c','c','d','d','e','e','e','e','e','e','e','j',
		'g','g','G','?','?','h','h','h','i','i','I','l','l','l','?','m',
		'm','m','n','n','N','o','?','o','o','r','r','r','r','r','r','r',
		'R','R','s','s','s','s','s','t','t','u','u','v','v','w','y','Y',
		'z','z','z','z','?','?','?','C','?','B','?','G','H','j','k','L',
		'q','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
		'h','h','j','r','r','r','R','w','y','\'','"','\'','\'','\'','\'','\'',
		'?','?','<','>','^','v','^','?','\'','?','\'','`',',','_','?','?',
		':','.',',','?','?','?','+','-','?','?','?','?','~','"','~','x',
		'?','l','s','x','?','?','?','?','?','?','?','?','?','=','"','?',
		'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
		'A','a','B','b','B','b','B','b','C','c','D','d','D','d','D','d',	// 1e00 to 1eff
		'D','d','D','d','E','e','E','e','E','e','E','e','E','e','F','f',
		'G','g','H','h','H','h','H','h','H','h','H','h','I','i','I','i',
		'K','k','K','k','K','k','L','l','L','l','L','l','L','l','M','m',
		'M','m','M','m','N','n','N','n','N','n','N','n','O','o','O','o',
		'O','o','O','o','P','p','P','p','R','r','R','r','R','r','R','r',
		'S','s','S','s','S','s','S','s','S','s','T','t','T','t','T','t',
		'T','t','U','u','U','u','U','u','U','u','U','u','V','v','V','v',
		'W','w','W','w','W','w','W','w','W','w','X','x','X','x','Y','y',
		'Z','z','Z','z','Z','z','h','t','w','y','a','f','?','?','?','?',
		'A','a','A','a','A','a','A','a','A','a','A','a','A','a','A','a',
		'A','a','A','a','A','a','A','a','E','e','E','e','E','e','E','e',
		'E','e','E','e','E','e','E','e','I','i','I','i','O','o','O','o',
		'O','o','O','o','O','o','O','o','O','o','O','o','O','o','O','o',
		'O','o','O','o','U','u','U','u','U','u','U','u','U','u','U','u',
		'U','u','Y','y','Y','y','Y','y','Y','y','?','?','?','?','?','?'};
char punctuation[208] = {
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '-','-',0x96,0x96,0x97,0x97,'?','?',0x91,0x92,',','?','"','"',0x84,'?',
      0x86,0x87,0x95,'?','.','?',0x85,'?','?','?','?','?','?','?','?','?',
      0x89,'?',0xb4,'?','?','`','?','?','?',0x8b,0x9b,'?','?','?',0xaf,'?',
      '?','?','?','?','/','?','?','?','?','?','?','?','?','?','*','?',
      '?','?','%','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?',0xd7,',','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?',0x80,'?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
      '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?'};
int ZoneBounds[47] = {0,900,1800,2700,3150,3600,4050,4500,5400,6300,7200,7650,8100,
   8550,9000,9450,9900,10350,10800,-900,-1800,-2250,-2475,-2700,-3150,-3600,-4050,-4500,
	-4950,-5175,-5400,-5850,-6300,-6600,-6750,-7200,-7650,-8100,-8550,-9000,-9450,-9900,
	-10350,-10800,-11475,-11700,-12600};
#if defined __unix__ || defined __APPLE__
const char * zatnames[47][6] = {
#else
char * zatnames[47][6] = {
#endif
    "UT  ","BST ","BST ","BDST","LMT","BHDT",	// 0
    "WAT ","EGST","WAT ","WAT ","LMT","WAHD",	// 1
    "AT  ","FDT ","AT  ","AT  ","LMT","AZHD",	// 2
    "BZT2","BZ2D","BZ2W","XXX ","LMT","BZHD",	// 3
    "NST ","NDT ","NWT ","NDDT","LMT","NHDT",	// 3.5
    "AST ","ADT ","AWT ","XXX ","LMT","AHD",		// 4
    "VET ","VET ","VET ","XXX ","LMT","VHDT",	// 4.5
    "EST ","EDT ","EWT ","XXX ","LMT","EHDT",	// 5
    "CST ","CDT ","CWT ","XXX ","LMT","CHDT",	// 6
    "MST ","MDT ","MWT ","XXX ","LMT","MHDT",	// 7
    "PST ","PDT ","PWT ","XXX ","LMT","PHDT",	// 8
	 "PIT ","PIDT","PIWT","XXX ","LMT","PIHD",	// 8.5
    "YST ","YDT ","YWT ","XXX ","LMT","YHDT",	// 9
    "MART","MART","MART","XXX ","LMT","MAHD",	// 9.5
    "AHST","AHDT","AHWT","XXX ","LMT","AHHD",	// 10
    "HST ","HDT ","HWT ","XXX ","LMT","HHDT",	// 10.5
    "BET ","BDT ","BWT ","XXX ","LMT","BHD",		// 11
    "SAMT","SAMT","SAMT","XXX ","LMT","SAHD",	// 11.5
    "IDLW","IDLW","IDLW","XXX ","LMT","IDLW",	// 12
    "CET ","CEDT","CEWT","XXX ","LMT","CEHD",	// -1
    "EET ","EEDT","EETW","XXX ","LMT","EEHD",	// -2
    "BEAT","BEAT","BEAT","XXX ","LMT","XXX",		// -2.5
    "BEAUT","BEAUT","BEAUT","XXX ","LMT","XXX",	// -2.75
    "BAT ","BADT","BAWT","XXX ","LMT","BAHD",	// -3
    "IT  ","IRDT","IRWT","XXX ","LMT","IRHD",	// -3.5
    "USZ3","USZ3S","USZ3","XXX ","LMT","UZ3HD",	// -4
    "AFT ","AFDT","AFWT","XXX ","LMT","AFHD",	// -4.5
    "USZ4","USZ4S","USZ4","XXX ","LMT","UZ4HD",	// -5
    "IST ","IDT ","IWT ","XXX ","LMT","IHDT",	// -5.5
    "NPT ","NPT ","NPT ","XXX ","LMT","NPHD",	// -5.75
    "USZ5","USZ5S","USZ5","XXX ","LMT","UZ5HD",	// -6
    "NSUT","NSDT","NSWT","XXX ","LMT","NSHD",	// -6.5
    "USZ6","USZ6S","USZ6","XXX ","LMT","UZ6HD",	// -7
    "MALT","MALT","MALT","XXX ","LMT","MLHD",	// -7.33333
    "JT  ","JTD ","JTW ","XXX ","LMT","JHD",		// -7.5
    "AWST","AWDT","AWWT","XXX ","LMT","AWHD",	// -8
    "MT  ","MODT","MOWT","XXX ","LMT","MOHD",	// -8.5
    "JST ","JDT ","JWT ","XXX ","LMT","JHDT",	// -9
    "ACST","ACDT","ACWT","XXX ","LMT","ACHD",	// -9.5
    "AEST","AEDT","AEWT","XXX ","LMT","AEHD",	// -10
    "LHST","LHDT","LHWT","XXX ","LMT","LHHD",	// -10.5
    "UZ10","UZ10S","UZ10","XXX ","LMT","UZ10H",	// -11
    "NZ  ","NZS ","NZW ","XXX ","LMT","NZHD",	// -11.5
    "NZT ","NZST","NZWT","XXX ","LMT","NZHDT",	// -12
    "CHAST","CHADT","XXX ","XXX ","LMT","CHHD",	// -12.75
    "UZ12","UZ12S","UZ12","XXX ","LMT","UZ12H",	// -13
	 "LINT","LINDT","LINT","XXX ","LMT","LINHD"};// -14
#if defined __unix__ || defined __APPLE__
static const char * ZoneNames[47] = {"Greenwich","West Africa (+1)","Azores (+2)",
#else
static char * ZoneNames[47] = {"Greenwich","West Africa (+1)","Azores (+2)",
#endif
"Brazil Zone 2 (+3)","Newfoundland (+3.5)","Atlantic (+4)","Venezuela (+4.5)",
"US Eastern (+5)","US Central (+6)","US Mountain (+7)","US Pacific (+8)","Pitcairn Island (+8.5)","Yukon (+9)",
"Marquesas (+9.5)","Alaska-Hawaii (+10)","Hawaiian (+10.5)","Bering (+11)","Samoa (+11.5)",
"Intl Date Line West (+12)","Central European (-1)","Eastern European (-2)",
"British East Africa (-2.5)","Brit. E Africa Unified (-2.75)","Baghdad (-3)",
"Iran (-3.5)","USSR Zone 3 (-4)","Afghanistan (-4.5)","USSR Zone 4 (-5)",
"Indian (-5.5)","Nepal (-5.75)","USSR Zone 5 (-6)","North Sumatra (-6.5)",
"USSR Zone 6 (-7)","Sarawak (-7.33)","Java (-7.5)",
"Australian Western (-8)","Moluccas (-8.5)","Japan (-9)",
"Australian Central (-9.5)","Australian Eastern (-10)","Lord Howe Island (-10.5)",
"USSR Zone 10 (-11)","New Zealand (-11.5)","New Zealand Standard (-12)",
"Chatham Island (-12.75)","Chukot Time (-13)","Line Islands (-14)"};

int SetCountryByNameA(LPCSTR countryname);
int GetCountyNumberFromNameA(LPCSTR county);
int GetDataFields(int citynumber, int * datafields);
int GetCountyNameFromNumberA(LPSTR county, int cn);
int SetCountryByIndex(int countrynumber);
// New GPD 24 May 2005
void ProcessAtlasString(char * placename, int namelen);
// End GPD
//----------------------------------------------------------------------------

/* This function compares two strings in the same way as the standard C
   string compare functions, but it ignores blanks.  That is, the strings
   are compared as if they were both compressed--all blanks removed.
   The upcase translation table converts lower case letters and high-bit-set
   characters (used in many of the city names in the International Atlas for
   various accent marks) to standard ASCII upper case letters.
*/
int noblankstricmp(const char *sin1, char *sin2)
{
	int i,j,k;
	unsigned char * s1, * s2;
	int * pskip;

	i = j = 0;
	s1 = (unsigned char *) sin1;
	s2 = (unsigned char *) sin2;
	if (m_nVersion < 20) pskip = doskip1; else pskip = doskip2;
	while (s1[i] != '\0' || s2[j] != '\0')
	  {
		while (pskip[s1[i]] == 1) i++;
	   while (pskip[s2[j]] == 1) j++;
		if (pskip[s1[i]] == 2) while (s1[i] != '\0') ++i;
		if (pskip[s2[j]] == 2) while (s2[j] != '\0') ++j;
	   k = m_pUpcase[s1[i]] - m_pUpcase[s2[j]];
	   if (k != 0) return (k);
// check for ae and oe
		if (pskip[s1[i]] == 3)
	     {
		   if (pskip[s2[j]] != 3)
			  {
		      k = 'E' - m_pUpcase[s2[j+1]];
			   if (k != 0) return (k);
				else ++j;
			  }
	     }
	   else if (pskip[s2[j]] == 3)
		  {
			k = m_pUpcase[s1[i+1]] - 'E';
			if (k != 0) return (k);
			else ++i;
		  }
	   if (s1[i] != '\0') i++;
	   if (s2[j] != '\0') j++;
	  }
	return (0);
}

//-------------------------------------------------------------------------------------------------------------------------

char dfcn(short int * cn)
{
	char df;
	if (m_nFileIndex[m_nCurrentCountry] == 4)
	  {
		*cn = 0;
		return 0;
	  }
	if (m_nVersion < 20)
	  {
		fread(cn, 2, 1, m_fpData[m_nFileIndex[m_nCurrentCountry]]);
		df = (char) (*cn/1000);
		*cn %= 1000;
	  }
	else
	  {
		df = fgetc(m_fpData[m_nFileIndex[m_nCurrentCountry]]);
		fread(cn, 2, 1, m_fpData[m_nFileIndex[m_nCurrentCountry]]);
	  }
	return (df & 3);
}

//-------------------------------------------------------------------------------------------------------------------------

int isleapyear(int year)
{
	if (year % 4 != 0) return FALSE;
	if (year % 400 == 0) return TRUE;
	if (year % 100 == 0) return FALSE;
	return TRUE;
}

//----------------------------------------------------------------------------
// Return index of city in county, or primary city if countynumber == 0
// Return -1 if no match for city, or no city in that county.

//int __declspec(dllexport) GetIndexOfCityInCountyA(LPCSTR cityname, int countynumber, int * lastbefore)
int GetIndexOfCityInCountyA(LPCSTR cityname, int countynumber, int * lastbefore)
{
	char dupcity;
	short int cnum;
	int lowrec, highrec, middle, bf, first;
	int way;
	char testcity[61], acity[61];

	if (m_nCurrentCountry < 0) return -2;
	if (m_nVersion < 20) CharToOem(cityname, acity);
	else strcpy(acity, cityname);
	lowrec = 0;
	highrec = m_nCounts[m_nCurrentCountry] - 1;
	while (lowrec <= highrec)
	  {
		middle = (lowrec + highrec)/2;
	   GetCityForLookup(testcity, middle);
	   way = noblankstricmp((const char *) acity, testcity);
	   if (way == 0)
        {
			dupcity = dfcn(&cnum);
			if ((countynumber != 0 && countynumber == cnum) || (countynumber == 0 && dupcity < 2)) return middle;
			first = middle;
// different county or non-primary duplicate:
// scan back through file to try to find right county or primary
			for (bf = middle - 1; bf >= 0; bf--)
			  {
			   GetCityForLookup(testcity, bf);
				if (noblankstricmp((const char *) acity, testcity) != 0) break;
				first = bf;
				dupcity = dfcn(&cnum);
				if ((countynumber != 0 && countynumber == cnum) || (countynumber == 0 && dupcity < 2)) return bf;
			  }
// no success, try forward
			for (bf = middle + 1; bf < m_nCounts[m_nCurrentCountry]; bf++)
			  {
			   GetCityForLookup(testcity, bf);
				if (noblankstricmp((const char *) acity, testcity) != 0) break;
				dupcity = dfcn(&cnum);
				if ((countynumber != 0 && countynumber == cnum) || (countynumber == 0 && dupcity < 2)) return bf;
			  }
			*lastbefore = first;
			if (countynumber != 0) return (-1);		// no match found
// no primary found: return first match in data file
			return first;
		  }
	   if (way < 0) highrec = middle - 1;
	   else lowrec = middle + 1;
	  }
	if (way > 0) ++middle;
	*lastbefore = middle;
	return -1;
}

int _stdcall ZGetIndexOfCityInCountyA(LPCSTR cityname, int countynumber, int * lastbefore)
{
	return GetIndexOfCityInCountyA(cityname, countynumber, lastbefore);
}

//int __declspec(dllexport) GetIndexOfCityInCountyW(LPCWSTR wcityname, int countynumber, int * lastbefore)
int GetIndexOfCityInCountyW(LPCWSTR wcityname, int countynumber, int * lastbefore)
{
	char cityname[61];
	Narrow(cityname, wcityname);
	return GetIndexOfCityInCountyA(cityname, countynumber, lastbefore);
}

int _stdcall ZGetIndexOfCityInCountyW(LPCWSTR wcityname, int countynumber, int * lastbefore)
{
	char cityname[61];
	Narrow(cityname, wcityname);
	return GetIndexOfCityInCountyA(cityname, countynumber, lastbefore);
}

//int __declspec(dllexport) GetIndexOfCityA(LPCSTR cityname, int * lastbefore)
int GetIndexOfCityA(LPCSTR cityname, int * lastbefore)
{
	int lowrec, highrec, middle;
	int way;
	char testcity[61], acity[61];

	*lastbefore = 0;
	if (m_nCurrentCountry < 0) return -2;
	if (m_nVersion < 20) CharToOem(cityname, acity);
	else strcpy(acity, cityname);
	lowrec = 0;
	highrec = m_nCounts[m_nCurrentCountry] - 1;
	while (lowrec <= highrec)
	  {
		middle = (lowrec + highrec)/2;
	   GetCityForLookup(testcity, middle);
	   way = noblankstricmp((const char *) acity, testcity);
	   if (way == 0) return middle;
	   if (way < 0) highrec = middle - 1;
	   else lowrec = middle + 1;
	  }
	if (way > 0) ++middle;
	*lastbefore = middle;
	return -1;
}

int _stdcall ZGetIndexOfCityA(LPCSTR cityname, int * lastbefore)
{
	return GetIndexOfCityA(cityname, lastbefore);
}

//int __declspec(dllexport) GetIndexOfCityW(LPCWSTR cityname, int * lastbefore)
int GetIndexOfCityW(LPCWSTR cityname, int * lastbefore)
{
	char cname[61];

	*lastbefore = 0;
	if (m_nCurrentCountry < 0) return -2;
	Narrow(cname, cityname);
	return GetIndexOfCityA(cname, lastbefore);
}

int _stdcall ZGetIndexOfCityW(LPCWSTR cityname, int * lastbefore)
{
	return GetIndexOfCityW(cityname, lastbefore);
}

// The lookup routine for a full placename: city, county, state

//int __declspec(dllexport) FindCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country, int * datafields)
int FindCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country, int * datafields)
{
	char place[255];
	int i, scn, len, lb;
	char * cp;

	city[0] = county[0] = country[0] = '\0';
	if (m_nEntries == 0) return (-5);
	datafields[5] = 0;
	strcpy(place, placename);
	len = strlen(place);
	while (len > 0 && place[len - 1] == ' ') --len;
	place[len] = '\0';
// The parts of the placename are separated by commas, but since there are cities, counties, and countries
// in the Atlas with commas in their names, various possibilities must be tried.
// Start by assuming the part after the last comma is the state or country
	cp = strrchr(place, ',');
	if (cp == NULL)								// no comma found, try Personal Atlas
	  {
		if (m_pPersonal == NULL) return (-3);
		m_nCurrentCountry = m_nEntries - 1;
		strcpy(city, place);
		strcpy(country, m_Names[m_nCurrentCountry]);
		county[0] = '\0';
	  }
	else
	  {
		i = cp - place;							// remember where the comma is
		++cp;
		while (*cp == ' ') ++cp;				// start of soc name (maybe)
		scn = SetCountryByNameA(cp);			// match?
		if (scn < 0)								// no
		  {
			place[i] = '\0';						// end string temporarily
			cp = strrchr(place, ',');			// see if any commas earlier in place
			place[i] = ',';						// back to comma
			if (cp == NULL) return -2;			// no match for soc
			i = cp - place;
			++cp;
			while (*cp == ' ') ++cp;
			scn = SetCountryByNameA(cp);		// match country name with comma?
			if (scn < 0) return -2;				// no, we don't have that soc name
		  }
		strcpy(country, cp);						// OK, we have a state or country
		while (i > 0 && place[i - 1] == ' ') --i;
		place[i] = '\0';
	  }
	scn = 0;
// Now same routine for county
	cp = strrchr(place, ',');
	if (cp != NULL)								// may have a county
	  {
		i = cp - place;							// remember where the comma is
		++cp;
		while (*cp == ' ') ++cp;				// start of county name (maybe)
		scn = GetCountyNumberFromNameA(cp);	// match?
		if (scn < 0)								// no
		  {
			place[i] = '\0';						// end string temporarily
			cp = strrchr(place, ',');			// see if any commas earlier in place
			place[i] = ',';						// back to comma
			if (cp != NULL)						// found another earlier
			  {
				i = cp - place;
				++cp;
				while (*cp == ' ') ++cp;
				scn = GetCountyNumberFromNameA(cp);	// match county name with comma?
				if (scn < 0) return -4;					// no, we don't have that county name
				strcpy(county, cp);						// found the county
				while (i > 0 && place[i - 1] == ' ') --i;
				place[i] = '\0';
			  }
			else scn = 0;
		  }
		else
		  {
			strcpy(county, cp);						// found the county
			while (i > 0 && place[i - 1] == ' ') --i;
			place[i] = '\0';
		  }
	  }
	strcpy(city, place);
	datafields[5] = m_nCurrentCountry;
// ready to try to match city
	i = GetIndexOfCityInCountyA(city, scn, &lb);
	if (i < 0) return -1;							// drat
	GetDataFields(i, datafields);
	if (scn != 0 && !m_bAlwaysReturnDupFlag)	// if entered county, don't say duplicate city
	  {
		datafields[0] %= 1000;
	  }
	if (m_nCurrentCountry >= 0)
	  {
		if (m_returnAbbreviations) strcpy(country, m_Abbrevs[m_nCurrentCountry]);
		else strcpy(country, m_Names[m_nCurrentCountry]);
		if ((datafields[0] % 1000) > 0) GetCountyNameFromNumberA(county, datafields[0] % 1000);
	  }
	return (i);
}

int _stdcall ZFindCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country, int * datafields)
{
	return FindCityA(placename, city, county, country, datafields);
}

// The lookup routine for a full placename: city | county | state

//int __declspec(dllexport) FindCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country, int * datafields)
int FindCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country, int * datafields)
{
	char cplace[101], ccity[61], ccounty[61], ccountry[61];
	int result;

	Narrow(cplace, placename);
	result = FindCityA(cplace, ccity, ccounty, ccountry, datafields);
	city[0] = 0;
	county[0] = 0;
	country[0] = 0;
	if (result < 0) return result;
	Widen(city, ccity);
	Widen(county, ccounty);
	Widen(country, ccountry);
	return result;
}

int _stdcall ZFindCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country, int * datafields)
{
	return FindCityW(placename, city, county, country, datafields);
}

// The lookup routine for a full placename: city, county, state

//int __declspec(dllexport) LookupCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country,
//	int& countydup, int& latitude, int& longitude, int& typetable, int& zonetable, int& soc)
int LookupCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country,
  int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc)
{
	int result, datafields[6];

	result = FindCityA(placename, city, county, country, datafields);
	if (result < 0)
	  {
		*countydup = *latitude = *longitude = *typetable = *zonetable = *soc = 0;
	  }
	else
	  {
		*countydup = datafields[0];
		*latitude = datafields[1];
		*longitude = datafields[2];
		*typetable = datafields[3];
		*zonetable = datafields[4];
		*soc = datafields[5];
	  }
	return result;
}

int _stdcall ZLookupCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country,
	int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc)
{
	return LookupCityA(placename, city, county, country, countydup, latitude, longitude, typetable, zonetable, soc);
}

// The lookup routine for a full placename: city, county, state

//int __declspec(dllexport) LookupCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country,
//	int& countydup, int& latitude, int& longitude, int& typetable, int& zonetable, int& soc)
int LookupCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country,
	int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc)
{
	char cplace[101], ccity[61], ccounty[61], ccountry[61];
	int result, datafields[6];

	Narrow(cplace, placename);
	result = FindCityA(cplace, ccity, ccounty, ccountry, datafields);
	if (result < 0)
	  {
		countydup = latitude = longitude = typetable = zonetable = soc = 0;
		city[0] = 0;
		county[0] = 0;
		country[0] = 0;
	  }
	else
	  {
		*countydup = datafields[0];
		*latitude = datafields[1];
		*longitude = datafields[2];
		*typetable = datafields[3];
		*zonetable = datafields[4];
		*soc = datafields[5];
		Widen(city, ccity);
		Widen(county, ccounty);
		Widen(country, ccountry);
	  }
	return result;
}

int _stdcall ZLookupCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country,
	int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc)
{
	return LookupCityW(placename, city, county, country, countydup, latitude, longitude, typetable, zonetable, soc);
}

//-------------------------------------------------------------------------------------------------------------------------

// This function calculates what day the first Sunday of the month falls on
//   for the given year: it only expects to be passed April or October as month.

int firstsunday(int month, int year)
{
	double d;
	int jd;
	int itemp;

	itemp = year/4 + daysofar[month-3] + 1;
	d = -693933.0 + 365.0*((double) year) + ((double) itemp);
	if (year < 0 && year%4 != 0) --d;
	itemp = year/400 - year/100 + 2;
	d += itemp;
	jd = (int) d;
	itemp = jd % 7;
	if (itemp == 0) return(1);
	else return(8 - itemp);
}

//-------------------------------------------------------------------------------------------------------------------------

int TClookup(short Year, short Month, short Day, short hour, short minute, short tzin, short ttin,
	short * tzout, short * IllFlag)
{
	int checkday;
	unsigned int dt;
	int hmonthday, hourmin, backup;
	int tindex, initialindex;

	*IllFlag = 0;
	if (m_nChangeCount == 0 || m_pTimeIndex == NULL) return (-5);
	backup = 0;
	hmonthday = Month * 100 + Day;
	if (hour < 0)
	  {
		hour = 12;
		minute = 0;
	  }
	hourmin = hour * 100 + minute;
	dt = Year * 624000 + Month * 48000 + Day * 1500 + hour * 60 + minute;
	if (Year < 0) dt = 0;
	while (tzin > 12000)
	  {
	   initialindex = m_pTimeIndex[tzin - 12000];
		tzin = m_pTableValues[initialindex];
	   for (tindex = initialindex; tindex < m_nChangeCount; tindex++)
	     {
	      if (dt > m_pTableDates[tindex]) continue;     // lookup date later, next entry
	      if (dt < m_pTableDates[tindex])
			  {
				if (tindex > initialindex) tzin = m_pTableValues[tindex - 1];
				break;        // lookup date earlier: stop
			  }
	      tzin = m_pTableValues[tindex];							 // lookup date == change date
	      break;
	     }
	   if (tindex >= m_nChangeCount) return -1;  // ran out of entries
	  }
	*tzout = tzin;
	if (ttin == 0) return -1;
	if (ttin < 50) return ttin;
	if ((ttin == 2524 || ttin == 2526 || ttin == 2528 || ttin == 2529) &&
		dt >= 1245504000 && m_nVersion < 21) ttin = 30001;
	while (ttin >= 50)
	  {
		initialindex = m_pTimeIndex[ttin - 50];
      if (initialindex == 0) return -1;
		ttin = m_pTableValues[initialindex];
		if (ttin == 30004)
		  {
			*tzout = tzin = 5400;
			if ((Year >= 1916 && dt < 1211917620) || (dt >= 1214157120 && dt < 1222753500)) *IllFlag = -1;
			if (m_illinoisTreatment != 0)
			  {
				++initialindex;		// Clock time
			  }
			else	// Can't use Table 41--missing in 2.0 & 2.1
			  {
				if (dt < 1175547720)
				  {
					*tzout = 30000;
					return 4;	// before 1883 11/18
				  }
				if (dt < 1211917620) return 0;	// before 1942 2/9
				if (dt < 1214157120) return 2;	// before 1945 9/30
				if (dt < 1222753500) return 0;	// before 1959 7/1
				if (dt < 1222933620) return 1;	// before 1959 10/25
				if (dt < 1224523620)					// before 1962 4/29
				  {
					ttin = 59;
				  }
				else if (dt < 1227645120)					// before 1967 4/30
				  {
					ttin = 60;
				  }
				else ttin = 52;
			  }
		  }
		if (ttin == 30005) ++initialindex;
      if (dt < m_pTableDates[initialindex])
		  {
			*tzout = 30000;
			return 4;	// local mean time
		  }
	   for (tindex = initialindex; tindex < m_nChangeCount; tindex++)
		  {
		   if (dt > m_pTableDates[tindex]) continue;     // lookup date later, next entry
		   if (dt < m_pTableDates[tindex])
			  {
				if (tindex > initialindex) ttin = m_pTableValues[tindex - 1];
				break;        // lookup date earlier: stop
			  }
			ttin = m_pTableValues[tindex];                 // lookup date == change date
			break;
		  }
	   if (tindex >= m_nChangeCount) return -1;      /* ran out of entries */
	   if (ttin <= 50)
	     {
			if (ttin == 4)			/* Half hour DST shift */
	        {
				if (m_bEnableHalfDST) ttin = 5;
				else
				  {
					ttin = 0;			// pretend is Standard Time
					*tzout -= 450;		/* shift zone half an hour */
				  }
			  }
		   if (ttin == 6)
	        {
	         ttin = 0;
	         *tzout -= 300;      /* 20 minute shift */
	        }
	      if (ttin == 7)
	        {
				ttin = 0;
		      *tzout -= 600;      /* 40 minute shift */
		     }
			return ttin;
		  }
		if (ttin >= 30000)
		  {
			ttin -= 30000;
			if (ttin < 1 || ttin > 3) return -1;
			switch (ttin)
			  {
				case 2:				 /* Indiana */
					if (tzin == 4500) return(0);  /* Eastern zone is standard */
					if (tzin == 5400) return(1);  /* Central zone is daylight */
					return(0);
	         case 3:				 /* Indiana currently */
					if (tzin == 4500) return(0);  /* Eastern zone is standard */
	         case 1:				 /* USA default shift */
					if (Month < 4 || Month > 10) return(0);    /* standard */
					if (Month > 4 && Month < 10) return(1);    /* daylight */
					checkday = firstsunday(Month, Year);
					if (Month == 4)
					  {
						if (Year < 1987)		/* last Sunday instead of first */
						  {
							checkday += 28;
							if (checkday > 30) checkday -= 7;
						  }
						if (Day < checkday) return(0);
						else if (Day > checkday) return(1);
						else if (hourmin < 200) return(0);
						else return(1);
					  }
					else
					  {
						checkday += 28;
						if (checkday > 31) checkday -= 7;
						if (Day < checkday) return(1);
						else if (Day > checkday) return(0);
						else if (hourmin < 200) return(1);
						else return(0);
					  }
					break;
				default:
					return -1;	// should never get here: error
			  } /* end special cases switch */
		  }   /* end > 30000 if */
	  }      /* end while */
	return ttin;
}

//-------------------------------------------------------------------------------------------------------------------------

//short __declspec(dllexport) ACStimechangelookupA(short Month, short Day, short Year, short Hour, short Minute,
//	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPSTR ZoneName)
short ACStimechangelookupA(short Month, short Day, short Year, short Hour, short Minute,
	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPSTR ZoneName)
{
	int i, result, ntype;

	ZoneName[0] = '\0';
	result = TClookup(Year, Month, Day, Hour, Minute, tzin, ttin, tzout, flagout);

	ntype = result;
	if (ntype < 0 || ntype > 5) ntype = 0;
	if (result >= 0)
	  {
		*ttout = (short) result;
		result = 0;
	  }
	if (ntype == 4) strcpy(ZoneName, "LMT ");
	else
	  {
		for (i = 0; i < 47; i++) if (ZoneBounds[i] == *tzout)
		  {
			strcpy(ZoneName, zatnames[i][ntype]);
			return result;
		  }
		if (*tzout < 30000)
		  {
			sprintf(ZoneName, "%.2f", ((double) *tzout)/900.0);
		  }
	  }
	return result;
}

short _stdcall ZACStimechangelookupA(short Month, short Day, short Year, short Hour, short Minute,
	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPSTR ZoneName)
{
	return ACStimechangelookupA(Month, Day, Year, Hour, Minute, tzin, ttin, tzout, ttout, flagout, ZoneName);
}

//-------------------------------------------------------------------------------------------------------------------------

//short __declspec(dllexport) ACStimechangelookupW(short Month, short Day, short Year, short Hour, short Minute,
//	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPWSTR ZoneName)
short ACStimechangelookupW(short Month, short Day, short Year, short Hour, short Minute,
	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPWSTR ZoneName)
{
	char czn[10];
	int result;
	result = ACStimechangelookupA(Month, Day, Year, Hour, Minute, tzin, ttin, tzout, ttout, flagout, czn);
	ZoneName[0] = 0;
	if (result < 0) return result;
	Widen(ZoneName, czn);
	return result;
}

short _stdcall ZACStimechangelookupW(short Month, short Day, short Year, short Hour, short Minute,
	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPWSTR ZoneName)
{
	return ACStimechangelookupW( Month, Day, Year, Hour, Minute, tzin, ttin, tzout, ttout, flagout, ZoneName);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) GetZoneCount()
int GetZoneCount()
{
	return 47;
}

int _stdcall ZGetZoneCount()
{
	return 47;
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) GetZoneNameAtIndexA(LPSTR zonename, int zindex)
int GetZoneNameAtIndexA(LPSTR zonename, int zindex)
{
	if (zindex < 0 || zindex >= 47)
	  {
		zonename[0] = '\0';
		return 30000;
	  }
	strcpy(zonename, ZoneNames[zindex]);
	return ZoneBounds[zindex];
}

int _stdcall ZGetZoneNameAtIndexA(LPSTR zonename, int zindex)
{
	return GetZoneNameAtIndexA(zonename, zindex);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) GetZoneNameAtIndexW(LPWSTR zonename, int zindex)
int GetZoneNameAtIndexW(LPWSTR zonename, int zindex)
{
	if (zindex < 0 || zindex >= 47)
	  {
		zonename[0] = 0;
		return 30000;
	  }
	Widen(zonename, ZoneNames[zindex]);
	return ZoneBounds[zindex];
}

int _stdcall ZGetZoneNameAtIndexW(LPWSTR zonename, int zindex)
{
	return GetZoneNameAtIndexW(zonename, zindex);
}

//-------------------------------------------------------------------------------------------------------------------------

//void __declspec(dllexport) GetZoneNameA(LPSTR zname, int zone)
void GetZoneNameA(LPSTR zname, int zone)
{
	int i;
	if (zone >= 30000)
	  {
		zname[0] = '\0';
		return;
	  }
	for (i = 0; i < 47; i++) if (ZoneBounds[i] == zone)
	  {
		strcpy(zname, ZoneNames[i]);
		return;
	  }
	sprintf(zname, "%.2f", ((double) zone)/900.0);
}

void _stdcall ZGetZoneNameA(LPSTR zname, int zone)
{
	GetZoneNameA(zname, zone);
}

//-------------------------------------------------------------------------------------------------------------------------

//void __declspec(dllexport) GetZoneNameW(LPWSTR wzname, int zone)
void GetZoneNameW(LPWSTR wzname, int zone)
{
	char zname[60];
	GetZoneNameA(zname, zone);
	Widen(wzname, zname);
}

void _stdcall ZGetZoneNameW(LPWSTR wzname, int zone)
{
	char zname[60];
	GetZoneNameA(zname, zone);
	Widen(wzname, zname);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) GetZoneFromNameA(LPCSTR zonename)
int GetZoneFromNameA(LPCSTR zonename)
{
	int i;
	double tz;

	for (i = 0; i < 47; i++) if (stricmp(ZoneNames[i], zonename) == 0) return ZoneBounds[i];
	if (isdigit(zonename[0]) || zonename[0] == '-')
	  {
		tz = atof(zonename)/900.0;
		if (tz < 0.0) tz -= 0.5;
		else tz += 0.5;
		return ((int) tz);
	  }
	return 30000;
}

int _stdcall ZGetZoneFromNameA(LPCSTR zonename)
{
	return GetZoneFromNameA(zonename);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) GetZoneFromNameW(LPCWSTR zonename)
int GetZoneFromNameW(LPCWSTR zonename)
{
	char zname[36];
	Narrow(zname, zonename);
	return GetZoneFromNameA(zname);
}

int _stdcall ZGetZoneFromNameW(LPCWSTR zonename)
{
	char zname[36];
	Narrow(zname, zonename);
	return GetZoneFromNameA(zname);
}

//-------------------------------------------------------------------------------------------------------------------------

//void __declspec(dllexport) GetZoneTypeAbbreviationA(LPSTR ztabbr, int zone, int type)
void GetZoneTypeAbbreviationA(LPSTR ztabbr, int zone, int type)
{
	int i;

	if (type < 0 || type > 5) type = 0;
	if (type == 4) strcpy(ztabbr, "LMT ");
	else
	  {
		ztabbr[0] = '\0';;
		for (i = 0; i < 47; i++) if (ZoneBounds[i] == zone)
		  {
			strcpy(ztabbr, zatnames[i][type]);
			return;
		  }
		if (zone < 30000)
		  {
			sprintf(ztabbr, "%.2f", ((double) zone)/900.0);
		  }
	  }
}

void _stdcall ZGetZoneTypeAbbreviationA(LPSTR ztabbr, int zone, int type)
{
	GetZoneTypeAbbreviationA(ztabbr, zone, type);
}

//-------------------------------------------------------------------------------------------------------------------------

//void __declspec(dllexport) GetZoneTypeAbbreviationW(LPWSTR wztabbr, int zone, int type)
void GetZoneTypeAbbreviationW(LPWSTR wztabbr, int zone, int type)
{
	char ztabbr[8];
	GetZoneTypeAbbreviationA(ztabbr, zone, type);
	Widen(wztabbr, ztabbr);
}

void _stdcall ZGetZoneTypeAbbreviationW(LPWSTR wztabbr, int zone, int type)
{
	char ztabbr[8];
	GetZoneTypeAbbreviationA(ztabbr, zone, type);
	Widen(wztabbr, ztabbr);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) BuildChangesTable(int tzin, int ttin, int endyear, int bGMT, int bExpandUS)
int BuildChangesTable(int tzin, int ttin, int endyear, int bGMT, int bExpandUS)
{
	int expanded, firstin, Mexico;
	int initialindex;
	int dt, tindex, lastzone, lasttype, startdt, enddt;
	int size, iat, ith, ctr, ct;
	int year, month, day, hm, checkday, adjust, lastadj, type, dmax;

	Mexico = ((ttin == 2524 || ttin == 2526 || ttin == 2528 || ttin == 2529) && m_nVersion < 21);
	if (bGMT > 0) bExpandUS = 15;
	size = 1;
	m_nChangesTable[0][0] = 0;
	m_nChangesTable[0][1] = tzin;
	m_nChangesTable[0][2] = ttin;
	do
	  {
		expanded = FALSE;
		for (iat = 0; iat < size; iat++)
		  {
			ith = iat;
			if (m_nChangesTable[ith][1] < 12000) continue;
			firstin = 15;
			lastzone = 30000;
			startdt = m_nChangesTable[ith][0];
			if (iat + 1 < size) enddt = m_nChangesTable[ith + 1][0];
			else enddt = 2147483647;
			initialindex = m_pTimeIndex[m_nChangesTable[ith][1] - 12000];
			for (tindex = initialindex; tindex < m_nChangeCount; tindex++)
			  {
				dt = m_pTableDates[tindex];
				if (dt >= enddt) break;
				tzin = m_pTableValues[tindex];
				if (dt < startdt)
				  {
					lastzone = tzin;
					continue;
				  }
				expanded = TRUE;
				if (firstin)
				  {
					firstin = FALSE;
					if (startdt == 0)
					  {
						startdt = dt;
						m_nChangesTable[ith][0] = dt;
					  }
					if (dt == startdt)
					  {
						m_nChangesTable[ith][1] = tzin;
						++ith;
						continue;
					  }
					if (lastzone == 30000)
					  {
						lastzone = tzin;
					  }
					m_nChangesTable[ith][1] = lastzone;	// put last zone at start date of table
					m_nChangesTable[ith][2] = ttin;
					++ith;
				  }
				for (ctr = size; ctr > ith; ctr--)
				  {
					m_nChangesTable[ctr][0] = m_nChangesTable[ctr - 1][0];
					m_nChangesTable[ctr][1] = m_nChangesTable[ctr - 1][1];
					m_nChangesTable[ctr][2] = m_nChangesTable[ctr - 1][2];
				  }
				m_nChangesTable[ith][0] = dt;
				m_nChangesTable[ith][1] = tzin;
				m_nChangesTable[ith][2] = ttin;
				++ith;
				++size;
			  }
			if (expanded) break;		// indices messed up--start from beginning again
		  }
	  } while (expanded);
	do
	  {
		expanded = FALSE;
		for (iat = 0; iat < size; iat++)
		  {
			ith = iat;
			lastzone = m_nChangesTable[ith][1];
			ct = m_nChangesTable[ith][2];
			if (ct < 50 || ct == 30000) continue;
			firstin = TRUE;
			lasttype = 30000;
			startdt = m_nChangesTable[ith][0];
			if (iat + 1 < size) enddt = m_nChangesTable[ith + 1][0];
			else enddt = 2147483647;
			if (ct > 30000)
			  {
				continue;
			  }
			initialindex = m_pTimeIndex[m_nChangesTable[ith][2] - 50];
			for (tindex = initialindex; tindex < m_nChangeCount; tindex++)
			  {
				dt = m_pTableDates[tindex];
				if (dt >= enddt) break;
				ttin = m_pTableValues[tindex];
				if (ttin == 30004 || ttin == 30005) continue;
				if (ttin == 30002)		// Indiana
				  {
					if (lastzone == 5400) ttin = 1;	// if CST, daylight
					else ttin = 0;							// otherwise, standard
				  }
				if (ttin == 30003)		// modern Indiana
				  {
					if (lastzone == 4500) ttin = 0;	// for EST, standard time
					else ttin = 10;						// for CST, US #1
				  }
				if (ttin == 30001) ttin = 10;	// US standard shift
				if (dt < startdt)
				  {
					lasttype = ttin;
					continue;
				  }
				expanded = TRUE;
				if (firstin)
				  {
					firstin = FALSE;
					if (startdt == 0)
					  {
						startdt = dt;
						m_nChangesTable[ith][0] = dt;
					  }
					if (dt == startdt)
					  {
						m_nChangesTable[ith][1] = lastzone;
						m_nChangesTable[ith][2] = ttin;
						++ith;
						continue;
					  }
					if (lasttype == 30000)
					  {
						if (ith == 0) lasttype = 0;
						else
						  {
							lasttype = m_nChangesTable[ith - 1][2];
							if (lasttype > 50)
							  {
								lasttype = ttin;
							  }
						  }
					  }
					m_nChangesTable[ith][1] = lastzone;
					m_nChangesTable[ith][2] = lasttype;	// put last type at start date of table
					++ith;
				  }
				if (ith > 0 && lastzone == m_nChangesTable[ith - 1][1] &&
					 ttin == m_nChangesTable[ith - 1][2]) continue;	// same as prev--omit
				for (ctr = size; ctr > ith; ctr--)
				  {
					m_nChangesTable[ctr][0] = m_nChangesTable[ctr - 1][0];
					m_nChangesTable[ctr][1] = m_nChangesTable[ctr - 1][1];
					m_nChangesTable[ctr][2] = m_nChangesTable[ctr - 1][2];
				  }
				m_nChangesTable[ith][0] = dt;
				m_nChangesTable[ith][1] = lastzone;
				m_nChangesTable[ith][2] = ttin;
				++ith;
				++size;
			  }
			if (firstin)	// ran through table without finding date in range
			  {
				m_nChangesTable[ith][2] = lasttype;
				expanded = TRUE;
			  }
			if (expanded) break;		// indices messed up--start from beginning again
		  }
	  } while (expanded);
	if (Mexico)	// add US #1 at end for 2.0
	  {
		m_nChangesTable[size][1] = m_nChangesTable[ith - 1][1];
		m_nChangesTable[size][2] = 10;
		++size;
	  }
// Convert 20 minute, 30 minute, and 40 minute DST shifts.  Remove duplicate entries.
	for (iat = 0; iat < size; iat++)
	  {
		ith = iat;
		lastzone = m_nChangesTable[ith][1];
		lasttype = m_nChangesTable[ith][2];
		if (lastzone == 30000 || lasttype == 30000)
		{
			for (ctr = ith; ctr + 1 < size; ctr++)
			{
				m_nChangesTable[ctr][0] = m_nChangesTable[ctr + 1][0];
				m_nChangesTable[ctr][1] = m_nChangesTable[ctr + 1][1];
				m_nChangesTable[ctr][2] = m_nChangesTable[ctr + 1][2];
			}
			--size;
			--iat;
			continue;
		}
		switch (lasttype)
		  {
			case 4:
				if (m_bEnableHalfDST) lasttype = 5;
				else
				  {
					lastzone -= 450;	// 30 minutes
					lasttype = 0;
				  }
				m_nChangesTable[ith][1] = lastzone;
				m_nChangesTable[ith][2] = lasttype;
				break;
			case 7:
				lastzone -= 300;	// for a total of 600 (40 minutes)
			case 6:
				lastzone -= 300;
				lasttype = 0;
				m_nChangesTable[ith][1] = lastzone;
				m_nChangesTable[ith][2] = 0;
				break;
			default:
				break;
		  }
		if (ith > 0 && lastzone == m_nChangesTable[ith - 1][1] && lasttype == m_nChangesTable[ith - 1][2])
		  {
			for (ctr = ith; ctr + 1 < size; ctr++)
			  {
				m_nChangesTable[ctr][0] = m_nChangesTable[ctr + 1][0];
				m_nChangesTable[ctr][1] = m_nChangesTable[ctr + 1][1];
				m_nChangesTable[ctr][2] = m_nChangesTable[ctr + 1][2];
			  }
			--size;
			--iat;
		  }
	  }
// expand US table to actual dates
	if (bExpandUS) do
	  {
		expanded = FALSE;
		for (iat = 0; iat < size; iat++)
		  {
			ith = iat;
			if (m_nChangesTable[ith][2] != 10) continue;
			dt = m_nChangesTable[ith][0];
			year = dt/624000;
			if (year > endyear) break;
			month = (dt % 624000)/48000;
			day = (dt % 48000)/1500;
			hm = dt % 1500;
			if (iat + 1 < size) enddt = m_nChangesTable[ith + 1][0];
			else enddt = 2147483647;
			if (month < 4)
			  {
				ttin = 0;
				month = 4;
			  }
			else if (month == 4)
			  {
				checkday = firstsunday(month,year);
				if (year < 1987)		/* last Sunday instead of first */
				  {
					checkday += 28;
					if (checkday > 30) checkday -= 7;
				  }
				if (day < checkday) ttin = 0;
				else if (day > checkday) ttin = 1;
				else if (hm < 120) ttin = 0;
				else ttin = 1;
				if (ttin == 1) month = 10;
			  }
			else if (month < 10)
			  {
				ttin = 1;
				month = 10;
			  }
			else if (month == 10)
			  {
				checkday += 28;
				if (checkday > 31) checkday -= 7;
				if (day < checkday) ttin = 1;
				else if (day > checkday) ttin = 0;
				else if (hm < 120) ttin = 1;
				else ttin = 0;
				if (ttin == 0)
				  {
					month = 4;
					++year;
				  }
			  }
			else
			  {
				ttin = 0;
				month = 4;
				++year;
			  }
			m_nChangesTable[ith][2] = ttin;
			while (year <= endyear)
			  {
				dt = year * 624000 + month * 48000;
				day = firstsunday(month,year);
				if (month == 4)
				  {
					if (year < 1987)		/* last Sunday instead of first */
					  {
						day += 28;
						if (day > 30) day -= 7;
					  }
					ttin = 1;
					month = 10;
				  }
				else
				  {
					day += 28;
					if (day > 31) day -= 7;
					ttin = 0;
					month = 4;
					++year;
				  }
				dt += day * 1500 + 120;
				if (dt >= enddt) break;
				expanded = TRUE;
				ith++;
				for (ctr = size; ctr > ith; ctr--)
				  {
					m_nChangesTable[ctr][0] = m_nChangesTable[ctr - 1][0];
					m_nChangesTable[ctr][1] = m_nChangesTable[ctr - 1][1];
					m_nChangesTable[ctr][2] = m_nChangesTable[ctr - 1][2];
				  }
				m_nChangesTable[ith][0] = dt;
				m_nChangesTable[ith][1] = m_nChangesTable[ith - 1][1];
				m_nChangesTable[ith][2] = ttin;
				++size;
				if (size >= MAX_CHANGES_TABLE_LENGTH)
				  {
					expanded = FALSE;
					break;
				  }
			  }
			break;
		  }
	  } while (expanded);
	if (bGMT)
	  {
		lastadj = m_nChangesTable[0][1]/15;	// minutes from Greenwich
		type = m_nChangesTable[0][2];
		if (type == 1 || type == 2) lastadj -= 60;
		if (type == 3) lastadj -= 120;
		if (type == 5) lastadj -= 30;
		for (ctr = 0; ctr < size; ctr++)
		  {
			dt = m_nChangesTable[ctr][0];
			year = dt/624000;
			dt %= 624000;
			month = dt/48000;
			dt %= 48000;
			day = dt/1500;
			hm = dt%1500;
			adjust = m_nChangesTable[ctr][1]/15;	// minutes from Greenwich
			type = m_nChangesTable[ctr][2];
			if (type == 1 || type == 2) adjust -= 60;
			if (type == 3) adjust -= 120;
			if (type == 5) adjust -= 30;
			hm += lastadj;
			lastadj = adjust;
			if (hm >= 1440)
			  {
				hm -= 1440;
				++day;
				dmax = MonthLength[month - 1];
				if (dmax == 28 && isleapyear(year)) dmax = 29;
				if (day > dmax)
				  {
					day = 1;
					++month;
					if (month > 12)
					  {
						month = 1;
						++year;
					  }
				  }
			  }
			if (hm < 0)
			  {
				hm += 1440;
				day--;
				if (day < 1)
				  {
					--month;
					if (month < 1)
					  {
						--year;
						month = 12;
					  }
					day = MonthLength[month - 1];
					if (day == 28 && isleapyear(year)) day = 29;
				  }
			  }
			m_nChangesTable[ctr][0] = year * 624000 + month * 48000 + day * 1500 + hm;
		  }
	  }
	for (ctr = 0; ctr < size; ctr++)
	  {
		year = m_nChangesTable[ctr][0]/624000;
		if (year > endyear) break;
	  }
	m_nNumberOfChanges = ctr;
	return (m_nNumberOfChanges);
}

int _stdcall ZBuildChangesTable(int tzin, int ttin, int endyear, int bGMT, int bExpandUS)
{
	return BuildChangesTable(tzin, ttin, endyear, bGMT, bExpandUS);
}

//-------------------------------------------------------------------------------------------------------------------------
// Build the table of time changes for the city; return the length up to and including the end year
// bGMT non-zero says return change dates for GMT, default is local time.
// -1 = nsoc out of range
// -2 = ncity out of range

//int __declspec(dllexport) MakeChangesTable(int nsoc, int ncity, int endyear, int bGMT, int bExpandUS)
int MakeChangesTable(int nsoc, int ncity, int endyear, int bGMT, int bExpandUS)
{
	int len;
	int datafields[6];

	if (nsoc < 0 || nsoc >= m_nEntries) return -1;
	if (ncity < 0 || ncity >= m_nCounts[nsoc]) return -2;
	if (m_nCurrentCountry != nsoc) SetCountryByIndex(nsoc);
	GetDataFields(ncity, datafields);
	len = BuildChangesTable(datafields[4], datafields[3], endyear, bGMT, bExpandUS);
	return len;
}

int _stdcall ZMakeChangesTable(int nsoc, int ncity, int endyear, int bGMT, int bExpandUS)
{
	return MakeChangesTable(nsoc, ncity, endyear, bGMT, bExpandUS);
}

//-------------------------------------------------------------------------------------------------------------------------
// Return an entry in the table of time changes for the city
// -3 = change table index out of range

//int __declspec(dllexport) GetChangeAtIndex(int tcindex, int * zone, int * type)
int GetChangeAtIndex(int tcindex, int * zone, int * type)
{
	if (tcindex < 0 || tcindex >= m_nNumberOfChanges) return -3;
	*zone = m_nChangesTable[tcindex][1];
	*type = m_nChangesTable[tcindex][2];
	return m_nChangesTable[tcindex][0];
}

int _stdcall ZGetChangeAtIndex(int tcindex, int * zone, int * type)
{
	return GetChangeAtIndex(tcindex, zone, type);
}

//-------------------------------------------------------------------------------------------------------------------------

void SoundexString(const char * scity, char * sxstr)
{
	char c, prev, curr;
	int i, j, len;

	len = strlen(scity);
	sxstr[0] = m_pUpcase[scity[0]];
	prev = 0;
	j = 1;
	for (i = 1; i < len && j < 4; i++)
	  {
		c = m_pUpcase[scity[i]];
		if (c >= 'A' && c <= 'Z')
		  {
			curr = soundexnum[c - 'A'];
			if (curr != '0' && curr != prev)
			  {
				sxstr[j++] = curr;
				prev = curr;
			  }
		  }
	  }
	for (i = j; i < 4; i++) sxstr[i] = '0';
	sxstr[4] = '\0';
}

// Return handle for Soundex lookup, and first city matching soundex of placename
// return value < 0 is error:
// -1 no soundex match
// -2 invalid country name
// -3 missing country, and no Personal Atlas found
// -5 path not set: data files not found

//short __declspec(dllexport) SoundexFirstCityA(LPCSTR placename, LPSTR city, LPSTR county)
short SoundexFirstCityA(LPCSTR placename, LPSTR city, LPSTR county)
{
	char sxstr[5], testcity[61];
	short lasthandle, ncounty;
	int lowrec, highrec, middle;
	int way, i, nf, len;
	char * cp;
	struct SoundexInfo * psi, * pnewsi;
	char place[101], country[61];

	city[0] = county[0] = '\0';
	if (m_nEntries == 0) return (-5);
	strcpy(place, placename);
	len = strlen(place);
	while (len > 0 && place[len - 1] == ' ') --len;
	place[len] = '\0';
// split character string in placename into city, possible county, & state
//	or country.  fields are separated by vertical bars
	cp = strchr(place, '|');
	if (cp == NULL)
	  {
		if (m_pPersonal == NULL) return (-3);
		m_nCurrentCountry = m_nEntries - 1;
		strcpy(city, place);
		county[0] = '\0';
	  }
	else
	  {
		i = cp - place;
		while (i > 0 && place[i - 1] == ' ') --i;
		place[i] = '\0';
		strcpy(city, place);
		++cp;
		while (*cp == ' ') ++cp;
		strcpy(country, cp);
	  }
	i = SetCountryByNameA(country);
	if (i < 0) return -2;
//	pnewsi = new SoundexInfo;		// go ahead and allocate now
	pnewsi = (struct SoundexInfo *)malloc(sizeof(struct SoundexInfo));		// go ahead and allocate now
	if (m_pSoundexList == NULL)
	  {
		psi = m_pSoundexList = pnewsi;
		pnewsi->handle = 1;
	  }
	else
	  {
		for (psi = m_pSoundexList; psi->pnext != NULL; psi = psi->pnext) lasthandle = psi->handle;
		psi->pnext = pnewsi;
		pnewsi->handle = lasthandle + 1;
	  }
	pnewsi->pnext = NULL;
	SoundexString(city, pnewsi->match);
	nf = m_nFileIndex[m_nCurrentCountry];
	pnewsi->nsoc = m_nCurrentCountry;
// find a record starting with the first letter of the input city
	lowrec = 0;
	highrec = m_nCounts[m_nCurrentCountry] - 1;
	while (lowrec <= highrec)
	  {
		middle = (lowrec + highrec)/2;
	   GetCityForLookup(testcity, middle);
		way = pnewsi->match[0] - m_pUpcase[testcity[0]];
		if (way == 0) break;
	   if (way < 0) highrec = middle - 1;
	   else lowrec = middle + 1;
	  }
	if (way != 0)		// no cities starting with that letter
	  {
		psi->pnext = NULL;
		if (psi == m_pSoundexList) m_pSoundexList = NULL;
//		delete pnewsi;
		free(pnewsi);
		return -1;
	  }
// scan back through file for first city starting with key letter
	for (--middle; middle >= lowrec; middle--)
	  {
	   GetCityForLookup(testcity, middle);
		if (pnewsi->match[0] != m_pUpcase[testcity[0]]) break;
	  }
// read records until find soundex match
	for (++middle; middle <= highrec; middle++)
	  {
	   GetCityForLookup(testcity, middle);
		SoundexString(testcity, sxstr);
		if (strcmp(sxstr, pnewsi->match) == 0) break;
	  }
	if (middle > highrec)	// oops, no soundex match found
	  {
		psi->pnext = NULL;
		if (psi == m_pSoundexList) m_pSoundexList = NULL;
//		delete pnewsi;
		free(pnewsi);
		return -1;
	  }
// success
	if (nf < 4)
	  {
		if (m_nVersion >= 20) fgetc(m_fpData[nf]);	// skip dupflag
		fread(&ncounty, 2, 1, m_fpData[nf]);
		ncounty %= 1000;
		pnewsi->FileOffset = ftell(m_fpData[nf]);
		if (pnewsi->FileOffset < 1)
		  {
			pnewsi->FileOffset = m_CityPointers[m_nCityPtrIndex[m_nCurrentCountry] + middle] + strlen(testcity) + 1;
		  }
		pnewsi->FileOffset += (m_nVersion < 20) ? 10 : 12;
		strcpy(city, testcity);
		GetCountyNameFromNumberA(county, ncounty);
	  }
	else
	  {
		pnewsi->FileOffset = middle + 1;
		county[0] = '\0';	// no counties in Personal Atlas
	  }

	return pnewsi->handle;
}

short _stdcall ZSoundexFirstCityA(LPCSTR placename, LPSTR city, LPSTR county)
{
	return SoundexFirstCityA(placename, city, county);
}

//short __declspec(dllexport) SoundexFirstCityW(LPCWSTR placename, LPWSTR city, LPWSTR county)
short SoundexFirstCityW(LPCWSTR placename, LPWSTR city, LPWSTR county)
{
	short int result;
	char place[255], ccity[61], ccounty[61];

	Narrow(place, placename);
	result = SoundexFirstCityA(place, ccity, ccounty);
	Widen(city, ccity);
	Widen(county, ccounty);
	return result;
}

short _stdcall ZSoundexFirstCityW(LPCWSTR placename, LPWSTR city, LPWSTR county)
{
	return SoundexFirstCityW(placename, city, county);
}

// Return next city matching soundex value of handle
// return values < 0 are errors:
// -6 invalid handle number
// -1 no more soundex matches

//short __declspec(dllexport) SoundexNextCityA(short handle, LPSTR city, LPSTR county)
short SoundexNextCityA(short handle, LPSTR city, LPSTR county)
{
	int clen;
	char sxstr[5], citybuf[80];
	int nf, sl, nread, datlen, nsoc;
	short int ncounty;
	struct SoundexInfo * psi;

	city[0] = county[0] = '\0';
	for (psi = m_pSoundexList; psi != NULL; psi = psi->pnext) if (psi->handle == handle) break;
	if (psi == NULL) return -6;	// no match for that handle
	nsoc = psi->nsoc;
	nf = m_nFileIndex[nsoc];
	datlen = (m_nVersion < 20) ? 12 : 15;
	if (nf == 4)
	  {
		for (nread = psi->FileOffset; nread < m_nCounts[nsoc]; nread++)
		  {
			SoundexString(m_pPersonal[nread]->Name, sxstr);
			if (strcmp(sxstr, psi->match) == 0)
			  {
				strcpy(city, m_pPersonal[nread]->Name);
				county[0] = '\0';
				psi->FileOffset = nread + 1;
				return 1;
			  }
		  }
		return -1;
	  }
	else
	  {
		fseek(m_fpData[nf], psi->FileOffset, SEEK_SET);
		while (1)
		  {
			if (m_nVersion < 20)
			  {
				clen = CityNameLen[nf];
				nread = fread(citybuf, 1, clen, m_fpData[nf]);
				if (nread < clen) return -1;	// probably end of file
				for (sl = clen; sl > 0; sl--) if (citybuf[sl - 1] != ' ') break;
				citybuf[sl] = '\0';
			  }
			else
			  {
				clen = fgetc(m_fpData[nf]);
				if (clen < 1) return -1;
				nread = fread(citybuf, 1, clen + 1, m_fpData[nf]);
                                // New GPD 24 May 2005
                                ProcessAtlasString(citybuf, clen);
                                // End GPD
				if (nread < clen + 1) return -1;
			  }
			if (m_pUpcase[citybuf[0]] != psi->match[0]) return -1;	// no more cities starting with key letter
			SoundexString(citybuf, sxstr);
			if (strcmp(sxstr, psi->match) == 0) break;
			fseek(m_fpData[nf], datlen, SEEK_CUR);
		  }
		psi->FileOffset = ftell(m_fpData[nf]);
		if (m_nVersion >= 20) fgetc(m_fpData[nf]);	// skip dupflag
		fread(&ncounty, 2, 1, m_fpData[nf]);
		ncounty %= 1000;
		psi->FileOffset += datlen;
		strcpy(city, citybuf);
		CountyFromFile(nsoc, ncounty, county);
	  }

	return 1;
}

short _stdcall ZSoundexNextCityA(short handle, LPSTR city, LPSTR county)
{
	return SoundexNextCityA(handle, city, county);
}

//short __declspec(dllexport) SoundexNextCityW(short handle, LPWSTR city, LPWSTR county)
short SoundexNextCityW(short handle, LPWSTR city, LPWSTR county)
{
	short int result;
	char ccity[61], ccounty[61];

	result = SoundexNextCityA(handle, ccity, ccounty);
	Widen(city, ccity);
	Widen(county, ccounty);
	return result;
}

short _stdcall ZSoundexNextCityW(short handle, LPWSTR city, LPWSTR county)
{
	return SoundexNextCityW(handle, city, county);
}

//short __declspec(dllexport) SoundexClose(short handle)
short SoundexClose(short handle)
{
	struct SoundexInfo * psi, * plast;

	plast = NULL;
	for (psi = m_pSoundexList; psi != NULL; psi = psi->pnext)
	  {
		if (psi->handle == handle)
		  {
			if (plast == NULL)
			  {
				m_pSoundexList = psi->pnext;
//				delete psi;
				free(psi);
			  }
			else
			  {
				plast->pnext = psi->pnext;
//				delete psi;
				free(psi);
			  }
			return 1;
		  }
		plast = psi;
	  }
	return -1;
}

short _stdcall ZSoundexClose(short handle)
{
	return SoundexClose(handle);
}
