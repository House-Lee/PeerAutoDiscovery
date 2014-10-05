###
 # =============================================================================
 # Yunlu Server Remote Management - Task Executor
 # Copyright (c) 2013 iQIYI inc.
 #
 # Author: House.Lee (lixiaodan@qiyi.com)
 # Created at: 2013-07-09 14:10
 #
 # =============================================================================
###
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
current_path =$(shell sh -c 'pwd')
root_path = $(subst executor,,$(current_path))

PROJECT = taskexecutor
DEPENDENCY_TARGETS = hiredis protobuf_c curl
INCLUDE = -I../deps/hiredis/include -I../deps/protobuf-c/include 
LIBS = ../deps/hiredis/lib/libhiredis.a ../deps/protobuf-c/lib/libprotobuf-c.a -lcurl
STD = -std=c99
CFLAGS = -Wall -D_BSD_SOURCE -D_POSIX_C_SOURCE
DEBUG = -g
CC = gcc
OBJS = executor.o distributor.o log.o msgtask.o task.o store.o publicreq.o
MAIN_OBJS = main.o $(OBJS)
TEST_CASE = test_executor.o test_queue.o test_map.o test_dstr.o test_store.o
#SOURCE = ../includes/*.c
SOURCE_PATH = ../includes
C_SOURCES = ../algorithm/*.c ../protos/compiled/*.c

ifeq ($(uname_S),Linux)
	DEPENDENCY_TARGETS += jemalloc
	INCLUDE += -I../deps/jemalloc/include
	CFLAGS += -DUSE_JEMALLOC
	LIBS += ../deps/jemalloc/lib/libjemalloc.a
endif

LIBS += -lpthread -ldl

.PHONY:main_obj
main_obj:$(MAIN_OBJS)
%.o:%.c
	$(CC) $(DEBUG) $(STD) $ -c $< -o $@ $(CFLAGS) $(INCLUDE)

.PHONY:install
install:$(PROJECT)

include $(SOURCE_PATH)/Makefile

$(PROJECT):pb commual $(MAIN_OBJS)
	$(CC) $(DEBUG) $(STD) -o $(PROJECT) $(INCLUDE) $(C_SOURCES) $(COMMUAL_OBJS) $(MAIN_OBJS) $(LIBS)



.PHONY:test
test:$(TEST_CASE)
$(TEST_CASE):%.o:%.c
	$(CC) $(DEBUG) $(STD) $< -o $@ $(CFLAGS) $(INCLUDE) $(C_SOURCES) $(COMMUAL_OBJS) $(OBJS) $(LIBS)

.PHONY:clean_test
clean_test:
	rm -f *.o
	rm -f test_*.o

.PHONY:pb
pb:
	@echo "Compiling protobuf files in ["$(root_path)"protos]"
	@mkdir -p $(root_path)protos/compiled
	cd $(root_path)protos &&\
	$(root_path)deps/protobuf-c/bin/protoc-c --c_out=./compiled *.proto
	@echo "Compile protobuf files finished"


.PHONY:deps
deps: $(DEPENDENCY_TARGETS)

.PHONY:clean_deps
clean_deps:
	rm -rf ../deps/hiredis
	rm -rf ../deps/jemalloc
	rm -rf ../deps/protobuf-c

hiredis:
	@echo "==================Installing hiredis================"
	cd ../deps/ && mkdir -p hiredis && unzip hiredis-master.zip
	cd ../deps/hiredis-master && make && make install PREFIX=$(root_path)deps/hiredis
	rm -rf ../deps/hiredis-master

protobuf_c:
	@echo "==================Installing protobuf-c=================="
	cd ../deps/ && tar -zxvf protobuf-2.5.0.tar.gz
	cd ../deps/protobuf-2.5.0 && ./configure
	cd ../deps/protobuf-2.5.0 && make clean && make && make install
	@echo "/usr/local/lib" >> /etc/ld.so.conf
	ldconfig -v
	cd ../deps/ && mkdir -p protobuf-c && tar -zxvf protobuf-c-0.15.tar.gz
	cd ../deps/protobuf-c-0.15 && ./configure --prefix=$(root_path)deps/protobuf-c
	cd ../deps/protobuf-c-0.15 && make && make install
	rm -rf ../deps/protobuf-2.5.0
	rm -rf ../deps/protobuf-c-0.15

jemalloc:
	@echo "==================Installing jemalloc=================="
	cd ../deps/ && mkdir -p jemalloc && tar -xvf jemalloc-3.4.0.tar.bz2
	cd ../deps/jemalloc-3.4.0 && ./configure --prefix=$(root_path)deps/jemalloc --with-jemalloc-prefix=je_ --enable-cc-silence
	cd ../deps/jemalloc-3.4.0 && make && make install
	rm -rf ../deps/jemalloc-3.4.0

curl:
	@echo "==================Installing libcurl=================="
	yum install libcurl* -y

.PHONY:clean
clean:
	rm -f test_*.o
	rm -f $(SOURCE_PATH)/*.o
	rm *.o $(PROJECT)

.PHONY:all_clean
all_clean: clean
	rm -rf ../deps/hiredis
	rm -rf ../deps/jemalloc
	rm -rf ../deps/protobuf-c
	
	
	
	
	
	