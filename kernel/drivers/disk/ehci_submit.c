#include <kernel.h>
static int ehci_submit_bulk(ehci_qh_t* qh, uint8_t ep, uint8_t pid, void* buffer, uint32_t len) {
    ehci_qtd_t* qtd = (ehci_qtd_t*)kmalloc_aligned(sizeof(ehci_qtd_t), 32);

    uint32_t max_packet = (pid == QTD_PID_IN) ? msc_ep_in_max_packet : msc_ep_out_max_packet;
    if (max_packet == 0) max_packet = 512;

    /* Конфигурируем характеристики конечной точки в QH */
    qh->horizontal_link = virtual_to_physical(async_qh_root) | EHCI_PTR_QH;
    qh->endpoint_char = msc_device_addr | (ep << 8) | (2 << 12) | (max_packet << 16); 
    qh->endpoint_caps = (1 << 30); 

    qtd->next_qtd = EHCI_PTR_TERMINATE;
    qtd->alt_next_qtd = EHCI_PTR_TERMINATE;
    qtd->token = (len << 16) | (pid << 8) | QTD_STATUS_ACTIVE | QTD_IOC;

    ehci_setup_qtd_buffers(qtd, buffer, len);

    /* Активируем дескриптор в очереди */
    qh->overlay.next_qtd = virtual_to_physical(qtd);
    qh->overlay.token = 0; 

    /* Включаем QH в асинхронный цикл выполнения */
    uint32_t root_phys = virtual_to_physical(async_qh_root);
    async_qh_root->horizontal_link = virtual_to_physical(qh) | EHCI_PTR_QH;

    int res = ehci_wait_qtd(qtd);

    /* Убираем QH из активного расписания */
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

    /* 1. Setup Stage (Всегда DATA0, бит QTD_TOGGLE не взводится) */
    qtd_setup->next_qtd = EHCI_PTR_TERMINATE;
    qtd_setup->alt_next_qtd = EHCI_PTR_TERMINATE;
    qtd_setup->token = (8 << 16) | (QTD_PID_SETUP << 8) | QTD_STATUS_ACTIVE;
    qtd_setup->buffer = virtual_to_physical(setup);

    ehci_qtd_t* prev = qtd_setup;

    /* 2. Data Stage (При наличии данных стартует с DATA1 -> взводим QTD_TOGGLE) */
    if (len > 0) {
        qtd_data = (ehci_qtd_t*)kmalloc_aligned(sizeof(ehci_qtd_t), 32);
        qtd_data->next_qtd = EHCI_PTR_TERMINATE;
        qtd_data->alt_next_qtd = EHCI_PTR_TERMINATE;
        qtd_data->token = (len << 16) | (QTD_PID_IN << 8) | QTD_STATUS_ACTIVE | QTD_TOGGLE; 
        ehci_setup_qtd_buffers(qtd_data, buffer, len);
        prev->next_qtd = virtual_to_physical(qtd_data);
        prev = qtd_data;
    }

    /* 3. Status Stage (Всегда использует DATA1 -> взводим QTD_TOGGLE) */
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