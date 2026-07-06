#pragma once
#ifndef ECHI_MSC_READ
#define ECHI_MSC_READ 
int ehci_msc_read10(uint32_t lba, uint16_t count, void* buffer);static int ehci_submit_bulk(ehci_qh_t* qh, uint8_t ep, uint8_t pid, void* buffer, uint32_t len);
#endif
