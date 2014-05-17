#include "myfun.h"
#include "mygst.h"
#include "mytype.h"
#include <signal.h>
#include <pthread.h>

static GMainLoop *loop;
static PlayerDataPointer data;
static ParasPointer parameter;
GstBus *bus;
GstBus *playbin2Bus;
char *mediaSrc = NULL;
GstStateChangeReturn ret;
GstMessage *msg;
int isFirst = 1;  // 是否是程序运行后的第一次播放
int shouldReplay = 0;  // 是否需要重播
char *lastplayMediaName = NULL;
RecordList records = NULL;
PlayItemList playlist = NULL;
InsertionList insertions = NULL;
pthread_t thread;
int isFirstDay = 1; 
char *firstDay; 
char *curDay; 

static void handle_message (PlayerDataPointer data, GstMessage *msg)
{
  GError *err;
  gchar *debug_info;

  switch (GST_MESSAGE_TYPE (msg))
  {
  case GST_MESSAGE_ERROR:
  {
    gst_message_parse_error (msg, &err, &debug_info);
    g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
    g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error (&err);
    g_free (debug_info);
    data->terminate = TRUE;
    break;
  }
  case GST_MESSAGE_EOS:
  {
    g_print ("End-Of-Stream reached.\n");
    data->terminate = TRUE;
    break;
  }
  case GST_MESSAGE_DURATION:
  {
    /* The duration has changed, mark the current one as invalid */
    data->duration = GST_CLOCK_TIME_NONE;
    break;
  }
  case GST_MESSAGE_STATE_CHANGED:
  {
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->playbin2))
    {
      g_print ("Pipeline state changed from %s to %s:\n",
               gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));

      /* Remember whether we are in the PLAYING state or not */
      data->playing = (new_state == GST_STATE_PLAYING);

      if (data->playing)
      {
        /* We just moved to PLAYING. Check if seeking is possible */
        GstQuery *query;
        gint64 start, end;
        query = gst_query_new_seeking (GST_FORMAT_TIME);
        if (gst_element_query (data->playbin2, query))
        {
          gst_query_parse_seeking (query, NULL, &data->seek_enabled, &start, &end);
          if (data->seek_enabled)
          {
            g_print ("Seeking is ENABLED from %" GST_TIME_FORMAT " to %" GST_TIME_FORMAT "\n",
                     GST_TIME_ARGS (start), GST_TIME_ARGS (end));
          }
          else
          {
            g_print ("Seeking is DISABLED for this stream.\n");
          }
        }
        else
        {
          g_printerr ("Seeking query failed.");
        }
        gst_query_unref (query);
      }
    }
  }
  break;
  default:
  {
    /* We should not reach here */
    g_printerr ("Unexpected message received.\n");
    break;
  }
  }
  gst_message_unref (msg);
}

/*
 * handle_kill: 处理程序被杀死时所执行的代码
 */
void
handle_kill(int sigNo)
{
  if (sigNo == SIGINT || sigNo == SIGQUIT || sigNo == SIGTERM)
  {
    /*saveStoppdPoint(); */
    exit(0);
  }
}

static gboolean
bus_call(GstBus *bus,
         GstMessage *msg,
         gpointer pointer)
{

  switch (GST_MESSAGE_TYPE(msg))
  {
  case GST_MESSAGE_EOS:
  {
    g_print("End of stream\n");
    g_main_loop_quit(loop);
    return TRUE;
  }
  case GST_MESSAGE_ERROR:
  {
    gchar *debug;
    GError *error;
    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);
    g_printerr("Error: %s\n", error->message);
    g_error_free(error);
    /*saveStoppdPoint();*/
    g_main_loop_quit(loop);
    return TRUE;
  }
  default:
  {
    break;
  }
  }
  return TRUE;
}

void *
handle_save(void *ptr)
{
  pthread_detach(pthread_self());

  // 获取播放时刻的日期
  curDay = getDate(); 
  // 如果还未跨零点的记录，并且检测到当前日期和播放器开启的时间不同
  if (isFirstDay && strcmp(curDay, firstDay)) {
    isFirstDay = 0; 
    records->next = NULL; 
  }

  saveRecords(records, parameter->workingDir, parameter->bus_id);          // 将“日志空间”保存到当天的日志文件

  pthread_exit(0);
  return ((void *) 0);
}

int
main(int argc,
     char **argv)
{

  // 判断参数调用是否正确
  if (argc != 2)
  {
    g_printerr("Usage: %s <parameter-src>\n", argv[0]);
    return -1;
  }

  // 获取播放器开启时日期
  firstDay = getDate(); 


  // 关联结束程序的信号及其相应的函数
  /////////////////////////////
  signal(SIGINT, handle_kill);
  signal(SIGQUIT, handle_kill);
  signal(SIGTERM, handle_kill);
  /////////////////////////////

  /*PlayerDataPointer data = (PlayerData *)malloc(sizeof(PlayerData)); */
  data = (PlayerData *)malloc(sizeof(PlayerData));
  /*ParasPointer parameter = (ParasPointer)malloc(sizeof(Parameter)); */
  parameter = (ParasPointer)malloc(sizeof(Parameter));
  initParameter(parameter, argv[1]); // 读入参数文件，初始化程序

  /* Initialize GStreamer */
  gst_init(&argc, &argv);
  loop = g_main_loop_new(NULL, FALSE);

  initGstElement(data, parameter);

  // 构造“日志空间”并且初始化
  initRecords(&records);
  createRecords(records, parameter);

  // 构造播放列表并且初始化
  initPlaylist(&playlist);

  // 构造插播列表并且初始化
  initInsertionList(&insertions);

  bus = gst_pipeline_get_bus(GST_PIPELINE(data->pipeline));
  gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);

  // 循环播放
  while (1)
  {
    PlayItemList playlistCursor = playlist;
    InsertionList insertionsCursor = insertions;

    // 每次循环开始都去检测播放列表文件，生成新的播放列表
    createPlaylist(playlist, parameter->playlistSrc);

    // 对于在播放列表中但不存在于“日志空间”的媒体文件，在“日志空间”中增加其记录
    mergeRecords(records, playlist);

    if ((playlistCursor = checkPlaylist(playlist, parameter->mediaDir)) == NULL)
    {
      runPlaybin2(data, concatenateName("file://", parameter->fixedMedia));
      g_main_loop_run(loop);
      gst_element_set_state(data->pipeline, GST_STATE_NULL);
      continue;
    }

    // 如果是第一次进入循环，则判断是否要从上回程序退出的那个媒体文件开始播放
    /*if(isFirst && (gsecond = loadStoppedPoint()) && (lastplayMediaName = loadLastPlay(parameter->lastPlaySrc)) && isInPlaylist(playlist, lastplayMediaName)) {*/
    if (isFirst && (lastplayMediaName = loadLastPlay(parameter->lastPlaySrc)) && isInPlaylist(playlist, lastplayMediaName))
    {
      while (playlistCursor->next)
      {
        // 将游标移动到播放列表链表中媒体文件名为lastplayMediaName的节点的上一个节点
        if (!strcmp(playlistCursor->next->mediaName, lastplayMediaName))
        {
          break;
        }
        else
        {
          playlistCursor = playlistCursor->next;
        }
      }
      shouldReplay = 1;   // 需要重播
      isFirst = 0;
    }

    // 顺序播放播放列表中的媒体文件
    while (playlistCursor->next)
    {
      playlistCursor = playlistCursor->next;

      if (!shouldReplay)
      {
        // 检测并生成插播列表
        while (!createInsertionList(insertions, parameter->insertionsSrc))
        {
          insertionsCursor = insertions;
          // 优先播放插播列表
          while (insertionsCursor->next)
          {
            insertionsCursor = insertionsCursor->next;
            mediaSrc = concatenateName(parameter->mediaDir,
                                       insertionsCursor->mediaName);
            changeRecords(records, insertionsCursor->mediaName);  // 将该媒体文件的播放次数+1
            saveLastPlay(parameter->lastPlaySrc, insertionsCursor->mediaName);  //将该媒体文件存为“最后播放”
            pthread_create(&thread, NULL, &handle_save, NULL);
            runPlaybin2(data, mediaSrc);  // 播放该媒体文件
            g_main_loop_run(loop);
            gst_element_set_state(data->pipeline, GST_STATE_NULL);
          }
        }
      }

      mediaSrc = concatenateName(parameter->mediaDir,
                                 playlistCursor->mediaName);
      changeRecords(records, playlistCursor->mediaName);      // 将该媒体文件的播放次数+1
      saveLastPlay(parameter->lastPlaySrc, playlistCursor->mediaName);  // 将该媒体文件存为“最后播放”
      pthread_create(&thread, NULL, &handle_save, NULL);
      runPlaybin2(data, mediaSrc);  // 播放该媒体文件
      g_main_loop_run(loop);
      gst_element_set_state(data->pipeline, GST_STATE_NULL);
      shouldReplay = 0;
    }
  }
  gst_object_unref(GST_OBJECT(data->pipeline)); // 释放gstreamer的相关资源

  return 0;
}

