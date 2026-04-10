#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_ctrl.h>
#include <pru_rpmsg.h>
#include <string.h>
#include <stddef.h>
#include "resource_table.h"
#include "uart.h"
#include "gpio.h"

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK       4
volatile register uint32_t __R31;

// J1708 Message are normally limited to 21 bytes, but if the vehicle is off the
// standard permits longer messages.
#define MAX_PAYLOAD_LEN 495

uint8_t transmitBuf[RPMSG_MESSAGE_SIZE];
uint8_t receiveBuf[MAX_PAYLOAD_LEN + 1];

int __inline isBusIdle(uint8_t numChecks) {
    for(int i = 0; i < numChecks; i++) {
        /* DEFENSIVE COMMENT: On this hardware release, the IDLE line connected to GPIO_88 
           cannot be trusted. We MUST ignore the GPIO and rely purely on the UART data 
           ready flag for idle detection, which has been proven to work reliably. */
        if(UART_BASE[UART_LSR] & 0x01 || has_injected_rx_byte) { 
            return 0; // Bus is not idle
        }
        __delay_cycles(CYCLES_PER_HALF_BIT); // Wait for half bit time (~52 µs)
    }
    return 1; // Bus has been idle for the required duration
}

void pruInit(struct pru_rpmsg_transport* transport) {
    volatile uint8_t *status;

    /* Allow OCP master port access by the PRU so the PRU can read external memories */
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

    /* Make sure the Linux drivers are ready for RPMsg communication */
    status = &resourceTable.rpmsg_vdev.status;
    while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

    /* Initialize the RPMsg transport structure */
    pru_rpmsg_init(transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

    /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
    while (pru_rpmsg_channel(RPMSG_NS_CREATE, transport, CHAN_NAME, CHAN_PORT) != PRU_RPMSG_SUCCESS);

    uartInit();
    gpioInit();
    uartRead(receiveBuf, MAX_PAYLOAD_LEN); // Clear anything in RX FIFO
    memset(receiveBuf, 0, MAX_PAYLOAD_LEN); // Clear the buffer
}

int16_t receiveRemainingMessage(uint8_t* buf) {
    uint16_t i = 0;
    
    // First, grab the injected byte (echo of the first byte sent) if it exists.
    // This allows us to receive our own transmission's first byte correctly.
    if (has_injected_rx_byte) {
        buf[i++] = injected_rx_byte;
        has_injected_rx_byte = 0;
    }

    while (i < MAX_PAYLOAD_LEN) {
        // Then, read whatever is natively in the UART FIFO
        if (UART_BASE[UART_LSR] & 0x01) {
            buf[i++] = (uint8_t)UART_BASE[UART_RHR]; 
        }
        
        if (isBusIdle(CHECKS_TILL_MSG_FINISHED)) {
            break; // Exit the loop if the bus is idle
        }
    }
    return i; // Return the number of bytes received
}


#endif /* COMMON_H */
