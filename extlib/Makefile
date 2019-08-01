INSTALLDIR=$(shell pwd)/install

LIBEVENT=libevent-2.1.10-stable
LUA51=lua-5.1.5
CMAKE=cmake-3.9.2
MONGOC=mongo-c-driver-1.13.0

#cmake库解压第一次能正常跑 再跑就出错??
#mongoc依赖高版本cmake

all: lua libevent 
#all: cmake mongoc lua libevent

mongoc:
	(cd $(MONGOC) && $(INSTALLDIR)/cmake/bin/cmake . -DCMAKE_INSTALL_PREFIX=$(INSTALLDIR)/mongoc && make install)

cmake:
	(cd $(CMAKE) && ./configure --prefix=$(INSTALLDIR)/cmake && make install)

lua:
	(cd $(LUA51) && make linux && make install INSTALL_TOP=$(INSTALLDIR)/lua51)

libevent:
	(cd $(LIBEVENT) && ./configure --prefix=$(INSTALLDIR)/libevent && make install)

clean:
	(cd $(LUA51) && make clean)
	(cd $(LIBEVENT) && make clean)
