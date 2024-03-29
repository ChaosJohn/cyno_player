#include "myfun.h"


/* 
 * initParameter: 从参数文件"parameterSrc"中读入参数并且初始化
 */
void 
initParameter(ParasPointer pp, 
    char *parameterSrc) {
  FILE *parameterFile = NULL; 
  if((parameterFile = fopen(parameterSrc, "rt"))) {
    char buf[BUFSIZE];  
    // 初始化“媒体文件目录”参数
    if(fgets(buf, BUFSIZE, parameterFile)) {
      pp->mediaDir = (char *)malloc(BUFSIZE * sizeof(char));
      sscanf(buf, "%s", pp->mediaDir); 
    }
    // 初始化“工作目录”参数 
    if(fgets(buf, BUFSIZE, parameterFile)) {
      pp->workingDir = (char *)malloc(BUFSIZE * sizeof(char));
      sscanf(buf, "%s", pp->workingDir); 
    }
    // 初始化“播放列表文件名（处于工作目录中）”参数 
    if(fgets(buf, BUFSIZE, parameterFile)){
      pp->playlistSrc = (char *)malloc(BUFSIZE * sizeof(char));
      sscanf(buf, "%s", pp->playlistSrc); 
      pp->playlistSrc = concatenateName(pp->workingDir, pp->playlistSrc); 
    }
    // 初始化“插播列表文件名（处于工作目录中）”参数 
    if(fgets(buf, BUFSIZE, parameterFile)) {
      pp->insertionsSrc = (char *)malloc(BUFSIZE * sizeof(char));
      sscanf(buf, "%s", pp->insertionsSrc); 
      pp->insertionsSrc = concatenateName(pp->workingDir, pp->insertionsSrc); 
    }
    // 初始化“记录上回播放的文件（处于工作目录中）”参数 
    if(fgets(buf, BUFSIZE, parameterFile)) {
      pp->lastPlaySrc = (char *)malloc(BUFSIZE * sizeof(char));
      sscanf(buf, "%s", pp->lastPlaySrc); 
      pp->lastPlaySrc = concatenateName(pp->workingDir, pp->lastPlaySrc); 
    }
    // 初始化播放器的位置大小参数
    if(fgets(buf, BUFSIZE, parameterFile)) 
      sscanf(buf, "%d %d %d %d %d", &(pp->posi_x), 
          &(pp->posi_y), &(pp->posi_w), 
          &(pp->posi_h), &(pp->rotate));

    // 初始化播放器播放文件出错时播放的示例视频
    if(fgets(buf, BUFSIZE, parameterFile)) {
      pp->fixedMedia = (char *)malloc(BUFSIZE * sizeof(char)); 
      sscanf(buf, "%s", pp->fixedMedia); 
      pp->fixedMedia = concatenateName(pp->workingDir, pp->fixedMedia); 
    }

    // 初始化bus_id
    if(fgets(buf, BUFSIZE, parameterFile)) {
      pp->bus_id = (char *)malloc(BUFSIZE * sizeof(char)); 
      sscanf(buf, "%s", pp->bus_id); // 得到bus_id文件在workingDir中的相对路径，赋给pp->bus_id
      pp->bus_id = concatenateName(pp->workingDir, pp->bus_id);   // 得到bus_id文件的绝对路径，赋给pp->bus_id 
      FILE *bus_id_file = NULL; 
      if((bus_id_file = fopen(pp->bus_id, "rt"))) { // 打开bus_id文件
        if (fgets(buf, BUFSIZE, bus_id_file)) {
          sscanf(buf, "%s", pp->bus_id);  // 读取bus_id文件中的bus_id,赋给pp->bus_id 
        } 
        fflush(bus_id_file); 
        fclose(bus_id_file); 
      } else {
        printf("Failed to get bus_id \n"); 
        exit(0); 
      } 
    } 

    printf("Initialize parameters successfully\n"); 
    fflush(parameterFile); 
    fclose(parameterFile);
    return;
  } else {
    printf("Failed Initialize parameters\n"); 
    exit(0); 
  }
}


/* 
 * getDate: 得到今天的日期，格式是“YYYY-MM-DD”
 */
char * 
getDate(void) {
  int num = 11; 

  char *pStr = (char *)malloc(num * sizeof(char)); 

  time_t curTime = time(NULL); 
  strftime(pStr, num, "%Y-%m-%d", localtime(&curTime)); 
  /*printf("Today is %s\n", pStr); */

  return pStr; 
}

/* 
 * getFilePath: 给系统路径加上"file://"的前缀
 */
char *
getFilePath(char *rawPath) {
  char prefix[] = "file://";

  return concatenateName(prefix, rawPath);
}

/* 
 * newLine: 自定义的换行函数
 */
void 
newLine(void) {
  printf("\n"); 
}

/* 
 * concatenateName: 输入两个字符串，将其连接后返回
 */
char * 
concatenateName(char *formerHalf, 
    char *latterHalf) {
  char *fh = formerHalf; 
  char *lh = latterHalf;

  // Get the lengths of formerHalf and latterHalf. 
  int fh_length = 0; 
  int lh_length = 0;
  for(; *fh; fh++, fh_length++); 
  for(; *lh; lh++, lh_length++);

  // Allocate memory for destination. 
  /*formerHalf = (char *)realloc(formerHalf, (fh_length + lh_length + 1) * sizeof(char));*/
  char *destination = (char *)malloc((fh_length + lh_length + 1) * sizeof(char));

  /*while((*fh++ = *latterHalf++));*/

  char *index = destination;
  while((*index++ = *formerHalf++)); 
  index--; 
  while((*index++ = *latterHalf++));

  /*return formerHalf;*/
  return destination;
}

/* 
 * generateRecordsFileName: 输入工作目录的路径dir，得到当天的记录文件的文件路径
 * [/home/.../workingDir/YYYY-MM-DDlist.txt] 
 */
char *
generateRecordsFileName(char *dir, 
    char *bus_id) {
  char *prefix = concatenateName(bus_id, "_"); 
  char *today = getDate();
  char *suffix = "list.txt"; 
  char *workspace="playstatelist/";
  return concatenateName(concatenateName(dir,workspace), concatenateName(prefix, concatenateName(today, suffix))); 
}

/* 
 * createRecords: 创建“日志空间”
 * 如果工作目录中存在当天的日志文件，则从该日志文件中读取记录回“日志空间”
 */
RecordList 
createRecords(RecordList records, 
    ParasPointer parameter){
  char *recordsFileName = generateRecordsFileName(parameter->workingDir, parameter->bus_id);
  if(isFileExists(recordsFileName)) {
    FILE *recordsFile = fopen(recordsFileName, "rt"); 
    RecordList temp = NULL; 
    RecordList cursor = records; 

    char buf[BUFSIZE]; 
    char *item;
    int times;
    while(fgets(buf, BUFSIZE, recordsFile) != NULL) {
      if(buf[0] == '\n' || buf[0] == ' ') {
        break; 
      } else {
        temp = (RecordList)malloc(sizeof(RecordNode)); 
        item = (char *)malloc(BUFSIZE * sizeof(char));
        sscanf(buf, "%s %d", item, &times); 
        temp->mediaName = item; 
        temp->times = times; 
        temp->next = NULL; 
        cursor->next = temp; 
        cursor = cursor->next; 
      }
    }

    fflush(recordsFile); 
    fclose(recordsFile);
  } else {
    // 清空lastplay文件
    remove(parameter->lastPlaySrc); 
    fclose(fopen(parameter->lastPlaySrc, "wt"));
  }   
  return records; 
}

/* 
 * initRecords: 初始化指向“日志空间”的指针
 */
void 
initRecords(RecordList *records) {
  (*records) = (RecordList)malloc(sizeof(RecordNode)); 
  (*records)->next = NULL; 
}

/* 
 * isFileExists: 判断文件知否存在
 * fileName可以是相对路径（相对于程序目录），也可以是绝对路径
 */
int 
isFileExists(char *fileName) {
  FILE *file = NULL; 
  if((file = fopen(fileName, "r"))) {
    fclose(file); 
    printf("File named %s exists\n", fileName); 
    return 1; 
  }
  printf("No such file named: %s\n", fileName);
  return 0; 
}

/* 
 * printRecords: 打印出“日志空间”中的记录，用于测试
 */
void 
printRecords(RecordList records) {
  RecordList cursor = records; 
  while(cursor->next) {
    cursor = cursor->next;
    printf("%s\t%d\n", cursor->mediaName, cursor->times); 
  }
}

/* 
 * createPlaylist: 从播放列表文件[playlistSrc]创建播放列表
 */
void 
createPlaylist(PlayItemList playlist, 
    char *playlistSrc) {
  FILE *playlistFile = NULL; 
  if(isFileExists(playlistSrc) && (playlistFile = fopen(playlistSrc, "rt"))) {
    char buf[BUFSIZE]; 
    char *item;
    PlayItemList temp = NULL; 
    PlayItemList cursor = playlist; 

    while(fgets(buf, BUFSIZE, playlistFile)) {
      if(buf[0] == '\n' || buf[0] == ' ') {
        break; 
      } else {
        item = (char *)malloc(BUFSIZE * sizeof(char)); 
        sscanf(buf, "%s", item); 
        temp = (PlayItemList)malloc(sizeof(PlayItemNode));
        temp->mediaName = item; 
        temp->next = NULL; 
        cursor->next = temp; 
        cursor = cursor->next; 
      }
    }

    fflush(playlistFile); 
    fclose(playlistFile); 
  } else {
    printf("Failed to create playlist\n"); 
    exit(0);
  }
}

/* 
 * initPlaylist: 初始化指向播放列表链表的指针
 */
void 
initPlaylist(PlayItemList *playlist) {
  (*playlist) = (PlayItemList)malloc(sizeof(PlayItemNode)); 
  (*playlist)->next = NULL; 
}

/* 
 * printPlaylist: 打印播放列表中的信息以用于测试
 */
void printPlaylist(PlayItemList playlist) {
  PlayItemList cursor = playlist; 
  while(cursor->next) {
    cursor = cursor->next; 
    printf("%s\n", cursor->mediaName);
  }
}

/* 
 * mergeRecords: 每次创建完播放列表后对新增的媒体文件在“日志空间”中追加记录
 */
void 
mergeRecords(RecordList records, 
    PlayItemList playlist) {
  PlayItemList cursor = playlist;
  RecordList tail = records;
  RecordList temp = NULL;
  while(tail->next) {
    tail = tail->next; 
  }
  while(cursor->next) {
    cursor = cursor->next; 
    if(!isInRecords(records, cursor->mediaName)) {
      temp = (RecordList)malloc(sizeof(RecordNode)); 
      temp->mediaName = cursor->mediaName; 
      temp->next = NULL; 
      temp->times = 0; 
      tail->next = temp; 
      tail = tail->next; 
    }
  }
}

/* 
 * isInRecords: 查看某个媒体文件是否在“日志空间”中存在记录
 * 存在：返回1；否则：返回0
 */
int  
isInRecords(RecordList records, 
    char *mediaName) {
  RecordList cursor = records; 
  while(cursor->next) {
    cursor = cursor->next; 
    if(!strcmp(cursor->mediaName, mediaName)) {
      return 1; 
    }
  }
  printf("Cannot find item in records whose name is %s\n", mediaName);
  return 0;
}

/* 
 * isInPlaylist: 查看某个媒体文件是否存在于播放列表中
 * 存在：返回1；否则：返回0
 */
int 
isInPlaylist(PlayItemList playlist, 
    char *mediaName) {
  PlayItemList cursor = playlist; 
  while(cursor->next) {
    cursor = cursor->next; 
    if(!strcmp(cursor->mediaName, mediaName)) {
      return 1; 
    }
  }
  printf("Cannot find item in playlist whose name is %s\n", mediaName); 
  return 0; 
}

/* 
 * initInsertionList: 初始化指向插播列表链表的指针
 */
void 
initInsertionList(InsertionList *insertions) {
  (*insertions) = (InsertionList)malloc(sizeof(InsertionNode)); 
  (*insertions)->next = NULL; 
}

/* 
 * createInsertionList: 从插播文件[insertionsSrc]创建插播列表
 */
int 
createInsertionList(InsertionList insertions, 
    char *insertionsSrc) {
  FILE *insertionsFile = NULL; 
  int isNone = 1; 
  if(isFileExists(insertionsSrc) && (insertionsFile = fopen(insertionsSrc, "rt"))) {
    InsertionList temp = NULL; 
    InsertionList cursor = insertions; 
    char buf[BUFSIZE]; 
    char *item = NULL; 
    while(fgets(buf, BUFSIZE, insertionsFile) != NULL) {
      if(buf[0] == '\n' || buf[0] == ' ') {
        break; 
      } else {
        isNone = 0; 
        item = (char *)malloc(BUFSIZE * sizeof(char)); 
        sscanf(buf, "%s", item); 
        temp = (InsertionList)malloc(sizeof(InsertionNode)); 
        temp->next = NULL; 
        temp->mediaName = item;
        cursor->next = temp; 
        cursor = cursor->next;
      }
    }
    printf("Open %s successfully\n", insertionsSrc); 
    fflush(insertionsFile); 
    fclose(insertionsFile);

    // 清空插播文件中列表
    remove(insertionsSrc); 
    fclose(fopen(insertionsSrc, "w"));

    if(isNone) {
      insertions->next = NULL; 
    }
    return isNone; 
  } else {
    printf("Cannot open %s\n", insertionsSrc); 
    exit(0); 
  }
}

/* 
 * printInsertionList: 打印插播列表总的信息以用于测试
 */
void 
printInsertionList(InsertionList insertions) {
  InsertionList cursor = insertions; 
  while(cursor->next) {
    cursor = cursor->next; 
    printf("%s\n", cursor->mediaName); 
  }
}

/* 
 * saveRecords: 保存“日志空间”的到当天的日志文件中去
 */
void 
saveRecords(RecordList records, 
    char *dir, 
    char *bus_id) {
  char *recordsFileName = generateRecordsFileName(dir, bus_id); 
  FILE *recordsFile = NULL; 
  if((recordsFile = fopen(recordsFileName, "wt"))) {
    RecordList cursor = records; 
    char *recordItem = NULL; 
    while(cursor->next) {
      cursor = cursor->next; 
      recordItem = (char *)malloc(BUFSIZE * sizeof(char)); 
      sprintf(recordItem, "%s %d\n", cursor->mediaName, cursor->times); 
      fputs(recordItem, recordsFile); 
    }
    printf("Records saved successfully\n");
    fflush(recordsFile); 
    fclose(recordsFile);
    return;
  } else {
    printf("Failed to open %s for save records\n", recordsFileName); 
  }
}

/*
 * changeRecords: 将文件名为mediaName的记录播放次数增加1
 */
void 
changeRecords(RecordList records, 
    char *mediaName) {
  RecordList cursor = records; 
  while(cursor->next) {
    cursor = cursor->next; 
    if(!strcmp(cursor->mediaName, mediaName)) {
      cursor->times++;
      return; 
    }
  }
  printf("Cannot find media %s in the records list\n", mediaName);
}

/* 
 * saveLastPlay: 保存播放的最后一个媒体文件的文件名
 */
void 
saveLastPlay(char *lastplaySrc, char *mediaName) {
  FILE *lastplayFile = NULL; 
  if((lastplayFile = fopen(lastplaySrc, "wt"))) {
    fputs(mediaName, lastplayFile); 
    fflush(lastplayFile); 
    fclose(lastplayFile);
    printf("Lastplay item saved to %s successfully\n", lastplaySrc); 
  } else {
    printf("Failed to save lastplay record"); 
  }
}

/* 
 * loadLastPlay: 加载上回播放器关闭时播放的最后一个媒体文件的文件名
 */
char * 
loadLastPlay(char *lastplaySrc) {
  FILE *lastplayFile = NULL; 
  if((lastplayFile = fopen(lastplaySrc, "rt"))) {
    char *mediaName = (char *)malloc(BUFSIZE * sizeof(char)); 
    char buf[BUFSIZE];
    if(fgets(buf, BUFSIZE, lastplayFile)) {
      sscanf(buf, "%s", mediaName);
      printf("Load lastplay successfully: %s\n", mediaName); 
      return mediaName; 
    } else {
      printf("No lastplay item\n"); 
      return NULL; 
    }
  } else {
    printf("Cannot open file: %s\n", lastplaySrc); 
    return NULL; 
  }
}


/* 
 * checkPlaylist: 判断播放列表的文件是否存在
 * 只要有一个存在，返回指向这个item前一个结点的指针，否则，返回NULL 
 */
PlayItemList 
checkPlaylist(PlayItemList playlist, 
    char *meidaDir) {
  PlayItemList cursor = playlist; 
  char *mediaSrc = NULL; 
  while(cursor->next) {
    mediaSrc = concatenateName(meidaDir, cursor->next->mediaName); 
    mediaSrc += 7;  // 去掉"file://"
    if(isFileExists(mediaSrc)) {
      return cursor; 
    }
    cursor = cursor->next; 
  }
  return NULL; 
}

