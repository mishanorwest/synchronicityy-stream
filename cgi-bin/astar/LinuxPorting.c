#include "LinuxPorting.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

/* Inner functions for processing assistance */
char* extractPath(char* filepath) {
	char delimiter = PATH_DELIMITER;
	short indexOfDelimiter = -1;
	short i = 0;
	for (i = 0; i < strlen(filepath); i++) {
		if (filepath[i] == delimiter) {
			indexOfDelimiter = i;
		}
	}
//Now indexOfDelimiter is the index of last slash in the path
	if (indexOfDelimiter < 0) return NULL;
	char* result = (char*)malloc((indexOfDelimiter + 2)*sizeof(char));
	strncpy(result, filepath, indexOfDelimiter + 1); 	
	result[indexOfDelimiter + 1] = '\0';
	return result;
}

char* extractFileName(char* filepath) {
	char delimiter = PATH_DELIMITER;
	short fpLen = 0;//length of File Path
	short fnLen = 0;//length of File Name
	short indexOfDelimiter = -1;
	short i = 0;
	for (i = 0; i < strlen(filepath); i++) {
		if (filepath[i] == delimiter) {
			indexOfDelimiter = i;
		}
	}
//Now indexOfDelimiter is the index of last slash in the path
	if (indexOfDelimiter < 0) return NULL;
	fpLen = strlen(filepath);
	fnLen = (fpLen - indexOfDelimiter);
	char* result = (char*)malloc(fnLen*sizeof(char));
	strncpy(result, filepath+indexOfDelimiter+1, fnLen - 1); 	
	result[fnLen - 1] = '\0';
	return result;
}

bool matchesFilename(char* fileName, char* fileTemplate) {
	char* wildCard = NULL;
	short wildCardIndex = -1;
	short templLen = strlen(fileTemplate);
	wildCard = strstr(fileTemplate, "*");
	if (wildCard == NULL) {
		return (strcmp(fileName, fileTemplate) == 0);
	} else {
		wildCardIndex = wildCard - fileTemplate;
		return (
			strncmp(fileName, fileTemplate, wildCardIndex) == 0 &&
			strncmp(
				fileName+strlen(fileName)-(templLen-wildCardIndex-1), 
				fileTemplate+templLen-(templLen-wildCardIndex-1),
				wildCardIndex-1
			) == 0
		);
	}
}

/* Implementation of functions from LinuxPorting.h */
void CharToOem(LPCSTR str, char* wstr) {
//TODO: implement conversion from ANSI to OEM charset

}

HANDLE FindFirstFile(LPSTR filepath, WIN32_FIND_DATA *fdata) {
	DIR *dir;
	struct dirent *dp;
	char* dirpath;
	char* filename;
	FILE* fp;
	dirpath = extractPath(filepath);
	filename = extractFileName(filepath);

	if ((dir = opendir (dirpath)) != NULL) {
		while ((dp = readdir (dir)) != NULL) {
			if (matchesFilename(dp->d_name, filename)) {
				if (fdata == NULL) break;
				fp = fopen(filepath, "r");
				if (fp == NULL) {
					continue;
				}
				fseek(fp, 0, SEEK_END);
				fdata->nFileSizeLow = ftell(fp);
				fclose(fp);
				closedir(dir);
				free(dirpath);
				free(filename);
				return 0;
			}
		}
		closedir(dir);
	}
	free(dirpath);
	free(filename);
	
	return INVALID_HANDLE_VALUE;
}

void FindClose(HANDLE file) {
//TODO: implement resource deallocation according to POSIX 
//I'm not sure if it's safe to be implemented to free the resources 
//allocated in FindFirstFile here, because it may be used concurrently
//so for now it will just be dummy
}

void GetLocalTime(SYSTEMTIME* date) {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	date->wDay = timeinfo->tm_mday;
	date->wMonth = timeinfo->tm_mon;
	date->wYear = timeinfo->tm_year;
}

/* Dummy functions doing and returning nothing just to allow dependent code to compile */
void _ASSERT(bool b) {
//dummy _ASSERT replacement of macros used for debugging
}

FILE* _fsopen(LPSTR path, LPCSTR rw, int access) {
	return NULL;
}


