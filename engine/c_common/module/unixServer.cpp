
#include "unixServer.h"
#include <sys/un.h>
#include "libeventBase.h"
#include <iostream>
#include <vector>
#include "luaService.h"
#include "luaBase.h"

namespace unixServer {

#define UNIX_DOMAIN "../../unix.domain"
int unixListenFd = -1;
struct event *unixListener = NULL;
unsigned int globalUfd = 1;
std::unordered_map<int, AcceptClient*> allAcceptClient;

static AcceptClient* queryClient(int ufd)
{
	std::unordered_map<int, AcceptClient*>::iterator it = allAcceptClient.find(ufd);
	if (it == allAcceptClient.end())
	{
		return NULL;
	}
	return it->second;
}

static void releaseClient(int ufd)
{
	AcceptClient* c = queryClient(ufd);
	if (c)
	{
		allAcceptClient.erase(ufd);
		delete c;
		printf("release accept ipc client:%d\n", ufd);
	}
	else
	{
		fprintf(stderr, "releaseClient error!ufd:%d\n", ufd);
	}
}

void AcceptClient::handle_event(const char* msg, size_t len)
{
	lua_State* GlobalL = luaBase::getLuaState();
	int top = lua_gettop(GlobalL);
	lua_pushcclosure(GlobalL, luaBase::error_fun, 0);
	lua_getglobal(GlobalL, "HandleUnixEvent");
	lua_pushnumber(GlobalL, this->getUfd());
	lua_pushlstring(GlobalL, msg, len);
	
	int result = lua_pcall(GlobalL, 2, 0, -2-2);
	if (result)
	{
		printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
	}
	lua_settop(GlobalL, top);
}

void AcceptClient::read_error()
{
	releaseClient(this->getUfd());
}

void unixMsgCb(struct bufferevent* bev, void* arg)
{
	unixServerUserData usd;
	usd.p = arg;
	int ufd = usd.ufd;

	AcceptClient* c = queryClient(ufd);
	if (c)
	{
		c->do_read(bev);
	}
	else
	{
		fprintf(stderr, "read data no client!\n");
	}
}

void unixEventCb(struct bufferevent *bev, short event, void *arg)
{
	unixServerUserData usd;
	usd.p = arg;
	int ufd = usd.ufd;
	releaseClient(ufd);
}

AcceptClient::~AcceptClient()
{
	if (this->fd != -1)
	{
		close(this->fd);
	}
}

AcceptClient::AcceptClient(int ufd, int sock_fd):ConnectionBase()
{
	this->ufd = ufd;	
	this->fd = sock_fd;

	int flags = fcntl(this->fd, F_GETFL, 0); 
	flags |= O_NONBLOCK;
	fcntl(this->fd, F_SETFL, flags);

	this->bev = bufferevent_socket_new(network::getBase(), this->fd, BEV_OPT_CLOSE_ON_FREE);
	unixServerUserData usd;
	usd.ufd = this->ufd;
	bufferevent_setcb(this->bev, unixMsgCb, NULL, unixEventCb, usd.p);
	bufferevent_enable(this->bev, EV_READ | EV_PERSIST);  	
}

void release()
{
	if (unixListenFd != -1)
	{
                close(unixListenFd);
                unixListenFd = -1; 
	}
	if (unixListener)
	{
		event_free(unixListener);
	}

	std::vector<int> v;
	std::unordered_map<int, AcceptClient*>::iterator it = allAcceptClient.begin();
	for (; it != allAcceptClient.end(); it++)
	{
		v.push_back(it->first);	
	}

	std::vector<int>::iterator vit = v.begin();
	for (; vit != v.end(); vit++)
	{
		releaseClient(*vit);		
	}
}

void newUnixClient(int fd, short event, void *arg)
{   
	struct sockaddr_in addr;
	int len = sizeof(addr);
	bzero(&addr, len);

	int unixFd = accept(fd, (struct sockaddr*)&addr, (socklen_t*)&len);
	if ( unixFd < 0 ) 
	{   
		printf("Accept Error:\n");
		return;
	}   
	int ufd = globalUfd++;
	AcceptClient* c = new AcceptClient(ufd, unixFd);	
	allAcceptClient[ufd] = c;
	// luaService::call(luaBase::getLuaState(), "GateConnected", ufd);
	printf("accept a unix client. ufd=%d \n", ufd);
}


bool listenUnixClient(int srv_id)
{
	std::string file_name = UNIX_DOMAIN + std::to_string(srv_id);
        unixListenFd = socket(AF_LOCAL, SOCK_STREAM, 0); 
        int iFlags = fcntl(unixListenFd, F_GETFL, 0); 
        if (iFlags == -1 || fcntl(unixListenFd, F_SETFL, iFlags | O_NONBLOCK))
        {   
                close(unixListenFd);
                unixListenFd = -1; 
		fprintf(stderr, "unix socket error!file_name:%s\n", file_name.c_str());
                return false;
        }   
        struct sockaddr_un addr;
        bzero(&addr, sizeof(addr));
        addr.sun_family = AF_LOCAL;

	strncpy(addr.sun_path,file_name.c_str(),sizeof(addr.sun_path)-1);
	unlink(file_name.c_str());

        bind(unixListenFd, (struct sockaddr * ) &addr, sizeof(addr));

        if (listen(unixListenFd, SOMAXCONN)==-1)
	{
                close(unixListenFd);
                unixListenFd = -1; 
		fprintf(stderr, "unix listen error!file_name:%s\n", file_name.c_str());
		return false;
	}

        unixListener = event_new(network::getBase(), unixListenFd, EV_READ|EV_PERSIST, newUnixClient, NULL);
        event_add(unixListener, NULL);
        printf("engine bind on unix socket:%s\n", file_name.c_str());

	return true;	
}

const luaL_reg libs[] =
{
	{NULL, NULL},
};

void openLibs(lua_State* L)
{
	// luaL_register(L, "IPC", libs);	
}

}
