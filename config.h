#ifndef UNTITLED1_CONFIG_H
#define UNTITLED1_CONFIG_H


#define EXT2_NDIR_BLOCKS 12
#define EXT2_IND_BLOCK     EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS   (EXT2_TIND_BLOCK + 1)
#define EXT2_NAME_LEN 255

#define EXT2_BAD_INO 1 /* Bad blocks inode */
#define EXT2_ROOT_INO 2 /* Root inode */
#define EXT2_ACL_IDX_INO 3 /* ACL inode */
#define EXT2_ACL_DATA_INO 4 /* ACL inode */
#define EXT2_BOOT_LOADER_INO 5 /* Boot loader inode */
#define EXT2_UNDEL_DIR_INO 6 /* Undelete directory inode */
#define EXT2_SUPER_MAGIC 61267

#endif //UNTITLED1_CONFIG_H
