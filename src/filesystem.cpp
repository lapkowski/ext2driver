#include "filesystem.hpp"

#include "helpers.hpp"
#include "inode.hpp"
#include "math.h"

#include <filesystem>
#include <time.h>
#include <vector>

#define EXT2_SUPERBLOCK      1024
#define EXT2_SUPERBLOCK_SIZE 1024
#define EXT2_ROOT_INODE      2

Filesystem::Filesystem(const char* path)
{
    this->file.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
    this->file.open(path, std::fstream::in | std::fstream::out | std::fstream::binary);
    this->file.seekg(EXT2_SUPERBLOCK);
    this->file.read((char*)&this->superblock, sizeof(SuperBlock));

    this->superblock.validate();

    if (this->superblock.version_major >= 1) {
        this->file.read((char*)&this->e_superblock, sizeof(ExSuperBlock));

        this->e_superblock_present = true;
        this->e_superblock.validate();
    }

    this->block_groups = ceil((double)this->superblock.total_blocks / (double)this->superblock.blocks_in_block_group);
    this->block_size   = 1024 << this->superblock.block_size_logarythm;
    this->inode_size   = (this->e_superblock_present) ? this->e_superblock.inode_size : 128;

    this->read_bgds();

    this->read_inode(Inode::ROOT_INODE, &this->root_inode);
}

void Filesystem::read_bgds()
{
    usize offset = (1 + this->superblock.superblock_block_number) * this->block_size;

    this->bgds = reinterpret_cast<BGD*>(smalloc(this->block_groups * sizeof(BGD)));

    this->file.seekg(offset);

    this->file.read(reinterpret_cast<char*>(this->bgds), sizeof(BGD) * this->block_groups);
}

void Filesystem::read_inode(u32 inode_id, Inode* buffer)
{
    usize group_index = (inode_id - 1) % this->superblock.inodes_in_block_group;
    BGD   bgd         = this->bgds[(inode_id - 1) / this->superblock.inodes_in_block_group];

    usize offset = (this->superblock.superblock_block_number + bgd.inode_table_address) * this->block_size;
    offset += group_index * this->inode_size;

    if (!buffer) buffer = (Inode*)smalloc(sizeof(Inode));

    this->file.seekg(offset);
    this->file.read(reinterpret_cast<char*>(buffer), sizeof(Inode));
}

NONNULL(u8*) Filesystem::read_block(u32 block_address, u8* buffer)
{
    if (!buffer) buffer = this->allocate_block();

    usize offset = (this->superblock.superblock_block_number + block_address) * this->block_size;

    this->file.seekg(offset);
    this->file.read(reinterpret_cast<char*>(buffer), this->block_size);

    return buffer;
}

void Filesystem::get_inode_from_path(const std::filesystem::path& path, Inode* inode)
{
    this->read_inode(Inode::ROOT_INODE, inode);
    u8*              buffer   = this->allocate_block();
    DirInodeIterator dir_iter = DirInodeIterator(this, *inode, buffer);

    for (std::filesystem::path path_element : path) {
        if (path_element == "/") continue;
        bool found = false;
        for (DirectoryEntry* entry : dir_iter) {
            if (entry->name(this) == path_element) {
                this->read_inode(entry->inode, inode);
                found = true;
                break;
            }
        }

        if (!found) PANIC("No such file or directory.");
    }

    free(buffer);
}

Filesystem::~Filesystem() { free(this->bgds); }
