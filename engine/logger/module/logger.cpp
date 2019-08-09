#include "logger.h"
#include <unordered_map>

namespace logger{
std::unordered_map<std::string, FILE*> allFiles;

static int writeLog(lua_State* L)
{
	size_t dirSize;
	const char* dir = lua_tolstring(L, 1, &dirSize);
	if (!dir)
	{
		fprintf(stderr, "dir null !\n");
		return 0;
	}
	size_t msgSize;
	const char* msg = lua_tolstring(L, 2, &msgSize);
	if (!msg)
	{
		fprintf(stderr, "msg null !\n");
		return 0;
	}

	/*
	std::string d(dir, dirSize);
	std::string m(msg, msgSize);
	FILE *fp = fopen(d.c_str(), "a");
	fputs(m.c_str(), fp);
	*/

	std::string d(dir, dirSize);
	FILE *fp = NULL;
	std::unordered_map<std::string, FILE*>::iterator it = allFiles.find(d);
	if (it != allFiles.end())
	{
		fp = it->second;	
	}
	else
	{
		fp = fopen(d.c_str(), "ab");
		allFiles[d] = fp;
	}

	fwrite(msg, msgSize, 1, fp);
	fflush(fp);
	lua_pushboolean(L, true);
	return 1;
}

void releaseAllFile()
{
	int cnt = 0;
	std::unordered_map<std::string, FILE*>::iterator it = allFiles.begin();
	for (; it != allFiles.end(); it++)
	{
		fclose(it->second);
		printf("fclose file=%s\n", it->first.c_str());
		++cnt;
	}
	printf("closed %d file.\n", cnt);
}

const luaL_reg libs[] =
{
	{"writeLog", writeLog},
	{NULL, NULL},
};

void openLibs(lua_State* L)
{
	luaL_register(L, "Logger", libs);	
}

}
