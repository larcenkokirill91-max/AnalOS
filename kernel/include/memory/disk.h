#pragma once

#ifndef DISK_H
#define DISK_H

static inline void outsl(int port, const void *addr, int cnt);
static inline void insl(int port, void *addr, int cnt);
void ata_delay(void);
void ata_wait_ready(void);
void ata_send_lba(unsigned int lba);
void ata_write_sector(unsigned int lba, unsigned short* buffer);
void ata_read_sector(unsigned int lba, unsigned short* buffer);
void install_to_disk(void);

#endif
