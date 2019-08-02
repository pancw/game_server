
#include "unixClient.h"
#include <sys/un.h>
#include "libeventBase.h"
#include <iostream>
#include <vector>
#include "luaService.h"
#include "luaBase.h"

namespace unixClient {

const std::string UNIX_DOMAIN("../../unix.domain");
std::unordered_map<int, UnixClient*> allUnixClient;

static UnixClient* queryUnixClient(int srv_id)
{
	std::unordered_map<int, UnixClient*>::iterator it = allUnixClient.find(srv_id);
	if (it == allUnixClient.end())
	{
		return NULL;
	}
	return it->second;
}

static void releaseUnixClient(int srv_id)
{
	UnixClient* c = queryUnixClient(srv_id);
	if (c)
	{
		allUnixClient.erase(srv_id);
		delete c;
		printf("release ipc client:%d\n", srv_id);
	}
	else
	{
		fprintf(stderr, "releaseUnixClient error!srv_id:%d\n", srv_id);
	}
}

UnixClient::UnixClient(int srv_id):ConnectionBase()
{
	this->srv_id = srv_id;	
	this->unix_connect_fd = -1;
	this->has_connected = false;
	this->file_name = UNIX_DOMAIN + std::to_string(srv_id);
}

UnixClient::~UnixClient()
{
	if (this->unix_connect_fd != -1)
	{
		close(this->unix_connect_fd);
	}
}

void UnixClient::handle_event(const char* msg, size_t len)
{
	lua_State* GlobalL = luaBase::getLuaState();
	int top = lua_gettop(GlobalL);
	lua_pushcclosure(GlobalL, luaBase::error_fun, 0);
	lua_getglobal(GlobalL, "AckJob");
	lua_pushnumber(GlobalL, this->get_srv_id());
	lua_pushlstring(GlobalL, msg, len);
	
	int result = lua_pcall(GlobalL, 2, 0, -2-2);
	if (result)
	{
		printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
	}
	lua_settop(GlobalL, top);
}

void UnixClient::read_error()
{
	releaseUnixClient(this->get_srv_id());
}

void unix_srv_event_cb(struct bufferevent *bev, short event, void *arg)
{
	unixClientUserData usd;
	usd.p = arg;
	int srv_id = usd.srv_id;

	UnixClient* c = queryUnixClient(srv_id);
	bool connected = false;
	if (event & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		// printf("some other error\n");
	}
	else if( event & BEV_EVENT_CONNECTED)
	{
		printf("has connected to unix server:%d\n", srv_id);
		connected = true;
	}

	if (!c)
	{
		printf("ipc client error!\n");
		return;
	}

	if (connected)
	{
		c->setHasConnected(connected);
	}
	else
	{
		releaseUnixClient(srv_id);
	}
}

void unix_srv_msg_cb(struct bufferevent* bev, void* arg)
{
	fprintf(stderr, "unix client read data !!!!!\n");
	/*
	unixClientUserData usd;
	usd.p = arg;
	int srv_id = usd.srv_id;

	UnixClient* c = queryUnixClient(srv_id);
	if (c)
	{
		c->do_read(bev);
	}
	else
	{
		fprintf(stderr, "read data no ipc client! srv_id:%d\n", srv_id);
	}
	*/
}

void tryConnectUnixSrv(int srv_id)
{
	UnixClient* c = queryUnixClient(srv_id);
	if (!c)
	{
		UnixClient* c = new UnixClient(srv_id);
		allUnixClient[srv_id] = c;
		c->tryConnectUnixSrv();
	}
}

void UnixClient::tryConnectUnixSrv()
{
	if (this->has_connected)
	{
		return;
	}
	this->unix_connect_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	//printf("unix sockfd=%d\n", this->unix_connect_fd);
	int iFlags = fcntl(this->unix_connect_fd, F_GETFL, 0);
	fcntl(this->unix_connect_fd, F_SETFL, iFlags | O_NONBLOCK);

	this->bev = bufferevent_socket_new(network::getBase(), this->unix_connect_fd, BEV_OPT_CLOSE_ON_FREE);

	struct sockaddr_un ServerAddr;  
	bzero(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sun_family = AF_LOCAL;
	strncpy(ServerAddr.sun_path,this->file_name.c_str(),sizeof(ServerAddr.sun_path) - 1) ;

	unixClientUserData usd;
	usd.srv_id = this->srv_id;
	bufferevent_socket_connect(bev, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
	bufferevent_setcb(bev, unix_srv_msg_cb, NULL, unix_srv_event_cb, usd.p);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);  	
	printf("try connect unix srv [%s].\n",this->file_name.c_str());
}

static int checkHasConnected(lua_State* L)
{
	int srv_id = lua_tonumber(L, 1);	
	UnixClient* c = queryUnixClient(srv_id);
	if (c && c->getHasConnected())
	{
		lua_pushboolean(L, true);
		return 1;
	}
	return 0;
}

static int connectUnixSrv(lua_State* L)
{
	int srv_id = lua_tonumber(L, 1);	
	tryConnectUnixSrv(srv_id);
	return 0;
}

static int sendMsg(lua_State* L)
{
	int srv_id = lua_tonumber(L, 1);	
	size_t size;
	const char* msg = lua_tolstring(L, 2, &size);
	if (!msg)
	{
		fprintf(stderr, "post null job!\n");
		return 0;
	}

	UnixClient* c = queryUnixClient(srv_id);
	if (!c)
	{
		fprintf(stderr, "post job to null ipc client !\n");
		return 0;
	}

	if (size > commonConnection::max_body_length)
	{
		fprintf(stderr, "post data too long!\n");
		return 0;
	}
	else
	{
		c->do_write(msg, size);
	}
	lua_pushboolean(L, true);
	return 1;
}

const luaL_reg libs[] =
{
	{"checkHasConnected", checkHasConnected},
	{"connectUnixSrv", connectUnixSrv},
	{"sendMsg", sendMsg},
	{NULL, NULL},
};

void openLibs(lua_State* L)
{
	luaL_register(L, "UnixClient", libs);	
}

void release()
{
	std::vector<int> v;
	std::unordered_map<int, UnixClient*>::iterator it = allUnixClient.begin();
	for (; it != allUnixClient.end(); it++)
	{
		v.push_back(it->first);	
	}

	std::vector<int>::iterator vit = v.begin();
	for (; vit != v.end(); vit++)
	{
		releaseUnixClient(*vit);		
	}
}

}
