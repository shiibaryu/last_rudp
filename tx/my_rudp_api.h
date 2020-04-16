#ifndef RUDP_API_H
#define RUDP_API_H



struct timespec end_time;

typedef enum {
  RUDP_EVENT_TIMEOUT, 
  RUDP_EVENT_CLOSED,
} rudp_event_t; 

typedef void *rudp_socket_t;

rudp_socket_t rudp_socket(int port);

int rudp_close(rudp_socket_t rsocket);

int rudp_sendto(rudp_socket_t rsocket, void* data, int len, 
        struct sockaddr_in* to);

int rudp_recvfrom_handler(rudp_socket_t rsocket, 
              int (*handler)(rudp_socket_t, 
                     struct sockaddr_in *, 
                     char *, int));

int rudp_event_handler(rudp_socket_t rsocket, 
               int (*handler)(rudp_socket_t, 
                      rudp_event_t, 
                      struct sockaddr_in *));
#endif 
