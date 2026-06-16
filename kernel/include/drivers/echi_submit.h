#pragma once
#ifndef ECHI_SUBMIT
#define ECHI_SUBMIT
static int ehci_submit_bulk(ehci_qh_t* qh, uint8_t ep, uint8_t pid, void* buffer, uint32_t len);
static int ehci_submit_control(usb_setup_packet_t* setup, void* buffer, uint32_t len);
#ifndef
