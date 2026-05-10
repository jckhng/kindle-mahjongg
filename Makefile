CC ?= gcc
PKG_CONFIG ?= pkg-config

CFLAGS += -Wall -Wextra -O2 $(shell $(PKG_CONFIG) --cflags gtk+-2.0 cairo)
LDLIBS += $(shell $(PKG_CONFIG) --libs gtk+-2.0 cairo) -lm

OBJS = main.o mahjongg_engine.o
TEST_OBJS = smoke_test.o mahjongg_engine.o

.PHONY: all clean

all: exact-mahjong-solitaire

exact-mahjong-solitaire: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

smoke-test: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LDLIBS)

clean:
	rm -f $(OBJS) $(TEST_OBJS) exact-mahjong-solitaire smoke-test
