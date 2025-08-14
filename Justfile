alias b := build
alias r := run
alias t := test

build_dir := "_build"
build_release := build_dir / "release"
build_debug := build_dir / "debug"

build-release +NINJA_FLAGS='':
    if [ ! -d {{build_release}} ]; then \
        mkdir -p {{build_release}}; \
        cd {{build_release}}; \
        cmake ../.. -DCMAKE_BUILD_TYPE=Release -G Ninja; \
    fi
    ninja -C {{build_release}} {{NINJA_FLAGS}}

build +NINJA_FLAGS='':
    if [ ! -d {{build_debug}} ]; then \
        mkdir -p {{build_debug}}; \
        cd {{build_debug}}; \
        cmake ../.. -DCMAKE_BUILD_TYPE=Debug -G Ninja -DTESTS=ON; \
    fi
    ninja -C {{build_debug}} {{NINJA_FLAGS}}

run +ARGS='': build
    cd {{build_debug}} && ./ext2_driver {{ARGS}}

debug: build
    cd {{build_debug}} && gdb ./ext2d_river

test: build
    ctest --output-on-failure --test-dir {{build_debug}}

[confirm]
clean:
    rm -rf {{build_dir}}

format:
    clang-format -i src/* tests/*

lint:
    cppcheck --enable=all --inconclusive --std=c++20 --suppress=missingInclude --suppress=unmatchedSuppression --suppress=internalAstError src tests

compile_commands: build
    cp {{build_debug}}/compile_commands.json .
