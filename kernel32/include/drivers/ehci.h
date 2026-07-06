#pragma once
#ifndef EHCI_H
#define EHCI_H

#include <stdint.h>

#define PCI_CLASS_SERIAL          0x0C
#define PCI_SUBCLASS_USB          0x03
#define PCI_PROGIF_EHCI           0x20

#define EHCI_CMD_RUN              (1 << 0)
#define EHCI_CMD_RESET            (1 << 1)
#define EHCI_CMD_ASE              (1 << 5)
#define EHCI_STS_HALTED           (1 << 12)
#define EHCI_STS_ASS              (1 << 5)

#define EHCI_PTR_TERMINATE        (1 << 0)
#define EHCI_PTR_QH               (2 << 1)

#define QTD_PID_SETUP             0
#define QTD_PID_OUT               1
#define QTD_PID_IN                2

#define QTD_STATUS_ACTIVE         (1 << 7)
#define QTD_IOC                   (1 << 15)
#define QTD_TOGGLE                (1 << 31)

typedef struct {
    uint8_t  cap_length;
    uint8_t  reserved;
    uint16_t hci_version;
    uint32_t hcs_params;
    uint32_t hcc_params;
} __attribute__((packed)) ehci_cap_regs_t;

typedef struct {
    uint32_t usb_cmd;
    uint32_t usb_sts;
    uint32_t usb_intr;
    uint32_t fr_index;
    uint32_t ctrl_ds_segment;
    uint32_t periodic_list_base;
    uint32_t async_list_addr;
    uint32_t reserved;
    uint32_t config_flag;
    uint32_t port_sc[16];
} __attribute__((packed)) ehci_op_regs_t;

typedef struct ehci_qtd {
    uint32_t next_qtd;
    uint32_t alt_next_qtd;
    uint32_t token;
    uint32_t buffer[5];
} __attribute__((packed)) __attribute__((aligned(32))) ehci_qtd_t;

typedef struct ehci_qh {
    uint32_t horizontal_link;
    uint32_t endpoint_char;
    uint32_t endpoint_caps;
    uint32_t current_qtd;
    ehci_qtd_t overlay;
} __attribute__((packed)) __attribute__((aligned(32))) ehci_qh_t;

typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) usb_setup_packet_t;

typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t transfer_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t cb_length;
    uint8_t cb[16];
} __attribute__((packed)) usb_cbw_t;

typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_residue;
    uint8_t status;
} __attribute__((packed)) usb_csw_t;

/* Главные функции интерфейса для твоей ОС */
void ehci_init(uint32_t pci_bar0, uint32_t pci_bus, uint32_t pci_slot, uint32_t pci_func);
int ehci_msc_read10(uint32_t lba, uint16_t count, void* buffer);
int ehci_msc_write10(uint32_t lba, uint16_t count, const void* buffer);

#endif
