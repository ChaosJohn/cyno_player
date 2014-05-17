cyno_player: main.o myfun.o mygst.o
	cc -o playbin2 main.o myfun.o mygst.o `pkg-config --cflags --libs gstreamer-0.10` 

main.o: main.c myfun.h mygst.h mytype.h 
	cc -c main.c `pkg-config --cflags --libs gstreamer-0.10` 

myfun.o: myfun.c mytype.h 
	cc -c myfun.c `pkg-config --cflags --libs gstreamer-0.10`

mygst.o: mygst.c mytype.h 
	cc -c mygst.c `pkg-config --cflags --libs gstreamer-0.10` 

clean: 
	rm cyno_player main.o myfun.o mygst.o

