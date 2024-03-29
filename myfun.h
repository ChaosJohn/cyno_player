#ifndef _MYFUN_H
#define _MYFUN_H 

#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "mytype.h"

#define BUFSIZE 100

void 
initParameter(ParasPointer pp, 
    char *parameterSrc); 

char *
getFilePath(char *rawPath);

char * 
getDate(void); 

void 
newLine(void); 

char * 
concatenateName(char *formerHalf, 
    char *latterHalf);

char *
generateRecordsFileName(char *dir, 
    char *bus_id);

int 
isFileExists(char *fileName); 

RecordList 
createRecords(RecordList records, 
    ParasPointer parameter); 

void
initRecords(RecordList *records);

void 
printRecords(RecordList records); 

void 
createPlaylist(PlayItemList playlist, 
    char *playlistSrc); 

void 
initPlaylist(PlayItemList *playlist); 

void 
printPlaylist(PlayItemList playlist);

void 
mergeRecords(RecordList records, 
    PlayItemList playlist);

int 
isInRecords(RecordList records, 
    char *mediaName);

int 
isInPlaylist(PlayItemList playlist, 
    char *mediaName); 

void 
initInsertionList(InsertionList *insertions); 

int 
createInsertionList(InsertionList insertions, 
    char *insertionsSrc); 

void 
printInsertionList(InsertionList insertions);

void 
saveRecords(RecordList records, 
    char *dir, 
    char *bus_id);

void 
changeRecords(RecordList records, 
    char *mediaName);

void 
saveLastPlay(char *lastplaySrc, char *mediaName);

char * 
loadLastPlay(char *lastplaySrc); 

PlayItemList 
checkPlaylist(PlayItemList playlist, 
    char *videoDir); 

#endif

