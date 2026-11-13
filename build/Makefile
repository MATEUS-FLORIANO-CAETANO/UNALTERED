CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2
LDLIBS  := $(shell pkg-config --libs raylib) -lm
CFLAGS  += $(shell pkg-config --cflags raylib)

SRCDIR  := ./codigo
SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(SRCS:.c=.o)
DEPS    := $(wildcard $(SRCDIR)/*.h)

unaltered: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f unaltered $(OBJS)

.PHONY: clean
