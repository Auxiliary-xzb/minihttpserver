CC = gcc
CFLAGS =-std=gnu99 -D_GNU_SOURCE -pg -Wall -pthread -g 

TARGETS := $(patsubst %.c,%,$(wildcard *.c))
LIB_SRCS:= $(wildcard ./src/*.c)
OBJS    := $(patsubst %.c,%.o,$(LIB_SRCS))
OBJ_DIR := ./obj/
SRC_DIR := ./src/
INCLUDE_DIR := ./include

OBJS 	:= $(notdir $(OBJS))
OBJS    := $(addprefix $(OBJ_DIR), $(OBJS))

all:$(TARGETS)

$(TARGETS):%:%.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ 

# 这里有一个疑惑，如果将target写成$(OBJ_DIR)/$(OBJS)
# 且不定义上面添加路径的OBJS，那么此时会出错。
# make -n --debug=verbose 看不见obj/pro_10_15.o的规则
# make -n -p 直接说没有obj/pro_10_15.o的规则
# 但是如果重定义了则可以通过
$(OBJS):$(OBJ_DIR)%.o:$(SRC_DIR)%.c
	$(CC) -c -fPIC -I $(INCLUDE_DIR) $(CFLAGS) $< -o $@

.PHONY:clean

clean:
	rm $(OBJS) $(TARGETS)
