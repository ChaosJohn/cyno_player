#ifndef _MYTYPE_H
#define _MYTYPE_H

#include <gst/gst.h> 
#include <glib.h> 

/* 
 * struct _PlayerData:
 * Store the some elements that gstreamer needs. 
 */
typedef struct _PlayerData {
  GstElement *pipeline; 
  GstElement *playbin2; 
  GstElement *sink;
  gboolean playing;       // Are we in PLAYING state? 
  gboolean terminate;     // Should we terminate the execution? 
  gboolean seek_enabled;  // Is seeking enabled? 
  gboolean seek_done;     // Have we performed the seek already? 
  gint64 duration;        // How long does this media last(in nanoseconds)? 
} PlayerData, *PlayerDataPointer;


/* 
 * struct _Record: 
 * Record the played-times for each media item in the playlist. 
 */
typedef struct _Record {
  //int index;          // The index in the playlist for the media item.  
  char *mediaName; // The filename of the media item. 
  int times;      // The times that the media item was played. 
  struct _Record *next; 
} RecordNode, *RecordList; 

/* 
 * struct _Insertion: 
 * represent the media item to insert into the playing-queue. 
 */
typedef struct _Insertion {
  //int index;                // The indx in the playlist for the media item. 
  char *mediaName;       // The filename of the media item. 
  struct _Insertion *next;  // The pointer point to the next node in the linked list.
} InsertionNode, *InsertionList; 


/*
 * struct _PlayItem: 
 * Item in the playlist. 
 */
typedef struct _PlayItem {
  char *mediaName; 
  struct _PlayItem *next; 
} PlayItemNode, *PlayItemList;

/* 
 * struct _Parameter: 
 * Parameters for the main() function. 
 */
typedef struct _Parameter {
  char *mediaDir; 
  char *workingDir; 
  char *playlistSrc; 
  char *insertionsSrc;
  char *lastPlaySrc; 
  int posi_x, posi_y, posi_w, posi_h, rotate; 
  char *fixedMedia; 
  char *bus_id; 
}Parameter, *ParasPointer; 

#endif

