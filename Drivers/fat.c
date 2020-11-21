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
#include "mini_uart.h"

int memcmp(void *s1, void *s2, int n)
{
    unsigned char *a=s1,*b=s2;
    while(n-->0){ if(*a!=*b) { return *a-*b; } a++; b++; }
    return 0;
}

extern unsigned char bss_end;

static unsigned int partitionlba = 0;

// the BIOS Parameter Block (in Volume Boot Record)
typedef struct {
    char            jmp[3];
    char            oem[8];
    unsigned char   bps0;
    unsigned char   bps1;
    unsigned char   spc;
    unsigned short  rsc;
    unsigned char   nf;
    unsigned char   nr0;
    unsigned char   nr1;
    unsigned short  ts16;
    unsigned char   media;
    unsigned short  spf16;
    unsigned short  spt;
    unsigned short  nh;
    unsigned int    hs;
    unsigned int    ts32;
    unsigned int    spf32;
    unsigned int    flg;
    unsigned int    rc;
    char            vol[6];
    char            fst[8];
    char            dmy[20];
    char            fst2[8];
} __attribute__((packed)) bpb_t;

// directory entry structure
typedef struct {
    char            name[8];
    char            ext[3];
    char            attr[9];
    unsigned short  ch;
    unsigned int    attr2;
    unsigned short  cl;
    unsigned int    size;
} __attribute__((packed)) fatdir_t;

/**
 * Get the starting LBA address of the first partition
 * so that we know where our FAT file system starts, and
 * read that volume's BIOS Parameter Block
 */
int fat_getpartition(void)
{
    unsigned char *mbr = &bss_end;
    bpb_t *bpb = (bpb_t*) &bss_end;
    // read the partitioning table
    if(sd_readblock(0,&bss_end,1)) {
        // check magic
        if(mbr[510]!=0x55 || mbr[511]!=0xAA) {
            uart_send_string("ERROR: Bad magic in MBR\r\n");
            return 0;
        }
        // check partition type
        if(mbr[0x1C2]!=0xE/*FAT16 LBA*/ && mbr[0x1C2]!=0xC/*FAT32 LBA*/) {
            uart_send_string("ERROR: Wrong partition type\r\n");
            return 0;
        }
        uart_send_string("MBR disk identifier: ");
        uart_hex(*((unsigned int*)((unsigned long)&bss_end+0x1B8)));
        uart_send_string("\nFAT partition starts at: ");
        // should be this, but compiler generates bad code...
        //partitionlba=*((unsigned int*)((unsigned long)&_end+0x1C6));
        partitionlba=mbr[0x1C6] + (mbr[0x1C7]<<8) + (mbr[0x1C8]<<16) + (mbr[0x1C9]<<24);
        uart_hex(partitionlba);
        uart_send_string("\r\n");
        // read the boot record
        if(!sd_readblock(partitionlba,&bss_end,1)) {
            uart_send_string("ERROR: Unable to read boot record\r\n");
            return 0;
        }
        // check file system type. We don't use cluster numbers for that, but magic bytes
        if( !(bpb->fst[0]=='F' && bpb->fst[1]=='A' && bpb->fst[2]=='T') &&
            !(bpb->fst2[0]=='F' && bpb->fst2[1]=='A' && bpb->fst2[2]=='T')) {
            uart_send_string("ERROR: Unknown file system type\r\n");
            return 0;
        }
        uart_send_string("FAT type: ");
        // if 16 bit sector per fat is zero, then it's a FAT32
        uart_send_string(bpb->spf16>0?"FAT16":"FAT32");
        uart_send_string("\r\n");
        return 1;
    }
    return 0;
}

/**
 * Find a file in root directory entries
 */
unsigned int fat_getcluster(char *fn)
{
    bpb_t *bpb=(bpb_t*)&bss_end;
    fatdir_t *dir=(fatdir_t*)(&bss_end+512);
    unsigned int root_sec, s;
    // find the root directory's LBA
    root_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
    s = (bpb->nr0 + (bpb->nr1 << 8)) * sizeof(fatdir_t);
    if(bpb->spf16==0) {
        // adjust for FAT32
        root_sec+=(bpb->rc-2)*bpb->spc;
    }
    // add partition LBA
    root_sec+=partitionlba;
    // load the root directory
    if(sd_readblock(root_sec,(unsigned char*)dir,s/512+1)) {
        // iterate on each entry and check if it's the one we're looking for
        for(;dir->name[0]!=0;dir++) {
            // is it a valid entry?
            if(dir->name[0]==0xE5 || dir->attr[0]==0xF) continue;
            // filename match?
            if(!memcmp(dir->name,fn,11)) {
                uart_send_string("FAT File ");
                uart_send_string(fn);
                uart_send_string(" starts at cluster: ");
                uart_hex(((unsigned int)dir->ch)<<16|dir->cl);
                uart_send_string("\r\n");
                // if so, return starting cluster
                return ((unsigned int)dir->ch)<<16|dir->cl;
            }
        }
        uart_send_string("ERROR: file not found\r\n");
    } else {
        uart_send_string("ERROR: Unable to load root directory\r\n");
    }
    return 0;
}

/**
 * List root directory entries in a FAT file system
 */
void fat_listdirectory(void)
{
    bpb_t *bpb=(bpb_t*)&bss_end;
    fatdir_t *dir=(fatdir_t*)&bss_end;
    unsigned int root_sec, s;
    // find the root directory's LBA
    root_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
    s = (bpb->nr0 + (bpb->nr1 << 8));
    uart_send_string("FAT number of root diretory entries: ");
    uart_hex(s);
    s *= sizeof(fatdir_t);
    if(bpb->spf16==0) {
        // adjust for FAT32
        root_sec+=(bpb->rc-2)*bpb->spc;
    }
    // add partition LBA
    root_sec+=partitionlba;
    uart_send_string("\nFAT root directory LBA: ");
    uart_hex(root_sec);
    uart_send_string("\r\n");
    // load the root directory
    if(sd_readblock(root_sec,(unsigned char*)&bss_end,s/512+1)) {
        uart_send_string("\nAttrib Cluster  Size     Name\r\n");
        // iterate on each entry and print out
        for(;dir->name[0]!=0;dir++) {
            // is it a valid entry?
            if(dir->name[0]==0xE5 || dir->attr[0]==0xF) continue;
            // decode attributes
            uart_send(dir->attr[0]& 1?'R':'.');  // read-only
            uart_send(dir->attr[0]& 2?'H':'.');  // hidden
            uart_send(dir->attr[0]& 4?'S':'.');  // system
            uart_send(dir->attr[0]& 8?'L':'.');  // volume label
            uart_send(dir->attr[0]&16?'D':'.');  // directory
            uart_send(dir->attr[0]&32?'A':'.');  // archive
            uart_send(' ');
            // staring cluster
            uart_hex(((unsigned int)dir->ch)<<16|dir->cl);
            uart_send(' ');
            // size
            uart_hex(dir->size);
            uart_send(' ');
            // filename
            dir->attr[0]=0;
            uart_send_string(dir->name);
            uart_send_string("\r\n");
        }
    } else {
        uart_send_string("ERROR: Unable to load root directory\r\n");
    }
}

/**
 * Read a file into memory
 */
char *fat_readfile(unsigned int cluster)
{
    // BIOS Parameter Block
    bpb_t *bpb=(bpb_t*)&bss_end;
    // File allocation tables. We choose between FAT16 and FAT32 dynamically
    unsigned int *fat32=(unsigned int*)(&bss_end+bpb->rsc*512);
    unsigned short *fat16=(unsigned short*)fat32;
    // Data pointers
    unsigned int data_sec, s;
    unsigned char *data, *ptr;
    // find the LBA of the first data sector
    data_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
    s = (bpb->nr0 + (bpb->nr1 << 8)) * sizeof(fatdir_t);
    if(bpb->spf16>0) {
        // adjust for FAT16
        data_sec+=(s+511)>>9;
    }
    // add partition LBA
    data_sec+=partitionlba;
    // dump important properties
    uart_send_string("FAT Bytes per Sector: ");
    uart_hex(bpb->bps0 + (bpb->bps1 << 8));
    uart_send_string("\nFAT Sectors per Cluster: ");
    uart_hex(bpb->spc);
    uart_send_string("\nFAT Number of FAT: ");
    uart_hex(bpb->nf);
    uart_send_string("\nFAT Sectors per FAT: ");
    uart_hex((bpb->spf16?bpb->spf16:bpb->spf32));
    uart_send_string("\nFAT Reserved Sectors Count: ");
    uart_hex(bpb->rsc);
    uart_send_string("\nFAT First data sector: ");
    uart_hex(data_sec);
    uart_send_string("\r\n");
    // load FAT table
    s=sd_readblock(partitionlba+1,(unsigned char*)&bss_end+512,(bpb->spf16?bpb->spf16:bpb->spf32)+bpb->rsc);
    // end of FAT in memory
    data=ptr=&bss_end+512+s;
    // iterate on cluster chain
    while(cluster>1 && cluster<0xFFF8) {
        // load all sectors in a cluster
        sd_readblock((cluster-2)*bpb->spc+data_sec,ptr,bpb->spc);
        // move pointer, sector per cluster * bytes per sector
        ptr+=bpb->spc*(bpb->bps0 + (bpb->bps1 << 8));
        // get the next cluster in chain
        cluster=bpb->spf16>0?fat16[cluster]:fat32[cluster];
    }
    return (char*)data;
}