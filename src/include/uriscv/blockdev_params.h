// SPDX-FileCopyrightText: 2004 Mauro Morsiani
// SPDX-FileCopyrightText: 2020 Mattia Biondi
//
// SPDX-License-Identifier: GPL-3.0-or-later

/****************************************************************************
 *
 * This module contains constant & macro definitions useful for block devices
 * handling and for file identification.
 *
 ****************************************************************************/

// file recognition tags (magic numbers)
#define DISKFILEID 0x00534952
#define FLASHFILEID 0x01534952
#define BIOSFILEID 0x02534952
#define COREFILEID 0x03534952
#define AOUTFILEID 0x04534952
#define STABFILEID 0x41534952

// DiskParams class items constants: position, min, max and default (DFL)
// values (where applicable) are given for each: see class definition

// number of parameters
#define DISKPNUM 6

// number of cylinders: 2 bytes (64 K)
#define CYLNUM 0
#define MAXCYL 0xFFFF
#define DFLCYL 32

// number of heads: 1 byte
#define HEADNUM 1
#define MAXHEAD 0xFF
#define DFLHEAD 2

// number of sectors: 1 byte
#define SECTNUM 2
#define MAXSECT 0xFF
#define DFLSECT 8

// 1 full rotation (in microseconds): here min and max values are given
#define ROTTIME 3
#define MINRPM 360
#define MAXRPM 10800
#define DFLROTTIME 16666

// average track-to-track seek time in microseconds
#define SEEKTIME 4
#define MAXSEEKTIME 10000
#define DFLSEEKTIME 100

// data percentage of sector: 1-100 (min and max values given)
#define DATASECT 5
#define MINDATAS 10
#define MAXDATAS 90
#define DFLDATAS 80

// FlashParams class items constants: position, min, max and default (DFL)
// values (where applicable) are given for each: see class definition

// number of parameters
#define FLASHPNUM 2

// number of blocks: 3 bytes
#define BLOCKSNUM 0
#define MAXBLOCKS 0xFFFFFF
#define DFLBLOCKS 512

// average write time in microseconds;
// read time will always be READRATIO to write time
#define WTIME 1
#define MAXWTIME MAXSEEKTIME * 10
#define DFLWTIME DFLSEEKTIME * 10
#define READRATIO 3 / 4
