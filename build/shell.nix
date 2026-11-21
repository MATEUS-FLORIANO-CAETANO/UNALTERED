{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = with pkgs; [
    raylib
    pkg-config
    gcc
    gnumake
    # cross-compiler pra windows
    pkgsCross.mingwW64.buildPackages.gcc
  ];
}
