#ifndef DISK_H
#define DISK_H

void ata_wait_ready();
void ata_send_lba(unsigned int lba);
void ata_read_sector(unsigned int lba, unsigned short* buffer);

#endif
