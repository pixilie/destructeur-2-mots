{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        fonts = with pkgs; [
          dejavu_fonts
          freefont_ttf
          google-fonts
          ubuntu-classic
        ];

        pythonEnv =
          pkgs.python3.withPackages (ps: [ ps.pillow ps.numpy ps.tqdm ]);

      in {
        devShells.default = pkgs.mkShell {
          buildInputs = [ pythonEnv ] ++ fonts;

          shellHook = ''
            echo "Building environnement..."
            mkdir -p ./fonts
            find ./fonts -type l -delete

            echo "Linking fonts..."
            for font in ${toString fonts}; do
              find $font -name "*.ttf" -exec ln -sf {} ./fonts/ \;
            done

            echo "Environnement ready!"
          '';
        };
      });
}
