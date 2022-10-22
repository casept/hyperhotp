# Hyperhotp

This is an open re-implementation of the programming software for the HOTP feature of the hyperFIDO security keys. It's based on reverse-engineering the Windows-based programmer available on Hypersecu's website [here](https://www.hypersecu.com/downloads).

While I have tested this with my key, I don't know how well it works for other models or even other devices of the same model.

Also, while this only issues FIDO commands to the device which have no relation to managing U2F credentials, I can't claim that this won't accidentally wipe your keys or brick your device. *As always, no warranty.*

## Usage

```shell
$ ./hyperhotp help
Usage: ./hyperhotp [help|check|reset|program] <8-character serial number> <40-character hex seed> <whether to generate 6-character (false) or 8-character (true) HOTP codes>
```

For full usage, see the man page `hyperhotp(1)`.

## Building

This program only depends on `libusb` at runtime, as well as a C compiler supporting at least `c11`, as well as `pkg-config` and `cmake` at build time. `gtk4` is also required if you want to build the GUI. On Unixoid platforms the build is done as usual:

```shell
$ mkdir build && cd build
$ cmake .. # Alternatively: cmake .. -DBUILD_GUI=ON to build with GUI (unfinished!)
$ make -j$(nproc)
```

If you have [Nix](https://nixos.org/download.html) installed you can simply open a `nix-shell` to get the exact environment this was developed in.

Building on Windows should also work, but hasn't been tested.

## Packaging status

[![Repology](https://repology.org/badge/vertical-allrepos/hyperhotp.svg)](https://repology.org/project/hyperhotp/versions)

## TODO (PRs welcome!)

* More testing
* Code cleanup + fixing compiler warnings
* Proper CLI syntax
* CI generating static binaries for all platforms
* Supporting programming multiple keys
