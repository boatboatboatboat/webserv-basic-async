# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: bruh <bruh@bruh.bruh>                        +#+                      #
#                                                    +#+                       #
#    Created: 2020/04/13 22:05:45 by bruh           #+#    #+#                 #
#    Updated: 2020/04/13 22:05:45 by bruh          ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

###########################

NAME			=	webserv

CXX             =   clang++
CC              =   clang
CC_FLAGS		=	-O3
CC_FLAGS_DEBUG	=	-g
CC_FLAGS_RDBG	=	-g
CXX_FLAGS		=	-O3 -Wall -Wextra -Werror -std=c++2a -DLOG_INFO=1
CXX_FLAGS_FULL  =   -DMODULE_LOADER_ENABLED -DEVIL_CHECK_ERRNO
CXX_FLAGS_DEBUG	=	-Wall -Wextra -g -std=c++2a
CXX_FLAGS_RDBG	=	-Wall -Wextra -Werror -std=c++2a -g
FLAGS_BIN		=	-O3 -pthread
FLAGS_BIN_ASAN	=	-fsanitize=address -g
FLAGS_BIN_LSAN  =   -fsanitize=leak -g

INCLUDE_DIRS	=	./modules/lua/
SRC_DIR			=	./
OBJ_DIR			=	./obj/
SRC_SUBDIRS		=	args boxed cgi config constants fs func futures http ioruntime json modules mutex net option regex utils modules/es modules/lua

SRC_C_FILES		= \
    modules/es/duk_print_alert modules/es/duktape \
    modules/lua/lapi modules/lua/lauxlib modules/lua/lbaselib \
    modules/lua/lcode modules/lua/lcorolib modules/lua/lctype \
    modules/lua/ldblib modules/lua/ldebug modules/lua/ldo \
    modules/lua/ldump modules/lua/lfunc \
    modules/lua/lgc modules/lua/linit modules/lua/liolib \
    modules/lua/llex modules/lua/lmathlib modules/lua/lmem \
    modules/lua/loadlib modules/lua/lobject modules/lua/lopcodes \
    modules/lua/loslib modules/lua/lparser modules/lua/lstate \
    modules/lua/lstring modules/lua/lstrlib modules/lua/ltable \
    modules/lua/ltablib modules/lua/ltm \
    modules/lua/lundump modules/lua/lutf8lib \
    modules/lua/lvm modules/lua/lzio

SRC_CXX_FILES	=  args/Args \
                      cgi/Cgi \
                      config/Config \
                      fs/File fs/Path \
                      func/Functor func/SetReadyFunctor \
                      futures/ForEachFuture futures/PollResult futures/Task futures/Waker \
                      http/DefaultPageReader http/DirectoryReader http/FutureSeReader \
                      http/Header http/IncomingBody http/IncomingMessage http/IncomingRequest \
                      http/IncomingResponse http/InfiniteReader http/MessageParser \
                      http/MessageReader http/Method http/OutgoingBody \
                      http/OutgoingMessage http/OutgoingRequest http/OutgoingResponse \
                      http/Proxy http/RequestParser http/RequestReader \
                      http/ResponseParser http/Server http/ServerHandlerResponse \
                      http/SpanReader http/Status http/StringReader \
                      http/Uri http/Version \
                      ioruntime/CharacterStream ioruntime/FdLineStream ioruntime/FdStringReadFuture \
                      ioruntime/FileDescriptor ioruntime/GlobalChildProcessHandler ioruntime/GlobalIoEventHandler \
                      ioruntime/GlobalRuntime ioruntime/GlobalTimeoutEventHandler ioruntime/IAsyncRead \
                      ioruntime/IAsyncWrite ioruntime/IEventHandler ioruntime/IExecutor \
                      ioruntime/IoEventHandler ioruntime/IoResult ioruntime/PooledExecutor \
                      ioruntime/Runtime ioruntime/RuntimeBuilder ioruntime/ThreadlessExecutor \
                      ioruntime/TimeoutEventHandler ioruntime/TimeoutFuture \
                      json/Json \
                      modules/ESLanguage modules/ESModule \
                      modules/GlobalModules \
                      modules/LuaLanguage modules/LuaModule \
                      mutex/mutex \
                      net/IpAddress net/Ipv4Address net/Ipv6Address \
                      net/Socket net/SocketAddr \
                      net/TcpListener net/TcpStream \
                      regex/Regex \
                      utils/base64 utils/cstr \
                      utils/dbg_puts utils/localtime utils/mem_copy \
                      utils/mem_zero utils/MemCompare utils/span \
                      utils/StringStream \
                      main

###########################

INCLUDE_DIRS := $(INCLUDE_DIRS:%=-I%)
SRC_C_FILES := $(SRC_C_FILES:%=%.o)
OBJ_C_FILES := $(SRC_C_FILES:%=$(OBJ_DIR)%)
SRC_CXX_FILES := $(SRC_CXX_FILES:%=%.oxx)
OBJ_CXX_FILES := $(SRC_CXX_FILES:%=$(OBJ_DIR)%)
OBJ_SUBDIRS := $(SRC_SUBDIRS:%=$(OBJ_DIR)%)

all: $(NAME)

$(NAME): $(OBJ_C_FILES) $(OBJ_CXX_FILES)
	@echo BUILD $(NAME) $(FLAGS_BIN)
	@$(CXX)	$^ \
			-o $(NAME) \
			$(CXX_FLAGS) \
			$(FLAGS_BIN)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p	$(OBJ_DIR) \
				$(OBJ_SUBDIRS)
	@$(CC)	$(INCLUDE_DIRS) \
			-c $^ \
			-o $@ \
			$(C_FLAGS)
	@echo CC $^

$(OBJ_DIR)%.oxx: $(SRC_DIR)%.cpp
	@mkdir -p	$(OBJ_DIR) \
				$(OBJ_SUBDIRS)
	@$(CXX)	$(INCLUDE_DIRS) \
			-c $^ \
			-o $@ \
			$(CXX_FLAGS)
	@echo CXX $^

re: fclean
	@$(MAKE)

clean:
	@$(RM) -rf $(OBJ_DIR)
	@echo CLEAN $(OBJ_DIR)

fclean: clean
	@$(RM) -f $(NAME)
	@echo FCLEAN $(NAME)

debug: FLAGS = $(FLAGS_DEBUG)
debug: all
bonus: CXX_FLAGS += $(CXX_FLAGS_FULL)
bonus: all
asan: FLAGS_BIN = $(FLAGS_BIN_ASAN)
asan: all
lsan: FLAGS_BIN = $(FLAGS_BIN_LSAN)
lsan: all

rdebug: fclean
	@$(MAKE) debug

srdb: FLAGS = $(FLAGS_RDBG)
srdb: all

rdb: fclean
	@$(MAKE) srdb

rafl: export CC = ~/afl/afl-clang
rafl: rasan

rasan: fclean
	@$(MAKE) asan

.PHONY:	all re clean fclean
