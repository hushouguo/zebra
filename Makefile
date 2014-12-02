TARGET = shard
SOURCE_DIR = src
DEPS_DIR = deps
JEMALLOC_LIB = deps/jemalloc/lib/libjemalloc.a
LIBEVENT_LIB = deps/libevent/lib/libevent.a
LUA_LIB = deps/lua/lib/liblua.a
LUACURL_LIB = deps/luacurl/lib/luacurl.a
CURL_LIB = -lcurl
MYSQL_LIB = -L/usr/lib64/mysql/ -lmysqlclient
LIBXML2_LIB = -lxml2

CC = gcc
CFLAGS = -I$(SOURCE_DIR) -I$(DEPS_DIR) -I$(DEPS_DIR)/libevent -I/usr/include/libxml2 -g -Wall -Werror -DLINUX 
LDFLAGS = \
		  -L/usr/lib64 \
		  -lrt \
		  -lz \
		  -lm \
		  $(JEMALLOC_LIB) \
		  -lpthread \
		  $(CURL_LIB) \
		  $(LIBEVENT_LIB) \
		  $(LUA_LIB) \
		  $(LUACURL_LIB) \
		  $(LIBXML2_LIB) \
		  $(MYSQL_LIB) 

SOURCES = $(shell find $(SOURCE_DIR) -depth -nowarn -name "*.c") 
OBJ_FILES = $(patsubst %.c, $(SOURCE_DIR)/%.o, $(notdir $(SOURCES)))

$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $(OBJ_FILES) $(LDFLAGS)

rebuild: clean $(TARGET)

$(SOURCE_DIR)/%.o:$(SOURCE_DIR)%.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@rm -fr $(OBJ_FILES) $(TARGET)

.PHONY: clean install release
