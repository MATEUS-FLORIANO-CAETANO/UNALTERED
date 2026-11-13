{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = with pkgs; [
    raylib
    pkg-config
    gcc
    gnumake
  ];
}
