/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

// Modified by Sean Phillips

#include "sd.h"
#include "delay.h"
//#include "gpio.h"
#include "mini_uart.h"
#include "peripherals/base.h"

// Reference page 66 in BCM2835 pdf
//! as a note the document: SD3.0_Host_AHB_eMMC4.4_Usersguide_ver5.9_jan11_10.pdf
//! is not publically available? according to source:
//! https://www.raspberrypi.org/forums/viewtopic.php?t=7638

// Contains the argument for the SD card specific command ACMD23.
#define EMMC_ARG2           ((volatile unsigned int*)(PBASE+0x00300000))
// Contains the number and size in bytes for data blocks to be transferred.
//! Maximum size of block is 1024 bytes
#define EMMC_BLKSIZECNT     ((volatile unsigned int*)(PBASE+0x00300004))
// Contains the arguments for all commands except for the SD card specific
// command ACMD23. Must be set before command is issued to CMDTM register.
#define EMMC_ARG1           ((volatile unsigned int*)(PBASE+0x00300008))
// This register is used to issue commands to the card. It also contains flag
// information informing the EMMC module what card response and type of data
// transfer to expect.
#define EMMC_CMDTM          ((volatile unsigned int*)(PBASE+0x0030000C))
// Register contains the status bits of the SD card s response.
//! In case of commands CMD2 and CMD10 it contains CID[31:0].
//! In case of command CMD9 it contains CSD[31:0].
// Note: the register is only valid once the last command has completed.
#define EMMC_RESP0          ((volatile unsigned int*)(PBASE+0x00300010))
//! In case of commands CMD2 and CMD10 this register contains CID[63:32].
//! In case of commands CMD9 this register contains CSD[63:32].
#define EMMC_RESP1          ((volatile unsigned int*)(PBASE+0x00300014))
//! In case of commands CMD2 and CMD10 this register contains CID[95:64].
//! In case of commands CMD9 this register contains CSD[95:64].
#define EMMC_RESP2          ((volatile unsigned int*)(PBASE+0x00300018))
//! In case of commands CMD2 and CMD10 this register contains CID[127:96].
//! In case of commands CMD9 this register contains CSD[127:96].
#define EMMC_RESP3          ((volatile unsigned int*)(PBASE+0x0030001C))
// This register is used to transfer data to/from the card.
//! Bit 1 of the interrupt register can be used to check if data is available.
#define EMMC_DATA           ((volatile unsigned int*)(PBASE+0x00300020))
// This register contains information intended for debugging. Its values change
// automatically according to the hardware.
//! It is not recommended to use this register for polling. Instead use the
//! INTERRUPT register which implements a handshake mechanism.
#define EMMC_STATUS         ((volatile unsigned int*)(PBASE+0x00300024))
// This register is used to configure the EMMC module.
//! Refer to Arasan documentation for more details: 
//! SD3.0_Host_AHB_eMMC4.4_Usersguide_ver5.9_jan11_10.pdf
#define EMMC_CONTROL0       ((volatile unsigned int*)(PBASE+0x00300028))
// This register is used to configure the EMMC module.
#define EMMC_CONTROL1       ((volatile unsigned int*)(PBASE+0x0030002C))
// This register holds the interrupt flags. Each flag can be disabled
// using the according bit in the IRPT_MASK register.
#define EMMC_INTERRUPT      ((volatile unsigned int*)(PBASE+0x00300030))
// The IRPT_MASK register used to mask the interrupt falgs.
//! Refer to Arasan documentation for more details:
//! SD3.0_Host_AHB_eMMC4.4_Usersguide_ver5.9_jan11_10.pdf
#define EMMC_INT_MASK       ((volatile unsigned int*)(PBASE+0x00300034))
// This register enables different interrupts in the INTERRUPT register to
// generate an interrupt on the int_to_arm output.
//! Refer to Arasan documentation for more details:
//! SD3.0_Host_AHB_eMMC4.4_Usersguide_ver5.9_jan11_10.pdf
#define EMMC_INT_EN         ((volatile unsigned int*)(PBASE+0x00300038))
// This register enables different interrupts in the INTERRUPT register to
// generate an interrupt on the int_to_arm output.
//! Refer to Arasan documentation for more details:
//! SD3.0_Host_AHB_eMMC4.4_Usersguide_ver5.9_jan11_10.pdf
#define EMMC_CONTROL2       ((volatile unsigned int*)(PBASE+0x0030003C))
// This register contains the version information and slot interrupt status.
//! Refer to Arasan documentation for more details:
//! SD3.0_Host_AHB_eMMC4.4_Usersguide_ver5.9_jan11_10.pdf
#define EMMC_SLOTISR_VER    ((volatile unsigned int*)(PBASE+0x003000FC))

// command flags
#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// COMMANDs
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// STATUS register settings
#define SR_READ_AVAILABLE   0x00000800
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

// INTERRUPT register settings
#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_CMD_DONE        0x00000001

#define INT_ERROR_MASK      0x017E8000

// CONTROL register settings
#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWITDH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

// SLOTISR_VER values
#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

// SCR flags
#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
// added by my driver
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000

unsigned long sd_scr[2], sd_ocr, sd_rca, sd_err, sd_hv;

/**
 * Wait for data or command ready
 */
int sd_status(unsigned int mask)
{
    int cnt = 500000; 
    while((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && cnt--) 
        wait_msec(1);
    return (cnt <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

/**
 * Wait for interrupt
 */
int sd_int(unsigned int mask)
{
    unsigned int r, m=mask | INT_ERROR_MASK;
    
    int cnt = 1000000; 
    while(!(*EMMC_INTERRUPT & m) && cnt--) 
        wait_msec(1);
    
    r = *EMMC_INTERRUPT;
    if(cnt<=0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT) )  { 
        *EMMC_INTERRUPT = r; 
        return SD_TIMEOUT; 
        
    } else if(r & INT_ERROR_MASK) { 
        *EMMC_INTERRUPT = r; 
        return SD_ERROR; 
    }

    *EMMC_INTERRUPT = mask;
    return 0;
}

/** Modified
 * Send a command
 */
int sd_cmd(unsigned int code, unsigned int arg)
{
    int r = 0;

    sd_err = SD_OK;

    if(code& CMD_NEED_APP) {
        r = sd_cmd(CMD_APP_CMD|(sd_rca?CMD_RSPNS_48:0),sd_rca);
        if(sd_rca && !r) { 
            uart_send_string("ERROR: failed to send SD APP command\r\n"); 
            sd_err=SD_ERROR;
            return 0;
        }
        code &= ~CMD_NEED_APP;
    }

    if(sd_status(SR_CMD_INHIBIT)) { 
        uart_send_string("ERROR: EMMC busy\r\n"); 
        sd_err= SD_TIMEOUT;
        return 0;
    }

    uart_send_string("EMMC: Sending command ");
    uart_hex(code);
    uart_send_string(" arg ");
    uart_hex(arg);
    uart_send_string("\r\n");
    
    *EMMC_INTERRUPT = *EMMC_INTERRUPT; 
    *EMMC_ARG1 = arg; 
    *EMMC_CMDTM = code;
    
    if(code == CMD_SEND_OP_COND) 
        wait_msec(1000); 
    else if(code == CMD_SEND_IF_COND || code == CMD_APP_CMD) 
        wait_msec(100);
    
    if((r=sd_int(INT_CMD_DONE))) {
        uart_send_string("ERROR: failed to send EMMC command\r\n");
        sd_err=r;
        return 0;
    }

    r = *EMMC_RESP0;
    if(code == CMD_GO_IDLE || code == CMD_APP_CMD) 
        return 0; 
    else if(code==(CMD_APP_CMD|CMD_RSPNS_48)) 
        return r & SR_APP_CMD; 
    else if(code==CMD_SEND_OP_COND) 
        return r; 
    else if(code==CMD_SEND_IF_COND) 
        return r == arg? SD_OK : SD_ERROR; 
    else if(code==CMD_ALL_SEND_CID) {
        r |= *EMMC_RESP3; 
        r |= *EMMC_RESP2; 
        r |= *EMMC_RESP1; 
        return r; 
    } else if(code == CMD_SEND_REL_ADDR) {
        sd_err=(((r&0x1fff))|((r&0x2000)<<6)|((r&0x4000)<<8)|((r&0x8000)<<8))&CMD_ERRORS_MASK;
        return r&CMD_RCA_MASK;
    }
    return r&CMD_ERRORS_MASK;
    // make gcc happy
    return 0;
}

/**
 * read a block from sd card and return the number of bytes read
 * returns 0 on error.
 */
int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num)
{
    
    int r, c = 0 ,d;

    if(num<1) 
        num = 1;

    uart_send_string("sd_readblock lba ");
    uart_hex(lba);
    uart_send_string(" num ");
    uart_hex(num);
    uart_send_string("\r\n");

    if(sd_status(SR_DAT_INHIBIT)) {
        sd_err = SD_TIMEOUT; 
        return 0;
    }
    
    unsigned int *buf = (unsigned int *) buffer;
    if(sd_scr[0] & SCR_SUPP_CCS) {
        if(num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD_SET_BLOCKCNT,num);
            if(sd_err) 
                return 0;
        }
        
        *EMMC_BLKSIZECNT = (num << 16) | 512;
        
        sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI,lba);
        
        if(sd_err) 
            return 0;
    } else {
        *EMMC_BLKSIZECNT = (1 << 16) | 512;
    }
    while( c < num ) {
        if(!(sd_scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD_READ_SINGLE,(lba+c)*512);
            if(sd_err) return 0;
        }
        if((r=sd_int(INT_READ_RDY))) {
            uart_send_string("\rERROR: Timeout waiting for ready to read\r\n");
            sd_err = r;
            return 0;
        }
        for(d = 0;d < 128; d++) 
            buf[d] = *EMMC_DATA;
        c++; 
        buf += 128;
    }
    if( num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) 
        sd_cmd(CMD_STOP_TRANS,0);
    return sd_err!=SD_OK || c!=num? 0 : num*512;
}


// int sd_readblock(unsigned int lba, unsigned char* buffer, unsigned int num) {
//     int r,c,d;
//     r = 0; c = 0; d = 0;
    
//     // Make sure num isn't negative
//     if(num < 1) num = 1;

//     uart_send_string("sd_readblock lba ");
//     uart_hex(lba);
//     uart_send_string(" num ");
//     uart_hex(num);
//     uart_send_string("\r\n");

//     if(sd_status(SR_DAT_INHIBIT)) {
//         sd_err = SD_TIMEOUT;
//         return 0;
//     }

//     int start_buff = buffer; // For resetting the address
//     unsigned int *buff  = (unsigned int*) buffer;
//     if(sd_scr[0] & SCR_SUPP_CCS) {
//         if(num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
//             sd_cmd(CMD_SET_BLOCKCNT, num);
//             if(sd_err) return 0;
//         }

//         // This may also need to be reset? No documentation avaliable.
//         *EMMC_BLKSIZECNT = (num << 16) | 512;

//         sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI, lba);

//         if(sd_err) return 0;
//     }  else {
//         *EMMC_BLKSIZECNT = (1 << 16) | 512;
//     }

//     while( c < num ) {
//         if(!(sd_scr[0] & SCR_SUPP_CCS)) {
//             sd_cmd(CMD_READ_SINGLE, (lba+c)*512);
//             if(sd_err) return 0;
//         }
//         if((r = sd_int(INT_READ_RDY))) {
//             uart_send_string("\rERROR: Timeout waiting for ready to read\r\n");
//             sd_err = r;
//             return 0;
//         }
//         for(d = 0; d < 128; d++)
//             buff[d] = *EMMC_DATA;
//         c++;
//         buff += 128;
//     }

//     if( num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) 
//         sd_cmd(CMD_STOP_TRANS,0);
//     return sd_err!=SD_OK || c!=num? 0 : num*512;
// }

/**
 * set SD clock to frequency in Hz
 */
int sd_clk(unsigned int f)
{
    unsigned int d,c=41666666/f,x,s=32,h=0;
    int cnt = 100000;
    while((*EMMC_STATUS & (SR_CMD_INHIBIT|SR_DAT_INHIBIT)) && cnt--) wait_msec(1);
    if(cnt<=0) {
        uart_send_string("ERROR: timeout waiting for inhibit flag\r\n");
        return SD_ERROR;
    }

    *EMMC_CONTROL1 &= ~C1_CLK_EN; wait_msec(10);
    x=c-1; if(!x) s=0; else {
        if(!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
        if(!(x & 0xff000000u)) { x <<= 8;  s -= 8; }
        if(!(x & 0xf0000000u)) { x <<= 4;  s -= 4; }
        if(!(x & 0xc0000000u)) { x <<= 2;  s -= 2; }
        if(!(x & 0x80000000u)) { x <<= 1;  s -= 1; }
        if(s>0) s--;
        if(s>7) s=7;
    }
    if(sd_hv>HOST_SPEC_V2) d=c; else d=(1<<s);
    if(d<=2) {d=2;s=0;}
    uart_send_string("sd_clk divisor ");uart_hex(d);uart_send_string(", shift ");uart_hex(s);uart_send_string("\r\n");
    if(sd_hv>HOST_SPEC_V2) h=(d&0x300)>>2;
    d=(((d&0x0ff)<<8)|h);
    *EMMC_CONTROL1=(*EMMC_CONTROL1&0xffff003f)|d; wait_msec(10);
    *EMMC_CONTROL1 |= C1_CLK_EN; wait_msec(10);
    cnt=10000; while(!(*EMMC_CONTROL1 & C1_CLK_STABLE) && cnt--) wait_msec(10);
    if(cnt<=0) {
        uart_send_string("ERROR: failed to get stable clock\r\n");
        return SD_ERROR;
    }
    return SD_OK;
}

// Not really ideal, but since these are pointers to memory, we don't
// want to break these macros that we already have.
#define MMIO_BASE       0x3F000000

#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

/**
 * initialize EMMC to read SDHC card
 */
int sd_init()
{
    long r, cnt, ccs = 0;
    // GPIO_CD
    r = *GPFSEL4; 
    r &= ~(7<<(7*3));
    *GPFSEL4 = r;
    *GPPUD = 2; 
    wait_cycles(150); 
    *GPPUDCLK1 = (1<<15); 
    wait_cycles(150); 
    *GPPUD = 0; 
    *GPPUDCLK1 = 0;
    r=*GPHEN1; 
    r |= 1<<15;
    *GPHEN1 = r;

    // GPIO_CLK, GPIO_CMD
    r = *GPFSEL4; 
    r |= (7<<(8*3)) | (7<<(9*3)); 
    *GPFSEL4=r;
    *GPPUD=2; 
    wait_cycles(150); 
    *GPPUDCLK1 = (1<<16)|(1<<17); 
    wait_cycles(150); 
    *GPPUD = 0; 
    *GPPUDCLK1 = 0;

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = *GPFSEL5; 
    r |= (7<<(0*3)) | (7<<(1*3)) | (7<<(2*3)) | (7<<(3*3));
    *GPFSEL5 = r;
    *GPPUD = 2; 
    wait_cycles(150);
    *GPPUDCLK1=(1<<18) | (1<<19) | (1<<20) | (1<<21);
    wait_cycles(150); 
    *GPPUD=0; 
    *GPPUDCLK1=0;

    sd_hv = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
    uart_send_string("EMMC: GPIO set up\r\n");
    // Reset the card.
    *EMMC_CONTROL0 = 0; 
    *EMMC_CONTROL1 |= C1_SRST_HC;
    cnt=10000; 
    do{
        wait_msec(10);
    } while( (*EMMC_CONTROL1 & C1_SRST_HC) && cnt-- );
    if(cnt<=0) {
        uart_send_string("ERROR: failed to reset EMMC\r\n");
        return SD_ERROR;
    }
    uart_send_string("EMMC: reset OK\r\n");
    *EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
    wait_msec(10);
    // Set clock to setup frequency.
    if((r=sd_clk(400000))) 
        return r;
    *EMMC_INT_EN = 0xffffffff;
    *EMMC_INT_MASK = 0xffffffff;
    sd_scr[0]=sd_scr[1]=sd_rca=sd_err=0;
    
    sd_cmd(CMD_GO_IDLE,0);
    
    if(sd_err) 
        return sd_err;

    sd_cmd(CMD_SEND_IF_COND,0x000001AA);
    
    if(sd_err) 
        return sd_err;
    cnt = 6; 
    r = 0; 
    while(!(r&ACMD41_CMD_COMPLETE) && cnt--) {
        wait_cycles(400);
        r = sd_cmd(CMD_SEND_OP_COND,ACMD41_ARG_HC);
        uart_send_string("EMMC: CMD_SEND_OP_COND returned ");
        if(r & ACMD41_CMD_COMPLETE)
            uart_send_string("COMPLETE ");
        if(r & ACMD41_VOLTAGE)
            uart_send_string("VOLTAGE ");
        if(r & ACMD41_CMD_CCS)
            uart_send_string("CCS ");
        uart_hex(r>>32);
        uart_hex(r);
        uart_send_string("\r\n");
        if(sd_err != SD_TIMEOUT && sd_err != SD_OK ) {
            uart_send_string("ERROR: EMMC ACMD41 returned error\r\n");
            return sd_err;
        }
    }
    if(!(r & ACMD41_CMD_COMPLETE) || !cnt ) 
        return SD_TIMEOUT;
    if(!(r & ACMD41_VOLTAGE)) 
        return SD_ERROR;
    if(r&ACMD41_CMD_CCS) 
        ccs=SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID,0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR,0);
    uart_send_string("EMMC: CMD_SEND_REL_ADDR returned ");
    uart_hex(sd_rca>>32);
    uart_hex(sd_rca);
    uart_send_string("\n");
    if(sd_err) 
        return sd_err;

    if((r=sd_clk(25000000))) 
        return r;

    sd_cmd(CMD_CARD_SELECT,sd_rca);
    if(sd_err) 
        return sd_err;

    if(sd_status(SR_DAT_INHIBIT)) 
        return SD_TIMEOUT;
    *EMMC_BLKSIZECNT = (1<<16) | 8;
    sd_cmd(CMD_SEND_SCR,0);
    if(sd_err) 
        return sd_err;
    if(sd_int(INT_READ_RDY)) 
        return SD_TIMEOUT;

    r=0; cnt=100000; while(r<2 && cnt) {
        if( *EMMC_STATUS & SR_READ_AVAILABLE )
            sd_scr[r++] = *EMMC_DATA;
        else
            wait_msec(1);
    }
    if(r!=2) 
        return SD_TIMEOUT;
    if(sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
        sd_cmd(CMD_SET_BUS_WIDTH,sd_rca|2);
        if(sd_err) 
            return sd_err;
        *EMMC_CONTROL0 |= C0_HCTL_DWITDH;
    }
    // add software flag
    uart_send_string("EMMC: supports ");
    if(sd_scr[0] & SCR_SUPP_SET_BLKCNT)
        uart_send_string("SET_BLKCNT ");
    if(ccs)
        uart_send_string("CCS ");
    uart_send_string("\r\n");
    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;
    return SD_OK;
}