/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/fs/ext4.h
 * ext4 filesystem for linuxab server
 */

#ifndef _LINUXAB_EXT4_H
#define _LINUXAB_EXT4_H

#include "types.h"

/* Magic */
#define EXT4_SUPER_MAGIC        0xEF53

/* Block size */
#define EXT4_MIN_BLOCK_SIZE     1024
#define EXT4_MAX_BLOCK_SIZE     65536
#define EXT4_MIN_BLOCK_LOG_SIZE 10

/* Inode size */
#define EXT4_MIN_INODE_SIZE     128
#define EXT4_MAX_INODE_SIZE     4096

/* Feature compat */
#define EXT4_FEATURE_COMPAT_DIR_PREALLOC        0x0001
#define EXT4_FEATURE_COMPAT_IMAGIC_INODES     0x0002
#define EXT4_FEATURE_COMPAT_HAS_JOURNAL         0x0004
#define EXT4_FEATURE_COMPAT_EXT_ATTR          0x0008
#define EXT4_FEATURE_COMPAT_RESIZE_INODE      0x0010
#define EXT4_FEATURE_COMPAT_DIR_INDEX         0x0020
#define EXT4_FEATURE_COMPAT_LAZY_BG           0x0040
#define EXT4_FEATURE_COMPAT_EXCLUDE_INODE     0x0080
#define EXT4_FEATURE_COMPAT_EXCLUDE_BITMAP    0x0100
#define EXT4_FEATURE_COMPAT_SPARSE_SUPER2     0x0200
#define EXT4_FEATURE_COMPAT_FAST_COMMIT       0x0400
#define EXT4_FEATURE_COMPAT_ORPHAN_PRESENT    0x0800

/* Feature incompat */
#define EXT4_FEATURE_INCOMPAT_COMPRESSION       0x0001
#define EXT4_FEATURE_INCOMPAT_FILETYPE          0x0002
#define EXT4_FEATURE_INCOMPAT_RECOVER           0x0004
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV       0x0008
#define EXT4_FEATURE_INCOMPAT_META_BG           0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS           0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT             0x0080
#define EXT4_FEATURE_INCOMPAT_MMP               0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG           0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE          0x0400
#define EXT4_FEATURE_INCOMPAT_DIRDATA           0x1000
#define EXT4_FEATURE_INCOMPAT_CSUM_SEED       0x2000
#define EXT4_FEATURE_INCOMPAT_LARGEDIR        0x4000
#define EXT4_FEATURE_INCOMPAT_INLINE_DATA     0x8000
#define EXT4_FEATURE_INCOMPAT_ENCRYPT           0x10000
#define EXT4_FEATURE_INCOMPAT_CASEFOLD        0x20000

/* Feature ro compat */
#define EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER     0x0001
#define EXT4_FEATURE_RO_COMPAT_LARGE_FILE       0x0002
#define EXT4_FEATURE_RO_COMPAT_BTREE_DIR        0x0004
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE        0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM         0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK        0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE      0x0040
#define EXT4_FEATURE_RO_COMPAT_HAS_SNAPSHOT     0x0080
#define EXT4_FEATURE_RO_COMPAT_QUOTA          0x0100
#define EXT4_FEATURE_RO_COMPAT_BIGALLOC       0x0200
#define EXT4_FEATURE_RO_COMPAT_METADATA_CSUM  0x0400
#define EXT4_FEATURE_RO_COMPAT_REPLICA          0x0800
#define EXT4_FEATURE_RO_COMPAT_READONLY         0x1000
#define EXT4_FEATURE_RO_COMPAT_PROJECT          0x2000
#define EXT4_FEATURE_RO_COMPAT_VERITY           0x8000
#define EXT4_FEATURE_RO_COMPAT_ORPHAN_PRESENT   0x10000

/* Superblock */
struct ext4_super_block {
    uint32_t    s_inodes_count;
    uint32_t    s_blocks_count_lo;
    uint32_t    s_r_blocks_count_lo;
    uint32_t    s_free_blocks_count_lo;
    uint32_t    s_free_inodes_count;
    uint32_t    s_first_data_block;
    uint32_t    s_log_block_size;
    uint32_t    s_log_cluster_size;
    uint32_t    s_blocks_per_group;
    uint32_t    s_clusters_per_group;
    uint32_t    s_inodes_per_group;
    uint32_t    s_mtime;
    uint32_t    s_wtime;
    uint16_t    s_mnt_count;
    uint16_t    s_max_mnt_count;
    uint16_t    s_magic;
    uint16_t    s_state;
    uint16_t    s_errors;
    uint16_t    s_minor_rev_level;
    uint32_t    s_lastcheck;
    uint32_t    s_checkinterval;
    uint32_t    s_creator_os;
    uint32_t    s_rev_level;
    uint16_t    s_def_resuid;
    uint16_t    s_def_resgid;
    uint32_t    s_first_ino;
    uint16_t    s_inode_size;
    uint16_t    s_block_group_nr;
    uint32_t    s_feature_compat;
    uint32_t    s_feature_incompat;
    uint32_t    s_feature_ro_compat;
    uint8_t     s_uuid[16];
    char        s_volume_name[16];
    char        s_last_mounted[64];
    uint32_t    s_algorithm_usage_bitmap;
    uint8_t     s_prealloc_blocks;
    uint8_t     s_prealloc_dir_blocks;
    uint16_t    s_reserved_gdt_blocks;
    uint8_t     s_journal_uuid[16];
    uint32_t    s_journal_inum;
    uint32_t    s_journal_dev;
    uint32_t    s_last_orphan;
    uint32_t    s_hash_seed[4];
    uint8_t     s_def_hash_version;
    uint8_t     s_jnl_backup_type;
    uint16_t    s_desc_size;
    uint32_t    s_default_mount_opts;
    uint32_t    s_first_meta_bg;
    uint32_t    s_mkfs_time;
    uint32_t    s_jnl_blocks[17];
    uint32_t    s_blocks_count_hi;
    uint32_t    s_r_blocks_count_hi;
    uint32_t    s_free_blocks_count_hi;
    uint16_t    s_min_extra_isize;
    uint16_t    s_want_extra_isize;
    uint32_t    s_flags;
    uint16_t    s_raid_stride;
    uint16_t    s_mmp_update_interval;
    uint64_t    s_mmp_block;
    uint32_t    s_raid_stripe_width;
    uint8_t     s_log_groups_per_flex;
    uint8_t     s_checksum_type;
    uint8_t     s_encryption_algos[4];
    uint8_t     s_checksum_seed;
    uint8_t     s_reserved_pad;
    uint64_t    s_kbytes_written;
    uint32_t    s_snapshot_inum;
    uint32_t    s_snapshot_id;
    uint64_t    s_snapshot_r_blocks_count;
    uint32_t    s_snapshot_list;
    uint32_t    s_error_count;
    uint32_t    s_first_error_time;
    uint32_t    s_first_error_ino;
    uint64_t    s_first_error_block;
    uint8_t     s_first_error_func[32];
    uint32_t    s_first_error_line;
    uint32_t    s_last_error_time;
    uint32_t    s_last_error_ino;
    uint64_t    s_last_error_block;
    uint8_t     s_last_error_func[32];
    uint32_t    s_last_error_line;
    uint8_t     s_mount_opts[64];
    uint32_t    s_usr_quota_inum;
    uint32_t    s_grp_quota_inum;
    uint32_t    s_overhead_blocks;
    uint32_t    s_backup_bgs[2];
    uint8_t     s_encrypt_algos[4];
    uint8_t     s_encrypt_pw_salt[16];
    uint32_t    s_lpf_ino;
    uint32_t    s_prj_quota_inum;
    uint32_t    s_checksum_seed;
    uint8_t     s_wtime_hi;
    uint8_t     s_mtime_hi;
    uint8_t     s_mkfs_time_hi;
    uint8_t     s_lastcheck_hi;
    uint8_t     s_first_error_time_hi;
    uint8_t     s_last_error_time_hi;
    uint8_t     s_first_error_errcode;
    uint8_t     s_last_error_errcode;
    uint16_t    s_reserved[29];
    uint32_t    s_checksum;
} __attribute__((packed));

/* Group descriptor (64-bit) */
struct ext4_group_desc {
    uint32_t    bg_block_bitmap_lo;
    uint32_t    bg_inode_bitmap_lo;
    uint32_t    bg_inode_table_lo;
    uint16_t    bg_free_blocks_count_lo;
    uint16_t    bg_free_inodes_count_lo;
    uint16_t    bg_used_dirs_count_lo;
    uint16_t    bg_flags;
    uint32_t    bg_exclude_bitmap_lo;
    uint16_t    bg_block_bitmap_csum_lo;
    uint16_t    bg_inode_bitmap_csum_lo;
    uint16_t    bg_itable_unused_lo;
    uint16_t    bg_checksum;
    uint32_t    bg_block_bitmap_hi;
    uint32_t    bg_inode_bitmap_hi;
    uint32_t    bg_inode_table_hi;
    uint16_t    bg_free_blocks_count_hi;
    uint16_t    bg_free_inodes_count_hi;
    uint16_t    bg_used_dirs_count_hi;
    uint16_t    bg_itable_unused_hi;
    uint32_t    bg_exclude_bitmap_hi;
    uint16_t    bg_block_bitmap_csum_hi;
    uint16_t    bg_inode_bitmap_csum_hi;
    uint32_t    bg_reserved;
} __attribute__((packed));

/* Inode */
struct ext4_inode {
    uint16_t    i_mode;
    uint16_t    i_uid;
    uint32_t    i_size_lo;
    uint32_t    i_atime;
    uint32_t    i_ctime;
    uint32_t    i_mtime;
    uint32_t    i_dtime;
    uint16_t    i_gid;
    uint16_t    i_links_count;
    uint32_t    i_blocks_lo;
    uint32_t    i_flags;
    union {
        struct {
            uint32_t    l_i_version;
        } linux1;
        struct {
            uint32_t    h_i_translator;
        } hurd1;
        struct {
            uint32_t    m_i_reserved1;
        } masix1;
    } osd1;
    uint32_t    i_block[15];
    uint32_t    i_generation;
    uint32_t    i_file_acl_lo;
    uint32_t    i_size_high;
    uint32_t    i_obso_faddr;
    union {
        struct {
            uint16_t    l_i_blocks_high;
            uint16_t    l_i_file_acl_high;
            uint16_t    l_i_uid_high;
            uint16_t    l_i_gid_high;
            uint16_t    l_i_checksum_lo;
            uint16_t    l_i_reserved;
        } linux2;
        struct {
            uint16_t    h_i_reserved1;
            uint16_t    h_i_mode_high;
            uint16_t    h_i_uid_high;
            uint16_t    h_i_gid_high;
            uint32_t    h_i_author;
        } hurd2;
        struct {
            uint16_t    m_i_reserved1;
            uint16_t    m_i_file_acl_high;
            uint32_t    m_i_reserved2[2];
        } masix2;
    } osd2;
    uint16_t    i_extra_isize;
    uint16_t    i_checksum_hi;
    uint32_t    i_ctime_extra;
    uint32_t    i_mtime_extra;
    uint32_t    i_atime_extra;
    uint32_t    i_crtime;
    uint32_t    i_crtime_extra;
    uint32_t    i_version_hi;
    uint32_t    i_projid;
} __attribute__((packed));

/* Inode mode */
#define EXT4_S_IXOTH    0x0001
#define EXT4_S_IWOTH    0x0002
#define EXT4_S_IROTH    0x0004
#define EXT4_S_IXGRP    0x0008
#define EXT4_S_IWGRP    0x0010
#define EXT4_S_IRGRP    0x0020
#define EXT4_S_IXUSR    0x0040
#define EXT4_S_IWUSR    0x0080
#define EXT4_S_IRUSR    0x0100
#define EXT4_S_ISVTX    0x0200
#define EXT4_S_ISGID    0x0400
#define EXT4_S_ISUID    0x0800
#define EXT4_S_IFIFO    0x1000
#define EXT4_S_IFCHR    0x2000
#define EXT4_S_IFDIR    0x4000
#define EXT4_S_IFBLK    0x6000
#define EXT4_S_IFREG    0x8000
#define EXT4_S_IFLNK    0xA000
#define EXT4_S_IFSOCK   0xC000

#define EXT4_S_ISLNK(m)     (((m) & EXT4_S_IFMT) == EXT4_S_IFLNK)
#define EXT4_S_ISREG(m)     (((m) & EXT4_S_IFMT) == EXT4_S_IFREG)
#define EXT4_S_ISDIR(m)     (((m) & EXT4_S_IFMT) == EXT4_S_IFDIR)
#define EXT4_S_ISCHR(m)     (((m) & EXT4_S_IFMT) == EXT4_S_IFCHR)
#define EXT4_S_ISBLK(m)     (((m) & EXT4_S_IFMT) == EXT4_S_IFBLK)
#define EXT4_S_ISFIFO(m)    (((m) & EXT4_S_IFMT) == EXT4_S_IFIFO)
#define EXT4_S_ISSOCK(m)    (((m) & EXT4_S_IFMT) == EXT4_S_IFSOCK)

/* Extent header */
struct ext4_extent_header {
    uint16_t    eh_magic;
    uint16_t    eh_entries;
    uint16_t    eh_max;
    uint16_t    eh_depth;
    uint32_t    eh_generation;
};

#define EXT4_EXT_MAGIC      0xF30A

/* Extent */
struct ext4_extent {
    uint32_t    ee_block;
    uint16_t    ee_len;
    uint16_t    ee_start_hi;
    uint32_t    ee_start_lo;
};

/* Extent index */
struct ext4_extent_idx {
    uint32_t    ei_block;
    uint32_t    ei_leaf_lo;
    uint16_t    ei_leaf_hi;
    uint16_t    ei_unused;
};

/* Directory entry */
struct ext4_dir_entry {
    uint32_t    inode;
    uint16_t    rec_len;
    uint16_t    name_len;
    char        name[EXT4_NAME_LEN];
};

#define EXT4_NAME_LEN       255

struct ext4_dir_entry_2 {
    uint32_t    inode;
    uint16_t    rec_len;
    uint8_t     name_len;
    uint8_t     file_type;
    char        name[EXT4_NAME_LEN];
};

/* File types */
#define EXT4_FT_UNKNOWN     0
#define EXT4_FT_REG_FILE    1
#define EXT4_FT_DIR         2
#define EXT4_FT_CHRDEV      3
#define EXT4_FT_BLKDEV      4
#define EXT4_FT_FIFO        5
#define EXT4_FT_SOCK        6
#define EXT4_FT_SYMLINK     7
#define EXT4_FT_MAX         8

/* Journal */
#define EXT4_JOURNAL_MAGIC  0xC03B3998

/* Mount flags */
#define EXT4_MOUNT_DEFAULTS       0
#define EXT4_MOUNT_JOURNAL_DATA   (1 << 0)
#define EXT4_MOUNT_ORDERED_DATA   (1 << 1)
#define EXT4_MOUNT_WRITEBACK_DATA (1 << 2)
#define EXT4_MOUNT_NO_MBCACHE     (1 << 3)
#define EXT4_MOUNT_NO_UID32       (1 << 4)
#define EXT4_MOUNT_DEBUG          (1 << 5)
#define EXT4_MOUNT_ERRORS_CONT    (1 << 6)
#define EXT4_MOUNT_ERRORS_RO      (1 << 7)
#define EXT4_MOUNT_ERRORS_PANIC   (1 << 8)
#define EXT4_MOUNT_DATA_FLAGS     0x00000003

/* Super operations */
struct ext4_sb_info {
    struct ext4_super_block *s_es;
    struct buffer_head *s_sbh;
    uint32_t s_desc_size;
    uint32_t s_inodes_per_block;
    uint32_t s_blocks_per_group;
    uint32_t s_clusters_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_itb_per_group;
    uint32_t s_gdb_count;
    uint32_t s_desc_per_block;
    uint32_t s_groups_count;
    uint32_t s_cluster_bits;
    uint32_t s_cluster_ratio;
    uint64_t s_blocks_count;
    uint64_t s_r_blocks_count;
    uint64_t s_free_blocks_count;
    uint64_t s_overhead;
    uint32_t s_inode_size;
    uint32_t s_first_ino;
    uint32_t s_sectors_per_block;
    uint32_t s_blocksize;
    uint32_t s_blocksize_bits;
    uint64_t s_inode_readahead_blks;
    uint32_t s_mount_flags;
    uint32_t s_def_mount_opt;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint32_t s_readahead_blks;
    uint32_t s_commit_interval;
    uint32_t s_min_batch_time;
    uint32_t s_max_batch_time;
    uint32_t s_max_dir_size_kb;
    uint32_t s_extent_max_zeroout_kb;
    atomic_t s_ext_lazy_init;
    atomic_t s_freeblocks_counter;
    atomic_t s_freeinodes_counter;
    atomic_t s_dirs_counter;
    atomic_t s_dirtyclusters_counter;
    struct percpu_counter s_sdblocks_counter;
    struct percpu_counter s_freeblocks_counter_pc;
    struct percpu_counter s_freeinodes_counter_pc;
    struct percpu_counter s_dirs_counter_pc;
    struct percpu_counter s_dirtyclusters_counter_pc;
    char s_es_buf[4096];
};

int ext4_mount(struct block_device *bdev, struct ext4_sb_info **sbi);
void ext4_umount(struct ext4_sb_info *sbi);
int ext4_read_inode(struct ext4_sb_info *sbi, uint32_t ino, struct ext4_inode *inode);
int ext4_write_inode(struct ext4_sb_info *sbi, uint32_t ino, struct ext4_inode *inode);
int ext4_read_block(struct ext4_sb_info *sbi, uint64_t block, void *buf);
int ext4_write_block(struct ext4_sb_info *sbi, uint64_t block, const void *buf);
int ext4_lookup(struct ext4_sb_info *sbi, uint32_t dir_ino, const char *name, uint32_t *ino);
int ext4_create(struct ext4_sb_info *sbi, uint32_t dir_ino, const char *name, uint16_t mode);
int ext4_mkdir(struct ext4_sb_info *sbi, uint32_t dir_ino, const char *name, uint16_t mode);
int ext4_unlink(struct ext4_sb_info *sbi, uint32_t dir_ino, const char *name);
int ext4_read_extent(struct ext4_sb_info *sbi, struct ext4_inode *inode,
                     uint64_t lblock, uint64_t *pblock, uint32_t *len);

#endif /* _LINUXAB_EXT4_H */