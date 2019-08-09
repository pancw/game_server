#include "libeventBase.h"

namespace network {
static struct event_base* base;
static struct event *signal_int;
static struct event *timeout;

event_base* getBase()
{
	return base;
}

static void tick(evutil_socket_t fd, short event, void *arg)
{   
	luaService::call(luaBase::getLuaState(), "Tick");
	fflush ( stdout ) ;
}   

static void signalCb(evutil_socket_t fd, short event, void *arg)
{
	struct event *signal = (struct event*)arg;
	printf("signalCb: got signal %d\n", event_get_signal(signal));

	event_del(signal);
	event_base_loopbreak(base);
}

void releaseBase()
{
	event_free(signal_int);
	event_free(timeout);
	event_base_free(base);
}

bool initBase()
{
	struct timeval tv;
	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return false;
	}

	signal_int = evsignal_new(base, SIGINT, signalCb, event_self_cbarg());
	event_add(signal_int, NULL);

	timeout = event_new(base, -1, EV_PERSIST, tick, (void*) timeout);
	evutil_timerclear(&tv);
	tv.tv_sec = 1;
	event_add(timeout, &tv);
	return true;
}

void dispatch()
{
	event_base_dispatch(base);          
}

}
