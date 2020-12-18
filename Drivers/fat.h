#ifndef _FAT_H
#define _FAT_H

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
int fat_getpartition(void);

/**
 * Find a file in root directory entries
 */
unsigned int fat_getcluster(char *fn);

/**
 * List root directory entries in a FAT file system
 */
void fat_listdirectory(void);

/**
 * Read a file into memory
 */
char *fat_readfile(unsigned int cluster);

#endif