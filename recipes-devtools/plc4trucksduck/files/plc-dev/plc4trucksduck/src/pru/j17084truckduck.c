/* PLC4TRUCKSDuck (c) 2020 National Motor Freight Traffic Association
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// TESTED ON: 10/17/2024 - Working with UTHP 1.0.0

#define PRU_NO 1
#define BBB_GPIO_PIN 40 // fake ILD (not needed for J1708 over PLC.. this is over the THVD1410 transceiver)
#define UART_NUM 2

/* Host-0 Interrupt sets bit 31 in register R31 */
#define HOST_INT                        ((uint32_t) 1 << 31)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST 18
#define FROM_ARM_HOST 19

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME                       "rpmsg-pru"
#define CHAN_DESC                       "Channel 31"
#define CHAN_PORT                       31

/* J1708 requires 10 bits of interframe spacing. J1708 also operates at 9600
 * baud and the clock rate of the PRUs is 200MHz. Also the line is active when
 * its low (digital zero). Thus:
 * bit_time = 1/9600 = 1.04e-4
 * time_for_10_bits = 10 * bit_time
 * num_clock_cycles_for_10_bits = time_for_10_bits * 200000000 ~= 208,000
 * Therefore, the code below is checking if the bus is available every half bit.
 */
#define CYCLES_PER_HALF_BIT 10400
#define CHECKS_TILL_BUS_IDLE 20

/* 15 bit times delay for message break:
// Max delay to receive next byte is 15 bit times (10 for char + up to 5 idle gap).
// 15 bit times * 2 half-bits/bit = 30 checks. */
#define CHECKS_TILL_MSG_FINISHED 30

#define J1708 // Remove if not testing UART
// #define J1708_TESTING // Remove if not testing J1708 with THVD1410

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_ctrl.h>
#include <pru_rpmsg.h>
#include <string.h>
#include "intc_map_1.h"
#include "common.h"

void main() {
    struct pru_rpmsg_transport transport;
    uint16_t src = 0;
    uint16_t dst = 0;
    uint16_t len = 0;

    pruInit(&transport);
    // Need to initialize src and dst
    while(pru_rpmsg_receive(&transport, &src, &dst, transmitBuf, &len) != PRU_RPMSG_SUCCESS);
    memset(transmitBuf, 0, RPMSG_MESSAGE_SIZE);

    while (1) {
        // Is there a message to transmit from the host?
        if (pru_rpmsg_receive(&transport, &src, &dst, transmitBuf, &len) == PRU_RPMSG_SUCCESS) {
            if (len > 0) {
                /* 
                 * =========================================================================
                 * PROVEN J1708 TRANSMIT LOGIC
                 * =========================================================================
                 * This pacing is identical to the J2497/PLC modem pacing logic.
                 *
                 * The mandatory transmission sequence is:
                 * 1. Send Byte 1 (MID).
                 * 2. Wait for the hardware echo to arrive.
                 * 3. Perform J1708 arbitration check on the echoed byte.
                 * 4. Delay exactly 2 bit-times (~208us) AFTER receiving the echo.
                 * 5. Send Byte 2.
                 * 6. Delay exactly 2 bit-times (~208us) AFTER Byte 2 has fully shifted out.
                 * 7. Send the remaining bytes (Byte 3..N) with exactly 1 bit-time (~104us)
                 *    idle gaps between each byte.
                 * =========================================================================
                 */

                // Clear any stale noise from the RX FIFO before transmitting
                uint8_t dummy_rx;
                while (uartGetC(&dummy_rx));

                /* 1. Send the first byte (MID) */
                uartPutC(transmitBuf[0]);
                uartWaitUntilTxEmpty();

                /* 2. Wait for echo of the first byte */
                uint8_t rx = hw_wait_and_read_char();
                injected_rx_byte = rx;
                has_injected_rx_byte = 1;

                /* 3. Check equality */
                if (rx == transmitBuf[0]) {
                    /* 4. Wait exactly two bit times (208us) */
                    __delay_cycles(41600);

                    if (len > 1) {
                        /* Send the second byte */
                        uartPutC(transmitBuf[1]);
                        uartWaitUntilTxEmpty();

                        if (len > 2) {
                            /* 5. 2 bit time gap (208us) between second and third byte */
                            __delay_cycles(41600);

                            /* 6. Transmit the rest of the bytes with 1 bit time gap (104us) */
                            for (uint16_t i = 2; i < len; ++i) {
                                uartPutC(transmitBuf[i]);
                                uartWaitUntilTxEmpty();
                                
                                if (i < len - 1) {
                                    __delay_cycles(20800);
                                }
                            }
                        }
                    }
                    uartWaitUntilTxEmpty();
                }
            }
        } else if (UART_BASE[UART_LSR] & 0x01 || has_injected_rx_byte) { 
            // Is there anything to receive from the UART or from our injected echo?
            uint16_t recvLen = receiveRemainingMessage(&receiveBuf[0]);
            pru_rpmsg_send(&transport, dst, src, receiveBuf, recvLen);
            memset(receiveBuf, 0, MAX_PAYLOAD_LEN);
        }
    }
}
