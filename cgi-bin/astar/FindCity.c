// FindCity.cpp : implementation file for FindCity DLL
//

#include "FindCity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#if defined __unix__ || defined __APPLE__
#include <string.h>
#include <stdlib.h>
#endif

HANDLE hDLL;		// save module handle in case need to access resources
extern int m_nVersion;
extern int m_nStates, m_nEntries, m_nAlternate;
extern int m_nCurrentCountry;
extern int m_nTimeRecLen, m_nChangeCount;
int m_returnAbbreviations;
int m_bEnableHalfDST;
int m_illinoisTreatment;
int m_bAlwaysReturnDupFlag;
extern char * m_pUpcase;
extern FILE * m_fpData[4];
extern int m_nNumberOfChanges;

extern char m_Abbrevs[MAXENTRIES][7];
extern char m_Names[MAXENTRIES][61];
extern char m_AltAbbrevs[MAXALTERNATES][7];
extern char m_Alternates[MAXALTERNATES][61];

extern unsigned int m_nCountyLows[MAXENTRIES];
extern unsigned int m_nCountyHighs[MAXENTRIES];
extern unsigned int m_nCountyPointers[MAXENTRIES];
extern unsigned int m_nStarts[MAXENTRIES];	// city pointers
extern int m_nCounts[MAXENTRIES];	// city counts
extern unsigned char m_nFileIndex[MAXENTRIES];
extern unsigned short int m_nAlternateIndex[MAXALTERNATES];
extern unsigned short int * m_pTimeIndex;
extern unsigned int * m_pTableDates;
extern short int * m_pTableValues;
extern unsigned int * m_CityPointers;
extern char * m_pCounties;
extern struct CityInfo ** m_pPersonal;
struct SoundexInfo * m_pSoundexList;

extern short int wuni[256];
extern char ununi[1024];
extern char punctuation[208];

// New variables for encrypted atlas - GPD 17 May 2005
char m_AtlasFileNames[3][80]; // US Atlas; Int Atlas; TTables
char m_EncryptionString[80];
int m_bIsEncrypted;
// End GPD

// SetPaths is called with two character strings, one for the main Atlas data files, the second for
// the personal atlas file.  Return codes:
//  0 - error
// 10 - Version 1 data files found & read
// 20 or greater - Version 2 data files found & read

// New routine - GPD 17 May 2005
int SetPathsEmbeddedA(LPCSTR pMainFiles, LPCSTR pPersonal,
                                 LPCSTR pAtlasFileUS, LPCSTR pAtlasFileInt, LPCSTR pAtlasFileTT,
                                 LPCSTR pEncryptionString)
{
        char filename[256];
        int nv, fnlen;
        HANDLE fh;
        WIN32_FIND_DATA fdata;

        CloseAtlasFiles();
        m_nEntries = m_nStates = 0;
//	for (i = 0; i < 4; i++) if (m_fpData[i] != NULL) fclose(m_fpData[i]);
//	if (m_pTimeIndex != NULL)
//	  {
//		free(m_pTimeIndex);
                m_pTimeIndex = NULL;
//	  }
//	if (m_pTableDates != NULL)
//	  {
//		free(m_pTableDates);
                m_pTableDates = NULL;
//	  }
//	if (m_pTableValues != NULL)
//	  {
//		free(m_pTableValues);
				m_pTableValues = NULL;
//	  }
        m_nCurrentCountry = -1;

        fnlen = strlen(pAtlasFileUS);
        if (fnlen > 0) strcpy(m_AtlasFileNames[0], pAtlasFileUS);
        else strcpy(m_AtlasFileNames[0], "acsua.dat");

        fnlen = strlen(pAtlasFileInt);
        if (fnlen > 0) strcpy(m_AtlasFileNames[1], pAtlasFileInt);
        else strcpy(m_AtlasFileNames[1], "acsia.dat");

        fnlen = strlen(pAtlasFileTT);
        if (fnlen > 0) strcpy(m_AtlasFileNames[2], pAtlasFileTT);
        else strcpy(m_AtlasFileNames[2], "acstt.dat");

        fnlen = strlen(pEncryptionString);
        if (fnlen > 0)
        {
          strcpy(m_EncryptionString, pEncryptionString);
          m_bIsEncrypted = 1;
        }
        else
        {
          m_bIsEncrypted = 0;
        }

        strcpy(filename, pMainFiles);
        fnlen = strlen(filename);
				if (fnlen > 0 && filename[fnlen - 1] != PATH_DELIMITER) filename[fnlen++] = PATH_DELIMITER;

        // Split generic search into two separately named files - GPD 17 May 2005
        //strcpy(filename + fnlen, "acsua.dat");
        //fh = FindFirstFile(filename, &fdata);
        strcpy(filename + fnlen, m_AtlasFileNames[0]);
        fh = FindFirstFile(filename, &fdata);
        if (fh == INVALID_HANDLE_VALUE)
        {
          strcpy(filename + fnlen, m_AtlasFileNames[1]);
          fh = FindFirstFile(filename, &fdata);
        }
        // End GPD
        if (fh == INVALID_HANDLE_VALUE)
          {
                strcpy(filename + fnlen, "timndx.bin");
                fh = FindFirstFile(filename, &fdata);
                filename[fnlen] = '\0';
                if (fh == INVALID_HANDLE_VALUE)
                  {
                        nv = FindAtlasFiles(filename);
                        if (nv == 0) return 0;				// not in any of the usual places
                  }
                else
                  {
                        FindClose(fh);
                        nv = 10;
                  }
          }
				else						
          {
                FindClose(fh);
                filename[fnlen] = '\0';
                nv = 20;
          }

        m_nVersion = nv;
        if (nv == 20) ReadVersion2(filename);
        else ReadVersion1(filename);
        if (strlen(pPersonal) > 0) ReadPersonal(pPersonal);
        else ReadPersonal(filename);

        return m_nVersion;
}

int _stdcall ZSetPathsEmbeddedA(LPCSTR pMainFiles, LPCSTR pPersonal,
                                           LPCSTR pAtlasFileUS, LPCSTR pAtlasFileInt, LPCSTR pAtlasFileTT,
                                           LPCSTR pEncryptionString)
{
        return SetPathsEmbeddedA(pMainFiles, pPersonal, pAtlasFileUS, pAtlasFileInt, pAtlasFileTT, pEncryptionString);
}

//int __declspec(dllexport) SetPathsA(LPCSTR pMainFiles, LPCSTR pPersonal)
int SetPathsA(LPCSTR pMainFiles, LPCSTR pPersonal)
{
        return SetPathsEmbeddedA(pMainFiles, pPersonal, "", "", "", "");
}

int _stdcall ZSetPathsA(LPCSTR pMainFiles, LPCSTR pPersonal)
{
	return SetPathsA(pMainFiles, pPersonal);
}

#ifdef _WIN32 
/* entry point for DLL loading and unloading */
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
{
	switch (dwFunction)
	  {
		case DLL_PROCESS_ATTACH:
			hDLL = hModule;
			InitializeAtlasVariables();
			SetPathsA("", "");
			break;
		case DLL_PROCESS_DETACH:
			CloseAtlasFiles();
		default:
			break;
	  }

	return TRUE;
}
#endif

void InitializeAtlasVariables()
{
	int ctr;

	m_nStates = m_nEntries = m_nAlternate = 0;
	m_illinoisTreatment = m_nChangeCount = 0;
	m_bEnableHalfDST = 0;
	m_nNumberOfChanges = 0;
	m_nCurrentCountry = -1;
	m_returnAbbreviations = 0;
	for (ctr = 0; ctr < 4; ctr++) m_fpData[ctr] = NULL;
	m_pTimeIndex = NULL;
	m_pTableDates = NULL;
	m_pTableValues = NULL;
	m_pCounties = NULL;
	m_CityPointers = NULL;
	m_pPersonal = NULL;
	m_pSoundexList = NULL;

        // Initialize new variables - GPD 17 May 2005
        m_bIsEncrypted = 0;
        strcpy(m_AtlasFileNames[0], "acsua.dat");
        strcpy(m_AtlasFileNames[1], "acsia.dat");
        strcpy(m_AtlasFileNames[2], "acstt.dat");
        // End GPD
}

void CloseAtlasFiles()
{
	int ctr, upto;
	struct SoundexInfo * psi;

	for (ctr = 0; ctr < 4; ctr++) if (m_fpData[ctr] != NULL) fclose(m_fpData[ctr]);
	if (m_pTimeIndex != NULL) free(m_pTimeIndex);
	if (m_pTableDates != NULL) free(m_pTableDates);
	if (m_pTableValues != NULL) free(m_pTableValues);
	if (m_pCounties != NULL) free(m_pCounties);
	if (m_CityPointers != NULL) free(m_CityPointers);
	if (m_pPersonal != NULL)
	  {
		upto = m_nCounts[m_nEntries - 1];
		for (ctr = 0; ctr < upto; ctr++)
		  {
			free(m_pPersonal[ctr]);
		  }
		free(m_pPersonal);
	  }

	psi = m_pSoundexList;
	while (psi != NULL)
	  {
		psi = psi->pnext;
//		delete m_pSoundexList;
		free(m_pSoundexList);
		m_pSoundexList = psi;
	  }
}

void Widen(LPWSTR wdest, LPCSTR csrc)
{
	int i;
	for (i = 0; csrc[i] != '\0'; i++) wdest[i] = wuni[(unsigned char) csrc[i]];
	wdest[i] = 0;
}

void Narrow(LPSTR cdest, LPCWSTR wsrc)
{
	int i;
	for (i = 0; wsrc[i] != 0; i++)
	  {
		if (wsrc[i] < 0x300) cdest[i] = ununi[wsrc[i]];
		else if (wsrc[i] >= 0x1e00 && wsrc[i] < 0x1f00) cdest[i] = ununi[wsrc[i] - 0x1b00];
		else if (wsrc[i] >= 0x2000 && wsrc[i] < 0x20d0) cdest[i] = punctuation[wsrc[i] - 0x2000];
		else cdest[i] = ' ';
	  }
	cdest[i] = '\0';
}

//int __declspec(dllexport) SetPathsW(LPCWSTR pMainFiles, LPCWSTR pPersonal)
int SetPathsW(LPCWSTR pMainFiles, LPCWSTR pPersonal)
{
	char mainfiles[256], personal[256];
	Narrow(mainfiles, pMainFiles);
	Narrow(personal, pPersonal);
	return SetPathsA(mainfiles, personal);
}

int _stdcall ZSetPathsW(LPCWSTR pMainFiles, LPCWSTR pPersonal)
{
	return SetPathsW(pMainFiles, pPersonal);
}

int SetPathsEmbeddedW(LPCWSTR pMainFiles, LPCWSTR pPersonal,
                                 LPCWSTR pAtlasFileUS, LPCWSTR pAtlasFileInt, LPCWSTR pAtlasFileTT,
                                 LPCWSTR pEncryptionString)
{
        char mainfiles[256], personal[256];
        char AtlasFileUS[256], AtlasFileInt[256], AtlasFileTT[256], EncryptionString[256];
        Narrow(mainfiles, pMainFiles);
        Narrow(personal, pPersonal);
        Narrow(AtlasFileUS, pAtlasFileUS);
        Narrow(AtlasFileInt, pAtlasFileInt);
        Narrow(AtlasFileTT, pAtlasFileTT);
        Narrow(EncryptionString, pEncryptionString);
        return SetPathsEmbeddedA(mainfiles, personal, AtlasFileUS, AtlasFileInt, AtlasFileTT, EncryptionString);
}

int _stdcall ZSetPathsEmbeddedW(LPCWSTR pMainFiles, LPCWSTR pPersonal,
                                           LPCWSTR pAtlasFileUS, LPCWSTR pAtlasFileInt, LPCWSTR pAtlasFileTT,
                                           LPCWSTR pEncryptionString)
{
  return SetPathsEmbeddedW(pMainFiles, pPersonal, pAtlasFileUS, pAtlasFileInt, pAtlasFileTT, pEncryptionString);
}

// Say whether to return country names as abbreviations in FindCity
// 0 = no
// non-zero = yes

//void __declspec(dllexport) SetReturnAbbreviations(int bretabbr)
void SetReturnAbbreviations(int bretabbr)
{
	m_returnAbbreviations = bretabbr;
}

void _stdcall ZSetReturnAbbreviations(int bretabbr)
{
	m_returnAbbreviations = bretabbr;
}

// Say whether to return the duplicate flag when a county name is passed to FindCity
// 0 = no
// non-zero = yes

//void __declspec(dllexport) SetDupReturnFlag(int dupret)
void SetDupReturnFlag(int dupret)
{
	m_bAlwaysReturnDupFlag = dupret;
}

void _stdcall ZSetDupReturnFlag(int dupret)
{
	m_bAlwaysReturnDupFlag = dupret;
}

// Say how to handle Illinois times in the Standard Time Law periods
// 0 = Standard Time always
// non-zero = Clock time

//void __declspec(dllexport) SetIllinoisTreatment(int IllinoisAction)
void SetIllinoisTreatment(int IllinoisAction)
{
	m_illinoisTreatment = IllinoisAction;
}

void _stdcall ZSetIllinoisTreatment(int IllinoisAction)
{
	m_illinoisTreatment = IllinoisAction;
}

// Say whether to return a time type of 5 (half hour DST shift)
// 0 = no
// non-zero = yes

//void __declspec(dllexport) SetHalfHourDST(int HalfDST)
void SetHalfHourDST(int HalfDST)
{
	m_bEnableHalfDST = HalfDST;
}

void _stdcall ZSetHalfHourDST(int HalfDST)
{
	m_bEnableHalfDST = HalfDST;
}

// Return the number of states & countries

//int __declspec(dllexport) GetCountryCount()
int GetCountryCount()
{
	return m_nEntries;
}

int _stdcall ZGetCountryCount()
{
	return m_nEntries;
}

// Get a country name in a single byte/character string
// Return value is the length of the country name.

//int __declspec(dllexport) GetCountryAtIndexA(LPSTR countryname, LPSTR abbreviation, int countrynumber)
int GetCountryAtIndexA(LPSTR countryname, LPSTR abbreviation, int countrynumber)
{
	if (countrynumber < 0 || countrynumber >= m_nEntries)
	  {
		countryname[0] = '\0';
		abbreviation[0] = '\0';
		return 0;
	  }
	strcpy(countryname, m_Names[countrynumber]);
	strcpy(abbreviation, m_Abbrevs[countrynumber]);
	return strlen(countryname);
}

int _stdcall ZGetCountryAtIndexA(LPSTR countryname, LPSTR abbreviation, int countrynumber)
{
	return GetCountryAtIndexA(countryname, abbreviation, countrynumber);
}

// Get a country name in a double byte/character string
// Return value is the length of the country name.

//int __declspec(dllexport) GetCountryAtIndexW(LPWSTR wcountryname, LPWSTR wabbreviation, int countrynumber)
int GetCountryAtIndexW(LPWSTR wcountryname, LPWSTR wabbreviation, int countrynumber)
{
	if (countrynumber < 0 || countrynumber >= m_nEntries)
	  {
		wcountryname[0] = 0;
		wabbreviation[0] = '\0';
		return 0;
	  }
	Widen(wcountryname, m_Names[countrynumber]);
	Widen(wabbreviation, m_Abbrevs[countrynumber]);
	return strlen(m_Names[countrynumber]);
}

int _stdcall ZGetCountryAtIndexW(LPWSTR wcountryname, LPWSTR wabbreviation, int countrynumber)
{
	return GetCountryAtIndexW(wcountryname, wabbreviation, countrynumber);
}

// Set the current country by its index
// Returns -1 if out of range, 1 if OK

//int __declspec(dllexport) SetCountryByIndex(int countrynumber)
int SetCountryByIndex(int countrynumber)
{
	if (m_nCurrentCountry == countrynumber) return 1;
	if (countrynumber < 0 || countrynumber >= m_nEntries) return -1;
	m_nCurrentCountry = countrynumber;
//	ReadCityPointers();
	return 1;
}

int _stdcall ZSetCountryByIndex(int countrynumber)
{
	if (m_nCurrentCountry == countrynumber) return 1;
	if (countrynumber < 0 || countrynumber >= m_nEntries) return -1;
	m_nCurrentCountry = countrynumber;
//	ReadCityPointers();
	return 1;
}

// Set current country by name
// Returns country index if successfull (>= 0) or -1 if not found in any list

//int __declspec(dllexport) SetCountryByNameA(LPCSTR countryname)
int SetCountryByNameA(LPCSTR countryname)
{
	int ctr;

	for (ctr = 0; ctr < m_nEntries; ctr++) if (stricmp(m_Names[ctr], countryname) == 0)
	  {
		if (m_nCurrentCountry != ctr)
		  {
			m_nCurrentCountry = ctr;
//			ReadCityPointers();
		  }
		return ctr;
	  }
	for (ctr = 0; ctr < m_nEntries; ctr++) if (stricmp(m_Abbrevs[ctr], countryname) == 0)
	  {
		if (m_nCurrentCountry != ctr)
		  {
			m_nCurrentCountry = ctr;
//			ReadCityPointers();
		  }
		return ctr;
	  }
	for (ctr = 0; ctr < m_nAlternate; ctr++) if (stricmp(m_Alternates[ctr], countryname) == 0)
	  {
		if (m_nCurrentCountry != m_nAlternateIndex[ctr])
		  {
			m_nCurrentCountry = m_nAlternateIndex[ctr];
//			ReadCityPointers();
		  }
		return m_nCurrentCountry;
	  }
	for (ctr = 0; ctr < m_nAlternate; ctr++) if (stricmp(m_AltAbbrevs[ctr], countryname) == 0)
	  {
		if (m_nCurrentCountry != m_nAlternateIndex[ctr])
		  {
			m_nCurrentCountry = m_nAlternateIndex[ctr];
//			ReadCityPointers();
		  }
		return m_nCurrentCountry;
	  }
	return -1;
}

int _stdcall ZSetCountryByNameA(LPCSTR countryname)
{
	return SetCountryByNameA(countryname);
}

//int __declspec(dllexport) SetCountryByNameW(LPCWSTR countryname)
int SetCountryByNameW(LPCWSTR countryname)
{
	char cname[61];

	Narrow(cname, countryname);
	return SetCountryByNameA(cname);
}

int _stdcall ZSetCountryByNameW(LPCWSTR countryname)
{
	char cname[61];

	Narrow(cname, countryname);
	return SetCountryByNameA(cname);
}

//int __declspec(dllexport) GetAlternateCount()
int GetAlternateCount()
{
	return m_nAlternate;
}

int _stdcall ZGetAlternateCount()
{
	return m_nAlternate;
}

//int __declspec(dllexport) GetAlternateAtIndexA(LPSTR AltName, LPSTR AltAbbrev, int nalt)
int GetAlternateAtIndexA(LPSTR AltName, LPSTR AltAbbrev, int nalt)
{
	if (nalt < 0 || nalt >= m_nAlternate)
	  {
		AltName[0] = '\0';
		AltAbbrev[0] = '\0';
		return -1;
	  }
	strcpy(AltName, m_Alternates[nalt]);
	strcpy(AltAbbrev, m_AltAbbrevs[nalt]);
	return m_nAlternateIndex[nalt];
}

int _stdcall ZGetAlternateAtIndexA(LPSTR AltName, LPSTR AltAbbrev, int nalt)
{
	return GetAlternateAtIndexA(AltName, AltAbbrev, nalt);
}

//int __declspec(dllexport) GetAlternateAtIndexW(LPWSTR wAltName, LPWSTR wAltAbbrev, int nalt)
int GetAlternateAtIndexW(LPWSTR wAltName, LPWSTR wAltAbbrev, int nalt)
{
	if (nalt < 0 || nalt >= m_nAlternate)
	  {
		wAltName[0] = '0';
		wAltAbbrev[0] = '0';
		return -1;
	  }
	Widen(wAltName, m_Alternates[nalt]);
	Widen(wAltAbbrev, m_AltAbbrevs[nalt]);
	return m_nAlternateIndex[nalt];
}

int _stdcall ZGetAlternateAtIndexW(LPWSTR wAltName, LPWSTR wAltAbbrev, int nalt)
{
	return GetAlternateAtIndexW(wAltName, wAltAbbrev, nalt);
}

// Return the number of cities in the current country
// -1 if current country not set

//int __declspec(dllexport) GetCityCount()
int GetCityCount()
{
	if (m_nCurrentCountry < 0) return -1;
	return m_nCounts[m_nCurrentCountry];
}

int _stdcall ZGetCityCount()
{
	if (m_nCurrentCountry < 0) return -1;
	return m_nCounts[m_nCurrentCountry];
}

// Return the number of administrative divisions in the selected S or C

//short __declspec(dllexport) GetCountyCount()
short GetCountyCount()
{
	if (m_nCurrentCountry < 0) return -1;
	return (m_nCountyHighs[m_nCurrentCountry] + 1 - m_nCountyLows[m_nCurrentCountry]);
}

short _stdcall ZGetCountyCount()
{
	if (m_nCurrentCountry < 0) return -1;
	return (m_nCountyHighs[m_nCurrentCountry] + 1 - m_nCountyLows[m_nCurrentCountry]);
}

int VersionNumber()
{
	return (122);
}

int _stdcall ZVersionNumber()
{
	return (122);
}
