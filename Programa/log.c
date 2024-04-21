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
#include "log.h"

#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void LogInit(void)
{
/* ************************************************************************** */
/* Set the PPS */
    // System Reg Unlock
    SYSKEY = 0x12345678; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write Key1 to SYSKEY
    SYSKEY = 0x556699AA; //write Key2 to SYSKEY
    // unlock PPS
    RPCONbits.IOLOCK = 0;
    // Mapeo de UART2
    RPOR0bits.RP4R = 0x0004;    //RA3->UART2:U2TX
    RPINR9bits.U2RXR = 0x0000;    //RA2->NADA - ( 0x0003 RA2->UART2:U2RX )
    // lock   PPS
    RPCONbits.IOLOCK = 1; 
    // System Reg Lock
    SYSKEY = 0x00000000; 
/* ************************************************************************** */
/* UART2 */
   IEC1bits.U2TXIE = 0;
   IEC1bits.U2RXIE = 0;
   // STSEL 1; PDSEL 8N; RTSMD disabled; OVFDIS disabled; ACTIVE disabled; RXINV disabled; WAKE disabled; BRGH enabled; IREN disabled; ON enabled; SLPEN disabled; SIDL disabled; ABAUD disabled; LPBACK disabled; UEN TX_RX; CLKSEL PBCLK; 
   U2MODE = (0x8008 & ~(1<<15));  // disabling UART
   // UTXISEL TX_ONE_CHAR; UTXINV disabled; ADDR 0; MASK 0; URXEN disabled; OERR disabled; URXISEL RX_ONE_CHAR; UTXBRK disabled; UTXEN disabled; ADDEN disabled; 
   U2STA = 0x0;
   // BaudRate = 115200; Frequency = 24000000 Hz; BRG 51; 
   U2BRG = 0x33;
    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
   U2STASET = _U2STA_UTXEN_MASK;
   U2MODESET = _U2MODE_ON_MASK;  // enabling UART ON bit
   U2STASET = _U2STA_URXEN_MASK; 
}

void Log(const char* msg)
{
    while(*msg)
    {
        while(U2STAbits.UTXBF);
        U2TXREG = *msg++;
    }
}

