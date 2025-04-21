#include "inode.hpp"
#include "filesystem.hpp"
#include "helpers.hpp"

u64 Inode::size_in_bytes(Filesystem* fs) {
    if (fs->superblock.version_major >= 1 && !this->is_directory())
        return this->lower_size | ((u64)this->upper_size_or_dir_acl << 32);
    return this->lower_size;
}

void InodeIterator::increment() {
    const u64 inode_size = this->inode.disk_sector_count * 512 / this->fs->block_size;
    if ((usize)this->counter >= inode_size) {
        this->counter = -1;
        return;
    }

    u32 pointer = this->get_current_block_pointer();
    this->counter++;
    if (pointer == 0) return this->increment();

    this->fs->read_block(pointer, this->buffer);

    this->current = std::span<u8>(this->buffer, this->get_required_buffer_size());
}

u32 InodeIterator::get_current_block_pointer() {
    if (this->counter < 12) return this->inode.block_pointers[this->counter];

    const u32 pointer_block_size = this->fs->block_size / 4;
    const u32 last_ind_element  = 12 + pointer_block_size;
    const u32 last_dind_element = 12 + pointer_block_size * pointer_block_size;
    const u32 last_tind_element = 12 + pointer_block_size * pointer_block_size * pointer_block_size;

    u32 in_blk_idx    = this->counter - 12;
    u32 l1_blk_idx    = Inode::IND_BLOCK;
    u32 ptr_idx       = in_blk_idx % pointer_block_size;
    u32 l2_blk_idx    = Inode::DIND_BLOCK;
    u32 first_in_blk  = 13;

    if (this->counter < last_ind_element) {
        ptr_idx = in_blk_idx;
    } else if (this->counter < last_dind_element) {
        in_blk_idx   -= pointer_block_size;
        l1_blk_idx    = in_blk_idx / pointer_block_size;
        first_in_blk += pointer_block_size - 1;
    } else if (this->counter < last_tind_element) {
        in_blk_idx   -= pointer_block_size * pointer_block_size;
        l2_blk_idx    = in_blk_idx / (pointer_block_size * pointer_block_size);
        l1_blk_idx    = (in_blk_idx / pointer_block_size) % pointer_block_size;
        first_in_blk += pointer_block_size * pointer_block_size - 1;
    } else {
        PANIC("File size is too large to physically fit in the filesystem. Run a filesystem check.");
    }

    if (this->counter >= last_dind_element) {
        if (l1_blk_idx == 0) this->indirect_blk_count++;
        this->fs->read_block(this->inode.block_pointers[Inode::TIND_BLOCK], buffer);
    }

    if (this->counter >= last_ind_element) {
        if (ptr_idx == 0) this->indirect_blk_count++;
        this->fs->read_block(this->inode.block_pointers[l2_blk_idx], buffer);
    }

    if (this->counter == first_in_blk) this->indirect_blk_count++;
    this->fs->read_block(
            ((this->counter < last_ind_element) 
                ? this->inode.block_pointers 
                : (u32*)this->buffer
            )[l1_blk_idx]
            , buffer
        );

    return ((u32*)buffer)[ptr_idx];

}

u16 InodeIterator::get_required_buffer_size() {
    u64 inode_size = this->inode.disk_sector_count*512/this->fs->block_size;

    if ((usize)this->counter != inode_size - this->indirect_blk_count) [[likely]]
        return this->fs->block_size;

    u16 bytes_from_last_block = inode.size_in_bytes(this->fs) % this->fs->block_size;
    if (bytes_from_last_block == 0) bytes_from_last_block = this->fs->block_size;

    return bytes_from_last_block;
}

void DirInodeIterator::increment() {
    if (this->counter == -1) return;

    if (this->current_block_offset >= this->fs->block_size) {
        ++this->iter;
        if (this->iter == this->iter.end()) {
            this->counter = -1;
            return;
        }

        this->current_block_offset = 0;
    }

    const u32 inode = *(u32*)(this->buffer + this->current_block_offset);
    const u16 entry_size = *(u16*)(this->buffer + this->current_block_offset + 4);

    if (inode == 0) {
        if (entry_size == 0) {
            this->counter = -1;
            return;
        }

        this->current_block_offset += entry_size;

        return this->increment();
    }

    this->current = (DirectoryEntry*)(this->buffer+this->current_block_offset);
    this->current_block_offset += entry_size;
    this->counter++;
}

std::string_view DirectoryEntry::name(Filesystem* fs) {
    u16 name_length = this->lower_name_length;
    if (!(fs->e_superblock_present && fs->e_superblock.has_required_feature(RequiredFeatures::DirectoryType)))
        name_length |= this->upper_name_length_or_type << 8;

    return std::string_view(this->name_data, name_length);
}
