#include <kernel.h>
#include <kernel.h> // Для твоих io_wait, outb, inb

static volatile ehci_cap_regs_t* cap_regs = 0;
static volatile ehci_op_regs_t* op_regs = 0;

static ehci_qh_t* async_qh_root = 0;
static ehci_qh_t* msc_qh_in = 0;
static ehci_qh_t* msc_qh_out = 0;

static uint8_t msc_device_addr = 1;
static uint8_t msc_ep_out = 0;
static uint8_t msc_ep_in = 0;
static uint16_t msc_ep_out_max_packet = 512;
static uint16_t msc_ep_in_max_packet = 512;
static uint32_t cbw_tag_counter = 0xDEADC0DE;

/* Глобальные переменные для хранения координат найденного контроллера */
static uint32_t usb_bar0 = 0;
static uint32_t usb_bus = 0;
static uint32_t usb_slot = 0;
static uint32_t usb_func = 0;

/* === Вспомогательные системные функции ядра === */

void* map_pci_mmio(uint32_t physical_address, uint32_t size) {
    return (void*)physical_address;
}

uint32_t virtual_to_physical(void* virtual_address) {
    return (uint32_t)virtual_address;
}

static uint32_t ehci_free_mem_ptr = 0x00500000;
void* kmalloc_aligned(uint32_t size, uint32_t alignment) {
    if (ehci_free_mem_ptr % alignment != 0) {
        ehci_free_mem_ptr += (alignment - (ehci_free_mem_ptr % alignment));
    }
    void* res = (void*)ehci_free_mem_ptr;
    ehci_free_mem_ptr += size;
    return res;
}

void delay_ms(uint32_t ms) {
    for (uint32_t m = 0; m < ms; m++) {
        for (volatile int i = 0; i < 600; i++) {
            io_wait();
        }
    }
}

/* === Чтение и запись конфигурации шины PCI === */

uint32_t pci_read_config(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t size) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    __asm__ __volatile__("outl %0, %1" : : "a"(address), "Nd"((uint16_t)0xCF8));
    uint32_t val;
    __asm__ __volatile__("inl %1, %0" : "=a"(val) : "Nd"((uint16_t)0xCFC));
    if (size == 1) return (val >> ((offset & 3) * 8)) & 0xFF;
    if (size == 2) return (val >> ((offset & 2) * 8)) & 0xFFFF;
    return val;
}

void pci_write_config(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val, uint32_t size) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    __asm__ __volatile__("outl %0, %1" : : "a"(address), "Nd"((uint16_t)0xCF8));
    __asm__ __volatile__("outl %0, %1" : : "a"(val), "Nd"((uint16_t)0xCFC));
}

/* Сканер PCI для автоматического поиска EHCI контроллера */
int pci_find_ehci(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            for (uint32_t func = 0; func < 8; func++) {
                uint32_t reg0 = pci_read_config(bus, slot, func, 0x00, 4);
                if ((reg0 & 0xFFFF) == 0xFFFF) continue;

                uint32_t reg8 = pci_read_config(bus, slot, func, 0x08, 4);
                uint8_t base_class = (reg8 >> 24) & 0xFF;
                uint8_t subclass   = (reg8 >> 16) & 0xFF;
                uint8_t interface  = (reg8 >> 8) & 0xFF;

                if (base_class == 0x0C && subclass == 0x03 && interface == 0x20) {
                    usb_bar0 = pci_read_config(bus, slot, func, 0x10, 4) & 0xFFFFFFF0;
                    usb_bus = bus;
                    usb_slot = slot;
                    usb_func = func;
                    return 1; // Успешно нашли контроллер USB 2.0 EHCI
                }
            }
        }
    }
    return 0; // На материнке нет EHCI или отключен
}

/* === Логика очередей выполнения (Транзакции шины USB) === */

static void ehci_bios_handoff(uint32_t bus, uint32_t slot, uint32_t func) {
    uint32_t hccparams = cap_regs->hcc_params;
    uint32_t eecp = (hccparams >> 8) & 0xFF;
    if (eecp < 0x40) return;

    uint32_t cap = pci_read_config(bus, slot, func, eecp, 4);
    if ((cap & 0xFF) != 1) return; 

    if (cap & (1 << 16)) { 
        pci_write_config(bus, slot, func, eecp, (cap | (1 << 24)), 4);
        int timeout = 1000;
        while (timeout--) {
            cap = pci_read_config(bus, slot, func, eecp, 4);
            if (!(cap & (1 << 16)) && (cap & (1 << 24))) break;
            delay_ms(1);
        }
    }
    uint32_t cap_legacy_ctl = pci_read_config(bus, slot, func, eecp + 4, 4);
    pci_write_config(bus, slot, func, eecp + 4, cap_legacy_ctl & 0x1F0000, 4);
}

static int ehci_wait_qtd(volatile ehci_qtd_t* qtd) {
    int timeout = 10000000;
    while ((qtd->token & QTD_STATUS_ACTIVE) && timeout > 0) {
        __asm__ volatile("pause");
        timeout--;
    }
    if (timeout <= 0) return -1;
    if (qtd->token & 0x7C) return -2;
    return 0;
}

static void ehci_setup_qtd_buffers(ehci_qtd_t* qtd, void* buffer, uint32_t len) {
    if (len == 0 || buffer == 0) {
        qtd->buffer = 0;
        return;
    }
    qtd->buffer = virtual_to_physical(buffer);
}

static int ehci_submit_bulk(ehci_qh_t* qh, uint8_t ep, uint8_t pid, void* buffer, uint32_t len) {
    ehci_qtd_t* qtd = (ehci_qtd_t*)kmalloc_aligned(sizeof(ehci_qtd_t), 32);
    uint32_t max_packet = (pid == QTD_PID_IN) ? msc_ep_in_max_packet : msc_ep_out_max_packet;
    if (max_packet == 0) max_packet = 512;

    qh->horizontal_link = virtual_to_physical(async_qh_root) | EHCI_PTR_QH;
    qh->endpoint_char = msc_device_addr | (ep << 8) | (2 << 12) | (max_packet << 16); 
    qh->endpoint_caps = (1 << 30); 

    qtd->next_qtd = EHCI_PTR_TERMINATE;
    qtd->alt_next_qtd = EHCI_PTR_TERMINATE;
    qtd->token = (len << 16) | (pid << 8) | QTD_STATUS_ACTIVE | QTD_IOC;

    ehci_setup_qtd_buffers(qtd, buffer, len);

    qh->overlay.next_qtd = virtual_to_physical(qtd);
    qh->overlay.token = 0; 

    uint32_t root_phys = virtual_to_physical(async_qh_root);
    async_qh_root->horizontal_link = virtual_to_physical(qh) | EHCI_PTR_QH;

    int res = ehci_wait_qtd(qtd);
    async_qh_root->horizontal_link = root_phys | EHCI_PTR_QH;
    return res;
}

static int ehci_submit_control(usb_setup_packet_t* setup, void* buffer, uint32_t len) {
    ehci_qh_t* qh = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);
    ehci_qtd_t* qtd_setup = (ehci_qtd_t*)kmalloc_aligned(sizeof(ehci_qtd_t), 32);
    ehci_qtd_t* qtd_data = 0;
    ehci_qtd_t* qtd_status = (ehci_qtd_t*)kmalloc_aligned(sizeof(ehci_qtd_t), 32);

    qh->horizontal_link = virtual_to_physical(async_qh_root) | EHCI_PTR_QH;
    qh->endpoint_char = (msc_device_addr) | (2 << 12) | (64 << 16) | (1 << 27); 
    qh->endpoint_caps = (1 << 30);
    qh->current_qtd = 0;

    qtd_setup->next_qtd = EHCI_PTR_TERMINATE;
    qtd_setup->alt_next_qtd = EHCI_PTR_TERMINATE;
    qtd_setup->token = (8 << 16) | (QTD_PID_SETUP << 8) | QTD_STATUS_ACTIVE;
    qtd_setup->buffer = virtual_to_physical(setup);

    ehci_qtd_t* prev = qtd_setup;

    if (len > 0) {
        qtd_data = (ehci_qtd_t*)kmalloc_aligned(sizeof(ehci_qtd_t), 32);
        qtd_data->next_qtd = EHCI_PTR_TERMINATE;
        qtd_data->alt_next_qtd = EHCI_PTR_TERMINATE;
        qtd_data->token = (len << 16) | (QTD_PID_IN << 8) | QTD_STATUS_ACTIVE | QTD_TOGGLE; 
        ehci_setup_qtd_buffers(qtd_data, buffer, len);
        prev->next_qtd = virtual_to_physical(qtd_data);
        prev = qtd_data;
    }

    qtd_status->next_qtd = EHCI_PTR_TERMINATE;
    qtd_status->alt_next_qtd = EHCI_PTR_TERMINATE;
    uint8_t status_pid = (len > 0) ? QTD_PID_OUT : QTD_PID_IN;
    qtd_status->token = (0 << 16) | (status_pid << 8) | QTD_STATUS_ACTIVE | QTD_IOC | QTD_TOGGLE;
    qtd_status->buffer = 0;
    prev->next_qtd = virtual_to_physical(qtd_status);

    qh->overlay.next_qtd = virtual_to_physical(qtd_setup);
    qh->overlay.token = 0;

    uint32_t root_phys = virtual_to_physical(async_qh_root);
    async_qh_root->horizontal_link = virtual_to_physical(qh) | EHCI_PTR_QH;

    int res = ehci_wait_qtd(qtd_status);
    async_qh_root->horizontal_link = root_phys | EHCI_PTR_QH;
    return res;
}

/* === Глобальная инициализация подсистемы USB === */

void ehci_init(uint32_t pci_bar0, uint32_t pci_bus, uint32_t pci_slot, uint32_t pci_func) {
    cap_regs = (ehci_cap_regs_t*)map_pci_mmio(pci_bar0, 0x100);
    op_regs = (ehci_op_regs_t*)((uint32_t)cap_regs + cap_regs->cap_length);

    ehci_bios_handoff(pci_bus, pci_slot, pci_func);

    op_regs->usb_cmd &= ~EHCI_CMD_RUN;
    while (!(op_regs->usb_sts & EHCI_STS_HALTED));

    op_regs->usb_cmd |= EHCI_CMD_RESET;
    while (op_regs->usb_cmd & EHCI_CMD_RESET);

    op_regs->ctrl_ds_segment = 0;
    op_regs->usb_intr = 0; 

    async_qh_root = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);
    async_qh_root->horizontal_link = virtual_to_physical(async_qh_root) | EHCI_PTR_QH;
    async_qh_root->endpoint_char = (1 << 15); 
    async_qh_root->endpoint_caps = 0;
    async_qh_root->current_qtd = 0;
    async_qh_root->overlay.next_qtd = EHCI_PTR_TERMINATE;
    async_qh_root->overlay.token = 0;

    msc_qh_in = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);
    msc_qh_out = (ehci_qh_t*)kmalloc_aligned(sizeof(ehci_qh_t), 32);

    op_regs->async_list_addr = virtual_to_physical(async_qh_root);
    op_regs->config_flag = 1; 
    op_regs->usb_cmd |= EHCI_CMD_ASE | EHCI_CMD_RUN;

    while (!(op_regs->usb_sts & EHCI_STS_ASS));

    /* Сброс порта, куда вставлена загрузочная флешка */
    op_regs->port_sc = (op_regs->port_sc & ~0x2A) | (1 << 8); 
    delay_ms(50);
    op_regs->port_sc &= ~(1 << 8);
    delay_ms(20);

    msc_device_addr = 0; 
    usb_setup_packet_t set_addr = {0x00, 0x05, 1, 0, 0}; 
    ehci_submit_control(&set_addr, 0, 0);
    delay_ms(10);
    msc_device_addr = 1;

    uint8_t* cfg_desc = (uint8_t*)kmalloc_aligned(256, 32);
    usb_setup_packet_t get_cfg = {0x80, 0x06, 0x0200, 0, 256};
    ehci_submit_control(&get_cfg, cfg_desc, 256);

    uint32_t ptr = 9; 
    uint32_t total_len = cfg_desc[2] | (cfg_desc[3] << 8);
    
    while (ptr < total_len) {
        uint8_t len = cfg_desc[ptr];
        uint8_t type = cfg_desc[ptr + 1];
        if (type == 0x05) { 
            uint8_t addr = cfg_desc[ptr + 2];
            uint8_t addr = cfg_desc[ptr + 2];
            uint8_t attr = cfg_desc[ptr + 3];
            uint16_t max_p = cfg_desc[ptr + 4] | (cfg_desc[ptr + 5] << 8);
            
            // Проверяем, что это Bulk-конечная точка (аттрибут 0x02)
            if ((attr & 0x03) == 0x02) { 
                if (addr & 0x80) { // Если выставлен 7-й бит, это направление IN (во внутренний буфер ОС)
                    msc_ep_in = addr & 0x0F;
                    msc_ep_in_max_packet = max_p;
                } else { // Иначе это направление OUT (запись на флешку)
                    msc_ep_out = addr & 0x0F;
                    msc_ep_out_max_packet = max_p;
                }
            }
        }
        ptr += len; // Переходим к следующему дескриптору в пакете конфигурации
    }

    // Выбираем конфигурацию №1 устройства для завершения логической инициализации
    usb_setup_packet_t set_cfg = {0x00, 0x09, 1, 0, 0};
    ehci_submit_control(&set_cfg, 0, 0);
    delay_ms(10);
}

/* Функция автозапуска подсистемы хранения для kernel.c */
int init_usb_storage(void) {
    if (pci_find_ehci()) {
        ehci_init(usb_bar0, usb_bus, usb_slot, usb_func);
        return 1; // Успешно нашли PCI устройство и запустили EHCI стек
    }
    return 0; // Контроллер USB 2.0 на шине PCI не обнаружен
}

/* === Протокол Bulk-Only Transport (BOT) чтения и записи секторов === */

int ehci_msc_read10(uint32_t lba, uint16_t count, void* buffer) {
    usb_cbw_t* cbw = (usb_cbw_t*)kmalloc_aligned(sizeof(usb_cbw_t), 32);
    usb_csw_t* csw = (usb_csw_t*)kmalloc_aligned(sizeof(usb_csw_t), 32);

    cbw->signature = 0x43425355; // Магическая сигнатура "USBC"
    cbw->tag = cbw_tag_counter++;
    cbw->transfer_length = count * 512;
    cbw->flags = 0x80; // Направление передачи данных: Флешка -> Процессор
    cbw->lun = 0;
    cbw->cb_length = 10;

    uint8_t* cmd_buf = (uint8_t*)&cbw->cb;
    for (int i = 0; i < 16; i++) cmd_buf[i] = 0;
    cmd_buf[0] = 0x28; // Команда SCSI Read(10)
    cmd_buf[2] = (lba >> 24) & 0xFF;
    cmd_buf[3] = (lba >> 16) & 0xFF;
    cmd_buf[4] = (lba >> 8) & 0xFF;
    cmd_buf[5] = lba & 0xFF;
    cmd_buf[7] = (count >> 8) & 0xFF;
    cmd_buf[8] = count & 0xFF;

    // Три фазыBOT-транзакции: Команда -> Данные -> Статус завершения
    if (ehci_submit_bulk(msc_qh_out, msc_ep_out, QTD_PID_OUT, cbw, 31) < 0) return -1;
    if (ehci_submit_bulk(msc_qh_in, msc_ep_in, QTD_PID_IN, buffer, count * 512) < 0) return -2;
    if (ehci_submit_bulk(msc_qh_in, msc_ep_in, QTD_PID_IN, csw, 13) < 0) return -3;

    // Проверяем валидность ответа "USBS" и статус операции (0 - успех)
    if (csw->signature != 0x53425355 || csw->status != 0) return -4;
    return 0;
}

int ehci_msc_write10(uint32_t lba, uint16_t count, const void* buffer) {
    usb_cbw_t* cbw = (usb_cbw_t*)kmalloc_aligned(sizeof(usb_cbw_t), 32);
    usb_csw_t* csw = (usb_csw_t*)kmalloc_aligned(sizeof(usb_csw_t), 32);

    cbw->signature = 0x43425355; // "USBC"
    cbw->tag = cbw_tag_counter++;
    cbw->transfer_length = count * 512;
    cbw->flags = 0x00; // Направление передачи данных: Процессор -> Флешка
    cbw->lun = 0;
    cbw->cb_length = 10;

    uint8_t* cmd_buf = (uint8_t*)&cbw->cb;
    for (int i = 0; i < 16; i++) cmd_buf[i] = 0;
    cmd_buf[0] = 0x2A; // Команда SCSI Write(10)
    cmd_buf[2] = (lba >> 24) & 0xFF;
    cmd_buf[3] = (lba >> 16) & 0xFF;
    cmd_buf[4] = (lba >> 8) & 0xFF;
    cmd_buf[5] = lba & 0xFF;
    cmd_buf[7] = (count >> 8) & 0xFF;
    cmd_buf[8] = count & 0xFF;

    // Зеркальные три фазы BOT-транзакции для аппаратной записи
    if (ehci_submit_bulk(msc_qh_out, msc_ep_out, QTD_PID_OUT, cbw, 31) < 0) return -1;
    if (ehci_submit_bulk(msc_qh_out, msc_ep_out, QTD_PID_OUT, (void*)buffer, count * 512) < 0) return -2;
    if (ehci_submit_bulk(msc_qh_in, msc_ep_in, QTD_PID_IN, csw, 13) < 0) return -3;

    if (csw->signature != 0x53425355 || csw->status != 0) return -4;
    return 0;
}
