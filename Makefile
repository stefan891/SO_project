CFLAGS   = -Wall -g -std=gnu99
INCLUDES = -I./inc
OBJDIR   = obj

CLIENT_SRCS = src/defines.c src/err_exit.c src/shared_memory.c src/semaphore.c src/fifo.c src/client_0.c
CLIENT_OBJS = $(addprefix $(OBJDIR)/, $(CLIENT_SRCS:.c=.o))

SERVER_SRCS = src/defines.c src/err_exit.c src/shared_memory.c src/semaphore.c src/fifo.c src/server.c
SERVER_OBJS = $(addprefix $(OBJDIR)/, $(SERVER_SRCS:.c=.o))

all: $(OBJDIR) client_0 server

client_0: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@  -lm

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@  -lm


$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	@rm -vf ${CLIENT_OBJS}
	@rm -vf ${SERVER_OBJS}
	@rm -vf client_0
	@rm -vf server
	@rm -rf ${OBJDIR}
	@ipcrm -a
	@echo "Removed object files and executables..."

.PHONY: run clean
