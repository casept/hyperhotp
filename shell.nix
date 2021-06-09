let
  sources = import ./nix/sources.nix;
  pkgs = import sources.nixpkgs { };
in pkgs.mkShell {
  buildInputs = [
    pkgs.niv
    pkgs.usbutils

    pkgs.gcc
    pkgs.gnumake
    pkgs.pkg-config
    pkgs.cmake
    pkgs.cmake-format
    pkgs.bash
    pkgs.libusb
    pkgs.gtk4

    pkgs.gdb
    pkgs.valgrind
    pkgs.clang-tools
    pkgs.cppcheck
    pkgs.bear
    pkgs.include-what-you-use
  ];
}
