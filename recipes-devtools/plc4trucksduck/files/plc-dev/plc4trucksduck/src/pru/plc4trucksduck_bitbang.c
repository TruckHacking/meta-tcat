/* PLC4TRUCKSDuck (c) 2024 National Motor Freight Traffic Association
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

// TESTED ON: 02/12/2025 - Working with UTHP 1.0.0

#define PRU_NO 0
#define BBB_GPIO_PIN 88 // IDLE LINE DETECT: needs to be set in overlays
#define UART_NUM 4

/* Host-0 Interrupt sets bit 30 in register R31 */
#define HOST_INT			((uint32_t) 1 << 30)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST 16
#define FROM_ARM_HOST 17

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 30"
#define CHAN_PORT			30

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
#define PLC // needed in common.h to enable GPIO Carrier Sense
// Side note: the SSCP485 doesn't include TP

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_ctrl.h>
#include <pru_rpmsg.h>
#include <string.h>
#include <stddef.h>
#include "intc_map_0.h"
#include "common.h"


#define TX_PAYLOAD_LEN 321
typedef struct  {
    uint16_t volatile bit_length;
    uint8_t volatile preamble;
    uint8_t volatile payload[TX_PAYLOAD_LEN];
} tx_frame_t;

/* Extracted from original master branch bitbanging code */

void emit_pos_symbol() {
    asm("        SUB      r2, r2, 8");
    asm("        SBBO     &r0, r2, 0, 4");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 240 ; sleep 484 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 232 ; sleep 467 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 224 ; sleep 452 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 217 ; sleep 437 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 211 ; sleep 426 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 205 ; sleep 414 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 200 ; sleep 403 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 195 ; sleep 393 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 190 ; sleep 384 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 186 ; sleep 375 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 182 ; sleep 368 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 178 ; sleep 360 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 175 ; sleep 353 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 171 ; sleep 346 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 168 ; sleep 340 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 165 ; sleep 334 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 162 ; sleep 328 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 160 ; sleep 323 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 157 ; sleep 318 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 154 ; sleep 312 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 153 ; sleep 309 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 150 ; sleep 303 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 148 ; sleep 300 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 146 ; sleep 295 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 144 ; sleep 291 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 142 ; sleep 288 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 140 ; sleep 284 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 138 ; sleep 280 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 137 ; sleep 277 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 135 ; sleep 274 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 133 ; sleep 270 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 132 ; sleep 268 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 130 ; sleep 264 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 129 ; sleep 262 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 128 ; sleep 259 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 126 ; sleep 256 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 125 ; sleep 254 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 123 ; sleep 249 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 161 ; sleep 325 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 269 ; sleep 542 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 294 ; sleep 592 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 289 ; sleep 582 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 285 ; sleep 573 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 280 ; sleep 563 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 276 ; sleep 555 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 271 ; sleep 546 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 267 ; sleep 538 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 264 ; sleep 531 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 260 ; sleep 523 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 257 ; sleep 517 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 253 ; sleep 509 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    // caller is responsible for 504 cycle delay
    asm("        LBBO     &r0, r2, 0, 4");
    asm("        ADD      r2, r2, 8");
}

#define EMIT_POS_SYMBOL_FINAL_CYCLES 504

void emit_neg_symbol() {
    asm("        SUB      r2, r2, 8");
    asm("        SBBO     &r0, r2, 0, 4");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 240 ; sleep 484 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 232 ; sleep 467 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 224 ; sleep 452 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 217 ; sleep 437 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 211 ; sleep 426 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 205 ; sleep 414 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 200 ; sleep 403 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 195 ; sleep 393 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 190 ; sleep 384 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 186 ; sleep 375 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 182 ; sleep 368 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 178 ; sleep 360 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 175 ; sleep 353 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 171 ; sleep 346 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 168 ; sleep 340 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 165 ; sleep 334 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 162 ; sleep 328 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 160 ; sleep 323 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 157 ; sleep 318 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 154 ; sleep 312 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 153 ; sleep 309 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 150 ; sleep 303 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 148 ; sleep 300 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 146 ; sleep 295 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 144 ; sleep 291 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 142 ; sleep 288 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 140 ; sleep 284 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 138 ; sleep 280 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 137 ; sleep 277 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 135 ; sleep 274 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 133 ; sleep 270 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 132 ; sleep 268 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 130 ; sleep 264 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 129 ; sleep 262 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 128 ; sleep 259 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 126 ; sleep 256 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 125 ; sleep 254 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 123 ; sleep 249 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 161 ; sleep 325 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 269 ; sleep 542 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 294 ; sleep 592 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 289 ; sleep 582 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 285 ; sleep 573 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 280 ; sleep 563 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 276 ; sleep 555 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 271 ; sleep 546 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 267 ; sleep 538 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        XOR      r0, r0, r0 ; for even number of sleep cycles");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 264 ; sleep 531 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 260 ; sleep 523 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 257 ; sleep 517 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        CLR      r30, r30, 1");
    asm("        .newblock");
    asm("        LDI32    r0, 253 ; sleep 509 cycles total");
    asm("$1:     SUB      r0, r0, 1");
    asm("        QBNE     $1, r0, 0");
    asm("        SET      r30, r30, 1");
    // caller is responsible for 504 cycle delay
    asm("        LBBO     &r0, r2, 0, 4");
    asm("        ADD      r2, r2, 8");
}

#define EMIT_NEG_SYMBOL_FINAL_CYCLES 504


#define TX_FRAME_PREAMBLE_LEN 8
#define PREAMBLE_EXTRA_CYCLES 2800 // 14us at 200MHz
#define PREAMBLE_TOTAL_CYCLES 22800 // 114us at 200MHz

#define BUS_ACCESS_IDLE_CYCLES (12 * PREAMBLE_TOTAL_CYCLES)

//NB: heavy use of __delay_cycles() in this function results in a large stack save/restore operation
int hw_send_preamble(volatile tx_frame_t *msg) {
    //emit negative preamble symbol
    emit_pos_symbol();
    __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                    - 6 // overhead of emit_pos_symbol() call
                    + PREAMBLE_EXTRA_CYCLES // silence time for preamble symbols only
                    );
    //emit negative preamble symbol
    emit_pos_symbol();
    __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                    - 6 // overhead of emit_pos_symbol() call
                    + PREAMBLE_EXTRA_CYCLES // silence time for preamble symbols only
                    );
    //emit negative preamble symbol
    emit_pos_symbol();
    __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                    - 6 // overhead of emit_pos_symbol() call
                    + PREAMBLE_EXTRA_CYCLES // silence time for preamble symbols only
                    - 8 // overhead of loop initialization and test below
                    );

    for(int i=0; i < TX_FRAME_PREAMBLE_LEN; ++i) {
        if(msg->preamble & (1 << i)) {
            //emit positive preamble symbol
            asm("   clr r30, r30, 1");
            __delay_cycles(PREAMBLE_TOTAL_CYCLES
                            - 14 // loop and test overhead
                            );
        } else {
            //emit negative preamble symbol
            emit_pos_symbol();
            __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                            - 6 // overhead of emit_pos_symbol() call
                            + PREAMBLE_EXTRA_CYCLES // silence time for preamble symbols only
                            - 14 // loop and test overhead
                            );
        }
    }

    __delay_cycles(8 // loop and test overhead not executed in last pass
                    );
    //emit positive preamble symbol
    asm("   clr r30, r30, 1");
    //caller is responsible for delay of PREAMBLE_TOTAL_CYCLES
    return 0;
}

void hw_send_payload(volatile tx_frame_t *msg) {
    register uint16_t bit_length = msg->bit_length;

    for(uint8_t i=0; i < 4; ++i) {
        emit_pos_symbol();
        __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                        - 2 // loop overhead
                        - 6 // overhead of next emit_pos_symbol() call
                        );
    }
    __delay_cycles(2 /*loop overhead not executed*/);
    emit_pos_symbol();
    __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                    - 6 // overhead of emit_pos_symbol() call
                    - 17 // loop and test overhead below
                    );

    for(uint16_t i=0; i < bit_length; ++i) {
        if( msg->payload[ i / 8 ] & (1 << (7-(i % 8))) ) {
            emit_pos_symbol();
            __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                - 14 // loop and test overhead
                - 6 // overhead of next emit_pos_symbol() call
                );
        } else {
            emit_neg_symbol();
            __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                - 15 // loop and test overhead
                - 6 // overhead of next emit_pos_symbol() call
                );
        }
    }
    // loop exit above is only 2 cycles max, we will ignore

    emit_pos_symbol();
    __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                    - 1 // loop setup overhead
                    - 6 // overhead of next emit_pos_symbol() call
                    - 2 // loop exit overhead above adjust
                    );
    for(uint8_t i=0; i < 5; ++i) {
        emit_pos_symbol();
        __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES
                        - 2 // loop overhead
                        - 6 // overhead of next emit_pos_symbol() call
                        );
    }
    emit_pos_symbol();
    //caller is responsible for delay of EMIT_POS_SYMBOL_FINAL_CYCLES
}


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
        // Is there a message to transmit?
        if (transmitBuf[0] != 0 || pru_rpmsg_receive(&transport, &src, &dst, transmitBuf, &len) == PRU_RPMSG_SUCCESS) {

            if (len > 0) {
                /* --- BITBANG TRANSMIT LOGIC --- */
                tx_frame_t local_msg;
                local_msg.preamble = 0xAA; // 10101010
                local_msg.bit_length = len * 10;
                memset((void*)local_msg.payload, 0, TX_PAYLOAD_LEN);

                uint16_t bit_idx = 0;
                for (uint16_t i = 0; i < len; i++) {
                    uint8_t b = transmitBuf[i];
                    bit_idx++; // Start bit is a 0, meaning we leave it as 0 in payload
                    
                    // 8 Data bits (LSB first)
                    for (int j = 0; j < 8; j++) {
                        if (b & (1 << j)) {
                            local_msg.payload[bit_idx / 8] |= (1 << (7 - (bit_idx % 8)));
                        }
                        bit_idx++;
                    }
                    // Stop bit (1)
                    local_msg.payload[bit_idx / 8] |= (1 << (7 - (bit_idx % 8)));
                    bit_idx++;
                }

                // Clear any stale noise from the RX FIFO before transmitting
                uint8_t dummy_rx;
                while (uartGetC(&dummy_rx));

                hw_send_preamble(&local_msg);
                __delay_cycles(PREAMBLE_TOTAL_CYCLES - 2 - 2 - 1 + 40);
                hw_send_payload(&local_msg);
                __delay_cycles(EMIT_POS_SYMBOL_FINAL_CYCLES - 2);
                asm("   clr r30, r30, 1");
                __delay_cycles(BUS_ACCESS_IDLE_CYCLES);

                // No injected bytes are needed here because we are bitbanging completely 
                // outside the UART peripheral! The host daemon won't receive an echo 
                // unless the bitbanging physically loops back into the UART RX pin.
                // Assuming it does (or if it doesn't, that's expected behavior for this specific bitbanger).
                
                /* --- END BITBANG TRANSMIT LOGIC --- */
}

            memset(transmitBuf, 0, RPMSG_MESSAGE_SIZE);
            
        } else if (UART_BASE[UART_LSR] & 0x01 || has_injected_rx_byte) { 
            // Is there anything to receive from the UART or from our injected echo?
            uint16_t recvLen = receiveRemainingMessage(&receiveBuf[0]);
            pru_rpmsg_send(&transport, dst, src, receiveBuf, recvLen);
            memset(receiveBuf, 0, MAX_PAYLOAD_LEN);
        }
    }
}
