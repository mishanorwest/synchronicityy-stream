// FindCity.h: declarations for FindCity DLL
#include <stdio.h>
#ifdef _WIN32
#define PATH_DELIMITER '\\'
#include <windows.h>
#include <io.h>
#elif defined __unix__ || defined __APPLE__
#include "LinuxPorting.h"
#endif
#include <fcntl.h>

#define MAXENTRIES 350
#define MAXALTERNATES 350

// city structure
struct CityInfo
{
	char Name[61];
	char IsDuplicate;
	int CountyNumber;
	int TypeTable;
	int ZoneTable;
	int Latitude;
	int Longitude;
};

struct SoundexInfo
{
	short int handle;
	char match[5];
	int nsoc;
	long FileOffset;
	struct SoundexInfo * pnext;
};

// functions

int FindAtlasFiles(char * filename);
void ReadVersion1(LPCSTR path);
void ReadVersion2(LPCSTR path);
void ReadPersonal(LPCSTR pathname);
//void ReadCityPointers();
void InitializeAtlasVariables();
void CloseAtlasFiles();
int noblankstricmp(const char *sin1, char *sin2);
int GetCityForLookup(LPSTR cityname, int citynumber);
void ReadDataFields(int citynumber, int * datafields);
void CountyFromFile(int nsoc, int ncounty, LPSTR county);
void Widen(LPWSTR wdest, LPCSTR csrc);
void Narrow(LPSTR cdest, LPCWSTR wsrc);
#if defined __unix__ || defined __APPLE__
/********** export declarations for .so lib **********/
//Initialization
int SetPathsA(LPCSTR pMainFiles, LPCSTR pPersonal);
int SetPathsW(LPCWSTR pMainFiles, LPCWSTR pPersonal);
int SetPathsEmbeddedA(LPCSTR pMainFiles, LPCSTR pPersonal,
                                 LPCSTR pAtlasFileUS, LPCSTR pAtlasFileInt, LPCSTR pAtlasFileTT,
                                 LPCSTR pEncryptionString);
int SetPathsEmbeddedW(LPCWSTR pMainFiles, LPCWSTR pPersonal,
                                 LPCWSTR pAtlasFileUS, LPCWSTR pAtlasFileInt, LPCWSTR pAtlasFileTT,
                                 LPCWSTR pEncryptionString);
void SetReturnAbbreviations(int bretabbr);
void SetDupReturnFlag(int dupret);
void SetIllinoisTreatment(int IllinoisAction);
void SetHalfHourDST(int HalfDST);
int VersionNumber();
//Countries and US States
int GetCountryCount();
int GetCountryAtIndexA(LPSTR countryname, LPSTR abbreviation, int countrynumber);
int GetCountryAtIndexW(LPWSTR wcountryname, LPWSTR wabbreviation, int countrynumber);
int GetAlternateCount();
int GetAlternateAtIndexA(LPSTR AltName, LPSTR AltAbbrev, int nalt);
int GetAlternateAtIndexW(LPWSTR wAltName, LPWSTR wAltAbbrev, int nalt);
int SetCountryByIndex(int countrynumber);
int SetCountryByNameA(LPCSTR countryname);
int SetCountryByNameW(LPCWSTR countryname);
//Cities
int GetCityCount();
int GetCityAtIndexA(LPSTR cityname, int citynumber, LPCSTR MarkPrimary);
int GetCityAtIndexW(LPWSTR wcityname, int citynumber, LPCWSTR MarkPrimary);
int GetIndexOfCityA(LPCSTR cityname, int * lastbefore);
int GetIndexOfCityW(LPCWSTR cityname, int * lastbefore);
int GetIndexOfCityInCountyA(LPCSTR cityname, int countynumber, int * lastbefore);
int GetIndexOfCityInCountyW(LPCWSTR wcityname, int countynumber, int * lastbefore);
int FindCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country, int * datafields);
int FindCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country, int * datafields);
int LookupCityA(LPCSTR placename, LPSTR city, LPSTR county, LPSTR country,
  int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc);
int LookupCityW(LPCWSTR placename, LPWSTR city, LPWSTR county, LPWSTR country,
	int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc);
short SoundexFirstCityA(LPCSTR placename, LPSTR city, LPSTR county);
short SoundexFirstCityW(LPCWSTR placename, LPWSTR city, LPWSTR county);
short SoundexNextCityA(short handle, LPSTR city, LPSTR county);
short SoundexNextCityW(short handle, LPWSTR city, LPWSTR county);
short SoundexClose(short handle);
int GetDataFields(int citynumber, int * datafields);
int GetCityData(int citynumber, int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc);
//Administrative Divisions (counties)
short GetCountyCount();
short GetCountyAtIndexA(LPSTR countyname, int cindex);
short GetCountyAtIndexW(LPWSTR countyname, int cindex);
int GetCountyNameFromNumberA(LPSTR county, int cn);
int GetCountyNameFromNumberW(LPWSTR wcounty, int cn);
int GetCountyNumberFromNameA(LPCSTR county);
int GetCountyNumberFromNameW(LPCWSTR wcounty);
//Time Zone and Type
short ACStimechangelookupA(short Month, short Day, short Year, short Hour, short Minute,
	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPSTR ZoneName);
short ACStimechangelookupW(short Month, short Day, short Year, short Hour, short Minute,
	short tzin, short ttin, short * tzout,	short * ttout, short * flagout, LPWSTR ZoneName);
int GetZoneCount();
int GetZoneNameAtIndexA(LPSTR zonename, int zindex);
int GetZoneNameAtIndexW(LPWSTR zonename, int zindex);
void GetZoneNameA(LPSTR zname, int zone);
void GetZoneNameW(LPWSTR wzname, int zone);
int GetZoneFromNameA(LPCSTR zonename);
int GetZoneFromNameW(LPCWSTR zonename);
void GetZoneTypeAbbreviationA(LPSTR ztabbr, int zone, int type);
void GetZoneTypeAbbreviationW(LPWSTR wztabbr, int zone, int type);
int MakeChangesTable(int nsoc, int ncity, int endyear, int bGMT, int bExpandUS);
int BuildChangesTable(int tzin, int ttin, int endyear, int bGMT, int bExpandUS);
int GetChangeAtIndex(int tcindex, int * zone, int * type);
int AddToPersonalA(LPCSTR city, int * cityfields);
int AddToPersonalW(LPCWSTR city, int * cityfields);
int AddPlaceToPersonalA(LPCSTR city, int county, int latitude, int longitude, int typetable, int zonetable);
int AddPlaceToPersonalW(LPCWSTR city, int county, int latitude, int longitude, int typetable, int zonetable);
int DeleteFromPersonalByIndex(int citynumber);
int DeleteFromPersonalByNameA(LPCSTR cityname);
int DeleteFromPersonalByNameW(LPCWSTR wcityname);
#endif
