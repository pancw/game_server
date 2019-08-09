
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <cstdio>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace logger{

void openLibs(lua_State* L);
void releaseAllFile();

}

#endif
