#include "mygst.h" 

/* 
 * initGstElement: 初始化gstreamer的各个组件[包括pipeline,playbin2和sink]，设置播放器的位置大小信息
 */
void 
initGstElement(PlayerDataPointer data, ParasPointer parameter) {
  data->playing = data->terminate = data->seek_enabled = data->seek_done = FALSE; 
  data->duration = GST_CLOCK_TIME_NONE;
  data->pipeline = gst_pipeline_new("media-player"); 
  data->playbin2 = gst_element_factory_make("playbin2", "bin"); 
  data->sink     = gst_element_factory_make("mfw_v4lsink", "meida-sink"); 
  //data->sink     = gst_element_factory_make("ffdec_mpeg4", "mpeg-decoder"); 
  if(!data->pipeline || !data->playbin2) {
    printf("One element could not be created...Exiting...\n"); 
    exit(0); 
  } else {
    printf("Pipeline and playbin2 have been created successfully\n"); 
  }
  gst_bin_add_many(GST_BIN(data->pipeline),data->playbin2,NULL); 
  g_object_set(G_OBJECT(data->sink), "disp-width", parameter->posi_w, 
      "disp-height", parameter->posi_h, "axis-top", parameter->posi_y, 
      "axis-left", parameter->posi_x, "rotate", parameter->rotate, NULL); 
}

/* 
 * runPlaybin2: 播放文件路径为mediaSrc的媒体文件
 */
void 
runPlaybin2(PlayerDataPointer data, char *mediaSrc) {
  g_object_set(G_OBJECT(data->playbin2), 
      "uri", mediaSrc, 
      "video-sink", data->sink, 
      NULL); 
  gst_element_set_state(data->pipeline, GST_STATE_PLAYING); 
  printf("Playing %s...\n", mediaSrc); 
}

/*
void 
handle_message(PlayerDataPointer data, 
    GstMessage *msg, 
    gpointer pointer) {

  GError *err; 
  gchar *debug_info;
  GMainLoop *loop = (GMainLoop *)pointer;
  
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: 
      {
        gst_message_parse_error(msg, &err, &debug_info); 
        g_printerr("Error received from element %s: %s\n", 
            GST_OBJECT_NAME(msg->src), err->message); 
        g_printerr("Debugging information: %s\n", 
            debug_info ? debug_info : "none"); 
        g_clear_error(&err); 
        g_free(debug_info); 
        data->terminate = TRUE; 
        //data->terminate = FALSE;
        //TODO Record the stopped time
        break; 
      } 
    case GST_MESSAGE_EOS: 
      {
        g_print("End-Of-Stream received.\n"); 
        data->terminate = TRUE; 
        break; 
      }
    case GST_MESSAGE_DURATION: 
      {
        data->duration = GST_CLOCK_TIME_NONE;
        break; 
      }
    case GST_MESSAGE_STATE_CHANGED: 
      {
        GstState old_state, new_state, pending_state; 
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state); 
        if(GST_MESSAGE_SRC(msg) == GST_OBJECT(data->playbin2)) {
          g_print("Pipeline state changed from %s to %s: \n", 
              gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
          data->playing = (new_state == GST_STATE_PLAYING); 

          if(data->playing) {
            GstQuery *query; 
            gint64 start, end; 
            query = gst_query_new_seeking(GST_FORMAT_TIME); 
            if(gst_element_query(data->playbin2, query)) {
              gst_query_parse_seeking(query, NULL, &data->seek_enabled, &start, &end); 
              if(data->seek_enabled) {
                g_print("Seeking is enabled from %" GST_TIME_FORMAT " to %" GST_TIME_FORMAT "\n", 
                    GST_TIME_ARGS(start), GST_TIME_ARGS(end)); 
              } else {
                g_print("Seeking is disable for this stream.\n"); 
              }
            } else {
              g_printerr("Seeking query failed."); 
            }
            gst_query_unref(query); 
          }
        }
        break; 
      }
    default: 
      {
        g_printerr("Unexpected message received.\n"); 
        break; 
      }
  }
  gst_message_unref(msg); 
}
*/

void 
handle_time(PlayerDataPointer data) {
  if(data->playing) {
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current = -1; 

    if(!gst_element_query_position(data->playbin2, &fmt, &current)) {
      g_printerr("could not query current position.\n"); 
    } 

    if(!GST_CLOCK_TIME_IS_VALID(data->duration)) {
      if(!gst_element_query_duration(data->playbin2, &fmt, &(data->duration))) {
        g_printerr("Could not query current duration.\n");
      }
    }

    g_print("Position %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\t %lu\r", 
        GST_TIME_ARGS(current), GST_TIME_ARGS(data->duration), current); 

    if(data->seek_enabled && !data->seek_done && current > 5 * GST_SECOND) {
      g_print("\nReached 5s, performing...\n"); 
      gst_element_seek_simple(data->playbin2, GST_FORMAT_TIME, 
          GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 25 * GST_SECOND); 
      data->seek_done = TRUE; 
    }
  }
}

/*
gboolean 
bus_call(GstBus *bus, 
    GstMessage *msg, 
    gpointer pointer) {
  GMainLoop *loop = (GMainLoop *)pointer; 

  switch(GST_MESSAGE_TYPE(msg)) {
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
        
        GstFormat fmt = GST_FORMAT_TIME; 
        gint64 current = -1; 
        gst_element_query_position(data->playbin2, &fmt, &current); 
        printf("time: %lu\n", current); 

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
*/

