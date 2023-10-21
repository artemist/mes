{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    let supportedSystems = [ "x86_64-linux" "i686-linux" "aarch64-linux" ];
    in (utils.lib.eachSystem supportedSystems (system:
      let pkgs = import nixpkgs { inherit system; };
      in rec {
        formatter = pkgs.nixfmt;

        devShells.rustybar = pkgs.mkShellNoCC {
          packages = with pkgs; [
            perl
            guile
            gnumake
            minimal-bootstrap.kaem
            minimal-bootstrap.mescc-tools
          ];

          CC="M2-Planet";
        };
        devShells.default = devShells.rustybar;
      }));
}

