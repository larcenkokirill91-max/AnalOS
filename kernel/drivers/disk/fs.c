#include <kernel.h>
#include <kernel.h>
#include <kernel.h>
void unpack_update_archive(void) {
    unsigned short sb_buffer[256];
    unsigned short archive_buffer[256];
    ata_read_sector(45, sb_buffer); 
    ata_read_sector(2000, archive_buffer);
    struct superblock* sb = (struct superblock*)sb_buffer;
    struct arhive_header* arch = (struct arhive_header*)archive_buffer;
    int free_index = -1;
    for (int i = 0; i < 10; i++) {
        if (sb->files[i].is_used == 1) {
            continue;
        }
        free_index = i;
        break;
    }
    if (free_index != -1) {
        unsigned int data_sector = 46 + free_index; 
        sb->files[free_index].is_used = 1;
        sb->files[free_index].start_sector = data_sector;
        sb->files[free_index].file_size = arch->size;
        strcpy(sb->files[free_index].name, arch->name);
        ata_write_sector(45, sb_buffer);
        ata_write_sector(data_sector, archive_buffer);
    }
}