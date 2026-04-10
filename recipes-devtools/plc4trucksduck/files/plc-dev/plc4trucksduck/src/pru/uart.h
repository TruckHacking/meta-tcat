#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "clock.h"

#if (UART_NUM == 4)
    #define UART_CLKCTRL_OFFSET 0x78
    #define UART_BASE_ADDR 0x481A8000
#elif (UART_NUM == 2)
    #define UART_CLKCTRL_OFFSET 0x70
    #define UART_BASE_ADDR 0x48024000
#else
    #define UART_CLKCTRL_OFFSET 0x74
    #define UART_BASE_ADDR 0x481A6000
#endif

#define UART_BASE ((volatile uint16_t *)(UART_BASE_ADDR))

#define UART_SYSC (0x54 / 2)
#define UART_SYSS (0x58 / 2)
#define UART_LCR  (0x0C / 2)
#define UART_EFR  (0x08 / 2)
#define UART_MCR  (0x10 / 2)
#define UART_FCR  (0x08 / 2)
#define UART_TLR  (0x1C / 2)
#define UART_SCR  (0x40 / 2)
#define UART_MDR1 (0x20 / 2)
#define UART_IER  (0x04 / 2)
#define UART_DLL  (0x00 / 2)
#define UART_DLH  (0x04 / 2)
#define UART_LSR  (0x14 / 2)
#define UART_RHR  (0x00 / 2)
#define UART_THR  (0x00 / 2)

void uartSetMode(uint16_t mode) {
    UART_BASE[UART_LCR] = mode;
}

void uartSoftReset() {
    UART_BASE[UART_SYSC] = 0x1;
    while((UART_BASE[UART_SYSS] & 0x1) == 0);
}

void uartInit() {
    clockWarmUp(UART_CLKCTRL_OFFSET);
    uartSoftReset();
    
    // Step 2: Switch to configuration mode A
    UART_BASE[UART_LCR] |= 0x40;

    // Step 3: Switch to config mode B to access EFR
    uint16_t saved_lcr = UART_BASE[UART_LCR];
    UART_BASE[UART_LCR] = 0x00BF;
    uint16_t saved_efr = UART_BASE[UART_EFR];
    // FORCE Auto-CTS/RTS OFF (clear bits 7 and 6)
    saved_efr &= ~(0xC0);
    UART_BASE[UART_EFR] = (saved_efr | 0x10);

    // Step 4: Switch to config mode A to access MCR
    UART_BASE[UART_LCR] = 0x0080;
    uint16_t saved_mcr = UART_BASE[UART_MCR];
    UART_BASE[UART_MCR] = (saved_mcr | 0x40);

    // Step 5: Configure FIFOs and trigger levels
    UART_BASE[UART_FCR] = 0x03; // enable + clear TX and RX FIFOs
    UART_BASE[UART_LCR] = 0x00BF;
    UART_BASE[UART_TLR] = 0x88;
    UART_BASE[UART_SCR] = 0xC0;

    // Step 6: Restore MCR, LCR
    UART_BASE[UART_EFR] = saved_efr;
    UART_BASE[UART_LCR] = 0x0080;
    UART_BASE[UART_MCR] = saved_mcr;
    UART_BASE[UART_LCR] = saved_lcr;

    // Step 7: Disable UART
    uint16_t saved_reg = UART_BASE[UART_MDR1];
    UART_BASE[UART_MDR1] = (saved_reg & 0xFFF8) | 0x07;

    // Step 8: Config mode B, enable enhanced, set divisor
    UART_BASE[UART_LCR] = 0x00BF;
    saved_efr = UART_BASE[UART_EFR];
    saved_efr &= ~(0xC0); // Ensure Auto-CTS/RTS stays off
    UART_BASE[UART_EFR] = (saved_efr | 0x10);

    UART_BASE[UART_LCR] = 0x0000;
    UART_BASE[UART_IER] = 0x0000;
    UART_BASE[UART_LCR] = 0x00BF;

    UART_BASE[UART_DLL] = 0x0038;
    UART_BASE[UART_DLH] = 0x0001;

    // Step 9: Restore EFR, set 8N1
    UART_BASE[UART_LCR] = 0x0000;
    UART_BASE[UART_IER] = 0x0000;
    UART_BASE[UART_LCR] = 0x00BF;

    UART_BASE[UART_EFR] = saved_efr;
    UART_BASE[UART_LCR] = 0x0003;

    // Step 10: Enable UART
    saved_reg = UART_BASE[UART_MDR1];
    UART_BASE[UART_MDR1] = (saved_reg & 0xFFF8);

    // Drain RX FIFO
    uint8_t dummy;
    while(UART_BASE[UART_LSR] & 0x01) {
        dummy = *((volatile uint8_t *)UART_BASE + 0);
    }
}

#define UART_SSR (0x44 / 2)

uint8_t injected_rx_byte = 0;
uint8_t has_injected_rx_byte = 0;

uint8_t uartGetC(uint8_t* c) {
    if (has_injected_rx_byte) {
        if (c) *c = injected_rx_byte;
        has_injected_rx_byte = 0;
        return 1;
    }
    if (UART_BASE[UART_LSR] & (1 << 0)) {
        if (c) *c = (uint8_t)UART_BASE[UART_RHR];
        return 1;
    }
    return 0;
}

uint16_t uartRead(uint8_t* buf, uint16_t len) {
    uint16_t count = 0;
    while (count < len) {
        if (uartGetC(&buf[count])) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

uint8_t hw_wait_and_read_char() {
    /* ~300 ns at 200 MHz */
    while (!(UART_BASE[UART_LSR] & (1 << 0))) { 
        __delay_cycles(60);
    }
    return (uint8_t)UART_BASE[UART_RHR];
}

void uartPutC(uint8_t c) {
    /* Wait until the TX FIFO is not full (SSR bit 0 is TX_FIFO_FULL) */
    while (UART_BASE[UART_SSR] & 0x01) {
        __delay_cycles(100);
    }
    UART_BASE[UART_THR] = c;
}

void uartWaitUntilTxEmpty() {
    /* Wait for Transmitter Empty (TEMT). LSR[6] = 1 means both THR and Shift Register are empty */
    while (!(UART_BASE[UART_LSR] & (1 << 6))) {        
        __delay_cycles(100);
    }
}

void uartWrite(uint8_t* buf, uint16_t len) {
    if (len == 0) return;
    
    for (uint16_t i = 0; i < len; ++i) {
        uartPutC(buf[i]);
    }
    
    uartWaitUntilTxEmpty();
}

#endif
