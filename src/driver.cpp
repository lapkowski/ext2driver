#include "config.hpp"
#include "filesystem.hpp"
#include "helpers.hpp"
#include "inode.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

bool g_force = false;

const char* generic_help = "%s: Manipulate ext2 images - Version " VERSION_STRING "\n"
                           "USAGE:\n"
                           "\t%s <ACTION> <ACTION ARGUMENTS>\n\n"
                           "ACTIONS:\n"
                           "\thelp \t\t\t\t\t\t - display help information\n"
                           "\tadd <IMAGE> <FROM> <TO (defaults to /)>\t\t - add a file to the image\n"
                           "\tmkdir <IMAGE> <PATH>\t\t\t\t - create a directory\n"
                           "\tremove <IMAGE> <PATH>\t\t\t\t - remove a file or directory\n"
                           "\tquery <IMAGE> <PATH TO DIRECTORY>\t\t - get the contents of the directory\n"
                           "\tget <IMAGE> <PATH TO FILE> <OUTPUT DIR>\t\t - get the file from the image\n";

int help(int argc, char** argv)
{
    UNUSED(argc);

    printf(generic_help, argv[-1], argv[-1]);
    exit(0);
}

int add(int argc, char** argv)
{
    if (argc != 3 && argc != 4) {
        printf("USAGE: %s add <IMAGE> <FROM> <TO (defaults to /)>\n", argv[-1]);
        exit(0);
    }

    todo
}

int mkdir(int argc, char** argv)
{
    if (argc != 2) {
        printf("USAGE: %s mkdir <IMAGE> <PATH>\n", argv[-1]);
        exit(0);
    }

    todo
}

int rm(int argc, char** argv)
{
    if (argc != 2) {
        printf("USAGE: %s remove <IMAGE> <PATH>\n", argv[-1]);
        exit(0);
    }

    todo
}

int query(int argc, char** argv)
{
    if (argc != 3) {
        printf("USAGE: %s query <IMAGE> <PATH TO DIRECTORY>\n", argv[-1]);
        exit(0);
    }

    std::filesystem::path path(argv[2]);

    if (!path.is_absolute()) PANIC("<PATH TO DIRECTORY> must be absolute.");

    Filesystem fs(argv[1]);

    Inode inode;
    fs.get_inode_from_path(path, &inode);

    u8* buffer = reinterpret_cast<u8*>(smalloc(fs.block_size));

    DirInodeIterator dir_iter(&fs, inode, buffer);

    for (DirectoryEntry* entry : dir_iter) { std::cout << "DirEntry: " << entry->name(&fs) << std::endl; }

    free(buffer);

    return 0;
}

int get(int argc, char** argv)
{
    if (argc != 3) {
        printf("USAGE: %s query <IMAGE> <PATH TO FILE>\n", argv[-1]);
        exit(0);
    }

    std::filesystem::path path(argv[2]);

    if (!path.is_absolute()) PANIC("<PATH TO FILE> must be absolute.");

    Filesystem fs(argv[1]);

    Inode inode;
    fs.get_inode_from_path(path, &inode);
    API_ASSERT(!inode.is_directory());

    std::fstream file(path.filename());
    file.exceptions(std::ios::failbit | std::ios::badbit);

    u8* buffer = reinterpret_cast<u8*>(smalloc(fs.block_size));

    InodeIterator iter(&fs, inode, buffer);

    for (std::span<u8> i : iter) { file.write(reinterpret_cast<char*>(i.data()), i.size()); }

    free(buffer);

    return 0;
}

int main(int argc, char** argv)
{
    char* env_force = getenv("FORCE");
    g_force =
        (env_force != NULL && (!strcmp(env_force, "1") || !strcmp(env_force, "true") || !strcmp(env_force, "TRUE")));

    if (argc < 2) {
        printf("USAGE: %s <ACTION> <FILE> <ACTION ARGUMENTS>\n", argv[0]);
        exit(1);
    }

    ACTION("help", help)
    ACTION("add", add)
    ACTION("mkdir", mkdir)
    ACTION("remove", rm)
    ACTION("query", query)
    ACTION("get", get)

    printf("USAGE: %s <ACTION> <FILE> <ACTION ARGUMENTS>\n", argv[0]);
    exit(1);
}
