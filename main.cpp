#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <cmath>

#include "config.h"
#include "structures.h"

using namespace std;

unsigned char buff_grp[4096];
unsigned char buff[4096];
int indev;
int BLKSIZE;

void invalid_args() {
    cout << "Wrong arguments" << endl;
}

void read_sb() {
    if(lseek(indev,1024,0) < 0) {
        perror("lseek");
        exit(-1);
    }

    if(read(indev,(char *)&sb, sizeof(sb)) == 0) {
        perror("read");
        exit(-1);
    }

    if(sb.s_magic != EXT2_SUPER_MAGIC) {
        cout << "It's not an Ext2 file system..." << sb.s_magic << endl;
        exit(-1);
    }


//    cout << "Superblock info -----------" << endl
//    << "\tInodes count: " << sb.s_inodes_count << endl
//    << "\tBlocks count: " << sb.s_blocks_count << endl
//    << "\tFree blocks count: " << sb.s_free_blocks_count << endl
//    << "\tFree inodes count: " << sb.s_free_inodes_count << endl
//    << "\tBlock size: " << (1024 <<  sb.s_log_block_size) << endl
//    << "\tFirst inode: " << sb.s_first_ino << endl
//    << "\tMagic: " << sb.s_magic << endl
//    << "\tInode size: " << sb.s_inode_size << endl
//    << "\tInodes per group: " << sb.s_inodes_per_group << endl
//    << "\tBlocks per group: " << sb.s_blocks_per_group << endl
//    << "\tFirst data block: " << sb.s_first_data_block << endl;
}

void read_gdt() {
    BLKSIZE = 1024 << sb.s_log_block_size;
    if(lseek(indev, (sb.s_first_data_block + 1) * BLKSIZE, 0) < 0) {
        perror("lseek");
        exit(-1);
    }

    if(read(indev, buff_grp, BLKSIZE) < 0) {
        perror("read");
        exit(-1);
    }
}

void get_inode(int inode_num, struct inode *in) {
    struct group_desc gd;
    uint64_t group, index, pos;
    group = (inode_num - 1) / sb.s_inodes_per_group;
    memset((void *)&gd, 0, sizeof(gd));
    memcpy((void *)&gd, buff_grp + (group * (sizeof(gd))), sizeof(gd));
    index = (inode_num - 1) % sb.s_inodes_per_group;
    pos = ((uint64_t)gd.bg_inode_table) * BLKSIZE + (index * sb.s_inode_size);
    pread64(indev, in, sb.s_inode_size, pos);
}

void read_iblock(struct inode *in, int blk_num) {
    uint32_t pos;
    int link_in_blk = BLKSIZE / sizeof(uint32_t);
    if (blk_num < 12) {
        pos = ((uint32_t) in->i_block[blk_num]) * BLKSIZE;
        pread64(indev, buff, BLKSIZE, pos);
    }
    else if (blk_num <= 11 + link_in_blk) {
        auto *pos1 = new uint32_t;

        pos  = (in->i_block[12]) * BLKSIZE + (blk_num - 12) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pread64(indev, buff, BLKSIZE, *pos1 * BLKSIZE);
        delete pos1;
    }
    else if (blk_num <= 11 + pow(link_in_blk, 2)) {
        int b_ind = blk_num - (12 + link_in_blk);
        auto *pos1 = new uint32_t;

        pos  = (in->i_block[13]) * BLKSIZE + (b_ind / link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pos = *pos1 * BLKSIZE + (b_ind % link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pread64(indev, buff, BLKSIZE, *pos1 * BLKSIZE);
        delete pos1;
    }
    else {
        int b_ind = blk_num - (12 + pow(link_in_blk, 2));
        auto *pos1 = new uint32_t;

        pos  = (in->i_block[14]) * BLKSIZE + (b_ind / pow(link_in_blk, 2)) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pos = *pos1 * BLKSIZE + ((b_ind % (link_in_blk * link_in_blk)) / link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pos = *pos1 * BLKSIZE + (b_ind % link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pread64(indev, buff, BLKSIZE, *pos1 * BLKSIZE);
        delete pos1;
    }
}

void change_bitmap(uint32_t block_num, uint32_t* bitmap) {
    char *p = new char;
    memcpy(p, bitmap + block_num/8, sizeof(char));
    *p &= ~(1<<(block_num%8));
    memcpy(bitmap + block_num/8, p, sizeof(char));
    delete p;
}

void hide_iblock(struct inode *in, int blk_num, uint32_t* bitmap) {
    uint32_t pos;
    int link_in_blk = BLKSIZE / sizeof(uint32_t);
    if (blk_num < 12) {
        pos = in->i_block[blk_num];
    }
    else if (blk_num <= 11 + link_in_blk) {
        auto *pos1 = new uint32_t;

        pos  = (in->i_block[12]) * BLKSIZE + (blk_num - 12) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);
        pos = *pos1;
        delete pos1;
    }
    else if (blk_num <= 11 + pow(link_in_blk, 2)) {
        int b_ind = blk_num - (12 + link_in_blk);
        auto *pos1 = new uint32_t;

        pos  = (in->i_block[13]) * BLKSIZE + (b_ind / link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pos = *pos1 * BLKSIZE + (b_ind % link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);
        pos = *pos1;
        delete pos1;
    }
    else {
        int b_ind = blk_num - (12 + pow(link_in_blk, 2));
        auto *pos1 = new uint32_t;

        pos  = (in->i_block[14]) * BLKSIZE + (b_ind / pow(link_in_blk, 2)) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pos = *pos1 * BLKSIZE + ((b_ind % (link_in_blk * link_in_blk)) / link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);

        pos = *pos1 * BLKSIZE + (b_ind % link_in_blk) * sizeof(uint32_t);
        pread64(indev, pos1, sizeof(uint32_t), pos);
        pos = *pos1;
        delete pos1;
    }
    change_bitmap(pos, bitmap);
}

void show_entry(const string& name) {
    int rec_len = 24;
    struct dir_entry_2 dent;
    for(;;) {
        memcpy((void *)&dent, (buff + rec_len), sizeof(dent));
        cout << dent.name << ' ';
        if(!dent.name_len) break;
        rec_len += dent.rec_len;
    }
    cout << endl;
}

void get_root_dentry() {
    struct inode in;
    get_inode(EXT2_ROOT_INO, &in);
    read_iblock(&in, 0);

    //show_entry("/");
}

int get_i_num(char *name) {
    int rec_len = 0;
    struct dir_entry_2 dent;
    for(;;) {
        memcpy((void *)&dent, (buff + rec_len), sizeof(dent));
        if(!dent.name_len) return -1;
        if(!memcmp(dent.name, name, dent.name_len)) break;
        rec_len += dent.rec_len;
    }
    return dent.inode;
}







void create_disk (char name[], int size) {
    string ddCom = "dd if=/dev/zero of=" + string(name) + ".bin bs=512 count=" + to_string(size * 2000);
    char ddComChar[ddCom.length() + 1];
    strcpy(ddComChar, ddCom.c_str());
    system(ddComChar);

    string mkCom = "mkfs " + string(name) + ".bin";
    char mkChar[mkCom.length()+1];
    strcpy(mkChar, mkCom.c_str());
    system(mkChar);
    cout << mkCom <<endl;
}

void show_file(const char full_path[], bool is_dir) {
    struct inode in;
    unsigned char buff1[EXT2_NAME_LEN];
    static int i = 1;
    int n, i_num, outf, type;
    const char *image_path;
    string file_path;
    for(int i = 0; ; i++) {
        if(full_path[i] == ':') {
            file_path = string(full_path);
            image_path = new char[i];
            memcpy((void *) image_path, full_path, i);
            file_path = file_path.substr(i+1);
          break;
        }
    }

    if(file_path[0] != '/') {
        perror("slash");
        exit(-1);
    }

    indev = open(image_path, O_RDONLY);
    delete image_path;

    if(indev < 0) {
        perror("open");
        exit(-1);
    }

    read_sb();
    read_gdt();
    get_root_dentry();

    if (file_path == "/" && is_dir) {
        close(indev);
        show_entry("/");
        return;
    }

    int slash_count = 0;
    for (char ch : file_path) {
        if (ch == '/') {
            slash_count++;
        }
    }

    for(; slash_count != 0; slash_count--) {
        memset(buff1,0, sizeof(buff1));
        for(n = 0 ; n < EXT2_NAME_LEN; n++, i++) {
            buff1[n] = file_path[i];
            if(buff1[n] == '/') {
                i++;
            }
        }
        buff1[n] = ' ';
        i_num = get_i_num(reinterpret_cast<char *>(buff1));
        if(i_num == -1) {
            cout << "Haven't this file!" << endl;
            break;
        }
        get_inode(i_num, &in);

        read_iblock(&in, 0);

        if(slash_count == 1) {
            type = ((in.i_mode & 0xF000) >> 12);
            switch(type) {

                case(0x04) :
                    if(is_dir) {
                        show_entry(string(reinterpret_cast<const char *>(buff1)));
                    } else cout << "It's a directory" << endl;
                    break;

                case(0x08) :
                    if(!is_dir) {
                        int last_b = in.i_size/BLKSIZE;
                        for(int i = 0; i <= last_b; i++) {
                            read_iblock(&in, i);
                            cout << buff;
                        }
                    } else cout << "It's a simple file" << endl;
                    break;

                case(0x06) :
                    cout << "It's a block device file" << endl;
                    break;

                case(0x02) :
                    cout << "It's a character device file" << endl;
                    break;

                default:
                    cout << "It's unknown type" << endl;
                    break;
            }
        }
    }
    close(indev);
}

void get_file(const string& path_from, const string& path_to) {
    struct inode in;
    unsigned char buff1[EXT2_NAME_LEN];
    static int i = 1;
    int n, i_num, outf, type;

    string destination_path;
    string image_path;
    string file_path;

    int dout_pos = path_from.find(':');
    file_path = path_from.substr(dout_pos + 1);
    image_path = path_from.substr(0, dout_pos);

    if(file_path[0] != '/') {
        perror("slash");
        exit(-1);
    }

    indev = open(image_path.c_str(),O_RDONLY);

    if(indev < 0) {
        perror("open");
        exit(-1);
    }

    read_sb();
    read_gdt();
    get_root_dentry();

    int slash_count = 0;
    for (char ch : file_path) {
        if (ch == '/') {
            slash_count++;
        }
    }

    for(; slash_count != 0; slash_count--) {
        memset(buff1,0, sizeof(buff1));
        for(n = 0 ; n < EXT2_NAME_LEN; n++, i++) {
            buff1[n] = file_path[i];
            if(buff1[n] == '/') {
                i++;
                break;
            }
        }
        buff1[n] = ' ';
        i_num = get_i_num(reinterpret_cast<char *>(buff1));
        if(i_num == -1) {
            cout << "Have not this file!" << endl;
            break;
        }
        get_inode(i_num, &in);

        read_iblock(&in, 0);

        if(slash_count == 1) {
            type = ((in.i_mode & 0xF000) >> 12);
            switch(type) {

                case(0x04) :
                    cout << "It's a directory" << endl;
                    break;

                case(0x08) : {
                    outf = open(path_to.c_str(), O_CREAT | O_RDWR, 0600);
                    int last_b = in.i_size / BLKSIZE;
                    for (int i = 0; i < last_b; i++) {
                        read_iblock(&in, i);
                        write(outf, buff, BLKSIZE);
                    }
                    read_iblock(&in, last_b);
                    write(outf, buff, in.i_size % BLKSIZE);
                    close(outf);
                    break;
                }

                case(0x06) :
                    cout << "It's a block device file" << endl;
                    break;

                case(0x02) :
                    cout << "It's a character device file" << endl;
                    break;

                default:
                    cout << "unknown type" << endl;
                    break;
            }
        }
    }
    close(indev);

}

void put_file(const string& path_from, const string& path_to) {
    cout << "Putting files haven't work yet" << endl;
}

void cp_file(const string& path1, const string& path2) {
    if(path1.find(':') < path1.length()) {
        get_file(path1, path2);
    } else {
        put_file(path2, path1);
    }
}

void hide_inode(int& inode_num) {
    uint64_t group = (inode_num - 1) / sb.s_inodes_per_group;
    struct group_desc gd;
    memset((void *)&gd, 0, sizeof(gd));
    memcpy((void *)&gd, buff_grp + (group * (sizeof(gd))), sizeof(gd));
    struct inode inode;
    get_inode(inode_num, &inode);

    int last_b = inode.i_size/BLKSIZE;
    for(int i = 0; i <= last_b; i++) {
        hide_iblock(&inode, i, &gd.bg_block_bitmap);
        cout << buff;
    }

    change_bitmap(inode_num, &gd.bg_inode_bitmap);

    uint64_t index, pos;
    index = (inode_num - 1) % sb.s_inodes_per_group;
    pos = ((uint64_t)gd.bg_inode_table) * BLKSIZE + (index * sb.s_inode_size) + 4;

    auto *p_links = new uint32_t;
    *p_links = 0;
    memcpy(&pos, &p_links, sizeof(uint32_t));
    delete p_links;
}

void delete_file(const string& full_path) {
    struct inode in;
    unsigned char buff1[EXT2_NAME_LEN];
    static int i = 1;
    int n, i_num, outf, type;

    string image_path;
    string file_path;

    int dout_pos = full_path.find(':');
    file_path = full_path.substr(dout_pos + 1);
    image_path = full_path.substr(0, dout_pos);

    if(file_path[0] != '/') {
        perror("slash");
        exit(-1);
    }

    indev = open(image_path.c_str(), O_RDWR);

    if(indev < 0) {
        perror("open");
        exit(-1);
    }

    read_sb();
    read_gdt();
    get_root_dentry();

    int slash_count = 0;
    for (char ch : file_path) {
        if (ch == '/') {
            slash_count++;
        }
    }

    for(; slash_count != 0; slash_count--) {
        memset(buff1,0, sizeof(buff1));
        for(n = 0 ; n < EXT2_NAME_LEN; n++, i++) {
            buff1[n] = file_path[i];
            if(buff1[n] == '/') {
                i++;
                break;
            }
        }
        buff1[n] = ' ';
        i_num = get_i_num(reinterpret_cast<char *>(buff1));
        if(i_num == -1) {
            cout << "Haven't this file!" << endl;
            break;
        }
        get_inode(i_num, &in);

        read_iblock(&in, 0);

        if(slash_count == 1) {
            type = ((in.i_mode & 0xF000) >> 12);

            if(type & 0x08) {
                hide_inode(i_num);
            }
            else cout << "It's not a simple file" << endl;
        }
    }
    close(indev);
}

int main(int argc, char* argv[]) {
    switch (argc) {
        case 3: {
            if (strcmp(argv[1], "cat") == 0) {
                show_file(argv[2], false);
            } else if (strcmp(argv[1], "ls") == 0) {
                show_file(argv[2], true);
            } else if (strcmp(argv[1], "rm") == 0) {
                delete_file(argv[2]);
            } else invalid_args();
            break;
        }
        case 4: {
            if (strcmp(argv[1], "cp") == 0) {
                cp_file(argv[2], argv[3]);
            } else if (strcmp(argv[1], "create") == 0) {
                create_disk(argv[2], atoi(argv[3]));
            } else invalid_args();
            break;
        }
        default:
            invalid_args();
            break;
    }
    return 0;
}