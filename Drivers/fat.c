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

int memcpy(void *dest, const void *src, int n) {
    char* _d = (char *) dest;
    const char* _s = (const char *) src;
    for(int x = 0; x < n ; x++)
        _d[x] = _s[x];
    return 0;
}

int zerodata(void *arr, int n) {
    char* _a = (char *) arr;
    for(int x = 0; x < n; x++)
        _a[x] = 0;
    return 0;
}

extern unsigned char bss_end;

static unsigned int partitionlba = 0;

unsigned char MBR[512];

// the BIOS Parameter Block (in Volume Boot Record)
typedef struct {
    // Jump instruction to boot code.
    char            jmp[3];
    // A name string.
    char            oem[8];
    // Count of bytes per sector; first byte.
    unsigned char   bps0;
    // Count of bytes per sector; second byte.
    unsigned char   bps1;
    // Number of sectors per allocation unit.
    unsigned char   spc;
    // Number of reserved sectors in the Reserved region
    // of the volume starting at the first sector of the volume.
    unsigned short  rsc;
    // The count of FAT data structures on the volume
    unsigned char   nf;
    // For FAT12 and FAT16 volumes, this field contains the count of
    // 32-byte directory entries in the root directory. For FAT32
    // volumes, this field must be set to 0.
    unsigned char   nr0;
    // Second byte of nr0
    unsigned char   nr1;
    // This field is the old 16-bit total count of sectors on the volume
    unsigned short  ts16;
    // 0xF8 is the standard value for "fixed" (non-removable) media.
    unsigned char   media;
    // This field is the FAT12/FAT16 16-bit count of sectors occupied by
    // ONE FAT. On FAT32 volumes this field must be zero.
    unsigned short  spf16;
    // Sectors per track for interrupt 0x13. Only relevant for media
    // with geometry.
    unsigned short  spt;
    // Number of heads for interrupt 0x13. Only relevant for media
    // with geometry.
    unsigned short  nh;
    // Count of hidden sectors preceding the partition that contains the FAT
    // volume. This field is only relevant for mediia visible on interrupt 0x13.
    unsigned int    hs;
    // This field is the new 32-bit total count of sectors on the volume. This
    // count includes the count of all sectors in all four regions of the volume.
    unsigned int    ts32;
    // This field is the FAT32 is the FAT32 32-bit count of the sectors occupied by
    // ONE FAT. spf16 field must be zero.
    unsigned int    spf32;
    unsigned int    flg;
    // This field is only defined for FAT32 media. This is set to the number
    // of cluster of the root directory.
    unsigned int    rc;
    char            vol[6];
    char            fst[8];
    char            dmy[20];
    // Always set to the string "FAT32   ".
    char            fst2[8];
} __attribute__((packed)) bpb_t;

bpb_t BPB;

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
    unsigned char sd_data[512];
    // uart_send_string("Dumping before reading from lba 0\r\n");
    //     uart_dump(MBR);
    // read the partitioning table
    if(sd_readblock(0, sd_data, 1)) {
        // uart_send_string("Dumping after reading from lba 0\r\n");
        // uart_dump(partition_data);
        memcpy(MBR, sd_data, 512);
        // uart_send_string("Copied data from local to MBR\r\n");
        // uart_dump(MBR);
        // check magic
        if(MBR[510] != 0x55 || MBR[511] != 0xAA) {
            uart_send_string("ERROR: Bad magic in MBR\r\n");
            return 0;
        }
        // check partition type
        if(MBR[0x1C2]!=0xE/*FAT16 LBA*/ && MBR[0x1C2]!=0xC/*FAT32 LBA*/) {
            uart_send_string("ERROR: Wrong partition type\r\n");
            return 0;
        }
        uart_send_string("MBR disk identifier: ");
        uart_hex(*((unsigned int*)((unsigned long)MBR+0x1B8)));
        uart_send_string("\nFAT partition starts at: ");
        // // should be this, but compiler generates bad code...
        // //partitionlba=*((unsigned int*)((unsigned long)&_end+0x1C6));
        partitionlba = MBR[0x1C6] + (MBR[0x1C7]<<8) + (MBR[0x1C8]<<16) + (MBR[0x1C9]<<24);
        uart_hex(partitionlba);
        uart_send_string("\r\n");
        // read the boot record
        if(!sd_readblock(partitionlba, sd_data, 1)) {
            uart_send_string("ERROR: Unable to read boot record\r\n");
            return 0;
        }
        // uart_send_string("Dumping after reading from lba: ");
        // uart_hex(partitionlba);
        // uart_send_string("\r\n");
        // uart_dump(partition_data);
        memcpy(&BPB, sd_data, sizeof(bpb_t));
        uart_send_string("Copied data from local to BPB\r\n");
        uart_dump(&BPB);
        // check file system type. We don't use cluster numbers for that, but magic bytes
        if( !(BPB.fst[0]=='F' && BPB.fst[1]=='A' && BPB.fst[2]=='T') &&
            !(BPB.fst2[0]=='F' && BPB.fst2[1]=='A' && BPB.fst2[2]=='T')) {
            uart_send_string("ERROR: Unknown file system type\r\n");
            return 0;
        }
        uart_send_string("FAT type: ");
        // if 16 bit sector per fat is zero, then it's a FAT32
        uart_send_string(BPB.spf16 > 0 ? "FAT16" : "FAT32");
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
    // bpb_t *bpb=(bpb_t*)&bss_end;
    // fatdir_t *dir = (fatdir_t*)(&bss_end+512);

    unsigned int root_sec, s;
    // find the root directory's LBA
    root_sec = ((BPB.spf16 ? BPB.spf16 : BPB.spf32) *BPB.nf) + BPB.rsc;
    s = (BPB.nr0 + (BPB.nr1 << 8)) * sizeof(fatdir_t);
    if(BPB.spf16 == 0) {
        // adjust for FAT32
        root_sec+=(BPB.rc-2)*BPB.spc;
    }
    // add partition LBA
    root_sec += partitionlba;

    int size = (s/512 + 1)*512;
    unsigned char sd_data[size];
    zerodata(sd_data, size);

    fatdir_t* dir = (fatdir_t*) sd_data;

    // load the root directory
    if(sd_readblock(root_sec, sd_data, s/512+1)) {
        // iterate on each entry and check if it's the one we're looking for
        for(;dir->name[0] != 0; dir++) {
            // is it a valid entry?
            if(dir->name[0]==0xE5 || dir->attr[0]==0xF) 
                continue;
            // filename match?
            if(!memcmp(dir->name, fn, 11)) {
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
    unsigned int root_sec, s;
    // find the root directory's LBA
    root_sec = ((BPB.spf16 ? BPB.spf16 : BPB.spf32) * BPB.nf) + BPB.rsc;
    s = (BPB.nr0 + (BPB.nr1 << 8));
    uart_send_string("FAT number of root directory entries: ");
    uart_hex(s);
    s *= sizeof(fatdir_t);
    if(BPB.spf16 == 0) {
        // adjust for FAT32
        root_sec += (BPB.rc-2)*BPB.spc;
    }

    // add partition LBA
    root_sec += partitionlba;
    uart_send_string("\nFAT root directory LBA: ");
    uart_hex(root_sec);
    uart_send_string("\r\n");

    int size = (s/512 + 1)*512;
    unsigned char sd_data[size];
    zerodata(sd_data, size);

    fatdir_t* dir = (fatdir_t*) sd_data;
    // load the root directory
    if(sd_readblock(root_sec, sd_data, s/512 + 1)) {
        uart_dump(sd_data);

        uart_send_string("\nAttrib Cluster  Size     Name\r\n");
        // iterate on each entry and print out
        for(;dir->name[0] != 0;dir++) {
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
    // bpb_t *bpb=(bpb_t*)&bss_end;

    // File allocation tables. We choose between FAT16 and FAT32 dynamically
    unsigned int *fat32 = (unsigned int*) (&bss_end + BPB.rsc * 512);
    unsigned short *fat16 = (unsigned short*) fat32;
    
    // Data pointers
    unsigned int data_sec, s;
    unsigned char *data, *ptr;

    // find the LBA of the first data sector
    data_sec = ((BPB.spf16 ? BPB.spf16 : BPB.spf32) * BPB.nf) + BPB.rsc;
    s = (BPB.nr0 + (BPB.nr1 << 8)) * sizeof(fatdir_t);
    if(BPB.spf16 > 0) {
        // adjust for FAT16
        data_sec += (s + 511) >> 9;
    }

    // add partition LBA
    data_sec += partitionlba;
    
    // dump important properties
    uart_send_string("FAT Bytes per Sector: ");
    uart_hex(BPB.bps0 + (BPB.bps1 << 8));
    uart_send_string("\r\nFAT Sectors per Cluster: ");
    uart_hex(BPB.spc);
    uart_send_string("\r\nFAT Number of FAT: ");
    uart_hex(BPB.nf);
    uart_send_string("\r\nFAT Sectors per FAT: ");
    uart_hex((BPB.spf16 ? BPB.spf16 : BPB.spf32));
    uart_send_string("\r\nFAT Reserved Sectors Count: ");
    uart_hex(BPB.rsc);
    uart_send_string("\r\nFAT First data sector: ");
    uart_hex(data_sec);
    uart_send_string("\r\n");
    
    // int size = (BPB.spf16 ? BPB.spf16 : BPB.spf32) + BPB.rsc;
    // unsigned char sd_data[size * 512];

    // // load FAT table
    // s = sd_readblock(partitionlba+1, sd_data, size);
    // // end of FAT in memory
    // data = ptr = &bss_end + 512 + s;

    // // iterate on cluster chain
    // while(cluster > 1 && cluster < 0xFFF8) {
    //     // load all sectors in a cluster
    //     sd_readblock((cluster-2) * BPB.spc + data_sec, ptr, BPB.spc);
    //     // move pointer, sector per cluster * bytes per sector
    //     ptr += BPB.spc*(BPB.bps0 + (BPB.bps1 << 8));
    //     // get the next cluster in chain
    //     cluster = BPB.spf16 > 0 ? fat16[cluster] : fat32[cluster];
    // }
    // return (char*)data;
    return 0;
}