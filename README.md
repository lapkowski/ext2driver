# A simple Read Only Ext2 driver written in C++

> [!CAUTION]
> I am not responsible for any data loss or corruption that may occur as a result of using this codebase.
> You have been warned.

## Requirements
- A C++20 compatible compiler
- GNU Make
- Cppcheck (optional, for static analysis)
- e2fsprogs (optional, for generating test images)

## Build
```
make
```
You can define `DEBUG` to enable debug mode. This will include additional debug information in the output and error messages.
```
make DEBUG=1
```
## Usage
To get the list of files in a directory:
```
_build/query query <IMAGE> <DIRECTORY>
```
To extract a file from an image:
```
_build/query query <IMAGE> <FILE>
```
## Testing
You can generate a test image by using the make img target.
This will copy all of the files from the testimgroot directory, and generate an image in BUILD_DIR/testimg.img
```
mkdir testimgroot
echo "Hello World!" > testimgroot/hello.txt
make img
_build/query get _build/testimg.img hello.txt
cat hello.txt
```
You may also use the make check target to run cppcheck on the codebase.
```
make check
cat _build/cppcheck_log.txt
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
