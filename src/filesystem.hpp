#pragma once
#include "helpers.hpp"
#include "inode.hpp"

#include <filesystem>
#include <fstream>

enum class FilesystemState : u16 {
    Clean     = 1,
    Unclean   = 2,
    Corrupted = 117 /* From the linux kernel ext2 driver */
};

enum class OnErrorAction : u16 {
    Ignore    = 1,
    RemountRO = 2,
    Panic     = 3
};

enum class OSID : u32 {
    Linux     = 0,
    GNUHurd   = 1,
    Masix     = 2,
    FreeBSD   = 3,
    OtherLite = 4
};

enum class RequiredFeatures : u32 {
    Compression   = 0x1,
    DirectoryType = 0x2,
    JournalReplay = 0x4,
    JournalDevice = 0x8,
    Other         = 0b1111111111111111111111111110000
};

enum class OptionalFeatures : u32 {
    Preallocation           = 0x1,
    AFSInodes               = 0x2,
    Journal                 = 0x3,
    InodeExtendedAttributes = 0x8,
    Resizing                = 0x10,
    DirectoryHashIndex      = 0x20,
    Other                   = 0b1111111111111111111111111000000
};

enum class WriteFeatures : u32 {
    SparseSuperBlocks = 0x1,
    _64BIT            = 0x2,
    BinaryTree        = 0x4,
    Other             = 0b1111111111111111111111111111000
};

enum class CompressionAlgorithms : u32 {
    LZV1   = 0x0000001,
    LZRW3A = 0x0000002,
    GZIP   = 0x0000004,
    BZIP2  = 0x0000008,
    LZO    = 0x0000010,
    Other  = 0b1111111111111111111111111100000
};

struct SuperBlock {
    u32             total_inodes;
    u32             total_blocks;
    u32             blocks_reserved_for_superuser;
    u32             unallocated_blocks;
    u32             unallocated_inodes;
    u32             superblock_block_number;
    u32             block_size_logarythm;
    u32             fragment_size_logarythm;
    u32             blocks_in_block_group;
    u32             fragments_in_block_group;
    u32             inodes_in_block_group;
    u32             last_mount_time_posix;
    u32             last_write_time_posix;
    u16             number_of_mounts_since_fsck;
    u16             number_of_mounts_since_fsck_allowed;
    u16             signature;
    FilesystemState filesystem_state;
    OnErrorAction   onerror_action;
    u16             version_minor;
    u32             last_fsck_posix;
    u32             time_between_forced_fsck_posix;
    OSID            os_id;
    u32             version_major;
    u16             user_id_for_reserved;
    u16             group_id_for_reserved;

    const static u32 EXT2_SIGNATURE = 0xEF53;

    inline void validate() const
    {
        if (this->signature != EXT2_SIGNATURE)
            PANIC_FORCABLE(
                "Invalid EXT2 signature, you might be reading an EXT3/4 filesystem as EXT2, which is unsafe.");

        if (this->version_major != 1 && this->version_major != 0)
            PANIC_FORCABLE("Unsupported EXT2 version %ud", this->version_major);

        if (this->filesystem_state != FilesystemState::Clean)
            PANIC_FORCABLE("Filesystem is %s. Run a filesystem check and rerun this program.",
                           (this->filesystem_state == FilesystemState::Unclean) ? "unclean" : "corrupted");

        if ((this->number_of_mounts_since_fsck + 1 > this->number_of_mounts_since_fsck_allowed &&
             this->number_of_mounts_since_fsck_allowed != 0) ||
            (time(NULL) - this->last_fsck_posix > this->time_between_forced_fsck_posix &&
             this->time_between_forced_fsck_posix != 0))
            PANIC_FORCABLE("Filesystem recommends running a periodic fsck. Run fsck and rerun this program.");
    }
} __attribute__((packed));

#define SuperBlock_dbg(x)                                                                                            \
    "SuperBlock: {\n"                                                                                                \
    "\ttotal_inodes: %ud\n"                                                                                          \
    "\ttotal_blocks: %ud\n"                                                                                          \
    "\tblocks_reserved_for_superuser: %ud\n"                                                                         \
    "\tunallocated_blocks: %ud\n"                                                                                    \
    "\tunallocated_inodes: %ud\n"                                                                                    \
    "\tsuperblock_block_number: %ud\n"                                                                               \
    "\tblock_size_logarythm: %ud\n"                                                                                  \
    "\tfragment_size_logarythm: %ud\n"                                                                               \
    "\tblocks_in_block_group: %ud\n"                                                                                 \
    "\tfragments_in_block_group: %ud\n"                                                                              \
    "\tinodes_in_block_group: %ud\n"                                                                                 \
    "\tlast_mount_time_posix: %ud\n"                                                                                 \
    "\tlast_write_time_posix: %ud\n"                                                                                 \
    "\tnumber_of_mounts_since_fsck: %ud\n"                                                                           \
    "\tnumber_of_mounts_since_fsck_allowed: %ud\n"                                                                   \
    "\tsignature: %ud\n"                                                                                             \
    "\tfilesystem_state: %s\n"                                                                                       \
    "\tonerror_action: %ud\n"                                                                                        \
    "\tversion_minor: %ud\n"                                                                                         \
    "\tlast_fsck_posix: %ud\n"                                                                                       \
    "\ttime_between_forced_fsck_posix: %ud\n"                                                                        \
    "\tos_id: %ud\n"                                                                                                 \
    "\tversion_major: %ud\n"                                                                                         \
    "\tuser_id_for_reserved: %ud\n"                                                                                  \
    "\tgroup_id_for_reserved: %ud\n"                                                                                 \
    "}\n",                                                                                                           \
        x.total_inodes, x.total_blocks, x.blocks_reserved_for_superuser, x.unallocated_blocks, x.unallocated_inodes, \
        x.superblock_block_number, x.block_size_logarythm, x.fragment_size_logarythm, x.blocks_in_block_group,       \
        x.fragments_in_block_group, x.inodes_in_block_group, x.last_mount_time_posix, x.last_write_time_posix,       \
        x.number_of_mounts_since_fsck, x.number_of_mounts_since_fsck_allowed, x.signature,                           \
        (x.filesystem_state == FilesystemState::Clean) ? "clean" : "unclean", static_cast<u32>(x.onerror_action),    \
        x.version_minor, x.last_fsck_posix, x.time_between_forced_fsck_posix, static_cast<u32>(x.os_id),             \
        x.version_major, x.user_id_for_reserved, x.group_id_for_reserved

struct ExSuperBlock {
    u32  first_non_reserved_inode;
    u16  inode_size;
    u16  block_group_of_superblock;
    u32  optional_features;
    u32  required_features;
    u32  write_features;
    u8   fsid[16];
    char volume_name[16];
    char last_mount_path[64];
    u32  compresion_algorythms;
    u8   blocks_to_preallocate_files;
    u8   blocks_to_preallocate_directories;
    u16  __unused;
    u8   journal_id[16];
    u32  journal_inode;
    u32  journal_device;
    u32  head_of_orphan_inode_list;

    inline void validate() const
    {
        if (this->optional_features & (u32)OptionalFeatures::Other)
            PANIC_FORCABLE("Filesystem suggests unknown extensions (OPT %b).", this->optional_features);

        if (this->required_features & (u32)RequiredFeatures::Other)
            PANIC_FORCABLE("Filesystem requires unknown extensions (REQ %b).", this->required_features);

        if (this->write_features & (u32)WriteFeatures::Other)
            PANIC_FORCABLE("Filesystem requires unknown extensions (W-ONLY %b).", this->write_features);

        if (this->compresion_algorythms & (u32)CompressionAlgorithms::Other)
            PANIC_FORCABLE("Filesystem suggests unknown extensions (ALGH %b).", this->compresion_algorythms);
    }

    inline bool has_optional_feature(OptionalFeatures feature) const
    {
        return (this->optional_features & (u32)feature) != 0;
    }

    inline bool has_required_feature(RequiredFeatures feature) const
    {
        return (this->required_features & (u32)feature) != 0;
    }

    inline bool has_write_feature(WriteFeatures feature) const { return (this->write_features & (u32)feature) != 0; }

    inline bool is_compressed_with(CompressionAlgorithms algorithm) const
    {
        return (this->compresion_algorythms & (u32)algorithm) != 0;
    }
} __attribute__((packed));

#define ExSuperBlock_dbg(x)                                                                                        \
    "ExSuperBlock: {\n"                                                                                            \
    "\tfirst_non_reserved_inode: %ud\n"                                                                            \
    "\tinode_size: %ud\n"                                                                                          \
    "\tblock_group_of_superblock: %ud\n"                                                                           \
    "\toptional_features: %s %s %s %s %s %s\n"                                                                     \
    "\trequired_features: %s %s %s %s\n"                                                                           \
    "\twrite_features: %s %s %s\n"                                                                                 \
    "\tfsid: [%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x]\n"                                   \
    "\tvolume_name: %.*s\n"                                                                                        \
    "\tlast_mount_path: %.*s\n"                                                                                    \
    "\tcompression_algorythms: %s %s %s %s %s\n"                                                                   \
    "\tblocks_to_preallocate_files: %ud\n"                                                                         \
    "\tblocks_to_preallocate_directories: %ud\n"                                                                   \
    "\t__unused: %ud\n"                                                                                            \
    "\tjournal_id: [%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x]\n"                             \
    "\tjournal_inode: %ud\n"                                                                                       \
    "\tjournal_device: %ud\n"                                                                                      \
    "\thead_of_orphan_inode_list: %ud\n"                                                                           \
    "}\n",                                                                                                         \
        x.first_non_reserved_inode, x.inode_size, x.block_group_of_superblock,                                     \
        (x.has_optional_feature(OptionalFeatures::AFSInodes)) ? "AfsInodes" : "",                                  \
        (x.has_optional_feature(OptionalFeatures::DirectoryHashIndex)) ? "DirectoryHashIndex" : "",                \
        (x.has_optional_feature(OptionalFeatures::InodeExtendedAttributes)) ? "InodeExtendedAtrributes" : "",      \
        (x.has_optional_feature(OptionalFeatures::Journal)) ? "Journal" : "",                                      \
        (x.has_optional_feature(OptionalFeatures::Preallocation)) ? "Preallocation" : "",                          \
        (x.has_optional_feature(OptionalFeatures::Resizing)) ? "Resizing" : "",                                    \
        (x.has_required_feature(RequiredFeatures::Compression)) ? "Compression" : "",                              \
        (x.has_required_feature(RequiredFeatures::DirectoryType)) ? "DirectoryType" : "",                          \
        (x.has_required_feature(RequiredFeatures::JournalReplay)) ? "JournalReplay" : "",                          \
        (x.has_required_feature(RequiredFeatures::JournalDevice)) ? "JournalDevice" : "",                          \
        (x.has_write_feature(WriteFeatures::SparseSuperBlocks)) ? "SparseSuperblocks" : "",                        \
        (x.has_write_feature(WriteFeatures::_64BIT)) ? "64BIT" : "",                                               \
        (x.has_write_feature(WriteFeatures::BinaryTree)) ? "BinaryTree" : "", x.fsid[0], x.fsid[1], x.fsid[2],     \
        x.fsid[3], x.fsid[4], x.fsid[5], x.fsid[6], x.fsid[7], x.fsid[8], x.fsid[9], x.fsid[10], x.fsid[11],       \
        x.fsid[12], x.fsid[13], x.fsid[14], x.fsid[15], 16, x.volume_name, 64, x.last_mount_path,                  \
        (x.is_compressed_with(CompressionAlgorithms::LZV1)) ? "LZV1" : "",                                         \
        (x.is_compressed_with(CompressionAlgorithms::LZRW3A)) ? "LZRW3A" : "",                                     \
        (x.is_compressed_with(CompressionAlgorithms::GZIP)) ? "GZIP" : "",                                         \
        (x.is_compressed_with(CompressionAlgorithms::BZIP2)) ? "BZIP2" : "",                                       \
        (x.is_compressed_with(CompressionAlgorithms::LZO)) ? "LZO" : "", x.blocks_to_preallocate_files,            \
        x.blocks_to_preallocate_directories, x.__unused, x.journal_id[0], x.journal_id[1], x.journal_id[2],        \
        x.journal_id[3], x.journal_id[4], x.journal_id[5], x.journal_id[6], x.journal_id[7], x.journal_id[8],      \
        x.journal_id[9], x.journal_id[10], x.journal_id[11], x.journal_id[12], x.journal_id[13], x.journal_id[14], \
        x.journal_id[15], x.journal_inode, x.journal_device, x.head_of_orphan_inode_list

typedef struct {
    u32 block_bitmap;
    u32 inode_bitmap;
    u32 inode_table_address;
    u16 unallocated_blocks;
    u16 unallocated_inodes;
    u16 directories_in_group;
    u8  __unused[14];
} __attribute__((packed)) BGD;

#define BGD_dbg(x)                                                                                         \
    "BGD: {\n"                                                                                             \
    "\tblock_bitmap: %ud\n"                                                                                \
    "\tinode_bitmap: %ud\n"                                                                                \
    "\tinode_table_address: %ud\n"                                                                         \
    "\tunallocated_blocks: %ud\n"                                                                          \
    "\tunallocated_inodes: %ud\n"                                                                          \
    "\tdirectories_in_group: %ud\n"                                                                        \
    "}\n",                                                                                                 \
        x.block_bitmap, x.inode_bitmap, x.inode_table_address, x.unallocated_blocks, x.unallocated_inodes, \
        x.directories_in_group

class Filesystem
{
  public:
    std::fstream file;
    SuperBlock   superblock;
    bool         e_superblock_present;
    ExSuperBlock e_superblock;
    u32          block_groups;
    u64          block_size;
    BGD*         bgds;
    u16          inode_size;
    Inode        root_inode;

  public:
    explicit Filesystem(char* path);
    ~Filesystem();
    inline u8* allocate_block() { return (u8*)smalloc(this->block_size); }
    NONNULL(u8*) read_block(u32 block_number, u8* buffer);
    void get_inode_from_path(const std::filesystem::path& path, Inode* inode);
    void read_inode(u32 inode_id, Inode* inode);

  private:
    void read_bgds();
};

#define Filesystem_dbg(x)           \
    "Filesystem: {\n"               \
    "\tsuperblock\n"                \
    "\te_superblock_present: %ud\n" \
    "\te_superblock\n"              \
    "\tblock_groups: %ud\n"         \
    "\tblock_size: %lu\n"           \
    "\tbgds: %p\n"                  \
    "\tinode_size: %ud\n"           \
    "\troot_inode\n"                \
    "}\n",                          \
        x.e_superblock_present, x.block_groups, x.block_size, x.bgds, x.inode_size
