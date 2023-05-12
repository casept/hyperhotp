{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell {
  nativeBuildInputs = [
    pkgs.gcc
    pkgs.gnumake
    pkgs.pkg-config
    pkgs.cmake
    pkgs.cmake-format
    pkgs.bash
    pkgs.libusb1

    pkgs.gdb
    pkgs.valgrind
    pkgs.clang-tools
    pkgs.cppcheck
    pkgs.bear
    pkgs.include-what-you-use
  ];
}
