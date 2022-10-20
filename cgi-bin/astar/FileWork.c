// FileWork.cpp: file manipulation functions for FindCity
#include "FindCity.h"
#ifdef _WIN32
#include <winreg.h>
#include <share.h>
#include <crtdbg.h>
#elif defined __unix__ || defined __APPLE__
#include "LinuxPorting.h"
#include <string.h>
#include <stdlib.h> //for atoi
#include <ctype.h> // for isdigit
#endif

// data

int m_nVersion;
int m_nStates, m_nEntries, m_nAlternate;
int m_nCurrentCountry;
int m_nTimeRecLen, m_nChangeCount;
int m_nMexico;
char * m_pUpcase;
FILE * m_fpData[4];

char m_strRevDate[9];
char m_strPersonalPath[256];
char m_Abbrevs[MAXENTRIES][7];
char m_Names[MAXENTRIES][61];
char m_AltAbbrevs[MAXALTERNATES][7];
char m_Alternates[MAXALTERNATES][61];

unsigned int m_nCountyLows[MAXENTRIES];
unsigned int m_nCountyHighs[MAXENTRIES];
unsigned int m_nCountyPointers[MAXENTRIES];
unsigned int m_nStarts[MAXENTRIES];	// city pointers
int m_nCounts[MAXENTRIES];	// city counts
unsigned char m_nFileIndex[MAXENTRIES];
unsigned int m_nCityPtrIndex[MAXENTRIES];
unsigned short m_nAlternateIndex[MAXALTERNATES];
unsigned short int * m_pTimeIndex;
unsigned int * m_pTableDates;
short int * m_pTableValues;
unsigned int * m_CityPointers;
char * m_pCounties;
struct CityInfo ** m_pPersonal;

extern char upcase1[256];
extern char upcase2[256];

int RecordLength[4] = {36, 34, 70, 72};
int CityNameLen[4] = {24, 24, 60, 60};
#if defined __unix__ || defined __APPLE__
const char * mexstates[32] = {"Aguascalientes","Baja California Norte",
#else
char * mexstates[32] = {"Aguascalientes","Baja California Norte",
#endif
   "Baja California Sur","Campeche","Chiapas","Chihuahua","Coahuila",
   "Coalima","District Federal","Durango","Guanajuato","Guerrero",
   "Hidalgo","Jalisco","México","Michoacán","Morelos","Nayarit",
   "Nuevo León","Oaxaca","Puebla","Queretaro","Quintana Roo",
   "San Luis Potosí","Sinaloa","Sonora","Tabasco","Tamaulipas",
   "Tlaxcala","Veracruz","Yucatán","Zacatecas"};

// New variables for encrypted atlas - GPD 17 May 2005
extern char m_AtlasFileNames[3][80]; // US Atlas; Int Atlas; TTables
extern char m_EncryptionString[80];
extern int m_bIsEncrypted;
// End GPD


// New routine to encrypt/unencrypt strings - GPD 24 May 2005
//   in/out: string - string to be encrypted or decrypted
//   in: password - string to use for encryption/decryption
// NB - If used with same password, using this function encrypts an unencrypted string
//      and decrypts an encrypted string
void EncryptDecrypt(char * string, int stringlen, char * password)
{
        unsigned char i, Pos, Pass, LS, LP, j;
        LS = stringlen; //strlen(string);
        LP = strlen(password);

        for (i = 1; i <= LS; i++)
          {
            if ((i % LP) == 0) j = -1;
            else               j =  0;
            Pos = (i % LP) - LP * j - 1;
            Pass = (int)password[Pos] % 256;
			//printf("%d-%x pass %x ", i-1, string[i-1], Pass);
            string[i-1] = string[i-1] ^ Pass;
			//printf("  %x\n", string[i-1]);
          }
        return;
}
// inline void ProcessAtlasString(char * placename, int namelen)
void ProcessAtlasString(char * placename, int namelen)
{
  if (m_bIsEncrypted != 0) EncryptDecrypt(placename, namelen, m_EncryptionString);
  return;
}
// End GPD



//----------------------------------------------------------------------------

int FindAtlasFiles(char * filename)
{
	char path[5][255];
	int i, best, rating, use;
	unsigned char * silly;
	int plen;
	LONG result;
	HANDLE fh;
	DWORD valtype, vallen;
	WIN32_FIND_DATA fdata;
#ifdef _WIN32
	HKEY rkey;

	GetPrivateProfileString( "ACSATLAS", "path", "C:\\PCATLAS\\", path[0], 255, "acsatlas.ini");
// if you copy this code somewhere else, don't forget to #include <winreg.h>
	silly = (unsigned char *) path[1];
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &rkey);
	if (result == ERROR_SUCCESS)
	  {
		vallen = 255;
		result = RegQueryValueEx(rkey, "CommonFilesDir", NULL, &valtype, silly, &vallen);
		if (result != ERROR_SUCCESS) strcpy(path[1], "C:\\Program Files\\Common Files");
		RegCloseKey(rkey);
	  }
	else 
		strcpy(path[1], "C:\\Program Files\\Common Files");
	strcpy(path[2], path[1]);
	strcat(path[1], "\\ACS\\Atlas\\");
	strcat(path[2], "\\ACS\\MiniAtlas\\");
	strcpy(path[3], "C:\\PCATLAS\\");
	strcpy(path[4], "C:\\ATLAS\\ATLAS\\");
#elif defined __unix__ || defined __APPLE__
//Note: the following paths are only for example. Use custom most appropriate paths instead.
	strcpy(path[0], "~/acsatlas/");
	strcpy(path[1], "~/acsatlas/");
	strcpy(path[2], "/usr/share/acsatlas/");
	strcpy(path[3], "/var/acsatlas/");
	strcpy(path[4], "/acsatlas/");
#endif
	use = best = 0;
	for (i = 0; i < 5; i++)
	  {
		plen = strlen(path[i]);
                // Replace hardcoded filename with variable - GPD 17 May 2005
                //strcpy(path[i] + plen, "acsua.dat");
                strcpy(path[i] + plen, m_AtlasFileNames[0]);
                // End GPD
		rating = 0;
		fh = FindFirstFile(path[i], &fdata);
		if (fh != INVALID_HANDLE_VALUE)
		  {
			if (fdata.nFileSizeLow > 1000000L) rating = 4;
			else rating = 3;
			FindClose(fh);
		  }
		else
		  {
                        // Replace hardcoded filename with variable - GPD 17 May 2005
                        //strcpy(path[i] + plen, "acsia.dat");
                        strcpy(path[i] + plen, m_AtlasFileNames[1]);
                        // End GPD
			fh = FindFirstFile(path[i], &fdata);
			if (fh != INVALID_HANDLE_VALUE)
			  {
				if (fdata.nFileSizeLow > 1000000L) rating = 4;
				else rating = 3;
				FindClose(fh);
			  }
			else
			  {
				strcpy(path[i] + plen, "usaatl.bin");
				fh = FindFirstFile(path[i], &fdata);
				if (fh != INVALID_HANDLE_VALUE)
				  {
					if (fdata.nFileSizeLow > 1000000L) rating = 2;
					else rating = 1;
					FindClose(fh);
				  }
			  }
		  }
		path[i][plen] = '\0';
		if (rating > best)
		  {
			best = rating;
			use = i;
		  }
	  }
	if (best == 0) return 0;	// no files found
	strcpy(filename, path[use]);
	if (best < 3) return 10;
	return 20;
}

//----------------------------------------------------------------------------

void ReadVersion2(LPCSTR apath)
{
	char clen;
	char cbuff[99], abuff[10], path[256];
	int lenpath;
	int ctr, upto, nsc, pri, nfi, revdate;
	int mcc;
	int counts[3];
	short stemp;
	FILE * afile;

	m_pUpcase = upcase2;
	pri = 1000;
	m_nMexico = -1;
	m_nChangeCount = mcc = m_nEntries = m_nAlternate = 0;
	revdate = 0;
	strcpy(path, apath);
	lenpath = strlen(path);
        // Replace hardcoded filename with variable - GPD 17 May 2005
        //strcpy(path + lenpath, "acsua.dat");
        strcpy(path + lenpath, m_AtlasFileNames[0]);
        // End GPD
	m_fpData[0] = _fsopen(path, "rb", _SH_DENYWR);
	if (m_fpData[0] == NULL)  m_fpData[0] = fopen(path, "rb");
	if (m_fpData[0] != NULL)
	  {
		fread(&stemp, 1, 2, m_fpData[0]);		// version
		m_nVersion = stemp;
		fread(cbuff, 1, 8, m_fpData[0]);		// rev date
		cbuff[8] = '\0';
		revdate = atoi(cbuff);
		strcpy(m_strRevDate, cbuff);
		fread(&stemp, 2, 1, m_fpData[0]);		// state count
		m_nStates = (int) stemp;
		fread(&stemp, 2, 1, m_fpData[0]);		// cross reference count
		m_nEntries = m_nStates;
	  }
        // Replace hardcoded filename with variable - GPD 17 May 2005
        //strcpy(path + lenpath, "acsia.dat");
        strcpy(path + lenpath, m_AtlasFileNames[1]);
        // End GPD
	m_fpData[1] = _fsopen(path, "rb", _SH_DENYWR);
	if (m_fpData[1] == NULL) m_fpData[1] = fopen(path, "rb");
	if (m_fpData[1] != NULL)
	  {
		fread(cbuff, 1, 10, m_fpData[1]);		// version & rev date
		cbuff[10] = '\0';
		ctr = atoi(cbuff + 2);
		if (ctr > revdate)
		  {
			revdate = ctr;
			strcpy(m_strRevDate, cbuff + 2);
		  }
		fread(&stemp, 2, 1, m_fpData[1]);		// country count
		nsc = (int) stemp;
		m_nEntries += nsc;
		fread(&stemp, 2, 1, m_fpData[1]);		// cross reference count
		m_nAlternate = (int) stemp;
		if (m_nAlternate > MAXALTERNATES) m_nAlternate = MAXALTERNATES;
	  }
	if (m_nEntries == 0) return;
	if (m_nEntries > MAXENTRIES) m_nEntries = MAXENTRIES;
	m_fpData[2] = NULL;
	m_fpData[3] = NULL;
	afile = m_fpData[0];
	nfi = 0;
	for (ctr = 0; ctr < m_nEntries; ctr++)
	  {
		if (ctr == m_nStates && m_fpData[1] != NULL)
		  {
			afile = m_fpData[1];
			nfi = 1;
		  }
		clen = fgetc(afile);				// length of abbreviation
		_ASSERT(clen < 7);
		fread(m_Abbrevs[ctr], 1, clen + 1, afile);
                // New GPD 24 May 2005
                ProcessAtlasString(m_Abbrevs[ctr], clen);
				//printf("%d:%s ", ctr, m_Abbrevs[ctr]);
                // End GPD
		clen = fgetc(afile);				// length of name
		_ASSERT(clen < 61);
		fread(m_Names[ctr], 1, clen + 1, afile);
                // New GPD 24 May 2005
                ProcessAtlasString(m_Names[ctr], clen);
				//printf("  %s\n", m_Names[ctr]);
                // End GPD
		if (strcmp(m_Names[ctr], "Mexico") == 0) m_nMexico = ctr;
		fread(&stemp, 2, 1, afile);	// county count
		m_nCountyLows[ctr] = 1;
		m_nCountyHighs[ctr] = (int) stemp;
		fread(counts, 4, 3, afile);
		m_nCountyPointers[ctr] = counts[0];		// first county pointer
		m_nCounts[ctr] = counts[1];		// city count
		m_nStarts[ctr] = counts[2];		// first city pointer
//		if (counts[1] > mcc) mcc = counts[1];
		m_nCityPtrIndex[ctr] = mcc;	// start of city pointers in memory
		mcc += counts[1];					// accumulate number of pointers for all SoC's
		m_nFileIndex[ctr] = nfi;
		if (strcmp(m_Abbrevs[ctr], "PR") == 0 && ctr > m_nStates)
		  {
			pri = ctr;
			--ctr;
			--m_nEntries;
		  }
	  }
	nsc = 0;
	if (m_fpData[1] != NULL) for (ctr = 0; ctr < m_nAlternate; ctr++)
	  {
		clen = fgetc(m_fpData[1]);
		fread(abuff, 1, clen + 1, m_fpData[1]);
                // New GPD 24 May 2005
                ProcessAtlasString(abuff, clen);
                // End GPD
		clen = fgetc(m_fpData[1]);
		fread(cbuff, 1, clen + 1, m_fpData[1]);
                // New GPD 24 May 2005
                ProcessAtlasString(cbuff, clen);
                // End GPD
		fread(&stemp, 2, 1, m_fpData[1]);
		stemp += (short) m_nStates;
		if (stemp >= pri) --stemp;	// correction for omitting Puerto Rico
		if (stemp >= m_nEntries) continue;
		strcpy(m_Alternates[nsc], cbuff);
		strcpy(m_AltAbbrevs[nsc], abuff);
		m_nAlternateIndex[nsc] = stemp;
		++nsc;
	  }
	m_nAlternate = nsc;
	m_nTimeRecLen = 6;
        // Replace hardcoded filename with variable - GPD 17 May 2005
        //strcpy(path + lenpath, "acstt.dat");
        strcpy(path + lenpath, m_AtlasFileNames[2]);
        // End GPD
	afile = _fsopen(path, "rb", _SH_DENYWR);
	if (afile == NULL) afile = fopen(path, "rb");
	if (afile != NULL)
	  {
		fread(cbuff, 1, 12, afile);		// version, rev date, zone count
		cbuff[10] = '\0';
		ctr = atoi(cbuff + 2);
		if (ctr > revdate)
		  {
			revdate = ctr;
			strcpy(m_strRevDate, cbuff + 2);
		  }
		fread(&nsc, 4, 1, afile);		// pointer start
		fread(&stemp, 2, 1, afile);		// pointer count
		upto = (int) stemp;
		fread(&nfi, 4, 1, afile);		// Changes start
		fread(&stemp, 2, 1, afile);		// Changes count
		m_nChangeCount = (int) stemp;
		fseek(afile, nsc, SEEK_SET);
		m_pTimeIndex = (unsigned short int *) calloc(upto, sizeof(short));
		if (m_pTimeIndex == NULL) return;		// presumably out of memory
		fread(m_pTimeIndex, 2, upto, afile);
		m_pTableDates = (unsigned int *) calloc(m_nChangeCount, sizeof(int));
		if (m_pTableDates == NULL)
		  {
			free(m_pTimeIndex);
			m_pTimeIndex = NULL;
			return;
		  }
		m_pTableValues = (short int *) calloc(m_nChangeCount, sizeof(short int));
		if (m_pTableValues == NULL)
		  {
			free(m_pTableDates);
			m_pTableDates = NULL;
			free(m_pTimeIndex);
			m_pTimeIndex = NULL;
			return;
		  }
		for (ctr = 0; ctr < m_nChangeCount; ctr++)
		  {
			fread(m_pTableDates + ctr, 4, 1, afile);
			fread(m_pTableValues + ctr, 2, 1, afile);
		  }
		fclose(afile);
	  }
	m_CityPointers = (unsigned int *) calloc(mcc, 4);
	for (ctr = 0; ctr < m_nEntries; ctr++)
	  {
		nfi = m_nFileIndex[ctr];
		if (m_fpData[nfi] == NULL) continue;	// file not found
		fseek(m_fpData[nfi], m_nStarts[ctr], SEEK_SET);
		fread(m_CityPointers + m_nCityPtrIndex[ctr], 4, m_nCounts[ctr], m_fpData[nfi]);
	  }
}

//---------------------------------------------------------------------------------------------------

void ReadVersion1(LPCSTR apath)
{
	char abbrev[7];
	char scname[33];
	char flnswitch[20];
	char line[81], path[256];
	char * cptr;
	short int stemp;
	short int ttdate[3];
	int i, tc, ctr;
	int ci, britain, canada, FileError;
	int pri = -1;
	int mcc, temp;
	FILE * fp, * bfp;
	DWORD se[2];
	unsigned int cse[2];
	int lenpath;

	FileError = 0;

	strcpy(m_strRevDate, "19961120");
	strcpy(path, apath);
	lenpath = strlen(path);
	strcpy(path + lenpath, "usandx.txt");	// State names in USA
	fp = fopen(path, "r");
	if (fp == NULL) FileError |= 2;
	m_pUpcase = upcase1;
	mcc = m_nAlternate = 0;
	m_nMexico = canada = britain = -1;
	strcpy(path + lenpath, "usandx.bin");
	bfp = fopen(path, "rb");
	if (bfp == NULL) FileError |= 4;
	if ((FileError & 6) == 0)
	  {
		m_nVersion = 10;
		i = 0;
	   while ((tc = getc(fp)) != EOF)
		  {
			_ASSERT (i < 81);
	      line[i++] = tc;					// read a line
	      if (tc != '\n') continue;
	      i = 0;
	      m_Abbrevs[m_nEntries][0] = abbrev[0] = line[0];			// two letter abbreviation
	      m_Abbrevs[m_nEntries][1] = abbrev[1] = line[1];
	      m_Abbrevs[m_nEntries][2] = abbrev[2] = '\0';
	      for (ctr = 0; ctr < 18; ctr++) scname[ctr] = line[ctr + 6];
	      for (ctr = 18; ctr > 0; ctr--) if (scname[ctr - 1] != ' ') break;
	      scname[ctr] = '\0';
	      if (strcmp(abbrev, "DC") == 0) strcpy(scname, "District of Columbia");
	      if (strcmp(abbrev, "PR") == 0) pri = m_nEntries;	// Puerto Rico index
	      strcpy(m_Names[m_nEntries], scname);
	      fseek(bfp, 24L, SEEK_CUR);
	      fread(se, 4, 2, bfp);
	      m_nCounts[m_nEntries] = se[1] + 1 - se[0];
//			if (m_nCounts[m_nEntries] > mcc) mcc = m_nCounts[m_nEntries];
			m_nCityPtrIndex[m_nEntries] = mcc;
			mcc += m_nCounts[m_nEntries];
	      m_nStarts[m_nEntries] = se[0] * 36;
			m_nFileIndex[m_nEntries] = 0;
	      ++m_nEntries;
	     }
	   fclose(fp);
	   fclose(bfp);
	  }
	m_nStates = m_nEntries;
// now Country names
	strcpy(path + lenpath, "intndx.txt");
	fp = fopen(path, "rt");
	if (fp == NULL) FileError |= 8;
	strcpy(path + lenpath, "intndx.bin");
	bfp = fopen(path, "rb");
	if (bfp == NULL) FileError |= 16;
	if ((FileError & 0x18) == 0)
	  {
		i = 0;
	   while ((tc = getc(fp)) != EOF)
		  {
			_ASSERT (i < 81);
	      line[i++] = tc;
	      if (tc != '\n') continue;
	      i = 0;
	      if (line[0] == '-') continue;			// skip --- entries
	      ctr = 7;
	      while (line[ctr] == ' ') ++ctr;
	      if (line[ctr] == '"' && line[ctr + 2] == ' ') continue;		// skip dittoed names
	      if (isdigit(line[ctr])) break;			// blank names at end--done
	      for (ctr = 0; ctr < 6; ctr++) abbrev[ctr] = line[ctr];
	      for (ctr = 6; ctr > 0; ctr--) if (abbrev[ctr - 1] != ' ') break;
	      abbrev[ctr] = '\0';
	      if (strcmp(abbrev, "PR") == 0 && pri >= 0) continue;
	      if (abbrev[0] == 'U' && abbrev[1] == 'K') continue;
	      for (ctr = 0; ctr < 17; ctr++) scname[ctr] = line[ctr + 7];
	      for (ctr = 17; ctr > 0; ctr--) if (scname[ctr - 1] != ' ') break;
	      scname[ctr] = '\0';
	      if (scname[1] == '"')
			  {
				if (canada < 0) continue;
				strcpy(m_Alternates[m_nAlternate], scname + 3);
				strcpy(m_AltAbbrevs[m_nAlternate], abbrev);
				m_nAlternateIndex[m_nAlternate] = canada;
				m_nAlternate++;
				continue;
			  }
	      if ((cptr = strchr(scname, ',')) != NULL)
	        {
				if (scname[0] == 'K') continue;		// skip Korea, North/South
	         if (scname[0] == 'I') continue;		// skip Ireland, Northern
	         strcpy(flnswitch, cptr + 2);
	         ctr = strlen(flnswitch);
	         flnswitch[ctr++] = ' ';
	         *cptr = '\0';
	         strcpy(flnswitch + ctr, scname);
	         strcpy(scname, flnswitch);
	        }
	      if (strcmp(abbrev, "CAR") == 0) strcpy(scname, "Central African Republic");
	      if (strcmp(abbrev, "DR") == 0) strcpy(scname, "Dominican Republic");
	      if (strcmp(abbrev, "NANT") == 0) strcpy(scname, "Netherlands Antilles");
	      if (strcmp(abbrev, "PITT") == 0) strcpy(scname, "Pacific Islands Trust Terr.");
	      if (strcmp(abbrev, "STPM") == 0) strcpy(scname, "St Pierre & Miquelon");
	      if (strcmp(abbrev, "SGEO") == 0) strcpy(scname, "South Georgia Island");
	      if (strcmp(abbrev, "UAE") == 0) strcpy(scname, "United Arab Emirates");
	      _ASSERT( strlen(scname) < 33);
	      strcpy(m_Abbrevs[m_nEntries], abbrev);
	      strcpy(m_Names[m_nEntries], scname);
			if (strcmp(scname, "Canada") == 0) canada = m_nEntries;
			if (strcmp(scname, "England") == 0) britain = m_nEntries;
			if (strcmp(scname, "Mexico") == 0) m_nMexico = m_nEntries;
	      sscanf(line + 41, "%d", &ci);
	      long recat = ((long) ci) * 32L - 8L;
	      fseek(bfp, recat, SEEK_SET);
	      fread(se, 4, 2, bfp);
	      m_nStarts[m_nEntries] = se[0] * 34;
	      m_nCounts[m_nEntries] = se[1] + 1 - se[0];
//			if (m_nCounts[m_nEntries] > mcc) mcc = m_nCounts[m_nEntries];
			m_nCityPtrIndex[m_nEntries] = mcc;
			mcc += m_nCounts[m_nEntries];
			m_nFileIndex[m_nEntries] = 1;
	      ++m_nEntries;
	     }
	   fclose(fp);
	   fclose(bfp);
	  }
	strcpy(path + lenpath, "uhospndx.bin");
	bfp = fopen(path, "rb");
	if (bfp != NULL)
	  {
		for (i = 0; i < m_nStates; i++)
		  {
			strcpy(m_Names[m_nEntries], m_Names[i]);
			strcat(m_Names[m_nEntries], " Hospitals");
			strcpy(m_Abbrevs[m_nEntries], m_Abbrevs[i]);
			strcat(m_Abbrevs[m_nEntries], "HOSP");
			fread(se, 4, 2, bfp);
			m_nStarts[m_nEntries] = se[0] * 72;
			m_nCounts[m_nEntries] = se[1] + 1 - se[0];
//			if (m_nCounts[m_nEntries] > mcc) mcc = m_nCounts[m_nEntries];
			m_nCityPtrIndex[m_nEntries] = mcc;
			mcc += m_nCounts[m_nEntries];
			m_nFileIndex[m_nEntries] = 3;
			++m_nEntries;
		  }
		fclose(bfp);
	  }
	strcpy(path + lenpath, "ihospndx.bin");
	bfp = fopen(path, "rb");
	if (bfp != NULL)
	  {
		fread(&stemp, 2, 1, bfp);
      strcpy(m_Names[m_nEntries], "Guam Hospitals");
	   strcpy(m_Abbrevs[m_nEntries], "GUAMHO");
	   fread(se, 4, 2, bfp);
	   m_nStarts[m_nEntries] = se[0] * 70;
	   m_nCounts[m_nEntries] = se[1] + 1 - se[0];
//		if (m_nCounts[m_nEntries] > mcc) mcc = m_nCounts[m_nEntries];
		m_nCityPtrIndex[m_nEntries] = mcc;
		mcc += m_nCounts[m_nEntries];
		m_nFileIndex[m_nEntries] = 2;
      ++m_nEntries;
		fread(&stemp, 2, 1, bfp);
      strcpy(m_Names[m_nEntries], "Pacific Islands Trust Terr. Hospitals");
	   strcpy(m_Abbrevs[m_nEntries], "PITTHO");
	   fread(se, 4, 2, bfp);
	   m_nStarts[m_nEntries] = se[0] * 70;
	   m_nCounts[m_nEntries] = se[1] + 1 - se[0];
//		if (m_nCounts[m_nEntries] > mcc) mcc = m_nCounts[m_nEntries];
		m_nCityPtrIndex[m_nEntries] = mcc;
		mcc += m_nCounts[m_nEntries];
		m_nFileIndex[m_nEntries] = 2;
      ++m_nEntries;
		fclose(bfp);
	  }
	if (m_nEntries == 0) return;
	if (britain > 0)
	  {
		strcpy(m_Alternates[m_nAlternate], "Britain");
		strcpy(m_AltAbbrevs[m_nAlternate], "BR");
		m_nAlternateIndex[m_nAlternate] = britain;
		m_nAlternate++;
		strcpy(m_Alternates[m_nAlternate], "Great Britain");
		strcpy(m_AltAbbrevs[m_nAlternate], "GB");
		m_nAlternateIndex[m_nAlternate] = britain;
		m_nAlternate++;
		strcpy(m_Alternates[m_nAlternate], "United Kingdom");
		strcpy(m_AltAbbrevs[m_nAlternate], "UK");
		m_nAlternateIndex[m_nAlternate] = britain;
		m_nAlternate++;
	  }
	strcpy(path + lenpath, "usaatl.bin");
	m_fpData[0] = _fsopen(path, "rb", _SH_DENYWR);
	if (m_fpData[0] == NULL)
	  {
	   m_fpData[0] = fopen(path, "rb");
		if (m_fpData[0] == NULL) FileError |= 0x20;
	  }
	strcpy(path + lenpath, "intatl.bin");
	m_fpData[1] = _fsopen(path, "rb", _SH_DENYWR);
	if (m_fpData[1] == NULL)
	  {
	   m_fpData[1] = fopen(path, "rb");
	   if (m_fpData[1] == NULL) FileError |= 0x40;
	  }
	strcpy(path + lenpath, "uhospatl.bin");
	m_fpData[3] = _fsopen(path, "rb", _SH_DENYWR);
	if (m_fpData[3] == NULL) m_fpData[3] = fopen(path, "rb");
	strcpy(path + lenpath, "ihospatl.bin");
	m_fpData[2] = _fsopen(path, "rb", _SH_DENYWR);
	if (m_fpData[2] == NULL) m_fpData[2] = fopen(path, "rb");
	for (i = 0; i < m_nEntries; i++)
	  {
		m_nCountyLows[i] = 1;
		m_nCountyHighs[i] = 0;
	  }
	strcpy(path + lenpath, "cntyusa.txt");
	fp = fopen(path, "rt");
	if (fp == NULL) FileError |= 0x80;
	else
	  {
		for (i = 0; i < m_nStates; i++)
	     {
			ctr = 0;
	      while ((tc = getc(fp)) != '\n') line[ctr++] = tc;
	      sscanf(line + 1,"%d %d", cse, cse + 1);
	      --cse[0];
	      --cse[1];
	      m_nCountyLows[i] = cse[0];
	      m_nCountyHighs[i] = cse[1];
	     }
	   fclose(fp);
	  }
	strcpy(path + lenpath, "cntyint.txt");
	fp = fopen(path, "rt");
	if (fp == NULL) FileError |= 0x100;
	else
	  {
		i = 0;
	   while ((tc = getc(fp)) != EOF)
	     {
			_ASSERT (i < 81);
	      line[i++] = tc;
	      if (tc != '\n') continue;
	      i = 0;
	      sscanf(line + 1,"%d %d", cse, cse + 1);
	      --cse[0];
	      --cse[1];
	      for (ci = 18; ci > 12; ci--) if (line[ci - 1] != ' ') break;
	      line[ci] = '\0';
	      for (ctr = m_nStates; ctr < m_nEntries; ctr++)
	        {
				if (strcmp(m_Abbrevs[ctr], line + 12) != 0) continue;
	         m_nCountyLows[ctr] = cse[0];
	         m_nCountyHighs[ctr] = cse[1];
	         break;
	        }
	     }
	   fclose(fp);
	  }
	strcpy(path + lenpath, "county.bin");
	bfp = _fsopen(path, "rb", _SH_DENYWR);
	if (bfp == NULL)
	  {
	   bfp = fopen(path, "rb");
	   if (bfp == NULL) FileError |= 0x200;
	  }
	if (bfp != NULL)
	  {
		fseek(bfp, 0L, SEEK_END);
		ctr = ftell(bfp);
		fseek(bfp, 0L, SEEK_SET);
		m_pCounties = (char *) malloc(ctr);
		if (m_pCounties != NULL) fread(m_pCounties, 1, ctr, bfp);
		fclose(bfp);
	  }
	strcpy(path + lenpath, "timndx.bin");
	m_nChangeCount = 0;
	bfp = fopen(path, "rb");
	if (bfp == NULL) FileError |= 0x400;
	else
	  {
		m_pTimeIndex = (unsigned short int *) calloc(3200, sizeof(short));
		if (m_pTimeIndex == NULL) return;		// presumably out of memory
		fread(m_pTimeIndex, 2, 3200, bfp);
		fclose(bfp);
	  }
	m_nTimeRecLen = 8;
	strcpy(path + lenpath, "timtab.bin");
	bfp = _fsopen(path, "rb", _SH_DENYWR);
	if (bfp == NULL)
	  {
	   bfp = fopen(path, "rb");
	   if (bfp == NULL) FileError |= 0x800;
	  }
	if (bfp == NULL) return;
	fseek(bfp, 0L, SEEK_END);
	m_nChangeCount = ftell(bfp)/8;
	fseek(bfp, 0L, SEEK_SET);
	m_pTableDates = (unsigned int *) calloc(m_nChangeCount, sizeof(int));
	if (m_pTableDates == NULL)
	  {
		if (m_pTimeIndex != NULL) free(m_pTimeIndex);
		m_pTimeIndex = NULL;
		return;
	  }
	m_pTableValues = (short int *) calloc(m_nChangeCount, sizeof(short));
	if (m_pTableValues == NULL)
	  {
		free(m_pTableDates);
		m_pTableDates = NULL;
		if (m_pTimeIndex != NULL) free(m_pTimeIndex);
		m_pTimeIndex = NULL;
		return;
	  }
	for (ctr = 0; ctr < m_nChangeCount; ctr++)
	  {
		fread(ttdate, 2, 3, bfp);	// read date, time
		m_pTableDates[ctr] = ttdate[0] * 624000 + (ttdate[1]/100) * 48000 +
		  (ttdate[1]%100) * 1500 + (ttdate[2]/100) * 60 + (ttdate[2]%100);
		fread(m_pTableValues + ctr, 2, 1, bfp);
	  }
	fclose(bfp);
	m_CityPointers = (unsigned int *) calloc(mcc, 4);
	for (ctr = 0; ctr < m_nEntries; ctr++)
	  {
		i = m_nFileIndex[ctr];
		if (m_fpData[i] == NULL) continue;	// file not found
		temp = m_nStarts[ctr];
		for (mcc = 0; mcc < m_nCounts[ctr]; mcc++)
		  {
			m_CityPointers[m_nCityPtrIndex[ctr] + mcc] = temp;
			temp += RecordLength[i];
		  }
	  }
}

//-------------------------------------------------------------------------------------------------------------------------

void ReadPersonal(LPCSTR path)
{
	char clen, pathname[255];
	short stemp, values[6];
	int lenpath, pfsize, ptr, cityptr, ctr, sl;
	FILE * fp;

	strcpy(pathname, path);
	lenpath = strlen(pathname);
	if (pathname[lenpath - 1] != PATH_DELIMITER) pathname[lenpath++] = PATH_DELIMITER;
	if (m_nVersion < 20)
	  {
		strcpy(pathname + lenpath, "PSLATL.BIN");	// try Version 1
		strcpy(m_strPersonalPath, pathname);		// save in case need to write to later
		fp = fopen(pathname, "rb");
		if (fp == NULL) return;
		fseek(fp, 0, SEEK_END);
		pfsize = ftell(fp);
		pfsize /= 36;		// number of records in personal atlas file
		fseek(fp, 0, SEEK_SET);
	  }
	else
	  {
		strcpy(pathname + lenpath, "acspa.dat");
		strcpy(m_strPersonalPath, pathname);		// save in case need to write to later
		fp = fopen(pathname, "rb");
		if (fp == NULL) return;
		fseek(fp, 10L, SEEK_SET);
		fread(&stemp, 2, 1, fp);		// record count
		pfsize = stemp;
		fread(&ptr, 4, 1, fp);			// start of pointers to records
	  }
	strcpy(m_Names[m_nEntries], "Personal Atlas");
	strcpy(m_Abbrevs[m_nEntries], "Prsnl");
	m_nFileIndex[m_nEntries] = 4;
	m_nCounts[m_nEntries] = pfsize;
	m_nCountyLows[m_nEntries] = 1;
	m_nCountyHighs[m_nEntries] = 0;

	m_pPersonal = (struct CityInfo **) calloc(pfsize, 4);
	for (ctr = 0; ctr < pfsize; ctr++)
	  {
		m_pPersonal[ctr] = (struct CityInfo *) malloc(sizeof(struct CityInfo));
		if (m_nVersion < 20)
		  {
			fread(m_pPersonal[ctr]->Name, 1, 24, fp);
			for (sl = 24; sl > 0; sl--) if (m_pPersonal[ctr]->Name[sl - 1] != ' ') break;
			m_pPersonal[ctr]->Name[sl] = '\0';
			fread(values, 2, 6, fp);
			m_pPersonal[ctr]->IsDuplicate = (values[0]/1000) & 3;
			m_pPersonal[ctr]->CountyNumber = values[0]%1000;
			m_pPersonal[ctr]->Latitude = ((long) values[1]) * 60L;
			m_pPersonal[ctr]->Longitude = ((long) values[2]) * 60L;
			if (values[5] != 9999)
			  {
				m_pPersonal[ctr]->Latitude += (long) (values[5]/100);
				m_pPersonal[ctr]->Longitude += (long) (values[5]%100);
			  }
			m_pPersonal[ctr]->TypeTable = values[3];
			m_pPersonal[ctr]->ZoneTable = values[4];
		  }
		else
		  {
			fseek(fp, ptr, SEEK_SET);
			ptr += 4;
			fread(&cityptr, 4, 1, fp);			// pointer to city
			fseek(fp, cityptr, SEEK_SET);
			clen = fgetc(fp);
			fread(m_pPersonal[ctr]->Name, 1, clen + 1, fp);
			m_pPersonal[ctr]->IsDuplicate = fgetc(fp);
			fread(values, 2, 1, fp);
			m_pPersonal[ctr]->CountyNumber = values[0];
			fread(&(m_pPersonal[ctr]->Latitude), 4, 1, fp);
			fread(&(m_pPersonal[ctr]->Longitude), 4, 1, fp);
			fread(values, 2, 2, fp);
			m_pPersonal[ctr]->TypeTable = values[0];
			m_pPersonal[ctr]->ZoneTable = values[1];
		  }
	  }
	++m_nEntries;
	fclose(fp);
}

//-------------------------------------------------------------------------------------------------------------------------
// Write all the Personal Atlas entries out
// return -1 if cannot open file

int WritePersonal(int psize)
{
	char clen, pdate[11], v1city[61];
	short int stemp, values[6];
	int ctr, ptr, i;
	FILE * fp;
	SYSTEMTIME curdate;

	fp = fopen(m_strPersonalPath, "wb");
	if (fp == NULL) return -1;

	if (m_nVersion >= 20)
	  {
		GetLocalTime(&curdate);
		wsprintf(pdate, "%2d%02d%02d%4d", m_nVersion, curdate.wMonth, curdate.wDay, curdate.wYear);
		fwrite(pdate, 1, 10, fp);
		stemp = (short) psize;
		fwrite(&stemp, 2, 1, fp);		// record count
		ptr = 16;
		fwrite(&ptr, 4, 1, fp);
		ptr += psize * 4;
		for (ctr = 0; ctr < psize; ctr++)
		  {
			fwrite(&ptr, 4, 1, fp);
			ptr += strlen(m_pPersonal[ctr]->Name) + 17;
		  }
	  }

	for (ctr = 0; ctr < psize; ctr++)
	  {
		if (m_nVersion < 20)
		  {
			strcpy(v1city, m_pPersonal[ctr]->Name);
			for (i = strlen(v1city); i < 24; i++) v1city[i] = ' ';
			fwrite(v1city, 1, 24, fp);
			values[0] = m_pPersonal[ctr]->IsDuplicate * 1000 + m_pPersonal[ctr]->CountyNumber;
			values[1] = (short) (m_pPersonal[ctr]->Latitude/60);
			values[2] = (short) (m_pPersonal[ctr]->Longitude/60);
			values[3] = (short) m_pPersonal[ctr]->TypeTable;
			values[4] = (short) m_pPersonal[ctr]->ZoneTable;
			values[5] = (m_pPersonal[ctr]->Latitude % 60) * 100 + (m_pPersonal[ctr]->Longitude % 60);
			fwrite(values, 2, 6, fp);
		  }
		else
		  {
			clen = strlen(m_pPersonal[ctr]->Name);
			fputc(clen, fp);
			fwrite(m_pPersonal[ctr]->Name, 1, clen + 1, fp);
			fputc(m_pPersonal[ctr]->IsDuplicate, fp);
			stemp = (short) m_pPersonal[ctr]->CountyNumber;
			fwrite(&stemp, 2, 1, fp);
			fwrite(&(m_pPersonal[ctr]->Latitude), 4, 1, fp);
			fwrite(&(m_pPersonal[ctr]->Longitude), 4, 1, fp);
			values[0] = m_pPersonal[ctr]->TypeTable;
			values[1] = m_pPersonal[ctr]->ZoneTable;
			fwrite(values, 2, 2, fp);
		  }
	  }
	fclose(fp);
	return psize;
}

//-------------------------------------------------------------------------------------------------------------------------
// Adds the city data passed to it to the Personal Atlas list.
// Returns the number of cities written to the Personal Atlas file.
// Error codes:
// -1 Unable to open the personal atlas file for writing.
// -2 Unable to allocate memory for internal data structures.

//int __declspec(dllexport) AddToPersonalA(LPCSTR city, int * cityfields)
int AddToPersonalA(LPCSTR city, int * cityfields)
{
	int psize, ctr;
	struct CityInfo ** pplus, * pnewcity;

	if (m_pPersonal != NULL) psize = m_nCounts[m_nEntries - 1];
	else psize = 0;
	pplus = (struct CityInfo **) calloc(psize + 1, 4);	// allocate larger pointer array
	if (pplus == NULL)
	  {
		return -2;
	  }
	pnewcity = (struct CityInfo *) malloc(sizeof(struct CityInfo));
	if (pnewcity == NULL)
	  {
		free(pplus);
		return -2;
	  }
	strcpy(pnewcity->Name, city);
	pnewcity->IsDuplicate = (cityfields[0]/1000) & 3;
	pnewcity->CountyNumber = cityfields[0] % 1000;
	pnewcity->TypeTable = cityfields[3];
	pnewcity->ZoneTable = cityfields[4];
	pnewcity->Latitude = cityfields[1];
	pnewcity->Longitude = cityfields[2];
	for (ctr = 0; ctr < psize; ctr++)
	  {
		if (noblankstricmp(city, m_pPersonal[ctr]->Name) < 0) break;
		pplus[ctr] = m_pPersonal[ctr];			// copy pointers up to where new one goes
	  }
	pplus[ctr] = pnewcity;							// add new city
	++psize;
	while (++ctr < psize) pplus[ctr] = m_pPersonal[ctr - 1];	// copy rest of city struct pointers
	if (m_pPersonal != NULL)
	  {
		free(m_pPersonal);
		m_nCounts[m_nEntries - 1] = psize;
	  }
	else
	  {
		strcpy(m_Names[m_nEntries], "Personal Atlas");
		strcpy(m_Abbrevs[m_nEntries], "Prsnl");
		m_nFileIndex[m_nEntries] = 4;
		m_nCountyLows[m_nEntries] = 1;
		m_nCountyHighs[m_nEntries] = 0;
		m_nCounts[m_nEntries++] = 1;
	  }
	m_pPersonal = pplus;
	return WritePersonal(psize);
}

int _stdcall ZAddToPersonalA(LPCSTR city, int * cityfields)
{
	return AddToPersonalA(city, cityfields);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) AddToPersonalW(LPCWSTR city, int * cityfields)
int AddToPersonalW(LPCWSTR city, int * cityfields)
{
	char ccity[61];
	Narrow(ccity, city);
	return AddToPersonalA(ccity, cityfields);
}

int _stdcall ZAddToPersonalW(LPCWSTR city, int * cityfields)
{
	char ccity[61];
	Narrow(ccity, city);
	return AddToPersonalA(ccity, cityfields);
}

//-------------------------------------------------------------------------------------------------------------------------
// Adds the city data passed to it to the Personal Atlas list.
// Returns the number of cities written to the Personal Atlas file.
// Error codes:
// -1 Unable to open the personal atlas file for writing.
// -2 Unable to allocate memory for internal data structures.

//int __declspec(dllexport) AddPlaceToPersonalA(LPCSTR city, int county, int latitude, int longitude, int typetable, int zonetable)
int AddPlaceToPersonalA(LPCSTR city, int county, int latitude, int longitude, int typetable, int zonetable)
{
	int psize, ctr;
	struct CityInfo ** pplus, * pnewcity;

	if (m_pPersonal != NULL) psize = m_nCounts[m_nEntries - 1];
	else psize = 0;
	pplus = (struct CityInfo **) calloc(psize + 1, 4);	// allocate larger pointer array
	if (pplus == NULL)
	  {
		return -2;
	  }
	pnewcity = (struct CityInfo *) malloc(sizeof(struct CityInfo));
	if (pnewcity == NULL)
	  {
		free(pplus);
		return -2;
	  }
	strcpy(pnewcity->Name, city);
	pnewcity->IsDuplicate = (county/1000) & 3;
	pnewcity->CountyNumber = county % 1000;
	pnewcity->TypeTable = typetable;
	pnewcity->ZoneTable = zonetable;
	pnewcity->Latitude = latitude;
	pnewcity->Longitude = longitude;
	for (ctr = 0; ctr < psize; ctr++)
	  {
		if (noblankstricmp(city, m_pPersonal[ctr]->Name) < 0) break;
		pplus[ctr] = m_pPersonal[ctr];			// copy pointers up to where new one goes
	  }
	pplus[ctr] = pnewcity;							// add new city
	++psize;
	while (++ctr < psize) pplus[ctr] = m_pPersonal[ctr - 1];	// copy rest of city struct pointers
	if (m_pPersonal != NULL)
	  {
		free(m_pPersonal);
		m_nCounts[m_nEntries - 1] = psize;
	  }
	else
	  {
		strcpy(m_Names[m_nEntries], "Personal Atlas");
		strcpy(m_Abbrevs[m_nEntries], "Prsnl");
		m_nFileIndex[m_nEntries] = 4;
		m_nCountyLows[m_nEntries] = 1;
		m_nCountyHighs[m_nEntries] = 0;
		m_nCounts[m_nEntries++] = 1;
	  }
	m_pPersonal = pplus;
	return WritePersonal(psize);
}

int _stdcall ZAddPlaceToPersonalA(LPCSTR city, int county, int latitude, int longitude, int typetable, int zonetable)
{
	return AddPlaceToPersonalA(city, county, latitude, longitude, typetable, zonetable);
}

//int __declspec(dllexport) AddPlaceToPersonalW(LPCWSTR city, int county, int latitude, int longitude, int typetable, int zonetable)
int AddPlaceToPersonalW(LPCWSTR city, int county, int latitude, int longitude, int typetable, int zonetable)
{
	char ccity[61];
	Narrow(ccity, city);
	return AddPlaceToPersonalA(ccity, county, latitude, longitude, typetable, zonetable);
}

int _stdcall ZAddPlaceToPersonalW(LPCWSTR city, int county, int latitude, int longitude, int typetable, int zonetable)
{
	char ccity[61];
	Narrow(ccity, city);
	return AddPlaceToPersonalA(ccity, county, latitude, longitude, typetable, zonetable);
}

//-------------------------------------------------------------------------------------------------------------------------
// Returns number of cities written, or error code:
// -1 cannot open personal atlas file for writing
// -2 no cities to delete, or index out of range

//int __declspec(dllexport) DeleteFromPersonalByIndex(int citynumber)
int DeleteFromPersonalByIndex(int citynumber)
{
	int ctr, psize;
	struct CityInfo * pcity;

	if (m_pPersonal == NULL) return -2;
	psize = m_nCounts[m_nEntries - 1];
	if (citynumber < 0 || citynumber >= psize) return -2;
	pcity = m_pPersonal[citynumber];
	for (ctr = citynumber; ctr + 1 < psize; ctr++) m_pPersonal[ctr] = m_pPersonal[ctr + 1];
//	delete pcity;
	if (pcity != NULL)
		free(pcity);
	--psize;
	m_nCounts[m_nEntries - 1] = psize;
	return WritePersonal(psize);
}

int _stdcall ZDeleteFromPersonalByIndex(int citynumber)
{
	return DeleteFromPersonalByIndex(citynumber);
}

//-------------------------------------------------------------------------------------------------------------------------
// Returns number of cities written, or error code:
// -1 cannot open personal atlas file for writing
// -2 no cities in personal atlas
// -3 city name not found in personal atlas

//int __declspec(dllexport) DeleteFromPersonalByNameA(LPCSTR cityname)
int DeleteFromPersonalByNameA(LPCSTR cityname)
{
	int ctr, psize;
	struct CityInfo * pcity;

	if (m_pPersonal == NULL) return -2;
	psize = m_nCounts[m_nEntries - 1];
	for (ctr = 0; ctr < psize; ctr++)
	  {
		pcity = m_pPersonal[ctr];
		if (stricmp(cityname, pcity->Name) == 0)
		  {
			while (ctr + 1 < psize)
			  {
				m_pPersonal[ctr] = m_pPersonal[ctr + 1];
				++ctr;
			  }
//			delete pcity;
			free(pcity);
			--psize;
			m_nCounts[m_nEntries - 1] = psize;
			return WritePersonal(psize);
		  }
	  }
	return -3;
}

int _stdcall ZDeleteFromPersonalByNameA(LPCSTR cityname)
{
	return DeleteFromPersonalByNameA(cityname);
}

//-------------------------------------------------------------------------------------------------------------------------

//int __declspec(dllexport) DeleteFromPersonalByNameW(LPCWSTR wcityname)
int DeleteFromPersonalByNameW(LPCWSTR wcityname)
{
	char ccity[61];
	Narrow(ccity, wcityname);
	return DeleteFromPersonalByNameA(ccity);
}

int _stdcall ZDeleteFromPersonalByNameW(LPCWSTR wcityname)
{
	char ccity[61];
	Narrow(ccity, wcityname);
	return DeleteFromPersonalByNameA(ccity);
}

//-------------------------------------------------------------------------------------------------------------------------

//void ReadCityPointers()
//{
//	int nf, temp;
//	int ctr;

//	nf = m_nFileIndex[m_nCurrentCountry];
//	if (nf == 4) return;
//	if (m_fpData[nf] == NULL) return;	// didn't find files
//	if (m_nVersion < 20)
//	  {
//		temp = m_nStarts[m_nCurrentCountry];
//		for (ctr = 0; ctr < m_nCounts[m_nCurrentCountry]; ctr++)
//		  {
//			m_CityPointers[ctr] = temp;
//			temp += RecordLength[nf];
//		  }
//	  }
//	else
//	  {
//		fseek(m_fpData[nf], m_nStarts[m_nCurrentCountry], SEEK_SET);
//		fread(m_CityPointers, 4, m_nCounts[m_nCurrentCountry], m_fpData[nf]);
//	  }
//}

//-------------------------------------------------------------------------------------------------------------------------
// Get a city name in a single byte/character string
// Return value is the duplicate flag, or -1 if current country not set, or -2 if index out of range
// or -3 if the country's data file is not open

//int __declspec(dllexport) GetCityAtIndexA(LPSTR cityname, int citynumber, LPCSTR MarkPrimary)
int GetCityAtIndexA(LPSTR cityname, int citynumber, LPCSTR MarkPrimary)
{
	int dup;

	cityname[0] = '\0';
	if (m_nCurrentCountry < 0) return -1;
	if (citynumber < 0 || citynumber >= m_nCounts[m_nCurrentCountry]) return -2;
	dup = GetCityForLookup(cityname, citynumber);
	if (dup < 0) return dup;
	if (dup == 1) strcat(cityname, MarkPrimary);
	return (dup);
}

int _stdcall ZGetCityAtIndexA(LPSTR cityname, int citynumber, LPCSTR MarkPrimary)
{
	return GetCityAtIndexA(cityname, citynumber, MarkPrimary);
}

// Get a country name in a double byte/character string
// Return value is the city duplicate flag.

//int __declspec(dllexport) GetCityAtIndexW(LPWSTR wcityname, int citynumber, LPCWSTR MarkPrimary)
int GetCityAtIndexW(LPWSTR wcityname, int citynumber, LPCWSTR MarkPrimary)
{
	char tcity[61];
	int dup;

	wcityname[0] = 0;
	dup = GetCityForLookup(tcity, citynumber);
	if (dup < 0) return dup;
	Widen(wcityname, tcity);
	if (dup == 1) wcscat(wcityname, MarkPrimary);
	return dup;
}

int _stdcall ZGetCityAtIndexW(LPWSTR wcityname, int citynumber, LPCWSTR MarkPrimary)
{
	return GetCityAtIndexW(wcityname, citynumber, MarkPrimary);
}

//-------------------------------------------------------------------------------------------------------------------------
// Read the city at citynumber, return duplicate flag

int GetCityForLookup(LPSTR cityname, int citynumber)
{
	int sl, clen, nf;
	short int stemp;

	cityname[0] = '\0';
	nf = m_nFileIndex[m_nCurrentCountry];
	if (nf == 4)	// personal
	  {
		strcpy(cityname, m_pPersonal[citynumber]->Name);
		return m_pPersonal[citynumber]->IsDuplicate;
	  }
	if (m_fpData[nf] == NULL) return -3;
	fseek(m_fpData[nf], m_CityPointers[m_nCityPtrIndex[m_nCurrentCountry] + citynumber], SEEK_SET);
	if (m_nVersion < 20)
	  {
		clen = CityNameLen[nf];
		fread(cityname, 1, clen, m_fpData[nf]);
		for (sl = clen; sl > 0; sl--) if (cityname[sl - 1] != ' ') break;
		cityname[sl] = '\0';
//		if (bMarkPrimary)
//		  {
		fread(&stemp, 2, 1, m_fpData[nf]);
		fseek(m_fpData[nf], -2, SEEK_CUR);
		clen = (stemp/1000) & 3;
//			if (stemp/1000 == 1)
//			  {
//				cityname[sl++] = ' ';
//				cityname[sl++] = '*';
//			  }
//		  }
	  }
	else
	  {
		clen = fgetc(m_fpData[nf]);
		fread(cityname, 1, clen + 1, m_fpData[nf]);
                // New GPD 24 May 2005
                ProcessAtlasString(cityname, clen);
                // End GPD
//		if (bMarkPrimary)
//		  {
		clen = fgetc(m_fpData[nf]) & 3;	// duplicate flag
		fseek(m_fpData[nf], -1, SEEK_CUR);
//			if (clen == 1) strcat(cityname, " *");
//		  }
	  }
//	return strlen(cityname);
	return clen;
}

//-------------------------------------------------------------------------------------------------------------------------
// This reads the data fields.  It assumes that the city name has just been read.
//   0 County number & duplicate flag (1000 for primary, 2000 for secondaries)
//   1 Latitude in seconds of arc
//   2 Longitude in seconds of arc
//   3 Time Type table number
//   4 Time Zone table number
//   5 State or Country index

void ReadDataFields(int citynumber, int * datafields)
{
	short int values[6];
	int nf, dupflag;

// since this is called after a successful city read, no error checking is done.
	nf = m_nFileIndex[m_nCurrentCountry];
	datafields[5] = m_nCurrentCountry;
	if (nf == 4)
	  {
		datafields[0] = m_pPersonal[citynumber]->IsDuplicate * 1000 + m_pPersonal[citynumber]->CountyNumber;
		datafields[1] = m_pPersonal[citynumber]->Latitude;
		datafields[2] = m_pPersonal[citynumber]->Longitude;
		datafields[3] = m_pPersonal[citynumber]->TypeTable;
		datafields[4] = m_pPersonal[citynumber]->ZoneTable;
		return;
	  }
	if (m_nVersion < 20)
	  {
		fread(values, 2, 6, m_fpData[nf]);
		dupflag = (values[0]/1000) & 3;
		values[0] %= 1000;
		datafields[0] = values[0] + dupflag * 1000;
		datafields[1] = values[1] * 60;
		datafields[2] = values[2] * 60;
		if (RecordLength[nf] - CityNameLen[nf] > 10 && values[5] != 9999)
		  {
			datafields[1] += values[5]/100;
			datafields[2] += values[5]%100;
		  }
		datafields[3] = values[3];
		datafields[4] = values[4];
	  }
	else
	  {
		dupflag = fgetc(m_fpData[nf]) & 3;
		fread(values, 2, 1, m_fpData[nf]);
		datafields[0] = values[0] + dupflag * 1000;
		fread(datafields + 1, 4, 2, m_fpData[nf]);
		fread(values, 2, 2, m_fpData[nf]);
		datafields[3] = values[0];
		datafields[4] = values[1];
	  }
}

//-------------------------------------------------------------------------------------------------------------------------
// Position the file pointer, then call ReadDataFields

//int __declspec(dllexport) GetDataFields(int citynumber, int * datafields)
int GetDataFields(int citynumber, int * datafields)
{
	int nf, clen;

	if (m_nCurrentCountry < 0) return -1;
	if (citynumber < 0 || citynumber >= m_nCounts[m_nCurrentCountry]) return -2;
	nf = m_nFileIndex[m_nCurrentCountry];
	if (nf < 4)
	  {
		if (m_nVersion < 20)
		  {
			fseek(m_fpData[nf], m_CityPointers[m_nCityPtrIndex[m_nCurrentCountry] + citynumber] + CityNameLen[nf], SEEK_SET);
		  }
		else
		  {
			fseek(m_fpData[nf], m_CityPointers[m_nCityPtrIndex[m_nCurrentCountry] + citynumber], SEEK_SET);
			clen = fgetc(m_fpData[nf]);
			fseek(m_fpData[nf], clen + 1, SEEK_CUR);
		  }
	  }
	ReadDataFields(citynumber, datafields);
	return 0;
}

int _stdcall ZGetDataFields(int citynumber, int * datafields)
{
	return GetDataFields(citynumber, datafields);
}

//-------------------------------------------------------------------------------------------------------------------------
// Position the file pointer, then call ReadDataFields

//int __declspec(dllexport) GetCityData(int citynumber, int& countydup, int& latitude, int& longitude, int& typetable, int& zonetable, int& soc)
int GetCityData(int citynumber, int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc)
{
	int nf, clen;
	int datafields[6];

	if (m_nCurrentCountry < 0) return -1;
	if (citynumber < 0 || citynumber >= m_nCounts[m_nCurrentCountry]) return -2;
	nf = m_nFileIndex[m_nCurrentCountry];
	if (nf < 4)
	  {
		if (m_nVersion < 20)
		  {
			fseek(m_fpData[nf], m_CityPointers[m_nCityPtrIndex[m_nCurrentCountry] + citynumber] + CityNameLen[nf], SEEK_SET);
		  }
		else
		  {
			fseek(m_fpData[nf], m_CityPointers[m_nCityPtrIndex[m_nCurrentCountry] + citynumber], SEEK_SET);
			clen = fgetc(m_fpData[nf]);
			fseek(m_fpData[nf], clen + 1, SEEK_CUR);
		  }
	  }
	ReadDataFields(citynumber, datafields);
	*countydup = datafields[0];
	*latitude = datafields[1];
	*longitude = datafields[2];
	*typetable = datafields[3];
	*zonetable = datafields[4];
	*soc = datafields[5];
	return 0;
}

int _stdcall ZGetCityData(int citynumber, int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc)
{
	return GetCityData(citynumber, countydup, latitude, longitude, typetable, zonetable, soc);
}

//----------------------------------------------------------------------------

//int __declspec(dllexport) GetCountyNumberFromNameA(LPCSTR county)
int GetCountyNumberFromNameA(LPCSTR county)
{
	char cntybuf[255];
	short int testnum;
	unsigned int rn, upto;
	int ctr, clen, nfile;
	char * cp;
	short int * iptr;
	FILE * cfile;

	if (m_nCurrentCountry < 0) return -1;
	nfile = m_nFileIndex[m_nCurrentCountry];
	if (nfile == 4) return -3;
	if (m_nCountyLows[m_nCurrentCountry] >= m_nCountyHighs[m_nCurrentCountry]) return -3;
	if (m_nVersion < 20)
	  {
		for (rn = m_nCountyLows[m_nCurrentCountry]; rn <= m_nCountyHighs[m_nCurrentCountry]; rn++)
		  {
			cp = m_pCounties + rn * 26;
			iptr = (short int *) cp;
			testnum = *iptr;
			cp += 2;
			for (ctr = 0; ctr < 24; ctr++) cntybuf[ctr] = *cp++;
			while (ctr > 0 && cntybuf[ctr - 1] == ' ') --ctr;
			cntybuf[ctr] = '\0';
			if (stricmp(county, cntybuf) == 0) return ((int) testnum);
			if (m_nCurrentCountry == m_nMexico && stricmp(county, mexstates[testnum - 1]) == 0) return ((int) testnum);
		  }
		return -2;
	  }
	upto = m_nCountyHighs[m_nCurrentCountry] + 1 - m_nCountyLows[m_nCurrentCountry];
	cfile = m_fpData[nfile];
	fseek(cfile, m_nCountyPointers[m_nCurrentCountry], SEEK_SET);
	for (rn = 0; rn < upto; rn++)
	  {
		fread(&testnum, 2, 1, cfile);
		clen = fgetc(cfile) + 1;
		fread(cntybuf, 1, clen, cfile);
                // New GPD 24 May 2005
                ProcessAtlasString(cntybuf, clen-1);
                // End GPD
		for (ctr = clen; ctr > 0; ctr--) if (cntybuf[ctr - 1] != ' ') break;
		cntybuf[ctr] = '\0';
		if (stricmp(county, cntybuf) == 0) return ((int) testnum);
	  }
	return -2;
}

int _stdcall ZGetCountyNumberFromNameA(LPCSTR county)
{
	return GetCountyNumberFromNameA(county);
}

//----------------------------------------------------------------------------

//int __declspec(dllexport) GetCountyNumberFromNameW(LPCWSTR wcounty)
int GetCountyNumberFromNameW(LPCWSTR wcounty)
{
	char county[61];
	Narrow(county, wcounty);
	return GetCountyNumberFromNameA(county);
}

int _stdcall ZGetCountyNumberFromNameW(LPCWSTR wcounty)
{
	char county[61];
	Narrow(county, wcounty);
	return GetCountyNumberFromNameA(county);
}

//----------------------------------------------------------------------------

//int __declspec(dllexport) GetCountyNameFromNumberA(LPSTR county, int cn)
int GetCountyNameFromNumberA(LPSTR county, int cn)
{
	short int testnum;
	unsigned int rn, upto;
	int ctr, clen, nfile;
	char * cp;
	short int * iptr;
	FILE * cfile;

	county[0] = '\0';
	if (m_nCurrentCountry < 0) return -1;
	nfile = m_nFileIndex[m_nCurrentCountry];
	if (nfile == 4) return -3;
	if (m_nCountyLows[m_nCurrentCountry] >= m_nCountyHighs[m_nCurrentCountry]) return -3;
	if (m_nVersion < 20)
	  {
		for (rn = m_nCountyLows[m_nCurrentCountry]; rn <= m_nCountyHighs[m_nCurrentCountry]; rn++)
		  {
			cp = m_pCounties + rn * 26;
			iptr = (short int *) cp;
			testnum = *iptr;
			if (testnum != cn) continue;
			cp += 2;
			for (ctr = 0; ctr < 24; ctr++) county[ctr] = *cp++;
			while (ctr > 0 && county[ctr - 1] == ' ') --ctr;
			county[ctr] = '\0';
			if (m_nCurrentCountry == m_nMexico) strcpy(county, mexstates[cn - 1]);
			return 1;
		  }
		return -2;
	  }
	upto = m_nCountyHighs[m_nCurrentCountry] + 1 - m_nCountyLows[m_nCurrentCountry];
	cfile = m_fpData[nfile];
	fseek(cfile, m_nCountyPointers[m_nCurrentCountry], SEEK_SET);
	for (rn = 0; rn < upto; rn++)
	  {
		fread(&testnum, 2, 1, cfile);
		clen = fgetc(cfile) + 1;
		fread(county, 1, clen, cfile);
                // New GPD 24 May 2005
                ProcessAtlasString(county, clen-1);
                // End GPD
		for (ctr = clen; ctr > 0; ctr--) if (county[ctr - 1] != ' ') break;
		county[ctr] = '\0';
		if (testnum != cn) continue;
		if (stricmp(county, county) == 0) return 1;
	  }
	return -2;
}

int _stdcall ZGetCountyNameFromNumberA(LPSTR county, int cn)
{
	return GetCountyNameFromNumberA(county, cn);
}

//----------------------------------------------------------------------------

//int __declspec(dllexport) GetCountyNameFromNumberW(LPWSTR wcounty, int cn)
int GetCountyNameFromNumberW(LPWSTR wcounty, int cn)
{
	char county[61];
	int result;

	wcounty[0] = 0;
	result = GetCountyNameFromNumberA(county, cn);
	if (result < 0) return result;
	Widen(wcounty, county);
	return result;
}

int _stdcall ZGetCountyNameFromNumberW(LPWSTR wcounty, int cn)
{
	return GetCountyNameFromNumberW(wcounty, cn);
}

//----------------------------------------------------------------------------

void CountyFromFile(int nsoc, int ncounty, LPSTR county)
{
	int ncsave;

	ncsave = m_nCurrentCountry;
	m_nCurrentCountry = nsoc;
	county[0] = '\0';				// in case GetCountyNameFromNumber fails
	GetCountyNameFromNumberA(county, ncounty);
	m_nCurrentCountry = ncsave;
}

//----------------------------------------------------------------------------
// Get county name, return county number or error code:
// -1 no soc selected
// -2 index out of range or no counties for selected soc

//short __declspec(dllexport) GetCountyAtIndexA(LPSTR countyname, int cindex)
short GetCountyAtIndexA(LPSTR countyname, int cindex)
{
	short int cnum;
	int rn;
	int ctr, clen, nfile;
	char * cp;
	short int * iptr;
	FILE * cfile;

	countyname[0] = '\0';
	if (m_nCurrentCountry < 0) return -1;
	if (cindex < 0 || cindex + m_nCountyLows[m_nCurrentCountry] > m_nCountyHighs[m_nCurrentCountry]) return -2;
	nfile = m_nFileIndex[m_nCurrentCountry];
	if (nfile == 4) return -2;
	if (m_nVersion < 20)
	  {
		rn = m_nCountyLows[m_nCurrentCountry] + cindex;
		cp = m_pCounties + rn * 26;
		iptr = (short int *) cp;
		cnum = *iptr;
		if (m_nCurrentCountry == m_nMexico) strcpy(countyname, mexstates[cnum - 1]);
		else
		  {
			cp += 2;
			for (ctr = 0; ctr < 24; ctr++) countyname[ctr] = *cp++;
			while (ctr > 0 && countyname[ctr - 1] == ' ') --ctr;
			countyname[ctr] = '\0';
		  }
		return cnum;
	  }
	cfile = m_fpData[nfile];
	fseek(cfile, m_nCountyPointers[m_nCurrentCountry], SEEK_SET);
	for (rn = 0; rn <= cindex; rn++)
	  {
		fread(&cnum, 2, 1, cfile);
		clen = fgetc(cfile) + 1;
		fread(countyname, 1, clen, cfile);
                // New GPD 24 May 2005
                ProcessAtlasString(countyname, clen-1);
                // End GPD
		for (ctr = clen; ctr > 0; ctr--) if (countyname[ctr - 1] != ' ') break;
		countyname[ctr] = '\0';
	  }
	return cnum;
}

short _stdcall ZGetCountyAtIndexA(LPSTR countyname, int cindex)
{
	return GetCountyAtIndexA(countyname, cindex);
}

//short __declspec(dllexport) GetCountyAtIndexW(LPWSTR countyname, int cindex)
short GetCountyAtIndexW(LPWSTR countyname, int cindex)
{
	char ccounty[61];
	int result;
	countyname[0] = 0;
	result = GetCountyAtIndexA(ccounty, cindex);
	if (result < 0) return result;
	Widen(countyname, ccounty);
	return result;
}

short _stdcall ZGetCountyAtIndexW(LPWSTR countyname, int cindex)
{
	return GetCountyAtIndexW(countyname, cindex);
}

//-------------------------------------------------------------------------------------------------------------------------

//void __declspec(dllexport) GetRevDateA(LPSTR revdate)
void GetRevDateA(LPSTR revdate)
{
	strcpy(revdate, m_strRevDate);
}

void _stdcall ZGetRevDateA(LPSTR revdate)
{
	strcpy(revdate, m_strRevDate);
}

//void __declspec(dllexport) GetRevDateW(LPWSTR wrevdate)
void GetRevDateW(LPWSTR wrevdate)
{
	Widen(wrevdate, m_strRevDate);
}

void _stdcall ZGetRevDateW(LPWSTR wrevdate)
{
	Widen(wrevdate, m_strRevDate);
}

