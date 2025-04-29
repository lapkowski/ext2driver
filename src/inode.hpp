#pragma once

#include "helpers.hpp"

#include <cstddef>
#include <iterator>
#include <span>

class Filesystem;

struct Inode {
    static const int BAD_BLOCKS_INODE   = 1;
    static const int ROOT_INODE         = 2;
    static const int ACL_INDEX_INODE    = 3;
    static const int ACL_DATA_INODE     = 4;
    static const int BOOT_LOADER_INODE  = 5;
    static const int UNDELETE_DIR_INODE = 6;

    static const int NDIR_BLOCKS = 12;
    static const int IND_BLOCK   = NDIR_BLOCKS;
    static const int DIND_BLOCK  = IND_BLOCK + 1;
    static const int TIND_BLOCK  = DIND_BLOCK + 1;
    static const int N_BLOCKS    = TIND_BLOCK + 1;

    u16 type_and_permissions;
    u16 user_id;
    u32 lower_size;
    u32 last_access_time;
    u32 creation_time;
    u32 last_modification_time;
    u32 deletion_time;
    u16 group_id;
    u16 hard_link_count;
    u32 disk_sector_count;
    u32 flags;
    u32 os_specyfic_one;
    u32 block_pointers[N_BLOCKS];
    u32 generation_number;
    u32 extended_attribute_block;
    u32 upper_size_or_dir_acl;
    u32 fragment_block_address;
    u32 os_specyfic_two;

    static const u32 FILE_TYPE_MASK      = 00170000;
    static const u32 FILE_TYPE_SOCKET    = 0140000;
    static const u32 FILE_TYPE_LINK      = 0120000;
    static const u32 FILE_TYPE_FILE      = 0100000;
    static const u32 FILE_TYPE_BLOCK_DEV = 0060000;
    static const u32 FILE_TYPE_DIRECTORY = 0040000;
    static const u32 FILE_TYPE_CHAR_DEV  = 0020000;
    static const u32 FILE_TYPE_FIFO      = 0010000;

    inline bool is_fifo() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_FIFO; }
    inline bool is_char_device() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_CHAR_DEV; }
    inline bool is_directory() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_DIRECTORY; }
    inline bool is_block_device() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_BLOCK_DEV; }
    inline bool is_file() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_FILE; }
    inline bool is_symbolic_link() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_LINK; }
    inline bool is_socket() const { return (this->type_and_permissions & FILE_TYPE_MASK) == FILE_TYPE_SOCKET; }

    static const u32 FILE_PERMISSION_OTHER_EXECUTE = 0x001;
    static const u32 FILE_PERMISSION_OTHER_WRITE   = 0x002;
    static const u32 FILE_PERMISSION_OTHER_READ    = 0x004;
    static const u32 FILE_PERMISSION_GROUP_EXECUTE = 0x008;
    static const u32 FILE_PERMISSION_GROUP_WRITE   = 0x010;
    static const u32 FILE_PERMISSION_GROUP_READ    = 0x020;
    static const u32 FILE_PERMISSION_USER_EXECUTE  = 0x040;
    static const u32 FILE_PERMISSION_USER_WRITE    = 0x080;
    static const u32 FILE_PERMISSION_USER_READ     = 0x100;
    static const u32 FILE_PERMISSION_STICKY        = 0x200;
    static const u32 FILE_PERMISSION_SET_GROUP_ID  = 0x400;
    static const u32 FILE_PERMISSION_SET_USER_ID   = 0x800;

    inline bool can_other_execute() const { return (this->type_and_permissions & FILE_PERMISSION_OTHER_EXECUTE) != 0; }
    inline bool can_other_write() const { return (this->type_and_permissions & FILE_PERMISSION_OTHER_WRITE) != 0; }
    inline bool can_other_read() const { return (this->type_and_permissions & FILE_PERMISSION_OTHER_READ) != 0; }
    inline bool can_group_execute() const { return (this->type_and_permissions & FILE_PERMISSION_GROUP_EXECUTE) != 0; }
    inline bool can_group_write() const { return (this->type_and_permissions & FILE_PERMISSION_GROUP_WRITE) != 0; }
    inline bool can_group_read() const { return (this->type_and_permissions & FILE_PERMISSION_GROUP_READ) != 0; }
    inline bool can_user_execute() const { return (this->type_and_permissions & FILE_PERMISSION_USER_EXECUTE) != 0; }
    inline bool can_user_write() const { return (this->type_and_permissions & FILE_PERMISSION_USER_WRITE) != 0; }
    inline bool can_user_read() const { return (this->type_and_permissions & FILE_PERMISSION_USER_READ) != 0; }
    inline bool is_sticky() const { return (this->type_and_permissions & FILE_PERMISSION_STICKY) != 0; }
    inline bool has_set_group_id() const { return (this->type_and_permissions & FILE_PERMISSION_SET_GROUP_ID) != 0; }
    inline bool has_set_user_id() const { return (this->type_and_permissions & FILE_PERMISSION_SET_USER_ID) != 0; }

    static const u32 FLAGS_SECURE_DELETION         = 0x1;
    static const u32 FLAGS_KEEP_COPY_ON_DELETE     = 0x2;
    static const u32 FLAGS_FILE_COMPRESSION        = 0x4;
    static const u32 FLAGS_SYNCHRONOUS_UPDATES     = 0x8;
    static const u32 FLAGS_IMMUTABLE_FILE          = 0x10;
    static const u32 FLAGS_APPEND_ONLY             = 0x20;
    static const u32 FLAGS_DUMP_NOT_INCLUDED       = 0x40;
    static const u32 FLAGS_DONT_UPDATE_LAST_ACCESS = 0x80;
    static const u32 FLAGS_HASH_INDEXED_DIRECTORY  = 0x10000;
    static const u32 FLAGS_AFS_DIRECTORY           = 0x20000;
    static const u32 FLAGS_JOURNAL_FILE_DATA       = 0x40000;

    inline bool delete_securely() const { return (this->flags & FLAGS_SECURE_DELETION) != 0; }
    inline bool keep_copy_on_delete() const { return (this->flags & FLAGS_KEEP_COPY_ON_DELETE) != 0; }
    inline bool compressed() const { return (this->flags & FLAGS_FILE_COMPRESSION) != 0; }
    inline bool write_immediately() const { return (this->flags & FLAGS_SYNCHRONOUS_UPDATES) != 0; }
    inline bool is_immutable() const { return (this->flags & FLAGS_IMMUTABLE_FILE) != 0; }
    inline bool append_only() const { return (this->flags & FLAGS_APPEND_ONLY) != 0; }
    inline bool not_included_in_dump() const { return (this->flags & FLAGS_DUMP_NOT_INCLUDED) != 0; }
    inline bool dont_update_last_access() const { return (this->flags & FLAGS_DONT_UPDATE_LAST_ACCESS) != 0; }
    inline bool has_hash_indexed_directory() const { return (this->flags & FLAGS_HASH_INDEXED_DIRECTORY) != 0; }
    inline bool afs_directory_present() const { return (this->flags & FLAGS_AFS_DIRECTORY) != 0; }
    inline bool has_journal_file_data() const { return (this->flags & FLAGS_JOURNAL_FILE_DATA) != 0; }

    u64 size_in_bytes(Filesystem* fs);
} __attribute__((packed));

#define Inode_dbg(x)                                                                                             \
    "Inode: {\n"                                                                                                 \
    "\ttype_and_permissions: %ud\n"                                                                              \
    "\tuser_id: %ud\n"                                                                                           \
    "\tlower_size: %ud\n"                                                                                        \
    "\tlast_access_time: %ud\n"                                                                                  \
    "\tcreation_time: %ud\n"                                                                                     \
    "\tlast_modification_time: %ud\n"                                                                            \
    "\tdeletion_time: %ud\n"                                                                                     \
    "\tgroup_id: %ud\n"                                                                                          \
    "\thard_link_count: %ud\n"                                                                                   \
    "\tdisk_sector_count: %ud\n"                                                                                 \
    "\tflags: %ud\n"                                                                                             \
    "\tos_specyfic_one: %ud\n"                                                                                   \
    "\tblock_pointers[0]: %ud\n"                                                                                 \
    "\tblock_pointers[1]: %ud\n"                                                                                 \
    "\tblock_pointers[2]: %ud\n"                                                                                 \
    "\tblock_pointers[3]: %ud\n"                                                                                 \
    "\tblock_pointers[4]: %ud\n"                                                                                 \
    "\tblock_pointers[5]: %ud\n"                                                                                 \
    "\tblock_pointers[6]: %ud\n"                                                                                 \
    "\tblock_pointers[7]: %ud\n"                                                                                 \
    "\tblock_pointers[8]: %ud\n"                                                                                 \
    "\tblock_pointers[9]: %ud\n"                                                                                 \
    "\tblock_pointers[10]: %ud\n"                                                                                \
    "\tblock_pointers[11]: %ud\n"                                                                                \
    "\tindirect_block_ponter: %ud\n"                                                                             \
    "\tdouble_indirect_block_pointer: %ud\n"                                                                     \
    "\ttriple_indirect_block_pointer: %ud\n"                                                                     \
    "\tgeneration_number: %ud\n"                                                                                 \
    "\textended_attribute_block: %ud\n"                                                                          \
    "\tupper_size_or_dir_acl: %ud\n"                                                                             \
    "\tfragment_block_address: %ud\n"                                                                            \
    "\tos_specyfic_two: %ud\n"                                                                                   \
    "}\n",                                                                                                       \
        x.type_and_permissions, x.user_id, x.lower_size, x.last_access_time, x.creation_time,                    \
        x.last_modification_time, x.deletion_time, x.group_id, x.hard_link_count, x.disk_sector_count, x.flags,  \
        x.os_specyfic_one, x.block_pointers[0], x.block_pointers[1], x.block_pointers[2], x.block_pointers[3],   \
        x.block_pointers[4], x.block_pointers[5], x.block_pointers[6], x.block_pointers[7], x.block_pointers[8], \
        x.block_pointers[9], x.block_pointers[10], x.block_pointers[11], x.block_pointers[Inode::IND_BLOCK],     \
        x.block_pointers[Inode::DIND_BLOCK], x.block_pointers[Inode::TIND_BLOCK], x.generation_number,           \
        x.extended_attribute_block, x.upper_size_or_dir_acl, x.fragment_block_address, x.os_specyfic_two

#define FS_IFMT   00170000
#define FS_IFSOCK 0140000
#define FS_IFLNK  0120000
#define FS_IFREG  0100000
#define FS_IFBLK  0060000
#define FS_IFDIR  0040000
#define FS_IFCHR  0020000
#define FS_IFIFO  0010000
#define FS_ISUID  0004000
#define FS_ISGID  0002000
#define FS_ISVTX  0001000

#define FS_INODE_FIFO(m)         (((m) & FS_IFMT) == FS_IFIFO)
#define FS_INODE_CHAR_DEVICE(m)  (((m) & FS_IFMT) == FS_IFCHR)
#define FS_INODE_DIRECTORY(m)    (((m) & FS_IFMT) == FS_IFDIR)
#define FS_INODE_BLOCK_DEVICE(m) (((m) & FS_IFMT) == FS_IFBLK)
#define FS_INODE_REGULAR_FILE(m) (((m) & FS_IFMT) == FS_IFREG)
#define FS_INODE_SYMBOIC_LINK(m) (((m) & FS_IFMT) == FS_IFLNK)
#define FS_INODE_SOCKET(m)       (((m) & FS_IFMT) == FS_IFSOCK)

#define FS_PERMISSION_OTHER_EXECUTE 0x001
#define FS_PERMISSION_OTHER_WRITE   0x002
#define FS_PERMISSION_OTHER_READ    0x004
#define FS_PERMISSION_GROUP_EXECUTE 0x008
#define FS_PERMISSION_GROUP_WRITE   0x010
#define FS_PERMISSION_GROUP_READ    0x020
#define FS_PERMISSION_USER_EXECUTE  0x040
#define FS_PERMISSION_USER_WRITE    0x080
#define FS_PERMISSION_USER_READ     0x100
#define FS_PERMISSION_STICKY        0x200
#define FS_PERMISSION_SET_GROUP_ID  0x400
#define FS_PERMISSION_SET_USER_ID   0x800

#define FS_INODE_FLAGS_SECURE_DELETION         0x1
#define FS_INODE_FLAGS_KEEP_COPY_ON_DELETE     0x2
#define FS_INODE_FLAGS_FILE_COMPRESSION        0x4
#define FS_INODE_FLAGS_SYNCHRONOUS_UPDATES     0x8
#define FS_INODE_FLAGS_IMMUTABLE_FILE          0x10
#define FS_INODE_FLAGS_APPEND_ONLY             0x20
#define FS_INODE_FLAGS_DUMP_NOT_INCLUDED       0x40
#define FS_INODE_FLAGS_DONT_UPDATE_LAST_ACCESS 0x80
#define FS_INODE_FLAGS_HASH_INDEXED_DIRECTORY  0x10000
#define FS_INODE_FLAGS_AFS_DIRECTORY           0x20000
#define FS_INODE_FALGS_JOURNAL_FILE_DATA       0x40000

struct DirectoryEntry {
    u32  inode;
    u16  total_entry_size;
    u8   lower_name_length;
    u8   upper_name_length_or_type;
    char name_data[]; /* Not NULL-terminated */

    std::string_view name(Filesystem* fs);
} __attribute__((packed));

#define DirectoryEntry_dbg(fs, x)                                                                                  \
    "DirectoryEntry: {\n"                                                                                          \
    "\tinode: %ud\n"                                                                                               \
    "\ttotal_entry_size: %ud\n"                                                                                    \
    "\tlower_name_length: %ud\n"                                                                                   \
    u"\tupper_name_length_or_type: %ud\n"                                                                          \
    "\tname: %.*s\n"                                                                                               \
    "}\n",                                                                                                         \
        x.inode, x.total_entry_size, x.lower_name_length, x.upper_name_length_or_type,                             \
        x.lower_name_length +                                                                                      \
            ((fs.has_required_feature(RequiredFeatures::DirectoryType)) ? 0 : (x.upper_name_length_or_type << 8)), \
        x.name

class InodeIterator
{
  public:
    using Block = std::span<u8>;

    using iterator_category = std::input_iterator_tag;
    using value_type        = Block;
    using diffrence_type    = std::ptrdiff_t;
    using pointer           = Block*;
    using reference         = Block&;

  private:
    Filesystem* fs;
    Inode&      inode;
    isize       counter            = 0;
    usize       indirect_blk_count = 0;
    u8*         buffer;
    Block       current;

  public:
    /* The buffer size must be >= fs->block_size */
    InodeIterator(Filesystem* fs, Inode& inode, u8* buffer) : fs(fs), inode(inode), buffer(buffer)
    {
        this->increment();
    }

    /* The buffer can be modified between ++ operations */
    inline InodeIterator& operator++()
    {
        this->increment();
        return *this;
    }

    constexpr Block& operator*() { return this->current; }
    constexpr Block* operator->() { return &this->current; }

    inline bool operator==(const InodeIterator& other) const { return this->counter == other.counter; }

    inline bool operator!=(const InodeIterator& other) const { return !(*this == other); }

    InodeIterator begin() const { return InodeIterator(this->fs, this->inode, this->buffer); }
    InodeIterator end() const { return InodeIterator(this->inode); }

  private:
    explicit InodeIterator(Inode& inode) : fs(NULL), inode(inode), counter(-1), buffer(NULL) {}
    void increment();
    u32  get_current_block_pointer();
    u16  get_required_buffer_size();
};

class DirInodeIterator
{
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = DirectoryEntry*; /* Will always be inside this->buffer */
    using diffrence_type    = std::ptrdiff_t;
    using pointer           = DirectoryEntry**;
    using reference         = DirectoryEntry*&;

  private:
    Filesystem*     fs;
    Inode&          inode;
    u8*             buffer;
    DirectoryEntry* current              = NULL;
    u64             current_block_offset = 0;
    isize           counter              = 0;
    InodeIterator   iter;

  public:
    /* The buffer size must be >= fs->block_size */
    DirInodeIterator(Filesystem* fs, Inode& inode, u8* buffer)
        : fs(fs), inode(inode), buffer(buffer), iter(InodeIterator(fs, inode, buffer))
    {
        this->increment();
    }

    inline DirInodeIterator& operator++()
    {
        this->increment();
        return *this;
    }

    constexpr DirectoryEntry*& operator*() { return this->current; }
    constexpr DirectoryEntry** operator->() { return &this->current; }

    inline bool operator==(const DirInodeIterator& other) const { return this->counter == other.counter; }

    inline bool operator!=(const DirInodeIterator& other) const { return !(*this == other); }

    DirInodeIterator begin() const { return DirInodeIterator(this->fs, this->inode, this->buffer); }
    DirInodeIterator end() const { return DirInodeIterator(this->inode, this->iter); }

  private:
    DirInodeIterator(Inode& inode, const InodeIterator& iter)
        : fs(NULL), inode(inode), buffer(NULL), counter(-1), iter(iter.end())
    {
    }
    void increment();
};
