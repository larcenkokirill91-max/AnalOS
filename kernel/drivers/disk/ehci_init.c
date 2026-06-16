#include <kernel.h>
void ehci_init(uint32_t pci_bar0, uint32_t pci_bus, uint32_t pci_slot, uint32_t pci_func) {
    cap_regs = (ehci_cap_regs_t*)map_pci_mmio(pci_bar0, 0x100);
    op_regs = (ehci_op_regs_t*)((uint32_t)cap_regs + cap_regs->cap_length);

    ehci_bios_handoff(pci_bus, pci_slot, pci_func);

    /* Стопорим контроллер */
    op_regs->usb_cmd &= ~EHCI_CMD_RUN;
    while (!(op_regs->usb_sts & EHCI_STS_HALTED));

    /* Сброс контроллера */
    op_regs->usb_cmd |= EHCI_CMD_RESET;
    while (op_regs->usb_cmd & EHCI_CMD_RESET);

    op_regs->ctrl_ds_segment = 0;
    op_regs->usb_intr = 0; /* Опрос производим вручную */

    /* Создаем базовый пустой корневой элемент */
    async_qh_root = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);
    async_qh_root->horizontal_link = virtual_to_physical(async_qh_root) | EHCI_PTR_QH;
    async_qh_root->endpoint_char = (1 << 15); /* Head of async list flag */
    async_qh_root->endpoint_caps = 0;
    async_qh_root->current_qtd = 0;
    async_qh_root->overlay.next_qtd = EHCI_PTR_TERMINATE;
    async_qh_root->overlay.token = 0;

    /* Инициализируем долгоживущие QH для Bulk-команд mass storage */
    msc_qh_in = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);
    msc_qh_out = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);

    op_regs->async_list_addr = virtual_to_physical(async_qh_root);
    op_regs->config_flag = 1; /* Перенаправляем порты с компаньон-контроллеров */
    op_regs->usb_cmd |= EHCI_CMD_ASE | EHCI_CMD_RUN;

    while (!(op_regs->usb_sts & EHCI_STS_ASS));

    /* Поиск подключенных девайсов и сброс портов */
    uint32_t ports = cap_regs->hcs_params & 0xF;
    for (uint32_t i = 0; i < ports; i++) {
        uint32_t status = op_regs->port_sc[i];
        if (status & 1) { 
            /* Выполняем процедуру сброса */
            op_regs->port_sc[i] = (op_regs->port_sc[i] & ~0x2A) | (1 << 8); 
            delay_ms(50);
            op_regs->port_sc[i] &= ~(1 << 8);
            delay_ms(20);

            /* Проверяем, что порт успешно перешел в состояние Enabled */
            int timeout = 100;
            while (timeout--) {
                status = op_regs->port_sc[i];
                if ((status & (1 << 2)) && (status & 1)) break; 
                delay_ms(2);
            }
        }
    }

    /* Назначаем устройству USB-адрес №1 */
    msc_device_addr = 0; 
    usb_setup_packet_t set_addr = {0x00, 0x05, 1, 0, 0}; 
    ehci_submit_control(&set_addr, 0, 0);
    delay_ms(10);
    msc_device_addr = 1;

    /* Запрашиваем Configuration Descriptor */
    uint8_t* cfg_desc = (uint8_t*)kmalloc_aligned(256, 32);
    usb_setup_packet_t get_cfg = {0x80, 0x06, 0x0200, 0, 256};
    ehci_submit_control(&get_cfg, cfg_desc, 256);

    uint32_t ptr = 9; 
    uint32_t total_len = cfg_desc[2] | (cfg_desc[3] << 8);
    
    /* Парсим дескрипторы для поиска Bulk конечных точек */
    while (ptr < total_len) {
        uint8_t len = cfg_desc[ptr];
        uint8_t type = cfg_desc[ptr + 1];
        if (type == 0x05) { 
            uint8_t addr = cfg_desc[ptr + 2];
            uint8_t attr = cfg_desc[ptr + 3];
            uint16_t max_p = cfg_desc[ptr + 4] | (cfg_desc[ptr + 5] << 8);
            if ((attr & 0x03) == 0x02) { 
                if (addr & 0x80) {
                    msc_ep_in = addr & 0x0F;
                    msc_ep_in_max_packet = max_p;
                } else {
                    msc_ep_out = addr & 0x0F;
                    msc_ep_out_max_packet = max_p;
                }
            }
        }
        ptr += len;
    }

    /* Выбираем конфигурацию №1 */
    usb_setup_packet_t set_cfg = {0x00, 0x09, 1, 0, 0};
    ehci_submit_control(&set_cfg, 0, 0);
    delay_ms(10);
}
