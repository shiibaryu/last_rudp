CC = clang
CFLAGS = -fsanitize=memory -fno-omit-frame-pointer -g -Wall 

all: rudp_recv

rudp_recv: rudp_recv.o my_rudp.o rudp_event.o
	$(CC) $(CFLAGS) $^ -o $@

rudp_recv.o my_rudp.o: my_rudp.h my_rudp_api.h rudp_event.h

rudp_event.o: rudp_event.h

rudp.tar: rudp_recv.c Makefile my_rudp_api.h my_rudp.h rudp_event.h \
	rudp_event.c my_rudp.c
	tar cf rudp.tar $^

clean:
		/bin/rm -f rudp_send*.o rudp.tar


