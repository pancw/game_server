#include <iostream>
#include <cstdio>
#include "libeventBase.h"
#include "luaBase.h"
#include "luaService.h"
#include "tcpServer.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
extern int luaopen_cmsgpack(lua_State *);
}

static void luaopen_libs(lua_State * L)
{
	luaopen_cmsgpack(L);
	tcpServer::openLibs(L);	
}

static void releaseAll()
{
	tcpServer::release();
	network::releaseBase();
	printf("All released.\n");
}

int main(int argc, char * argv[])
{
	/*
	if (argc < 2)
	{
		printf("Usage:%s <port>\n", argv[0]);
		return -1;
	}
	*/

	if (chdir("logic") == -1)
	{
		fprintf(stderr, "bad logic path to dir:%s\n", "../logic");
		return 1;
	}

	// init lua
	luaBase::initLua();
	lua_State* L = luaBase::getLuaState();
	luaopen_libs(L);

	// main lua
	lua_pushcclosure(L, luaService::error_fun, 0); 
	int err = luaL_loadfile(L, "main.lua");   
	if (err)
	{   
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
	}   

	int ret = lua_pcall(L, 0, 0, -2);
	if (ret)
	{   
		fprintf(stderr, "call main error:%s\n", lua_tostring(L, -1));
		return 1;
	}

	// config
	lua_State* GlobalL = luaBase::getLuaState();
	lua_getglobal(GlobalL, "GetConfig");
	ret = lua_pcall(GlobalL, 0, 2, 0);
	if (ret)
	{   
		fprintf(stderr, "call config error:%s\n", lua_tostring(GlobalL, -1));
		return 1;
	}
	const int gameClientPort = lua_tonumber(GlobalL, -1);
	if (!gameClientPort)
	{
		fprintf(stderr, "gameClientPort error!\n");
		return 1;
	}
	const int gateUnixServerPort = lua_tonumber(GlobalL, -2);
	if (!gateUnixServerPort)
	{
		fprintf(stderr, "gateUnixServerPort error!\n");
		return 1;
	}
	// printf("gameClientPort:%d,gateUnixServerPort=%d\n", gameClientPort, gateUnixServerPort);

	if (!network::initBase())
	{
		fprintf(stderr, "libevent base init error!\n");
		return 1;
	}

	if (!tcpServer::listenTcpClient(gameClientPort))
	{
		fprintf(stderr, "listenTcpClient error!\n");
		return 1;
	}

	luaService::call(luaBase::getLuaState(), "BeforDispatch");
	network::dispatch();
	luaService::call(luaBase::getLuaState(), "BeforShutdown");
	releaseAll();
	return 0;
}
