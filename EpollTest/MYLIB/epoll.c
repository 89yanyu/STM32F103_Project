typedef struct {
    int fd;
    void *ptr;
    int len;
} EPOLL_EVENT;

EPOLL_CALLBACK *cb_array[128] = {};
int lv_array[128] = {};
int NextUnusedFd = 0;

#define EVENTQUEUEMAX 256
EPOLL_EVENT EventQueue[EVENTQUEUEMAX];
int EventHead = 0, EventTail = 0;

int Epoll_Reg(int lv, EPOLL_CALLBACK *cb, int lastFd)
{
    int fd;
    if (-1 == lastFd)
    {
        fd = NextUnusedFd;
        NextUnusedFd++;
    }
    else
    {
        fd = lastFd;
    }

    lv_array[fd] = lv;
    cb_array[fd] = cb;

    return fd;
}

int Epoll_in(int fd, void *ptr, int len)
{
    if (NULL == cb_array[fd])
    {
        return 0;
    }
    if (EventTail - EventHead >= EVENTQUEUEMAX)
    {
        return 1;
    }
    EventQueue[EventTail % EVENTQUEUEMAX].fd = fd;
    EventQueue[EventTail % EVENTQUEUEMAX].ptr = ptr;
    EventQueue[EventTail % EVENTQUEUEMAX].len = len;
    EventTail++;
    return 0;
}

int Epoll_isWaiting()
{
    return EventTail - EventHead;
}

int Epoll_Reduce()
{
    int ret = 0;
    EPOLL_EVENT *event;
    if (EventHead < EventTail)
    {
        event = EventQueue[EventHead];
        EventHead++;
        ret = cb_array[event->fd](event->ptr, event->len);
    }

    return ret;
}

