{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    let supportedSystems = [ "x86_64-linux" "i686-linux" "aarch64-linux" ];
    in (utils.lib.eachSystem supportedSystems (system:
      let
        pkgs = import nixpkgs { inherit system; };
        stage0Arch = {
          "aarch64-linux" = "aarch64";
          "i686-linux" = "x86";
          "x86_64-linux" = "amd64";
        }.${system} or (throw "Unsupported system: ${system}");
        mesArch = {
          "aarch64-linux" = "aarch64";
          "i686-linux" = "x86";
          "x86_64-linux" = "x86_64";
        }.${system} or (throw "Unsupported system: ${system}");
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

          CC = "M2-Planet";
          stage0_cpu = stage0Arch;
          mes_cpu = mesArch;
        };
        devShells.default = devShells.rustybar;
      }));
}

