// 1-25-2013 convert new atlas to python
#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int num_dups, dup_num;
static int countydup, latitude, longitude, typetable, zonetable, soc;
static int first_index;  // for duplicates
static int D, M, Y, H, Mi;
static short ttout, tzout, flagout;
static char Lat[20], Longg[20], ZoneName[15];
static char country[40], county[40], city[100];
int LookupCityA(char *placename, char *city, char *county, char *country, int* countydup, int* latitude, int* longitude, int* typetable, int* zonetable, int* soc);
int GetIndexOfCityA(char *cityname, int * lastbefore);
short ACStimechangelookupA(short Month, short Day, short Year, short Hour, short Minute, short tzin, short ttin, short * tzout,	short * ttout, short * flagout, char *ZoneName);
int GetCountyNameFromNumberA(char *county, int cn);
int GetCountryCount(void);
int GetCountryAtIndexA(char *countryname, char *abbreviation, int countrynumber);
int GetDataFields(int citynumber, int * datafields);
int SetCountryByNameA(char *countryname);
int SetPathsEmbeddedA(char *pMainFiles, char *pPersonal, char *pAtlasFileUS, char *pAtlasFileInt, char *pAtlasFileTT, char *pEncryptionString);
int GetCityCount(void);
int SetCountryByIndex(int countrynumber);
int GetCityAtIndexA(char *cityname, int citynumber, char *MarkPrimary);
void SetDupReturnFlag(int n);

// call with city,state/country
PyObject *lookup_city(PyObject *self, PyObject *args)
{
	char *place, code[5], *s;
	int n, i=-1;
	int index, next;
	char error[50];

	if (!PyArg_ParseTuple(args, "s", &place))
		return NULL;
	n = LookupCityA(place, city, county, country, &countydup, &latitude, &longitude, &typetable, &zonetable, &soc);
	if (n >= 0)
	{
		char u = 'N', ul = 'W';
		int d, dl, m, ml;
		if (countydup/1000 != 0) // duplicate city
		{
			index = GetIndexOfCityA(city, &next);
			int datafields[6], dup, numdups=1;
			first_index = index-1;
			index++;  // look at next city to see if it is also a dup
			while(1)
			{
				dup = GetCityAtIndexA(city, index, "");
				numdups++;
				if (dup == 0)  break;
				index++;
			}
			sprintf(error, "%d Duplicate Cities", numdups);
			return Py_BuildValue("ss", error, "Duplicate");
		}
		d = latitude/3600;
		m = (latitude - d*3600) /  60. + 0.5;
		if (latitude < 0)
		{
			u = 'S';
			if (d < 0) d = -d;
			if (m < 0) m = -m;
		}
		dl = longitude/3600;
		ml = (longitude - dl*3600) /  60. + 0.5;
		if (longitude < 0)
		{
			ul = 'E';
			if (dl < 0) dl = -dl;
			if (ml < 0) ml = -ml;
		}
		sprintf(Lat, "%d%c%02d", d, u , m);
		sprintf(Longg, "%d%c%02d", dl, ul, ml);
		index = GetIndexOfCityA(city, &next);
		return Py_BuildValue("sssii", Lat, Longg, county, soc, index);
	}
	if (n < 0)
	{
		if (n == -1) sprintf(error, "City %s not found", place);
		else if (n == -2) strcpy(error, "State/Country not found");
		else if (n == -3)  strcpy(error, "No state or country");
		else  if (n == -4) strcpy(error, "Country not found");
		else if (n == -5) strcpy(error, "Atlas not found");
		else strcpy(error, "Other Atlas Error\n");
		return Py_BuildValue("ss", error, "Error");
	}
	return Py_BuildValue("ss", "", "");  // to keep compiler from complaining!
}

PyObject *timechangelookup(PyObject *self, PyObject *args)
{
	char *date1, *time, *s;
	char date[15];
	if (!PyArg_ParseTuple(args, "ss", &date1, &time))
		return NULL;
	strcpy(date, date1);
	s = strtok(date, "/. ");
	if (s) M = atoi(s);
	else M = 0;
	s = strtok(NULL, "/. ");
	if (s) D = atoi(s);
	else D = 0;
	s = strtok(NULL, "/. ");
	if (s) Y = atoi(s);
	else Y = 0;
	H = atoi(time);
	s = strchr(time, ':');
	Mi = 0;
	if (s) Mi = atoi(s+1);
	s = strpbrk(time, "AaPp");  // look for AM/am/PM/pm start
	if (s && (*s == 'P' || *s == 'p') && H != 12)
		H += 12;
	else if (H == 12)   // 12 midnight
		H = 0;
	if (D == 0 || M == 0 || Y == 0)
		return Py_BuildValue("ss", "Bad date", "Error");
	short l = ACStimechangelookupA(M, D, Y, H, Mi, zonetable, typetable, &tzout, &ttout, &flagout, ZoneName);
	if (l < 0)
	{
		char error[50];
		if (l == -1) strcpy(error, "Invalid table number");
		else if (l == -2) strcpy(error, "Time Table not Found");
		else strcpy(error, "Other Time Table Error");
		return Py_BuildValue("ss", error, "Error");
	}

	float TZ = tzout/900.;
	char tztext[50];
	if (ttout == 0)
		;
	else if (ttout == 1 || ttout == 2)
		TZ -= 1;
	else if (ttout == 3)
		TZ -= 2;
	else if (ttout == 4)
		TZ = (longitude/60.)/900.;
	else if (ttout == 5)
		TZ -= 0.5;
	if (ttout < 4 && (int)TZ == TZ)
		sprintf(tztext, "%4.2f", TZ);
	else
	{
		int da, min, s;
		da = (int)TZ;
		min = (int)((TZ - da) * 60.);
		s = (int)((((TZ-da)*60.) - min)*60.);
		if (min < 0 && da != 0) min = -min;
		if (s < 0 && (min != 0 || da != 0)) s = -s;
		if (s != 0)
			sprintf(tztext, "%d:%02d:%02d", da, min, s);
		else
			sprintf(tztext, "%d:%02d", da, min);
	}
	return Py_BuildValue("ss", ZoneName, tztext);
}

PyObject *atlas_dups(PyObject *self, PyObject *args)
{
	int i, n;
	int d, m, dl, ml, latitude, longitude;
	char u='N', ul='W';
	char *date1=NULL, *timE=NULL, *s;
	char date[15], City[40], TIME[20];
	if (!PyArg_ParseTuple(args, "i|ss", &i, &date1, &timE))
			return NULL;
	if (date1 == NULL)
	{
	  struct tm *T;
	  time_t timer = time(NULL);
	  T = localtime(&timer);
	  sprintf(date, "%d/%02d/%d", T->tm_mon+1, T->tm_mday, T->tm_year+1900);
	  sprintf(TIME, "%d:%02d", T->tm_hour, T->tm_min);
	}
	else
	{
	    strcpy(date, date1);
	    strcpy(TIME, timE);
	}
	s = strtok(date, "/. ");
	if (s) M = atoi(s);
	s = strtok(NULL, "/. ");
	if (s) D = atoi(s);
	s = strtok(NULL, "/. ");
	if (s) Y = atoi(s);
	H = atoi(TIME);
	s = strchr(TIME, ':');
	Mi = 0;
	if (s) Mi = atoi(s+1);
	s = strpbrk(TIME, "AaPp");  // look for AM/am/PM/pm start
	if (s && (*s == 'P' || *s == 'p') && H != 12)
		H += 12;
	else if (H == 12)   // 12 midnight
		H = 0;
	int datafields[6], dup;
	int index = first_index+i;
	dup = GetCityAtIndexA(City, index, "");
	n = GetDataFields(index, datafields);
	if (n < 0)
	{
		if (n == -1)
			return Py_BuildValue("ss", "State/Country Not Set", "Error");
		else if (n == -2)
			return Py_BuildValue("ss", "City Number Out of Range", "Error");
	}
	if (dup == 1)
	  sprintf(city, "*%s", City); // mark as primary
	else
	  strcpy(city, City);
	GetCountyNameFromNumberA(county, datafields[0]%1000);
	latitude = datafields[1];
	longitude = datafields[2];
	typetable = datafields[3];
	zonetable = datafields[4];
	d = latitude/3600;
	m = (latitude - d*3600) /  60. + 0.5;
	if (latitude < 0)
	{
		u = 'S';
		if (d < 0) d = -d;
		if (m < 0) m = -m;
	}
	dl = longitude/3600;
	ml = (longitude - dl*3600) /  60. + 0.5;
	if (longitude < 0)
	{
		ul = 'E';
		if (dl < 0) dl = -dl;
		if (ml < 0) ml = -ml;
	}
	sprintf(Lat, "%d%c%02d", d, u , m);
	sprintf(Longg, "%d%c%02d", dl, ul, ml);
	short l = ACStimechangelookupA(M, D, Y, H, Mi, zonetable, typetable, &tzout, &ttout, &flagout, ZoneName);
	float TZ = tzout/900.;
	char tztext[50];
	if (ttout == 0)
		;
	else if (ttout == 1 || ttout == 2)
		TZ -= 1;
	else if (ttout == 3)
		TZ -= 2;
	else if (ttout == 4)
		TZ = (longitude/60.)/900.;
	else if (ttout == 5)
		TZ -= 0.5;
	if (ttout < 4 && (int)TZ == TZ)
		sprintf(tztext, "%4.2f", TZ);
	else
	{
		int da, min, s;
		da = (int)TZ;
		min = (int)((TZ - da) * 60.);
		s = (int)((((TZ-da)*60.) - min)*60.);
		if (min < 0 && da != 0) min = -min;
		if (s < 0 && (min != 0 || da != 0)) s = -s;
		if (s != 0)
			sprintf(tztext, "%d:%02d:%02d", da, min, s);
		else
			sprintf(tztext, "%d:%02d", da, min);
	}
	return Py_BuildValue("ssssss", Lat, Longg, tztext, ZoneName, county, city);
}


PyObject *country_count(PyObject *self, PyObject *args)
{
	int n = GetCountryCount();
	return Py_BuildValue("i", n);
}

PyObject *next_country(PyObject *self, PyObject *args)
{
        int i, n;
        if (!PyArg_ParseTuple(args, "i", &i))
                return NULL;
		char country[65], abbrev[12];
		n = GetCountryAtIndexA(country, abbrev, i);
		if (n == 0)
			return Py_BuildValue("ss", "Bad country number", "Error");
		return Py_BuildValue("ss", country, abbrev);
}

PyObject *city_count(PyObject *self, PyObject *args)
{
	int n = GetCityCount();
	return Py_BuildValue("i", n);
}

PyObject *next_city(PyObject *self, PyObject *args)
{
        int i;
        if (!PyArg_ParseTuple(args, "i", &i))
                return NULL;
		char country[65], dupnum[20];
		int n = GetCityAtIndexA(country, i, "");
		if (n == -1)
			return Py_BuildValue("ss", "Country not Set", "Error");
		else if (n == -2)
			return Py_BuildValue("ss", "Index out of Range", "Error");
		else if (n == -3)
			return Py_BuildValue("ss", "Country's data file not Opened", "Error");
		sprintf(dupnum, "%d", n);
		return Py_BuildValue("ss", country, dupnum);
}

PyObject *set_country(PyObject *self, PyObject *args)
{
        char *i;
	int j, n;
	PyObject *y;

        n = 1;
        if (!PyArg_ParseTuple(args, "O", &y))
                return NULL;
	//int n = PyString_Check(y);
	if (n)  // a string
	{
		PyArg_ParseTuple(args, "s", &i);
		n = SetCountryByNameA(i);
	}
	else
	{
		PyArg_ParseTuple(args, "i", &j);
		n = SetCountryByIndex(j);
	}
	return Py_BuildValue("i", n);
}


// name mapping
static PyMethodDef AtlasMethods[] = 
{
	{"LookupCity", lookup_city, METH_VARARGS, "Location of City"},
	{"TimeChangeLookup", timechangelookup, METH_VARARGS, "Location of City"},
	{"DupCity", atlas_dups, METH_VARARGS, "Location of City"},
	{"CountryCount", country_count, METH_NOARGS, "Location of City"},
	{"CountryAtIndex", next_country, METH_VARARGS, "Location of City"},
	{"CityCount", city_count, METH_NOARGS, "Location of City"},
	{"CityAtIndex", next_city, METH_VARARGS, "Location of City"},
	{"SetCountry", set_country, METH_VARARGS, "Location of City"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef atlasmodule = {
    PyModuleDef_HEAD_INIT,
    "atlas",
    "Python interface ACS Atlas",
    -1,
    AtlasMethods
};

// Module initialization
PyMODINIT_FUNC PyInit_atlas(void) {
	int i;
	char fileNamePrefix[4], mPath[75];
	char atlasFileUS[10];
	char atlasFileInt[10];
	char atlasFileTT[10];
	char enc[25];
	strcpy(enc, "z97iroJTa37yipD");
	strcpy(fileNamePrefix, "dk");
	//append prefixes to file names
	strcpy(atlasFileUS, fileNamePrefix);
	strcat(atlasFileUS, "ua.dat");
	strcpy(atlasFileInt, fileNamePrefix);
	strcat(atlasFileInt, "ia.dat");
	strcpy(atlasFileTT, fileNamePrefix);
	strcat(atlasFileTT, "tt.dat");
	FILE *fd  = fopen("acsatlas.txt", "r");
	if (fd == NULL)
		strcpy(mPath, "atlas/");
	else
	{
		fgets(mPath, 74, fd);
		char *s = strchr(mPath, '\n');
		if (s) *s = '\0';
	}
	int mm = SetPathsEmbeddedA(mPath, mPath, atlasFileUS, atlasFileInt, atlasFileTT, enc);
	SetDupReturnFlag(1);
	return PyModule_Create(&atlasmodule);
}
