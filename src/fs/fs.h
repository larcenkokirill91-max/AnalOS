#ifndef FS_H
#define FS_H
struct disk_file {
    char name[32];
    unsigned int start_sector;
    unsigned int file_size;
    unsigned int is_used;
} __attribute__((packed));
struct superblock {
    struct disk_file files[10];
    unsigned int magic;
} __attribute__((packed));
#endif