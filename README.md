# A simple Read Only Ext2 driver written in C++

> [!CAUTION]
> I am not responsible for any data loss or corruption that may occur as a result of using this codebase.
> You have been warned.

## Requirements
- A C++20 compatible compiler
- GNU Make/Ninja
- CMake
- Clang-Format (optional, but needed for contributing)
- Cppcheck (optional, for static analysis)
- Python 3 (optional, for running the test suite)
- e2fsprogs (optional, for running the test suite)
- Nix (optional, for a developer environment)

## Build
```sh
mkdir _build && cd _build
cmake .. # -DCMAKE_BUILD_TYPE=Debug for debugging -DTESTS=ON for test compilation
make
```
You can also use nix to build the project, or to automatically download the dependencies.
```sh
nix develop # For a development shell
nix build   # For building the project
```
## Usage
To get the list of files in a directory:
```sh
_build/ext2driver query <IMAGE> <DIRECTORY>
```
To extract a file from an image:
```sh
_build/ext2driver get <IMAGE> <FILE>
```
## Testing
You can run driver tests using ctest (Note that the TESTS option must be ON while configuring)
``` sh
ctest --output-on-failure .
```

You may also use the make lint target to run cppcheck on the codebase.
```sh
make lint
```

## TODO
- [ ] Proper error handling for C++ I/O operations.
- [ ] Filesystem creation on streams rather than files.
- [ ] Hashed directory support
- [ ] Support for compressed files
- [ ] Write support
- [ ] Ext3/4 support

## License
This project is licensed under the MIT-0 license. See the [LICENSE](LICENSE) file for details.
