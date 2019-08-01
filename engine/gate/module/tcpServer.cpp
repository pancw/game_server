
#include "libeventBase.h"
#include "luaService.h"
#include "luaBase.h"
#include "tcpServer.h"

namespace tcpServer {
struct evconnlistener *listener = NULL;
unsigned int globalVfd = 0;

std::unordered_map<unsigned int, Client*> allClients;
static Client* queryClient(unsigned int vfd)
{
	std::unordered_map<unsigned int, Client*>::iterator it = allClients.find(vfd);
	if (it == allClients.end())
	{
		return NULL;
	}
	return it->second;
}

static void tryReleaseClient(unsigned int vfd)
{
	Client* c = queryClient(vfd);
	if (c)
	{
		allClients.erase(vfd);
		delete c;
		printf("release a tcp client.%d\n", vfd);
	}	
	else
	{
		fprintf(stderr, "tryReleaseClient error!vfd:%d\n", vfd);
	}
	// lua_disconnect(vfd);
	luaService::call(luaBase::getLuaState(), "Disconnect", vfd);
}

void release()
{
	std::vector<int> v;
	std::unordered_map<unsigned int, Client*>::iterator it = allClients.begin();
	for (; it != allClients.end(); it++)
	{
		v.push_back(it->first);	
	}

	std::vector<int>::iterator vit = v.begin();
	for (; vit != v.end(); vit++)
	{
		tryReleaseClient(*vit);		
	}
}

static void
tcpListenerCb(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
	unsigned vfd = ++globalVfd;
	struct event_base *evBase = (struct event_base *)user_data;
	std::string ip = inet_ntoa(((sockaddr_in*)sa)->sin_addr);
	Client* client = new Client(fd, vfd, evBase, ip);
	if (!allClients.insert(std::make_pair(vfd, client)).second)
	{
		close(fd);
		delete client;
		fprintf(stderr, "client insert error!\n");
		return;
	}
	
	printf("accept a tcp client. \n");
}

bool listenTcpClient(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	static struct event_base* base = network::getBase();

	listener = evconnlistener_new_bind(base, tcpListenerCb, (void *)base,
	    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
	    (struct sockaddr*)&sin,
	    sizeof(sin));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return false;
	}
	printf("Accepting tcp client connections on port %d.\n", port);
	return true;	
}

static void socketEventCb(struct bufferevent *bev, short events, void *arg)
{
	cb_user_data usd;
	usd.p = arg;
	unsigned int vfd = usd.vfd;

	if (events & BEV_EVENT_EOF)
		printf("vfd:%d connection close.\n", vfd);
	else if (events & BEV_EVENT_ERROR)
		printf("vfd:%d some other error!\n", vfd);

	tryReleaseClient(vfd);
}

static void socketReadCb(struct bufferevent *bev, void *arg)
{
	cb_user_data usd;
	usd.p = arg;
	unsigned int vfd = usd.vfd;

	Client* client = queryClient(vfd);	
	if (client)
	{
		client->do_read(bev);
	}
	else
	{
		fprintf(stderr, "read data no client!\n");
	}
}
// ---------------------------------- tcp client -----------------------------------
Client::Client(int fd, unsigned int vfd, struct event_base *evBase, std::string clientIp):ConnectionBase()
{
	this->vfd = vfd;
	this->fd = fd;
	this->bev = bufferevent_socket_new(evBase, fd, BEV_OPT_CLOSE_ON_FREE);
	this->ip = clientIp;

	cb_user_data usd;
	usd.vfd = vfd;
	bufferevent_setcb(this->bev,  socketReadCb, NULL, socketEventCb, usd.p);
	bufferevent_enable(this->bev, EV_READ|EV_WRITE);//|EV_PERSIST);
}

Client::~Client()
{
	if (this->fd >0)
		close(this->fd);
}


void Client::handle_event(const char* msg, size_t len)
{
	lua_State* GlobalL = luaBase::getLuaState();
	int top = lua_gettop(GlobalL);
	lua_pushcclosure(GlobalL, luaBase::error_fun, 0);
	lua_getglobal(GlobalL, "RecvJob");
	lua_pushnumber(GlobalL, this->getVfd());
	lua_pushlstring(GlobalL, msg, len);
	
	int result = lua_pcall(GlobalL, 2, 0, -2-2);
	if (result)
	{
		printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
	}
	lua_settop(GlobalL, top);
}

void Client::read_error()
{
	tryReleaseClient(this->getVfd());
}

static int sendMsgToTcpClient(lua_State* L)
{
	int vfd = lua_tonumber(L, 1);	
	size_t size;
	const char* msg = lua_tolstring(L, 2, &size);
	if (!msg)
	{
		fprintf(stderr, "send null msg!\n");
		return 0;
	}

	Client* c = queryClient(vfd);
	if (!c)
	{
		fprintf(stderr, "send msg to null tcp client !\n");
		return 0;
	}

	if (size > commonConnection::max_body_length)
	{
		const char * temp = "msg too long";
		fprintf(stderr, "send data too long!\n");
		c->do_write(temp, strlen(temp));
	}
	else
	{
		c->do_write(msg, size);
	}
	lua_pushboolean(L, true);
	return 1;
}

static int kickTcpClient(lua_State* L)
{
	int vfd = lua_tonumber(L, 1);	
	tryReleaseClient(vfd);		
	return 0;
}

const luaL_reg libs[] =
{
	{"send", sendMsgToTcpClient},
	{"kick", kickTcpClient},
	{NULL, NULL},
};

void openLibs(lua_State* L)
{
	luaL_register(L, "GAME_CLIENT", libs);	
}

}
