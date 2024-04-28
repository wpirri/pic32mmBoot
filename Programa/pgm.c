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
/*
 *       ***** El HEX file debe estar normalizado *****
 *      https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_uart/blob/master/docs/GUID-3E6213D5-3312-49A9-A6C7-897B8AD57414.md
 */
#include <xc.h>

#include "pgm.h"
        
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "log.h"

typedef struct
{
    uint8_t *start;
    uint8_t len;
    uint8_t status;
}T_REC;

typedef struct 
{
	uint8_t RecDataLen;
	DWORD_VAL Address;
	uint8_t RecType;
	uint8_t* Data;
	uint8_t CheckSum;	
	DWORD_VAL ExtSegAddress;
	DWORD_VAL ExtLinAddress;
}T_HEX_RECORD;	

/* ************************************************************************** */
int readLine(void *ptr, size_t max_len, FSFILE *stream)
{
    int len = 0;
    char *p;
    int rec_len;

    p = ptr;

    while(FSfread(p,1,1,stream))
    {
        if(*p != 0x0d && *p != 0x0a) break;
    }
    
    /* Verifico que sea el inicio de l?nea */
    if(*p != ':') return 0;
    /* Avanzo el puntero */
    p++;
    len++;
    /* Leo el tipo de registro */
    FSfread(p,1,2,stream);
    p[2] = 0;
    /* Segun el tipo de registro me cargo el tama?o */
    switch((int)strtol(p, NULL, 16))
    {
        case 0x00:
            rec_len = 11;
            break;
        case 0x02:
            rec_len = 15;
            break;
        case 0x04:
            rec_len = 19;
            break;
        case 0x08:
            rec_len = 27;
            break;
        case 0x0c:
            rec_len = 35;
            break;
        case 0x10:
            rec_len = 43;
            break;
        default:
            return 0;
    }
    /* Avanzo el puntero */
    p += 2;
    len += 2;
    /* le saco los 3 caracteres ya leidos leidos */
    rec_len -= 3;
    /* Le agrego uno para que quede adentro el linefeed */
    rec_len += 1;
    /* si es un tipo de registro conocido leo todo el registro de una */
    if(FSfread(p,1,rec_len,stream) != rec_len) return 0;
    p[rec_len] = 0x00;
    len += rec_len;
    return len;
}

/* ************************************************************************** */
void __attribute__((optimize("-O0"))) JumpToApp( void )
{
    void (*fptr)(void);
    
    STATUS_LED = 1;
    MODE_LED = 1;
    AUX_LED = 0;
    Log("[JumpToApp] Iniciando....");
    for(unsigned long i = 10000000; i > 0; i--);

    fptr = (void (*)(void))KVA0_TO_KVA1(USER_APP_RESET_ADDRESS);

    (void) __builtin_disable_interrupts();

    fptr();
}

/* ************************************************************************** */
void ConvertAsciiToHex(uint8_t* asciiRec, uint8_t* hexRec)
{
	uint8_t i = 0;
	uint8_t k = 0;
	uint8_t hex;
	
	
	while((asciiRec[i] >= 0x30) && (asciiRec[i] <= 0x66))
	{
		// Check if the ascci values are in alpha numeric range.
		
		if(asciiRec[i] < 0x3A)
		{
			// Numerical reperesentation in ASCII found.
			hex = asciiRec[i] & 0x0F;
		}
		else
		{
			// Alphabetical value.
			hex = 0x09 + (asciiRec[i] & 0x0F);						
		}
	
		// Following logic converts 2 bytes of ASCII to 1 byte of hex.
		k = i%2;
		
		if(k)
		{
			hexRec[i/2] |= hex;
			
		}
		else
		{
			hexRec[i/2] = (hex << 4) & 0xF0;
		}	
		i++;		
	}		
	
}

/* ************************************************************************** */
void EraseFlash(void)
{
    Log("[EraseFlash] Blanqueando area de programa.");

    for(uint32_t flashAddr = APP_FLASH_BASE_ADDRESS;
        flashAddr < APP_FLASH_END_ADDRESS;
        flashAddr += ERASE_BLOCK_SIZE)
    {
        (void) NVM_PageErase(flashAddr);

        while(NVM_IsBusy()) {}

        // Blink LED to indicate erase is in progress.
        BlinkLed(10);
    }
}

/* ************************************************************************** */
int WriteHexRecord2Flash(uint8_t* HexRecord)
{
    static T_HEX_RECORD HexRecordSt;
    uint32_t WrData[2];
    uint32_t ProgAddress;
    uint8_t Checksum = 0;
    uint8_t i;

    HexRecordSt.RecDataLen = HexRecord[0];
    HexRecordSt.RecType = HexRecord[3];
    HexRecordSt.Data = &HexRecord[4];
	
    // Hex Record checksum check.
    for(i = 0; i < HexRecordSt.RecDataLen + 5; i++)
    {
            Checksum += HexRecord[i];
    }
	
    if(Checksum != 0)
    {
        //Error. Hex record Checksum mismatch.
        //Indicate Error by switching ON all LEDs.
        Log("[WriteHexRecord2Flash] Error de checksum en archivo HEX.");
        return (-5);
    }
    else
    {
        // Hex record checksum OK.
        switch(HexRecordSt.RecType)
        {
            case DATA_RECORD:  //Record Type 00, data record.
                HexRecordSt.Address.byte.MB = 0;
                HexRecordSt.Address.byte.UB = 0;
                HexRecordSt.Address.byte.HB = HexRecord[1];
                HexRecordSt.Address.byte.LB = HexRecord[2];
                // Derive the address.
                HexRecordSt.Address.Val = HexRecordSt.Address.Val + HexRecordSt.ExtLinAddress.Val + HexRecordSt.ExtSegAddress.Val;
                while(HexRecordSt.RecDataLen) // Loop till all bytes are done.
                {
                    // Convert the Physical address to Virtual address.
                    ProgAddress = (uint32_t)PA_TO_KVA0(HexRecordSt.Address.Val);
                    // Make sure we are not writing boot area and device configuration bits.
                    if( ((ProgAddress >= (uint32_t)APP_FLASH_BASE_ADDRESS) && (ProgAddress <= (uint32_t)APP_FLASH_END_ADDRESS))
                        && ((ProgAddress < (uint32_t)DEV_CONFIG_REG_BASE_ADDRESS) || (ProgAddress > (uint32_t)DEV_CONFIG_REG_END_ADDRESS)))
                    {
                        if(ProgAddress & 0x00000007)
                        {
                            Log("[WriteHexRecord2Flash] Address no alineada.");
                            return (-9);
                        }
                        WrData[0] = 0xFFFFFFFF;
                        WrData[1] = 0xFFFFFFFF;
                        /* First WORD */
                        memcpy(&WrData[0], HexRecordSt.Data, (HexRecordSt.RecDataLen < 4)?HexRecordSt.RecDataLen:4);
                        if(HexRecordSt.RecDataLen > 4)
                        {
                            memcpy(&WrData[1], HexRecordSt.Data+4, ((HexRecordSt.RecDataLen-4) < 4)?(HexRecordSt.RecDataLen-4):4);
                        }
                        // Write the data into flash.
                        // Assert on error. This must be caught during debug phase.
                        if( !NVM_DoubleWordWrite((uint32_t *)&WrData, ProgAddress))
                        {
                            Log("[WriteHexRecord2Flash] Error al escribir programa.");
                            return (-6);
                        }
                    }
                    // Increment the address.
                    HexRecordSt.Address.Val += 8;
                    // Increment the data pointer.
                    HexRecordSt.Data += 8;
                    // Decrement data len.
                    if(HexRecordSt.RecDataLen >= 8)
                    {
                        HexRecordSt.RecDataLen -= 8;
                    }
                    else
                    {
                        HexRecordSt.RecDataLen = 0;
                    }
                }
                break;
        case EXT_SEG_ADRS_RECORD:  // Record Type 02, defines 4th to 19th bits of the data address.
            HexRecordSt.ExtSegAddress.byte.MB = 0;
            HexRecordSt.ExtSegAddress.byte.UB = HexRecordSt.Data[0];
            HexRecordSt.ExtSegAddress.byte.HB = HexRecordSt.Data[1];
            HexRecordSt.ExtSegAddress.byte.LB = 0;
            // Reset linear address.
            HexRecordSt.ExtLinAddress.Val = 0;
            break;
        case EXT_LIN_ADRS_RECORD:   // Record Type 04, defines 16th to 31st bits of the data address.
            HexRecordSt.ExtLinAddress.byte.MB = HexRecordSt.Data[0];
            HexRecordSt.ExtLinAddress.byte.UB = HexRecordSt.Data[1];
            HexRecordSt.ExtLinAddress.byte.HB = 0;
            HexRecordSt.ExtLinAddress.byte.LB = 0;
            // Reset segment address.
            HexRecordSt.ExtSegAddress.Val = 0;
            break;
        case END_OF_FILE_RECORD:  //Record Type 01, defines the end of file record.
            HexRecordSt.ExtSegAddress.Val = 0;
            HexRecordSt.ExtLinAddress.Val = 0;
            return 1; /* Listo para saltar a la APP */
            break;
        default:
            HexRecordSt.ExtSegAddress.Val = 0;
            HexRecordSt.ExtLinAddress.Val = 0;
            break;
        }
    }
    return 0;
}	

/* ************************************************************************** */
BOOL ValidAppPresent(void)
{
	if(*((const uint32_t*)(USER_APP_RESET_ADDRESS)) == 0xFFFFFFFF)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}			

/* ************************************************************************** */
int CheckHexRecord(uint8_t* HexRecord)
{
    uint8_t Checksum = 0;
    uint8_t RecDataLen = HexRecord[0];
    uint8_t i;

    // Hex Record checksum check.
    for(i = 0; i < RecDataLen + 5; i++)
    {
            Checksum += HexRecord[i];
    }

    if(Checksum != 0)
    {
        Log("[CheckHexRecord] Error de CHKSUM.");
        /* Error de checksum en un registro */
        return (-1);
    }
    else
    {
        // Hex record checksum OK.
        switch(HexRecord[3])
        {
        case DATA_RECORD:  //Record Type 00, data record.
            /*
            if(HexRecord[2] & 0x00000007)
            {
                Log("[CheckHexRecord] Error de alineacion.");
                return (-1);
            }
            */
            break;
        case EXT_SEG_ADRS_RECORD:  // Record Type 02, defines 4th to 19th bits of the data address.
            break;
        case EXT_LIN_ADRS_RECORD:   // Record Type 04, defines 16th to 31st bits of the data address.
            break;
        case END_OF_FILE_RECORD:  //Record Type 01, defines the end of file record.
            return 1; /* Listo para saltar a la APP */
            break;
        default:
            break;
        }
    }
    /* Le falta el record final */
    return 0;
}
