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

        devShells.mes-m2 = pkgs.mkShellNoCC {
          packages = with pkgs; [
            minimal-bootstrap.kaem
            minimal-bootstrap.mescc-tools
          ];

          CC = "M2-Planet";
          stage0_cpu = stage0Arch;
          mes_cpu = mesArch;

          shellHook = ''
            export GUILE_LOAD_PATH=$PWD/mes/module:$PWD/module:${pkgs.minimal-bootstrap.mes.nyacc.guilePath}
          '';
        };

        devShells.mes-guile = pkgs.mkShell {
          packages = with pkgs; [
            perl
            guile
            gnumake
            minimal-bootstrap.mescc-tools
          ];

          shellHook = ''
            export GUILE_LOAD_PATH=$PWD/module:${pkgs.minimal-bootstrap.mes.nyacc.guilePath}
          '';
        };
        devShells.default = devShells.mes-m2;
      }));
}

