#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "my_rudp_api.h"
#include "rudp_event.h"
#include "vsftp.h"

/*rudpのファイルの転送
　特定の１つの相手にパケットを送信
*/

int usage();
int eventhandler(rudp_socket_t rsocket, rudp_event_t event, struct sockaddr_in *remote);
void send_file(char *filename);
int filesender(int fd, void *socket);

int debug = 0;
struct sockaddr_in client;


int usage() {
  fprintf(stderr, "Usage: vs_send [-d] host1:port1 [host2:port2] ... file1 [file2]... \n");
  exit(1);
}

int main(int argc, char* argv[]) {
    int port;
    int i;
    int c;
    struct in_addr *addr;
    struct hostent *hp;
    struct timespec start_time;
    char *hoststr;
    
    clock_gettime(CLOCK_REALTIME, &start_time);

    opterr = 0;

    while ((c = getopt(argc, argv, "d")) != -1) {
        if (c == 'd') {
            debug = 1;
        }
        else{
            usage();
        }
    }

    for(i = optind;i<argc;i++){
        if(strchr(argv[i], ':') == NULL){
            break;
        }
        hoststr = (char *)malloc(strlen(argv[i]) + 1);
        if (hoststr == NULL) {
             fprintf(stderr, "vs_send: malloc failed\n");
             exit(1);
        }
        strcpy(hoststr,argv[i]);
        port = atoi(strchr(hoststr, ':') + 1); 
        if (port <= 0) {
            fprintf(stderr, "Bad destination port: %d\n", 
            atoi(strchr(hoststr, ':') + 1));
            exit(1);
        }
        *strchr(hoststr, ':') = '\0';
        if((hp = gethostbyname(hoststr)) == NULL || 
      (addr = (struct in_addr *)hp->h_addr) == NULL){
            fprintf(stderr,"Can't locate host \"%s\"\n", hoststr); 
            return 0;
        }
        memset((char *)&client,0,sizeof(struct sockaddr_in));
        client.sin_family = AF_INET;
        client.sin_port = htons(port);
        memcpy(&client.sin_addr,addr,sizeof(struct in_addr));/*(struct in_addr)*/
        free(hoststr);
    }


    send_file(argv[2]);

    eventloop();
    /*clock_gettime(CLOCK_REALTIME,&end_time);*/
    printf("%ld.%ld\n",end_time.tv_sec - start_time.tv_sec,end_time.tv_nsec - start_time.tv_nsec);
    
    return 0;

}

int eventhandler(rudp_socket_t rsocket, rudp_event_t event, struct sockaddr_in *remote) {
  
  switch (event) {
  case RUDP_EVENT_TIMEOUT:
    if (remote) {
      fprintf(stderr, "rudp_sender: time out in communication with %s:%d\n",
        inet_ntoa(remote->sin_addr),
        ntohs(remote->sin_port));
    }
    else {
      fprintf(stderr, "rudp_sender: time out\n");
    }
    exit(1);
    break;
  case RUDP_EVENT_CLOSED:
    if (debug) {
      fprintf(stderr, "rudp_sender: socket closed\n");
    }
    break;
  }
  return 0;
}

void send_file(char *filename){
    int file = 0;
    rudp_socket_t rsock;
    struct vsftp vs;
    int vslen;
    int namelen;
    char *filename1;

    if((file = open(filename, O_RDONLY)) < 0) {
        perror("sender: fail to open\n");
        exit(1);
    }
    rsock = rudp_socket(0);
    if(rsock == NULL){
        fprintf(stderr,"sender: fail to rudp_socket\n");
    }

    rudp_event_handler(rsock,eventhandler);
    
    vs.vs_type = htonl(VS_TYPE_BEGIN);

    filename1 = filename;

    namelen = strlen(filename1) < VS_FILENAMELENGTH  ? strlen(filename1) : VS_FILENAMELENGTH;
    strncpy(vs.vs_info.vs_filename, filename1, namelen);
    vslen = sizeof(vs.vs_type) + namelen;

    if (debug) {
      fprintf(stderr, "vs_send: send BEGIN \"%s\" (%d bytes) to %s:%d\n",
        filename, vslen, 
        inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    }
    if(rudp_sendto(rsock,(char *)&vs,vslen,&client)<0){
        fprintf(stderr,"rudp_sender: fail to send");
        rudp_close(rsock);
        return;
    }

    event_fd(file, filesender, rsock, "filesender");
}

int filesender(int file,void *socket){
    rudp_socket_t rsock = (rudp_socket_t)socket;
    int file_read;
    struct vsftp vs;
    int vslen;
 

    file_read = read(file,&vs.vs_info.vs_data,VS_MAXDATA);
    if(file_read < 0){
        perror("filesender: read");
        event_fd_delete(filesender, rsock);
        rudp_close(rsock);    
    }
    else if (file_read == 0) {
        vs.vs_type = htonl(VS_TYPE_END);
        vslen = sizeof(vs.vs_type);
        if(rudp_sendto(rsock, (char *) &vs, vslen, &client) < 0) {
            fprintf(stderr,"rudp_sender: send failure\n");
        }
        event_fd_delete(filesender, rsock);
        rudp_close(rsock);
    }
    else{
        vs.vs_type = htonl(VS_TYPE_DATA);
        vslen = sizeof(vs.vs_type) + file_read;
        if (debug) {
            fprintf(stderr, "vs_send: send DATA (%d bytes) to %s:%d\n", 
            vslen, inet_ntoa(client.sin_addr), htons(client.sin_port));        
        }
        if((rudp_sendto(rsock,(char *)&vs,vslen,&client))<0){
            fprintf(stderr,"rudp_sender: fail to send\n");
            event_fd_delete(filesender,rsock);
            rudp_close(rsock);
        }   
    }
  return 0;
}



