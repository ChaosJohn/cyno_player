#ifndef _MYGST_H
#define _MYGST_H 

#include <gst/gst.h> 
#include <glib.h> 
#include "mytype.h"


void 
initGstElement(PlayerDataPointer data, ParasPointer parameter); 

void 
runPlaybin2(PlayerDataPointer data, char *mediaSrc);

void 
handle_time(PlayerDataPointer data); 

/*
void 
handle_message(PlayerDataPointer data, 
    GstMessage *msg, 
    gpointer pointer); 
*/

/*
gboolean 
bus_call(GstBus *bus, 
    GstMessage *msg, 
    gpointer pointer); 
*/    

#endif 
