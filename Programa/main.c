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
#include "flash/flash.h"

#define __DEBUG__
#define READ_RETRY 100
#define VERIFY_PROGRAM

#define PROGRAM_FILE_NAME       "pgm.hex"
#define PROGRAM_FILE_NAME_BK    "pgmbk.hex"


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

/* 
 * Translate a kernel address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((pa) | 0x80000000)
#define PA_TO_KVA1(pa)	((pa) | 0xa0000000)

/* translate between KSEG0 and KSEG1 addresses */
#define KVA0_TO_KVA1(v)	((v) | 0x20000000)
#define KVA1_TO_KVA0(v)	((v) & ~0x20000000)

#define WORD_ALIGN_MASK         (~(sizeof(uint32_t) - 1U))

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
/* Parámetros afectados en Linker file
 * _ebase_address
 * 
 * _RESET_ADDR
 * _BEV_EXCPT_ADDR
 * _DBG_EXCPT_ADDR
 * 
 * kseg0_program_mem
 * debug_exec_mem
 * kseg0_boot_mem
 * kseg1_boot_mem
 * 
 */
/* APP_FLASH_BASE_ADDRESS and APP_FLASH_END_ADDRESS reserves program Flash for the application*/
/* Rule:
 		1)The memory regions kseg0_program_mem, kseg0_boot_mem, exception_mem and
 		kseg1_boot_mem of the application linker script must fall with in APP_FLASH_BASE_ADDRESS
 		and APP_FLASH_END_ADDRESS

 		2)The base address and end address must align on  4K address boundary */

#define APP_FLASH_BASE_ADDRESS 	0x9D006000
#define APP_FLASH_END_ADDRESS   0x9D03FFFF

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
volatile unsigned long before_run_delay_count;

char* g_buffer[256];

void JumpToApp( void );
BOOL ValidAppPresent(void);
void EraseFlash(void);
int WriteHexRecord2Flash(UINT8* HexRecord);
int CheckHexRecord(UINT8* HexRecord);
void ConvertAsciiToHex(UINT8* asciiRec, UINT8* hexRec);
int readLine(void *ptr, size_t max_len, FSFILE *stream);
void BlinkLed(unsigned int period);
void Error( unsigned int err );
void Log(const char* msg);



/*****************************************************************************/
int main(void)
{
    volatile UINT i;
    g_boot_file = NULL;
    long bytecount;
    int byteread;
    int readretry;
    int validHex = 0;

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
    RPOR0bits.RP4R = 0x0004;    //RA3->UART2:U2TX
    RPINR9bits.U2RXR = 0x0000;    //RA2->NADA - ( 0x0003 RA2->UART2:U2RX )
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
/* ************************************************************************** */
    // System Reg Lock
    SYSKEY = 0x00000000; 
/* ************************************************************************** */
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA4 = 0;
    
    g_led_count = 0;
/* ************************************************************************** */
/* INTERRUPT Initialize */
    // Enable Multi Vector Configuration
    INTCONbits.MVEC = 1;
    asm volatile("ei $0");
/* ************************************************************************** */

    Log("PIC32MM0256 Boot Loader Init Ok\n");
            
    // Initialize the File System
    if(!FSInit())
    {
        Log("No se pudo montar la terjeta SD\n");
        /* Si no puedo iniciaizar la SD trato de saltar al programa */
        if(ValidAppPresent())
        {
            Log("Iniciando programa previamente cargado\n");
            JumpToApp();
        }
        else
        {
            //Indicate error and stay in while loop.
            Log("No hay programa cargado\n");
            Error(1);
        }
    }         

    STATUS_LED = 1;
    
    /* Siempre que haya un archivo en la SD lo cargo */
    g_boot_file = FSfopen(PROGRAM_FILE_NAME, "r");

    if(g_boot_file == NULL)// Make sure the file is present.
    {
        Log("La tarjeta SD no tiene PGM.HEX\n");
        if(ValidAppPresent())
        {
            Log("Iniciando programa previamente cargado\n");
            JumpToApp();
        }
        else
        {
            /* Trato de abrir un bacup */
            g_boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");

            if(g_boot_file == NULL)// Make sure the file is present.
            {
                Log("La tarjeta SD no tiene PGMBK.HEX\n");
                //Indicate error and stay in while loop.
                Error(2);
            }
        }
    }     

    Log("PGM.HEX encontrado\n");
    MODE_LED = 1;

#ifdef VERIFY_PROGRAM
    Log("Verificando archivo PGM.HEX\n");
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

    STATUS_LED = 0;

    /* Si el HEX no sirve */
    if(validHex == 0)
    {
        Log("Archivo PGM.HEX invalido\n");
        if(ValidAppPresent())
        {
            Log("Iniciando programa previamente cargado\n");
            JumpToApp();
        }
        else
        {
            Log("No hay programa cargado\n");
            //Indicate error and stay in while loop.
            Error(3);
        }
    }

    /* Cierro el archivo */
    FSfclose(g_boot_file);
    Log("PGM.HEX verificado\n");
    /* Lo vuelvo a abrir */
    g_boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
    if( !g_boot_file) g_boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
#endif /* VERIFY_PROGRAM */

    FLASH_Unlock(FLASH_UNLOCK_KEY);

    // Erase Flash (Block Erase the program Flash)
    EraseFlash();

    MODE_LED = 0;
    
    Log("Cargando programa\n");
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
                Log("Fin de carga de programa\n");
                if(ValidAppPresent())
                {
                    Log("Iniciando programa cargado\n");
                    JumpToApp();
                }
                else
                {
                    Log("No hay programa cargado\n");
                    Error(8);
                }
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
    Log("El bootloader termino sin cargar programa\n");
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
* Function: 	JumpToApp( void )
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
********************************************************************/
void JumpToApp( void )
{
    STATUS_LED = 1;
    MODE_LED = 1;
    AUX_LED = 0;

    void (*fptr)(void);

    fptr = (void (*)(void))USER_APP_RESET_ADDRESS;

    Log("Iniciando...\n");
    for(before_run_delay_count = 10000000; before_run_delay_count > 0; before_run_delay_count--);

    __builtin_disable_interrupts();

    /*fptr();*/ while(1);
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
	uint32_t pFlash;
    INT i;
    char str[256];

    Log("Blanqueando area de programa\n");
    pFlash = (uint32_t)APP_FLASH_BASE_ADDRESS;									
    for( i = 0; i < ((APP_FLASH_END_ADDRESS - APP_FLASH_BASE_ADDRESS + 1)/FLASH_PAGE_SIZE); i++ )
    {
        // Assert on NV error. This must be caught during debug phase.
#ifdef __DEBUG__
        sprintf(str, "Flash Erase Address: 0x%X\n", ( pFlash + (i*FLASH_PAGE_SIZE)));
        Log(str);
#endif
        if( !FLASH_ErasePage( pFlash + (i*FLASH_PAGE_SIZE) ))
        {
           // We have a problem. This must be caught during the debug phase.
            Log("Error al blanquear memoria de programa");
            Error(7);
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
    uint32_t WrData0;
    uint32_t WrData1;
    uint32_t ProgAddress;
    UINT8 Checksum = 0;
    UINT8 i;
    char str[256];

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
        Log("Error de checksum en archivo HEX\n");
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
                    ProgAddress = (uint32_t)PA_TO_KVA0(HexRecordSt.Address.Val);
                    // Make sure we are not writing boot area and device configuration bits.
                    if(((ProgAddress >= (uint32_t)APP_FLASH_BASE_ADDRESS) && (ProgAddress <= (uint32_t)APP_FLASH_END_ADDRESS))
                       && ((ProgAddress < (uint32_t)DEV_CONFIG_REG_BASE_ADDRESS) || (ProgAddress > (uint32_t)DEV_CONFIG_REG_END_ADDRESS)))
                    {
                        if(ProgAddress & 0x00000007)
                        {
                            /* Second WORD */
                            if(HexRecordSt.RecDataLen < 4)
                            {
                                // Sometimes record data length will not be in multiples of 4. Appending 0xFF will make sure that..
                                // we don't write junk data in such cases.
                                WrData1 = 0xFFFFFFFF;
                                memcpy(&WrData1, HexRecordSt.Data, HexRecordSt.RecDataLen);
                            }
                            else
                            {
                                memcpy(&WrData1, HexRecordSt.Data, 4);
                            }
                            WrData0 = FLASH_ReadWord(ProgAddress - 4);
                            ProgAddress -= 4;
                        }
                        else
                        {
                            /* First WORD */
                            if(HexRecordSt.RecDataLen < 4)
                            {
                                // Sometimes record data length will not be in multiples of 4. Appending 0xFF will make sure that..
                                // we don't write junk data in such cases.
                                WrData0 = 0xFFFFFFFF;
                                memcpy(&WrData0, HexRecordSt.Data, HexRecordSt.RecDataLen);
                            }
                            else
                            {
                                memcpy(&WrData0, HexRecordSt.Data, 4);
                            }
                            WrData1 = FLASH_ReadWord(ProgAddress + 4);
                        }
                        // Write the data into flash.
                        // Assert on error. This must be caught during debug phase.
#ifdef __DEBUG__
                        sprintf(str, "Flash Write Address: 0x%X Data0: 0x%08X Data1: 0x%08X\n", ProgAddress, WrData0, WrData1);
                        Log(str);
#endif
                        if( !FLASH_WriteDoubleWord(ProgAddress, WrData0, WrData1 ))
                        {
                            Log("Error al escribir programa\n");
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
	
	if(FLASH_ReadWord(USER_APP_RESET_ADDRESS) == 0xFFFFFFFF)
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

void Log(const char* msg)
{
    while(*msg)
    {
        while(U2STAbits.UTXBF);
        U2TXREG = *msg++;
    }
}
