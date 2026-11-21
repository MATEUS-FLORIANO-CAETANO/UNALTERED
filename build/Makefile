# := e nao ?= porque o pacote do MinGW no shell.nix exporta CC=x86_64-w64-mingw32-gcc
# (ainda da pra trocar com: make CC=clang)
CC      := gcc
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
DEPS    := $(wildcard $(SRCDIR)/*.h)
# objetos ficam em obj-linux/ pra nao misturar com o codigo nem com o build do Windows
OBJDIR  := $(MKDIR)obj-linux
OBJS    := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

unaltered: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@

# ---- build pro Windows (cross-compile com MinGW-w64) ----
# objetos ficam em obj-win/ pra nao misturar com os .o do Linux
WIN_CC     := x86_64-w64-mingw32-gcc
WIN_RAYLIB := $(MKDIR)vendor/raylib-6.0_win64_mingw-w64
WIN_CFLAGS := -Wall -Wextra -O2 -I$(WIN_RAYLIB)/include
# -static: embute o runtime do MinGW (nao precisa mandar DLLs junto)
# -mwindows: esconde a janela de console
WIN_LDLIBS := -L$(WIN_RAYLIB)/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm -static -mwindows
WIN_OBJDIR := $(MKDIR)obj-win
WIN_OBJS   := $(patsubst $(SRCDIR)/%.c,$(WIN_OBJDIR)/%.o,$(SRCS))

windows: unaltered.exe

unaltered.exe: $(WIN_OBJS)
	$(WIN_CC) $(WIN_CFLAGS) -o $@ $(WIN_OBJS) $(WIN_LDLIBS)

$(WIN_OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(WIN_OBJDIR)
	$(WIN_CC) $(WIN_CFLAGS) -c -o $@ $<

$(WIN_OBJDIR):
	mkdir -p $@

clean:
	rm -f unaltered unaltered.exe
	rm -rf $(OBJDIR) $(WIN_OBJDIR)

.PHONY: clean windows
