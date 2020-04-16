#ifndef EVENT_H
#define EVENT_H

int event_timeout(struct timeval timer,
int (*callback)(int, void*), void *callback_arg, char *idstr);
int event_timeout_delete(int (*callback)(int, void*), void *callback_arg);
int event_fd_delete(int (*callback)(int, void*), void *callback_arg);
int event_fd(int fd, int (*callback)(int, void*), void *callback_arg, 
             char *idstr);
int eventloop();

#endif /* EVENT_H */
