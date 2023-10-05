{
  inputs.nixpkgs.url = "nixpkgs/nixos-22.05";

  description = "A simple and minimal music player. Uses mpd as backend.";

  outputs = { self, nixpkgs }:
    let pkgs = import nixpkgs { system = "x86_64-linux"; };
        libtickit = let
          name = "libtickit";
          version = "0.4.3";
        in
          with pkgs; stdenv.mkDerivation {
            inherit name;
            inherit version;
            src = fetchurl {
              url = "http://www.leonerd.org.uk/code/${name}/${name}-${version}.tar.gz";
              sha256="sha256-qDBYj6H0yZ1UjBHm31AoHCPfoB914quVFR8CcV22vWM=";

            };
            buildInputs = [ pkg-config libtool ];
            nativeBuildInputs = [ unibilium libtermkey ];
            installPhase = ''
	      runHook preInstall
              make install PREFIX="$out" DESTDIR=""
	      runHook postInstall
            '';
          };
        seamus = with pkgs; stdenv.mkDerivation {
          name = "seamus";
          src = ./.;
          nativeBuildInputs = [ pkg-config ];
          buildInputs = [ libtickit libmpdclient ];
	  configurePhase = ''
	    runHook preConfigure
	    env PREFIX=$out ./configure
	    runHook postConfigure
	  '';
        };
    in {
      packages.x86_64-linux.libtickit = libtickit;
      packages.x86_64-linux.seamus = seamus;
      packages.x86_64-linux.default = seamus;
    };
}
