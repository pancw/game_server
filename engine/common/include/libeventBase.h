#ifndef __LIVEVENT_BASE_H__
#define __LIVEVENT_BASE_H__

#include <signal.h>
#include <sys/socket.h>

#include <sys/types.h>

#include <event2/event-config.h>

#include <sys/stat.h>
#ifndef _WIN32
#include <sys/queue.h>
#include <unistd.h>
#endif
#include <time.h>
#ifdef EVENT__HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "luaBase.h"
#include "luaService.h"

namespace network {

void dispatch();
bool initBase();
event_base* getBase();
void releaseBase();

}
#endif
