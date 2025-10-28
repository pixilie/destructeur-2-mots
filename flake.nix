{
  inputs = { nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05"; };

  outputs = { self, nixpkgs }:
    let
      inherit (nixpkgs.lib) genAttrs;

      forAllSystems =
        genAttrs [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" ];
      forAllPkgs = function: forAllSystems (system: function pkgs.${system});

      pkgs = forAllSystems (system: import nixpkgs { inherit system; });
    in {
      devShells = forAllPkgs (pkgs:
        with pkgs.lib; {
          default = pkgs.mkShell {
            buildInputs = with pkgs; [ gcc pkg-config gtk3 glib gdk-pixbuf bear ];

            shellHook = ''
              echo "CFLAGS for GTK:"
              pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0 glib-2.0
            '';
          };
        });
    };
}
