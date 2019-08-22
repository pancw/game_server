
INSTALLDIR=$(shell pwd)/../../extlib/install
ROOT=$(shell pwd)

BIN=main

DEPS=$(shell find ./ -name "*.h")

C_SRC=$(shell find ./ -name "*.c") \
	$(ROOT)/../c_common/module/lua_cmsgpack.c


CPP_SRC=$(shell find ./ -name "*.cpp") \
	$(ROOT)/../c_common/module/connectionBase.cpp \
	$(ROOT)/../c_common/module/luaService.cpp \
	$(ROOT)/../c_common/module/luaBase.cpp \
	$(ROOT)/../c_common/module/libeventBase.cpp \
	$(ROOT)/../c_common/module/unixServer.cpp \
	$(ROOT)/../c_common/module/unixClient.cpp \
	$(ROOT)/../c_common/module/cmongo.cpp 

OBJ=$(C_SRC:%.c=%.o) $(CPP_SRC:%.cpp=%.o)

INCLUDE= \
	-I $(ROOT)/include \
	-I $(ROOT)/../c_common/include \
	-I $(INSTALLDIR)/lua51/include \
	-I $(INSTALLDIR)/mongoc/include/libbson-1.0 \
	-I $(INSTALLDIR)/mongoc/include/libmongoc-1.0 \
	-I $(INSTALLDIR)/libevent/include

LIB= \
	$(INSTALLDIR)/lua51/lib/liblua.a \
	$(INSTALLDIR)/mongoc/lib/libmongoc-static-1.0.a \
	$(INSTALLDIR)/mongoc/lib/libbson-static-1.0.a \
	$(INSTALLDIR)/libevent/lib/libevent.a \
	$(INSTALLDIR)/libevent/lib/libevent_core.a \
	$(INSTALLDIR)/libevent/lib/libevent_extra.a \
	$(INSTALLDIR)/libevent/lib/libevent_pthreads.a


$(BIN): $(OBJ) $(LIB)
	#g++ -o $@ $^ $(LIB) $(INCLUDE) -g -Wall -Wl,-Bstatic -lstdc++ -lc -lrt -lssl -lresolv -licuuc -licudata -lz -lcrypto -Wl,-Bdynamic -ldl -lpthread
	g++ -o $@ $^ $(LIB) $(INCLUDE) -g -Wall -Wl,-Bstatic -lstdc++ -lc -lrt -lssl -lresolv -lz -lcrypto -Wl,-Bdynamic -ldl -lpthread

%.o: %.c $(DEPS)
	gcc -c $< -o $@ $(INCLUDE) -Wall

%.o: %.cpp $(DEPS)
	g++ $< -o $@ $(INCLUDE) -c -Wall -std=c++11
#-c 不可或缺

clean:
	rm -rf $(OBJ) $(BIN)

