#!/bin/sh
cd "$(dirname "$0")" || exit 1
WINEPREFIX="$HOME/.wine64" WINEARCH=win64 exec wine build/unaltered.exe "$@"
