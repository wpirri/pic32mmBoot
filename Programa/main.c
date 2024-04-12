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
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd/fsio.h"
#include "flash/NVMem.h"

/*
 RA0    Led status  (azul)
 RA1    Led modo    (verde)
 RA4    Led aux     (amarillo)
 RB0    SPI2:SDO2
 RB1    SPI2:SDI2
 RA2    SPI2:SCK2OUT
 RA2    SPI2:SCK2OUT
 */

/* ************************************************************************** */
// FDEVOPT
#pragma config SOSCHP = OFF    //Secondary Oscillator High Power Enable bit->SOSC oprerates in normal power mode.
#pragma config ALTI2C = OFF    //Alternate I2C1 Pins Location Enable bit->Primary I2C1 pins are used
#pragma config FUSBIDIO = OFF    //USBID pin control->USBID pin is controlled by the USB module
#pragma config FVBUSIO = OFF    //VBUS Pin Control->VBUS pin is controlled by the USB module

// FICD
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)
#pragma config ICS = PGx1               // ICE/ICD Communication Channel Selection bits (Communicate on PGEC3/PGED3)

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config RETVR = OFF              // Retention Voltage Regulator Enable bit (Retention regulator is disabled)
#pragma config LPBOREN = ON    //Downside Voltage Protection Enable bit->Low power BOR is enabled, when main BOR is disabled

// FWDT
#pragma config SWDTPS = PS1048576    //Sleep Mode Watchdog Timer Postscale Selection bits->1:1048576
#pragma config FWDTWINSZ = PS25_0    //Watchdog Timer Window Size bits->Watchdog timer window size is 25%
#pragma config WINDIS = OFF    //Windowed Watchdog Timer Disable bit->Watchdog timer is in non-window mode
#pragma config RWDTPS = PS1024    //Run Mode Watchdog Timer Postscale Selection bits->1:1024
#pragma config RCLKSEL = LPRC    //Run Mode Watchdog Timer Clock Source Selection bits->Clock source is LPRC (same as for sleep mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT is disabled)

// FOSCSEL
#pragma config FNOSC = PLL    //Oscillator Selection bits->Primary or FRC oscillator with PLL
#pragma config PLLSRC = FRC    //System PLL Input Clock Selection bit->FRC oscillator is selected as PLL reference input on device reset
#pragma config SOSCEN = OFF    //Secondary Oscillator Enable bit->Secondary oscillator is disabled
#pragma config IESO = ON    //Two Speed Startup Enable bit->Two speed startup is enabled
#pragma config POSCMOD = OFF    //Primary Oscillator Selection bit->Primary oscillator is disabled
#pragma config OSCIOFNC = OFF    //System Clock on CLKO Pin Enable bit->OSCO pin operates as a normal I/O
#pragma config SOSCSEL = ON    //Secondary Oscillator External Clock Enable bit->SCLKI pin configured for Digital mode
#pragma config FCKSM = CSECME    //Clock Switching and Fail-Safe Clock Monitor Enable bits->Clock switching is enabled; Fail-safe clock monitor is enabled

// FSEC
#pragma config CP = OFF                 // Code Protection Enable bit (Code protection is disabled)
/* ************************************************************************** */
#define STATUS_LED      LATAbits.LATA0
#define MODE_LED        LATAbits.LATA1
#define AUX_LED         LATAbits.LATA4

#define PROGRAM_FILE_NAME       "pgm.hex"
#define PROGRAM_FILE_NAME_BK    "pgmbk.hex"
#define VERIFY_PROGRAM

/******************************************************************************
Macros used in this file
*******************************************************************************/
#define AUX_FLASH_BASE_ADRS             (0x7FC000)
#define AUX_FLASH_END_ADRS              (0x7FFFFF)
#define DEV_CONFIG_REG_BASE_ADDRESS     (0xF80000)
#define DEV_CONFIG_REG_END_ADDRESS      (0xF80012)

/* ************************************************************************** */
/* Estos valores dependen de lo definido en el Linker Script                  */
/* ************************************************************************** */
/* APP_FLASH_BASE_ADDRESS and APP_FLASH_END_ADDRESS reserves program Flash for the application*/
/* Rule:
 		1)The memory regions kseg0_program_mem, kseg0_boot_mem, exception_mem and
 		kseg1_boot_mem of the application linker script must fall with in APP_FLASH_BASE_ADDRESS
 		and APP_FLASH_END_ADDRESS

 		2)The base address and end address must align on  4K address boundary */

#define APP_FLASH_BASE_ADDRESS 	0x9D005000
#define APP_FLASH_END_ADDRESS   0x9D003FFF

/* Address of  the Flash from where the application starts executing */
/* Rule: Set APP_FLASH_BASE_ADDRESS to _RESET_ADDR value of application linker script*/

// For PIC32MX1xx and PIC32MX2xx Controllers only
#define USER_APP_RESET_ADDRESS 	(APP_FLASH_BASE_ADDRESS + 0x1000)

#define REC_FLASHED 0
#define REC_NOT_FOUND 1
#define REC_FOUND_BUT_NOT_FLASHED 2

/* ************************************************************************** */
#define DATA_RECORD 		0
#define END_OF_FILE_RECORD 	1
#define EXT_SEG_ADRS_RECORD 2
#define EXT_LIN_ADRS_RECORD 4

#define READ_RETRY 100

typedef struct
{
    UINT8 *start;
    UINT8 len;
    UINT8 status;
}T_REC;

typedef struct 
{
	UINT8 RecDataLen;
	DWORD_VAL Address;
	UINT8 RecType;
	UINT8* Data;
	UINT8 CheckSum;	
	DWORD_VAL ExtSegAddress;
	DWORD_VAL ExtLinAddress;
}T_HEX_RECORD;	

/* ************************************************************************** */

FSFILE * g_boot_file;
unsigned int    g_led_count;

UINT8 g_ascii_buffer[80];
UINT8 g_hex_rec[100];

void JumpToApp(void);
BOOL ValidAppPresent(void);
void EraseFlash(void);
int WriteHexRecord2Flash(UINT8* HexRecord);
int CheckHexRecord(UINT8* HexRecord);
void ConvertAsciiToHex(UINT8* asciiRec, UINT8* hexRec);
int readLine(void *ptr, size_t max_len, FSFILE *stream);
void BlinkLed(unsigned int period);
void Error( unsigned int err );



/*****************************************************************************/
int main(void)
{
    volatile UINT i;
    g_boot_file = NULL;
    long bytecount;
    int byteread;
    int readretry;
    int validHex = 0;

    INTCONbits.MVEC = 1;
    asm volatile("ei $0");

/* ************************************************************************** */
/*  Setting the Output Latch SFR(s) */
    LATA = 0x00000000;
    LATB = 0x00000000;
    LATC = 0x00000000;

/* ************************************************************************** */
/* Setting the GPIO Direction SFR(s) */
    TRISA = 0xFFFFFFFF;
    TRISB = 0xFFFFFFFF;
    TRISC = 0xFFFFFFFF;
    
/* ************************************************************************** */
/* Setting the Weak Pull Up and Weak Pull Down SFR(s) */
    CNPDA = 0x00000000;
    CNPDB = 0x00000000;
    CNPDC = 0x00000000;
    CNPUA = 0x00000000;
    CNPUB = 0x00000000;
    CNPUC = 0x00000000;

/* ************************************************************************** */
/* Setting the Open Drain SFR(s) */
    ODCA = 0x00000000;
    ODCB = 0x00000000;
    ODCC = 0x00000000;

/* ************************************************************************** */
/* Setting the Analog/Digital Configuration SFR(s) */
    ANSELA = 0x00000000;
    ANSELB = 0x00000000;

/* ************************************************************************** */
/* Mï¿½dulo CPP */
    CCP1CON1 = 0x00000000;    
    CCP2CON1 = 0x00000000;    
    CCP3CON1 = 0x00000000;    
    
/* ************************************************************************** */
/* Change notification */
    CNCONB = 0x00000000;

/* ************************************************************************** */
/* Set the PPS */
    // System Reg Unlock
    SYSKEY = 0x12345678; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write Key1 to SYSKEY
    SYSKEY = 0x556699AA; //write Key2 to SYSKEY
    // unlock PPS
    RPCONbits.IOLOCK = 0;
    /* Agregado para mapeo de SPI */
    RPOR1bits.RP6R = 0x0008;        //RB0->SPI2:SDO2
    RPINR11bits.SDI2R = 0x0007;     //RB1->SPI2:SDI2
    RPOR0bits.RP3R = 0x0009;        //RA2->SPI2:SCK2OUT
    RPINR11bits.SCK2INR = 0x0003;   //RA2->SPI2:SCK2OUT
    /* Agregado para mapeo de SPI */
    // lock   PPS
    RPCONbits.IOLOCK = 1; 
    // System Reg Lock
    SYSKEY = 0x00000000; 
/* ************************************************************************** */
/* OSCILLATOR Init */
    // System Reg Unlock
    SYSKEY = 0x12345678; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write Key1 to SYSKEY
    SYSKEY = 0x556699AA; //write Key2 to SYSKEY
    // CF No Clock Failure; FRCDIV FRC/1; SLPEN Device will enter Idle mode when a WAIT instruction is issued; NOSC SPLL; SOSCEN disabled; CLKLOCK Clock and PLL selections are not locked and may be modified; OSWEN Complete; 
    OSCCON = 0x100;
    // TUN Center frequency; 
    OSCTUN = 0x0;
    // PLLODIV 1:1; PLLMULT 3x; PLLICLK FRC; 
    SPLLCON = 0x10080;
    // SWRST disabled; 
    RSWRST = 0x0;
    // WDTO disabled; GNMI disabled; CF disabled; WDTS disabled; NMICNT 0; LVD disabled; SWNMI disabled; 
    RNMICON = 0x0;
    // SBOREN disabled; VREGS disabled; RETEN disabled; 
    PWRCON = 0x0;
    // System Reg Lock
    SYSKEY = 0x00000000; 
    // WDTO disabled; EXTR disabled; POR disabled; SLEEP disabled; BOR disabled; PORIO disabled; IDLE disabled; PORCORE disabled; BCFGERR disabled; CMR disabled; BCFGFAIL disabled; SWR disabled; 
    RCON = 0x0;
    // ON disabled; DIVSWEN disabled; RSLP disabled; ROSEL SYSCLK; OE disabled; SIDL disabled; RODIV 0; 
    REFO1CON = 0x0;
    // ROTRIM 0; 
    REFO1TRIM = 0x0;
    // SPDIVRDY disabled; 
    CLKSTAT = 0x0;
/* ************************************************************************** */
/* Init RTCC */
    // System Reg Unlock
    SYSKEY = 0x12345678; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write Key1 to SYSKEY
    SYSKEY = 0x556699AA; //write Key2 to SYSKEY

    RTCCON1CLR = (1 << _RTCCON1_WRLOCK_POSITION);
    RTCCON1CLR = (1 << _RTCCON1_ON_POSITION);
    // set 2024-01-01 00-00-00
    RTCDATE = 0x24010101; // Year/Month/Date/Wday
    RTCTIME = 0x000000; //  hours/minutes/seconds
    // ON enabled; OUTSEL Alarm Event; WRLOCK disabled; AMASK Every Half Second; ALMRPT 0; RTCOE disabled; CHIME disabled; ALRMEN disabled; 
    RTCCON1 = 0x8000;
    // DIV 15999; CLKSEL LPRC; FDIV 0; 
    RTCCON2 = 0x3E7F0001;
    // Enable RTCC 
    RTCCON1SET = (1 << _RTCCON1_ON_POSITION);
    RTCCON1SET = (1 << _RTCCON1_WRLOCK_POSITION);

    // System Reg Lock
    SYSKEY = 0x00000000; 
/* ************************************************************************** */
/* INTERRUPT Initialize */
    // Enable Multi Vector Configuration
    INTCONbits.MVEC = 1;
/* ************************************************************************** */

    
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA4 = 0;
    
    g_led_count = 0;

    // Initialize the File System
    if(!FSInit())
    {
        /* Si no puedo iniciaizar la SD trato de saltar al programa */
        if(ValidAppPresent())
        {
            JumpToApp();
        }
        else
        {
            //Indicate error and stay in while loop.
            Error(1);
        }
    }         

    STATUS_LED = 1;
    
    /* Siempre que haya un archivo en la SD lo cargo */
    g_boot_file = FSfopen(PROGRAM_FILE_NAME, "r");

    if(g_boot_file == NULL)// Make sure the file is present.
    {
        if(ValidAppPresent())
        {
            JumpToApp();
        }
        else
        {
            /* Trato de abrir un bacup */
            g_boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");

            if(g_boot_file == NULL)// Make sure the file is present.
            {
                //Indicate error and stay in while loop.
                Error(2);
            }
        }
    }     

    MODE_LED = 1;

#ifdef VERIFY_PROGRAM
    /* Verifico el archivo HEX */
    bytecount = 0;
    readretry = READ_RETRY;
    while(readretry && bytecount < g_boot_file->size && validHex == 0)
    {
        while((byteread = readLine(g_ascii_buffer, 80, g_boot_file)) > 0)
        {
            /* Cada vez que leo bien reseteo el contador de errores */
            readretry = READ_RETRY;
            /* Voy contando lo bytes leidos */
            bytecount += byteread;
            /* Salt?o los ':' iniciales */
            ConvertAsciiToHex(&g_ascii_buffer[1],g_hex_rec);

            if( CheckHexRecord(g_hex_rec))
            {
                validHex = 1;
                break;
            }
            // Blink LED
            BlinkLed(300);
        }//while(1)

        /* Me fijo si ley? todo */
        if(readretry && bytecount < g_boot_file->size && validHex == 0)
        {
            readretry--;
            /* Cierro el archivo */
            FSfclose(g_boot_file);
            /* Lo vuelvo a abrir */
            g_boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
            if( !g_boot_file) g_boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
            /* me paro donde se hab?a cortado */
            FSfseek(g_boot_file, bytecount, SEEK_SET);
        }

    }

    /* Si el HEX no sirve */
    if(validHex == 0)
    {
        if(ValidAppPresent())
        {
            JumpToApp();
        }
        else
        {
            //Indicate error and stay in while loop.
            Error(3);
        }
    }

    /* Cierro el archivo */
    FSfclose(g_boot_file);
    /* Lo vuelvo a abrir */
    g_boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
    if( !g_boot_file) g_boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
#endif /* VERIFY_PROGRAM */
    // Erase Flash (Block Erase the program Flash)
    EraseFlash();

    /* Un parche de reintentos para salvar errores de lectura de las SD */
    bytecount = 0;
    readretry = READ_RETRY;
    while(readretry && bytecount < g_boot_file->size)
    {
        while((byteread = readLine(g_ascii_buffer, 80, g_boot_file)) > 0)
        {
            /* Cada vez que leo bien reseteo el contador de errores */
            readretry = READ_RETRY;
            /* Voy contando lo bytes leidos */
            bytecount += byteread;
            /* Salt?o los ':' iniciales */
            ConvertAsciiToHex(&g_ascii_buffer[1],g_hex_rec);

            if(WriteHexRecord2Flash(g_hex_rec))
            {
                /* Cuando encuentra el registro final sale con 1 */
                STATUS_LED = 0;
                MODE_LED = 0;
                AUX_LED = 0;
                JumpToApp();
            }
            // Blink LED
            BlinkLed(300);
        }//while(1)

        /* Me fijo si ley? todo */
        if(readretry && bytecount < g_boot_file->size)
        {
            readretry--;
            /* Cierro el archivo */
            FSfclose(g_boot_file);
            /* Lo vuelvo a abrir */
            g_boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
            if( !g_boot_file) g_boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
            /* me paro donde se hab?a cortado */
            FSfseek(g_boot_file, bytecount, SEEK_SET);
        }

    }
    /* Si pasa por ac? es un error en el archivo HEX */
    FSfclose(g_boot_file);
    Error(4);
    return 0;
}

/******************************************************************************
 * 
 *****************************************************************************/
void BlinkLed(unsigned int period)
{
    if( !((++g_led_count) % period) ) AUX_LED ^= 1;
}

/******************************************************************************
 * 
 *****************************************************************************/
void __attribute__((optimize("-O0"))) Error( unsigned int err )
{
    volatile unsigned long delay_count;
    unsigned int pulse_count;
    unsigned int loop_count;
    /* Valores de err:
     * 1: No hay SD y no hay programa en el micro
     * 2: No hay programa en la SD ni en el micro
     * 3: El programa en la SD no es valido y no hay programa en el micro
     * 4: Error de lectura cargando programa
     * 5: Error de checksum cargando programa
     */
    AUX_LED = 0;
    pulse_count = err;
    loop_count = 10;
    for(delay_count = 1000000; delay_count > 0; delay_count--);
    while(1)
    {
        for(delay_count = 1000000; delay_count > 0; delay_count--);
        if(pulse_count)
        {
            AUX_LED = 1;
            pulse_count--;
        }
        loop_count--;
        for(delay_count = 1000000; delay_count > 0; delay_count--);
        AUX_LED = 0;
        if( !loop_count)
        {
            pulse_count = err;
            loop_count = 10;
        }
    }
}

/********************************************************************
* Function: 	JumpToApp()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		
*
* Side Effects:	No return from here.
*
* Overview: 	Jumps to application.
*
*			
* Note:		 	None.
* void JumpToApp(void)
 * {
 *  // Cargar la dirección de inicio de la aplicación en el registro t9
 *  asm volatile ("lui t9, 0x2000");   
 *  // Saltar a la dirección de inicio de la aplicación
 *  asm volatile ("jr t9");            
 * }
********************************************************************/
void JumpToApp(void)
{
#ifdef ALLOW_WRITES
    int bkp = 0;
    char backup_name[TOTAL_FILE_SIZE+1];

    /* Reenombro el archivo del programa para no cargarlo de nuevo */
    strcpy(backup_name, PROGRAM_FILE_NAME);
    if(g_boot_file)
    {
        while(bkp < 1000)
        {
            sprintf( (char*)(strchr(backup_name, '.')+1), "%03i", bkp );
            if(FSrename(backup_name, g_boot_file) == 0) break;
            bkp++;
        }
        if(bkp == 1000)
        {
            FSfclose(g_boot_file);
            FSremove(PROGRAM_FILE_NAME);
        }
    }
#endif /* ALLOW_WRITES */

    /* Antes de saltar a la aplicacion deben estar todas las interrupciones inhabilitadas */

    /* Salto */
    void (*fptr)(void);
    fptr = (void (*)(void))USER_APP_RESET_ADDRESS;
    fptr();
}	

/********************************************************************
* Function: 	ConvertAsciiToHex()
*
* Precondition: 
*
* Input: 		Ascii buffer and hex buffer.
*
* Output:		
*
* Side Effects:	No return from here.
*
* Overview: 	Converts ASCII to Hex.
*
*			
* Note:		 	None.
********************************************************************/
void ConvertAsciiToHex(UINT8* asciiRec, UINT8* hexRec)
{
	UINT8 i = 0;
	UINT8 k = 0;
	UINT8 hex;
	
	
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
// Do not change this
#define FLASH_PAGE_SIZE 0x1000
/********************************************************************
* Function: 	EraseFlash()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		
*
* Side Effects:	No return from here.
*
* Overview: 	Erases Flash (Block Erase).
*
*			
* Note:		 	None.
********************************************************************/
void EraseFlash(void)
{
	void * pFlash;
    UINT result;
    INT i;

    pFlash = (void*)APP_FLASH_BASE_ADDRESS;									
    for( i = 0; i < ((APP_FLASH_END_ADDRESS - APP_FLASH_BASE_ADDRESS + 1)/FLASH_PAGE_SIZE); i++ )
    {
	     result = NVMemErasePage( pFlash + (i*FLASH_PAGE_SIZE) );
        // Assert on NV error. This must be caught during debug phase.

        if(result != 0)
        {
           // We have a problem. This must be caught during the debug phase.
            while(1);
        } 
        // Blink LED to indicate erase is in progress.
        BlinkLed(10);
    }			           	     
}

/********************************************************************
* Function: 	WriteHexRecord2Flash()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		
*
* Side Effects:	No return from here.
*
* Overview: 	Writes Hex Records to Flash.
*
*			
* Note:		 	None.
********************************************************************/
int WriteHexRecord2Flash(UINT8* HexRecord)
{
    static T_HEX_RECORD HexRecordSt;
    UINT8 Checksum = 0;
    UINT8 i;
    UINT WrData;
    UINT RdData;
    void* ProgAddress;
    UINT result;

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
        Error(5);
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
                    ProgAddress = (void *)PA_TO_KVA0(HexRecordSt.Address.Val);
                    // Make sure we are not writing boot area and device configuration bits.
                    if(((ProgAddress >= (void *)APP_FLASH_BASE_ADDRESS) && (ProgAddress <= (void *)APP_FLASH_END_ADDRESS))
                       && ((ProgAddress < (void*)DEV_CONFIG_REG_BASE_ADDRESS) || (ProgAddress > (void*)DEV_CONFIG_REG_END_ADDRESS)))
                    {
                        if(HexRecordSt.RecDataLen < 4)
                        {
                            // Sometimes record data length will not be in multiples of 4. Appending 0xFF will make sure that..
                            // we don't write junk data in such cases.
                            WrData = 0xFFFFFFFF;
                            memcpy(&WrData, HexRecordSt.Data, HexRecordSt.RecDataLen);
                        }
                        else
                        {
                            memcpy(&WrData, HexRecordSt.Data, 4);
                        }
                        // Write the data into flash.
                        result = NVMemWriteWord(ProgAddress, WrData);
                        // Assert on error. This must be caught during debug phase.
                        if(result != 0)
                        {
                            Error(6);
                        }
                    }
                    // Increment the address.
                    HexRecordSt.Address.Val += 4;
                    // Increment the data pointer.
                    HexRecordSt.Data += 4;
                    // Decrement data len.
                    if(HexRecordSt.RecDataLen > 3)
                    {
                        HexRecordSt.RecDataLen -= 4;
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

/********************************************************************
* Function: 	ValidAppPresent()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		TRUE: If application is valid.
*
* Side Effects:	None.
*
* Overview: 	Logic: Check application vector has 
				some value other than "0xFFFFFF"
*
*			
* Note:		 	None.
********************************************************************/
BOOL ValidAppPresent(void)
{
	volatile UINT32 *AppPtr;
	
	AppPtr = (UINT32*)USER_APP_RESET_ADDRESS;

	if(*AppPtr == 0xFFFFFFFF)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}			

/********************************************************************
* Function: 	readLine()
*
* Precondition:
*
* Input:
*
* Output:
*
* Side Effects:	None.
*
* Overview:
*
*
* Note:
********************************************************************/
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

/********************************************************************
 *
 ********************************************************************/
int CheckHexRecord(UINT8* HexRecord)
{
    UINT8 Checksum = 0;
    UINT8 RecDataLen;
    UINT8 i;

    // Hex Record checksum check.
    for(i = 0; i < RecDataLen + 5; i++)
    {
            Checksum += HexRecord[i];
    }

    if(Checksum != 0)
    {
        /* Error de checksum en un registro */
        return 0;
    }
    else
    {
        // Hex record checksum OK.
        switch(HexRecord[3])
        {
            case DATA_RECORD:  //Record Type 00, data record.
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
