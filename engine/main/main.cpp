#include <iostream>
#include <cstdio>
#include "libeventBase.h"
#include "luaBase.h"
#include "luaService.h"
#include "unixServer.h"
#include "unixClient.h"
#include "cmongo.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
extern int luaopen_cmsgpack(lua_State *);
}

static void luaopen_libs(lua_State * L)
{
	cMongo::luaopenMongo(L);
	luaopen_cmsgpack(L);
	//tcpServer::openLibs(L);	
	unixServer::openLibs(L);
	unixClient::openLibs(L);
}

static void releaseAll()
{
	//tcpServer::release();
	unixClient::release();
	unixServer::release();
	network::releaseBase();
	cMongo::release();
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

	if (chdir("../main_logic") == -1)
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

	// connect db
	lua_State* GlobalL = luaBase::getLuaState();
	lua_getglobal(GlobalL, "GetMongoConfig");
	ret = lua_pcall(GlobalL, 0, 1, 0);
	if (ret)
	{
		fprintf(stderr, "call mongo config error:%s\n", lua_tostring(GlobalL, -1));
		return 1;
	}
	const char* dbUrl = lua_tostring(GlobalL, -1);
	if (!dbUrl)
	{
		fprintf(stderr, "db url error!\n");
		return 1;
	}
	if (!cMongo::connectMongo(dbUrl))
	{
		return 1;
	}
	

	// config
	lua_getglobal(GlobalL, "GetConfig");
	ret = lua_pcall(GlobalL, 0, 4, 0);
	if (ret)
	{
		fprintf(stderr, "call config error:%s\n", lua_tostring(GlobalL, -1));
		return 1;
	}
	const int dbUnixServerPort = lua_tonumber(GlobalL, -1);
	if (!dbUnixServerPort)
	{
		fprintf(stderr, "dbUnixServerPort error!\n");
		return 1;
	}
	const int logUnixServerPort = lua_tonumber(GlobalL, -2);
	if (!logUnixServerPort)
	{
		fprintf(stderr, "logUnixServerPort error!\n");
		return 1;
	}
	const int mainUnixServerPort = lua_tonumber(GlobalL, -3);
	if (!mainUnixServerPort)
	{
		fprintf(stderr, "mainUnixServerPort error!\n");
		return 1;
	}
	const int gateUnixServerPort = lua_tonumber(GlobalL, -4);
	if (!gateUnixServerPort)
	{
		fprintf(stderr, "gateUnixServerPort error!\n");
		return 1;
	}

	if (!network::initBase())
	{
		fprintf(stderr, "libevent base init error!\n");
		return 1;
	}
	if (!unixServer::listenUnixClient(mainUnixServerPort))
	{
		fprintf(stderr, "listenUnixClient error!\n");
		return 1;
	}
	/*
	if (!tcpServer::listenTcpClient(gameClientPort))
	{
		fprintf(stderr, "listenTcpClient error!\n");
		return 1;
	}
	*/

	luaService::call(luaBase::getLuaState(), "BeforDispatch");
	network::dispatch();
	luaService::call(luaBase::getLuaState(), "BeforShutdown");
	releaseAll();
	return 0;
}
