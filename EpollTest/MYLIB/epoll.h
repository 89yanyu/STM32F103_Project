#ifndef __89YY_EPOLL__

#define __89YY_EPOLL__

#include "stm32f10x.h"

typedef int EPOLL_CALLBACK(void *, int);

int Epoll_Reg(int lv, EPOLL_CALLBACK cb);

int Epoll_in(int fd, void *ptr, int len);

int Epoll_isWaiting();

int

#endif
