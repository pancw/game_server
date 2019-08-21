

#ifndef __C_MONGO_H_
#define __C_MONGO_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace cMongo {
bool connectMongo(const char*);
void release();
bool insert();
void find();
void deleting();
void update();
void executing();
void createDoc();
void testDoc();
void luaopenMongo(lua_State*);
}
#endif
