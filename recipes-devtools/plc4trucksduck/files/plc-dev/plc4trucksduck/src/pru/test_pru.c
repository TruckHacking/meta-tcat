#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PRU_NO 0
#define BBB_GPIO_PIN 88
#define UART_NUM 4
#define HOST_INT ((uint32_t) 1 << 30)
#define TO_ARM_HOST 16
#define FROM_ARM_HOST 17
#define CHAN_NAME "rpmsg-pru"
#define CHAN_DESC "Channel 30"
#define CHAN_PORT 30
#define CYCLES_PER_HALF_BIT 10400
#define CHECKS_TILL_BUS_IDLE 20
#define CHECKS_TILL_MSG_FINISHED 11
#define J1708
#define PRU_RPMSG_SUCCESS 0
#define RPMSG_MESSAGE_SIZE 256
#define MAX_PAYLOAD_LEN 256

uint8_t transmitBuf[RPMSG_MESSAGE_SIZE];
uint8_t receiveBuf[MAX_PAYLOAD_LEN];
uint8_t uart_tx_buf[1024];
int uart_tx_len = 0;

struct pru_rpmsg_transport { int dummy; };

void pruInit(void* transport) {}
void uartWrite(uint8_t* buf, uint16_t len) {
    for (int i=0; i<len; i++) {
        uart_tx_buf[uart_tx_len++] = buf[i];
    }
}
uint8_t uartGetC(uint8_t* byte) {
    if (uart_tx_len > 0) {
        *byte = uart_tx_buf[uart_tx_len - 1]; // echo last byte
        return 1;
    }
    return 0;
}
void __delay_cycles(uint32_t c) {}
uint8_t isBusIdle(int c) { return 1; }
int pru_rpmsg_send(void* t, uint16_t d, uint16_t s, void* b, uint16_t l) { return 0; }
uint16_t receiveRemainingMessage(uint8_t* b) { return 0; }

int rpmsg_idx = 0;
int pru_rpmsg_receive(void* transport, uint16_t* src, uint16_t* dst, uint8_t* buf, uint16_t* len) {
    if (rpmsg_idx == 0) {
        rpmsg_idx++;
        return PRU_RPMSG_SUCCESS; // pass the init loop with a dummy message
    } else if (rpmsg_idx == 1) {
        buf[0] = 0x0a;
        buf[1] = 0x00;
        buf[2] = 0xf6; // The real checksum
        *len = 3;
        rpmsg_idx++;
        return PRU_RPMSG_SUCCESS;
    } else if (rpmsg_idx == 2) {
        if (uart_tx_len != 3 || uart_tx_buf[0] != 0x0a || uart_tx_buf[1] != 0x00 || uart_tx_buf[2] != 0xf6) {
            printf("FAIL: PRU modified the buffer! Expected 0a00f6, got ");
            for (int i=0; i<uart_tx_len; i++) printf("%02x", uart_tx_buf[i]);
            printf("\n");
            exit(1);
        }
        printf("PASS: PRU transmitted 0a00f6 verbatim on the UART wire, preventing recalculation.\n");
        exit(0);
    }
    return 1; 
}

// STUB OUT ALL PRU HEADERS
#define _PRU_CFG_H_
#define _PRU_INTC_H_
#define _PRU_CTRL_H_
#define _PRU_RPMSG_H_
#define _INTC_MAP_0_H_
#define COMMON_H

// Pull in the actual PRU C file directly, which will use our mocked functions above
#include "plc4trucksduck.c"
