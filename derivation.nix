{ stdenv, lib, cmake, pkg-config, libusb, static ? false }:
stdenv.mkDerivation {
  name = "hyperhotp";
  src = ./.;
  nativeBuildInputs = [ cmake pkg-config ];
  buildInputs = [ libusb ];
  cmakeFlags = [ ] ++ lib.optional static [ "-DCMAKE_BUILD_STATIC=ON" ];
}
