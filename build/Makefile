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

# ---- build pro navegador (WebGL, com emscripten) ----
# raylib pre-compilado pra web: raylib-6.0_webassembly.zip das releases do raylib
# sai em web/ (index.html + .js + .wasm + .data); `make web-zip` gera o zip pro itch.io
WEB_CC     := emcc
WEB_RAYLIB := $(MKDIR)vendor/raylib-6.0_webassembly
WEB_CFLAGS := -Wall -Wextra -O2 -I$(WEB_RAYLIB)/include -DPLATFORM_WEB
# ASYNCIFY: deixa o while (!WindowShouldClose()) do main.c funcionar sem mudar o codigo
# --preload-file: empacota assets/ no .data, montado em /assets (main.c faz cd pra /)
# STACK_SIZE: o padrao do emscripten e 64KB, e o Game g do main (com balas[256]) nao cabe.
# no Linux/Windows a pilha e ~8MB, por isso so quebra na web
WEB_LDFLAGS := -L$(WEB_RAYLIB)/lib -lraylib.web -sUSE_GLFW=3 -sASYNCIFY -sSTACK_SIZE=1048576 \
               -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 \
               --preload-file $(MKDIR)../assets@/assets \
               --shell-file $(MKDIR)web-shell.html
# make web DEBUG=1: nomes de funcao no stack trace + checagens do emscripten (lento)
ifdef DEBUG
WEB_CFLAGS  += -g
WEB_LDFLAGS += -g -sASSERTIONS=2 -sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2
endif
WEB_OBJDIR := $(MKDIR)obj-web
WEB_OBJS   := $(patsubst $(SRCDIR)/%.c,$(WEB_OBJDIR)/%.o,$(SRCS))
WEB_OUT    := $(MKDIR)web

web: $(WEB_OUT)/index.html

$(WEB_OUT)/index.html: $(WEB_OBJS) $(wildcard $(MKDIR)../assets/*) $(MKDIR)web-shell.html | $(WEB_OUT)
	$(WEB_CC) $(WEB_CFLAGS) -o $@ $(WEB_OBJS) $(WEB_LDFLAGS)

$(WEB_OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(WEB_OBJDIR)
	$(WEB_CC) $(WEB_CFLAGS) -c -o $@ $<

$(WEB_OBJDIR) $(WEB_OUT):
	mkdir -p $@

# zip com index.html na raiz (o itch.io exige isso)
web-zip: web
	cd $(WEB_OUT) && rm -f ../unaltered-web.zip && zip -q ../unaltered-web.zip index.html index.js index.wasm index.data

# testar local: http://localhost:8080 (abrir o index.html direto nao funciona)
web-serve: web
	cd $(WEB_OUT) && python3 -m http.server 8080

clean:
	rm -f unaltered unaltered.exe unaltered-web.zip
	rm -rf $(OBJDIR) $(WIN_OBJDIR) $(WEB_OBJDIR) $(WEB_OUT)

.PHONY: clean windows web web-zip web-serve
