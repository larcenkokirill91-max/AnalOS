#include "ehci.h"
int ehci_msc_read10(uint32_t lba, uint16_t count, void* buffer) {
    usb_cbw_t* cbw = (usb_cbw_t*)kmalloc_aligned(sizeof(usb_cbw_t), 32);
    usb_csw_t* csw = (usb_csw_t*)kmalloc_aligned(sizeof(usb_csw_t), 32);

    cbw->signature = 0x43425355; 
    cbw->tag = cbw_tag_counter++;
    cbw->transfer_length = count * 512;
    cbw->flags = 0x80; /* Направление: устройства -> хост */
    cbw->lun = 0;
    cbw->cb_length = 10;

    /* Команда SCSI Read(10) внутри CBW */
    uint8_t* cmd_buf = (uint8_t*)&cbw->cb;
    for (int i = 0; i < 16; i++) cmd_buf[i] = 0;
    cmd_buf[0] = 0x28; 
    cmd_buf[2] = (lba >> 24) & 0xFF;
    cmd_buf[3] = (lba >> 16) & 0xFF;
    cmd_buf[4] = (lba >> 8) & 0xFF;
    cmd_buf[5] = lba & 0xFF;
    cmd_buf[7] = (count >> 8) & 0xFF;
    cmd_buf[8] = count & 0xFF;

    /* Фаза 1: Отправка командного блока (CBW) */
    if (ehci_submit_bulk(msc_qh_out, msc_ep_out, QTD_PID_OUT, cbw, 31) < 0) return -1;
    
    /* Фаза 2: Чтение полезных секторов в буфер */
    if (ehci_submit_bulk(msc_qh_in, msc_ep_in, QTD_PID_IN, buffer, count * 512) < 0) return -2;
    
    /* Фаза 3: Прием блока статуса (CSW) */
    if (ehci_submit_bulk(msc_qh_in, msc_ep_in, QTD_PID_IN, csw, 13) < 0) return -3;

    /* Валидация сигнатур выполнения */
    if (csw->signature != 0x53425355 || csw->status != 0) return -4;

    return 0;
}
