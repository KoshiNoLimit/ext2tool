#ifndef UNTITLED1_STRUCTURES_H
//include <stdint.h>
#include <cstdint>


struct super_block {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuit;
    uint16_t s_def_resgid;

    uint32_t s_first_ino;
    uint16_t s_inode_size;
} sb;

struct group_desc {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;//////////была [7]
    uint32_t bg_reserved;
};

struct inode {
    uint16_t i_mode;			// Format of the file, and access rights
    uint16_t i_uid;			// User id associated with file
    uint32_t i_size;			// Size of file in bytes
    uint32_t i_atime;			// Last access time, POSIX
    uint32_t i_ctime;			// Creation time
    uint32_t i_mtime;			// Last modified time
    uint32_t i_dtime;			// Deletion time
    uint16_t i_gid;			// POSIX group access
    uint16_t i_links_count;	// How many links
    uint32_t i_blocks;		// # of 512-bytes blocks reserved to contain the data
    uint32_t i_flags;			// EXT2 behavior
    uint32_t i_osdl;			// OS dependent value
    uint32_t i_block[15];		// Block pointers. Last 3 are indirect
    uint32_t i_generation;	// File version
    uint32_t i_file_acl;		// Block # containing extended attributes
    uint32_t i_dir_acl;
    uint32_t i_faddr;			// Location of file fragment
    uint32_t i_osd2[3];
};

struct dir_entry_2 {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[EXT2_NAME_LEN];
};

#endif //UNTITLED1_STRUCTURES_H
