{
    description = "ext2driver";

    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, flake-utils }:
        flake-utils.lib.eachDefaultSystem (system:
            let 
                pkgs = import nixpkgs { inherit system; };
                buildInputs = [];
                nativeBuildInputs  = with pkgs; [ 
                    cppcheck 
                    clang 
                    cmake 
                    ninja 
                    gtest 
                    clang-tools 
                    valgrind 
                    gdb 
                    pkg-config 
                ];
            in with pkgs; rec {
                devShell = mkShell {
                    inherit nativeBuildInputs;
                    name = "ext2driver";

                    shellHook = ''
                        export CMAKE_EXPORT_COMPILE_COMMANDS=ON
                        export CMAKE_BUILD_TYPE=Debug
                        export TESTS=ON

                        echo "Entered devShell, Welcome!"
                        echo ""
                        echo "Configure: mkdir _build && cd _build && cmake .."
                        echo "Build:     make -C _build"
                        echo "Test:      cd _build && ctest --output-on-failure {-T memcheck for memory check}"
                        echo "Lint:      make -C _build lint"
                        echo "Format:    make -C _build format"
                        echo "Clean:     make -C _build clean"
                        echo "Install:   make -C _build install"
                        echo ""
                    '';
                };

                packages.app = stdenv.mkDerivation {
                    inherit nativeBuildInputs buildInputs;

                    name = "ext2driver";
                    src = ./.;

                    cmakeFlags = [
                        "-DCMAKE_BUILD_TYPE=Release"
                        "-DTESTS=OFF"
                    ];
                };

                packages.tests = stdenv.mkDerivation {
                    inherit nativeBuildInputs buildInputs;

                    name = "ext2driver-tests";
                    src = ./.;

                    cmakeFlags = [
                        "-DCMAKE_BUILD_TYPE=Debug"
                        "-DTESTS=ON"
                        "-DMEMCHECK_COMMAND=${pkgs.valgrind}/bin/valgrind"
                    ];

                    checkPhase = ''
                        ctest -T memcheck
                    '';

                    doCheck = true;
                };

                defaultPackage = packages.app;
            }
        );
}
