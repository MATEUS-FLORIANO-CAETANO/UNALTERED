CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2

# NixOS (shell.nix) e quem instalou raylib com .pc: usa pkg-config.
# senao (ex: Ubuntu com raylib compilado do fonte e `make install`, que nao
# gera raylib.pc), cai pra RAYLIB_PREFIX (padrao ~/.local se tiver raylib.h
# la, senao /usr/local; build estatico).
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
ifeq ($(RAYLIB_LIBS),)
RAYLIB_PREFIX ?= $(if $(wildcard $(HOME)/.local/include/raylib.h),$(HOME)/.local,/usr/local)
RAYLIB_CFLAGS := -I$(RAYLIB_PREFIX)/include
RAYLIB_LIBS   := -L$(RAYLIB_PREFIX)/lib -lraylib -lGL -lpthread -ldl -lrt -lX11
endif

LDLIBS  := $(RAYLIB_LIBS) -lm
CFLAGS  += $(RAYLIB_CFLAGS)

# diretorio deste Makefile, pra funcionar rodando de qualquer lugar
MKDIR   := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRCDIR  := $(MKDIR)../codigo
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
