#ifndef LinuxPorting
#define LinuxPorting

#define PATH_DELIMITER '/'

#include <stdbool.h>
#include <wchar.h>

//WIN-specific types
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t wchar;
typedef wchar* LPWSTR; 
typedef const wchar* LPCWSTR; 
typedef int HANDLE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef int BOOL;
typedef struct _WIN32_FIND_DATA {
	DWORD nFileSizeLow;
} WIN32_FIND_DATA;
typedef struct _SYSTEMTIME {
  WORD wYear;
	WORD wMonth;
	WORD wDay;
} SYSTEMTIME;

#define TRUE 1
#define FALSE 0

#define stricmp strcasecmp
#define wsprintf sprintf 

#define _stdcall 
#define INVALID_HANDLE_VALUE -1
#define _SH_DENYWR 0

void CharToOem(LPCSTR str, char *wstr);
HANDLE FindFirstFile(LPSTR filepath, WIN32_FIND_DATA *fdata);
void FindClose(HANDLE file);
void GetLocalTime(SYSTEMTIME *date);
/* Dummy functions doing and returning nothing just to allow dependent code to compile */
void _ASSERT(bool b);
FILE *_fsopen(LPSTR path, LPCSTR rw, int access); 
#endif
