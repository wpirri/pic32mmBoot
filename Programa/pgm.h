/***************************************************************************
    Copyright (C) 2024   Walter Pirri

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/
#ifndef _PGM_H
#define _PGM_H

#include "sd/fsio.h"
#include "flash/plib_nvm.h"

#define ERASE_BLOCK_SIZE                        (2048UL)
#define DEV_CONFIG_REG_BASE_ADDRESS             (0xF80000)
#define DEV_CONFIG_REG_END_ADDRESS              (0xF80012)

#define REC_FLASHED 0
#define REC_NOT_FOUND 1
#define REC_FOUND_BUT_NOT_FLASHED 2

#define DATA_RECORD 		0
#define END_OF_FILE_RECORD 	1
#define EXT_SEG_ADRS_RECORD 2
#define EXT_LIN_ADRS_RECORD 4

#define WORD_ALIGN_MASK         (~(sizeof(uint32_t) - 1U))

int readLine(void *ptr, size_t max_len, FSFILE *stream);

BOOL ValidAppPresent(void);
void JumpToApp( void );

void EraseFlash(void);
int WriteHexRecord2Flash(uint8_t* HexRecord);
int CheckHexRecord(uint8_t* HexRecord);
void ConvertAsciiToHex(uint8_t* asciiRec, uint8_t* hexRec);

void RestartForRun(void);

#endif /* _PGM_H */
