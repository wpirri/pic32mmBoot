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

#include "config.h"
#include "log.h"
#include "pgm.h"

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
#pragma config OSCIOFNC = OFF    //OFF para RA4 como Digital I/O
#pragma config SOSCSEL = ON    //Secondary Oscillator External Clock Enable bit->SCLKI pin configured for Digital mode
#pragma config FCKSM = CSECME    //Clock Switching and Fail-Safe Clock Monitor Enable bits->Clock switching is enabled; Fail-safe clock monitor is enabled

// FSEC
#pragma config CP = OFF                 // Code Protection Enable bit (Code protection is disabled)
/* ************************************************************************** */

/* * Manejo de excepciones ************************************************** */
static enum {
    EXCEP_IRQ = 0,            // interrupt
    EXCEP_AdEL = 4,            // address error exception (load or ifetch)
    EXCEP_AdES,                // address error exception (store)
    EXCEP_IBE,                // bus error (ifetch)
    EXCEP_DBE,                // bus error (load/store)
    EXCEP_Sys,                // syscall
    EXCEP_Bp,                // breakpoint
    EXCEP_RI,                // reserved instruction
    EXCEP_CpU,                // coprocessor unusable
    EXCEP_Overflow,            // arithmetic overflow
    EXCEP_Trap,                // trap (possible divide by zero)
    EXCEP_IS1 = 16,            // implementation specfic 1
    EXCEP_CEU,                // CorExtend Unuseable
    EXCEP_C2E                // coprocessor 2
} _excep_code;


void __attribute__((weak, nomips16)) _general_exception_handler (void)
{
    static unsigned int _excep_addr;
    static char* _except_str;
    char str[80];
    
    asm volatile("mfc0 %0,$13" : "=r" (_excep_code));
    asm volatile("mfc0 %0,$14" : "=r" (_excep_addr));

    _excep_code = (_excep_code & 0x0000007C) >> 2;

    switch(_excep_code){
        case EXCEP_IRQ: _except_str = "interrupt"; break;
        case EXCEP_AdEL: _except_str = "address error exception (load or ifetch)"; break;
        case EXCEP_AdES: _except_str = "address error exception (store)"; break;
        case EXCEP_IBE: _except_str = "bus error (ifetch)"; break;
        case EXCEP_DBE: _except_str = "bus error (load/store)"; break;
        case EXCEP_Sys: _except_str = "syscall"; break;
        case EXCEP_Bp: _except_str = "breakpoint"; break;
        case EXCEP_RI: _except_str = "reserved instruction"; break;
        case EXCEP_CpU: _except_str = "coprocessor unusable"; break;
        case EXCEP_Overflow: _except_str = "arithmetic overflow"; break;
        case EXCEP_Trap: _except_str = "trap (possible divide by zero)"; break;
        case EXCEP_IS1: _except_str = "implementation specfic 1"; break;
        case EXCEP_CEU: _except_str = "CorExtend Unuseable"; break;
        case EXCEP_C2E: _except_str = "coprocessor 2"; break;
    }

    Log("[Exception] Compilacion " __DATE__ " " __TIME__ "\n");
    Log("[Exception] Compilador " __VERSION__ "\n");
    sprintf(str, "[Exception] %s\n", _except_str);
    Log(str);
    sprintf(str, "[Exception] Cde: 0x%04X Addr: 0x%08X\n", _excep_code, _excep_addr);
    Log(str);

    while(1);
}


/*****************************************************************************/
int main(void)
{
    FSFILE *boot_file = NULL;
    long bytecount;
    int byteread;
    int readretry;
    int validHex = 0;
    uint8_t ascii_buffer[80];
    uint8_t hex_rec[100];
    int rc;

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
/* Modulo CPP */
    CCP1CON1 = 0x00000000;    
    CCP2CON1 = 0x00000000;    
    CCP3CON1 = 0x00000000;    
    
/* ************************************************************************** */
/* Change notification */
    CNCONB = 0x00000000;
/* ************************************************************************** */
/* OSCILLATOR Init */
    // System Reg Unlock
    SYSKEY = 0x00000000; 
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
    // WDTO disabled; EXTR disabled; POR disabled; SLEEP disabled; BOR disabled; PORIO disabled; IDLE disabled; PORCORE disabled; BCFGERR disabled; CMR disabled; BCFGFAIL disabled; SWR disabled; 
    RCON = 0x0;
    // ON disabled; DIVSWEN disabled; RSLP disabled; ROSEL SYSCLK; OE disabled; SIDL disabled; RODIV 0; 
    REFO1CON = 0x0;
    // ROTRIM 0; 
    REFO1TRIM = 0x0;
    // SPDIVRDY disabled; 
    CLKSTAT = 0x0;
    // System Reg Lock
    SYSKEY = 0x00000000; 
/* ************************************************************************** */
/* Init RTCC */
    // System Reg Unlock
    SYSKEY = 0x00000000; //write invalid key to force lock
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
    SYSKEY = 0x00000000; //write invalid key to force lock
/* ************************************************************************** */
    RCON = 0x00000000;
/* ************************************************************************** */
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA4 = 0;
    
/* ************************************************************************** */
/* INTERRUPT Initialize */
    // Enable Multi Vector Configuration
    INTCONbits.MVEC = 1;
    __builtin_enable_interrupts();
/* ************************************************************************** */

    LogInit();
            
    NVM_Initialize();
    
    // Initialize the File System
    if(!FSInit())
    {
        Log("[main] No se pudo montar la terjeta SD.");
        /* Si no puedo iniciaizar la SD trato de saltar al programa */
        if(ValidAppPresent())
        {
            Log("[main] Iniciando programa previamente cargado.");
            JumpToApp();
        }
        else
        {
            //Indicate error and stay in while loop.
            Log("[main] No hay programa cargado.");
            Error(1);
        }
    }         

    STATUS_LED = 1;
    
    /* Siempre que haya un archivo en la SD lo cargo */
    boot_file = FSfopen(PROGRAM_FILE_NAME, "r");

    if(boot_file == NULL)// Make sure the file is present.
    {
        Log("[main] La tarjeta SD no tiene PGM.HEX.");
        if(ValidAppPresent())
        {
            Log("[main] Iniciando programa previamente cargado.");
            JumpToApp();
        }
        else
        {
            /* Trato de abrir un bacup */
            boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");

            if(boot_file == NULL)// Make sure the file is present.
            {
                Log("[main] La tarjeta SD no tiene PGMBK.HEX.");
                //Indicate error and stay in while loop.
                Error(2);
            }
        }
    }     

    Log("[main] PGM.HEX encontrado.");
    MODE_LED = 1;

#ifdef VERIFY_PROGRAM
    Log("[main] Verificando archivo PGM.HEX.");
    /* Verifico el archivo HEX */
    bytecount = 0;
    readretry = READ_RETRY;
    rc = 0;
    while(readretry && bytecount < boot_file->size && rc == 0 )
    {
        while((byteread = readLine(ascii_buffer, 80, boot_file)) > 0)
        {
            /* Cada vez que leo bien reseteo el contador de errores */
            readretry = READ_RETRY;
            /* Voy contando lo bytes leidos */
            bytecount += byteread;
            /* Salt?o los ':' iniciales */
            ConvertAsciiToHex(&ascii_buffer[1],hex_rec);

            rc = CheckHexRecord(hex_rec);
            if(rc < 0)
            {
                Log((char*)ascii_buffer);
                break;
            }
            else if(rc > 0)
            {
                validHex = 1;
                break;
            }
            // Blink LED
            BlinkLed(300);
        }//while(1)

        /* Me fijo si ley? todo */
        if(readretry && bytecount < boot_file->size && validHex == 0)
        {
            readretry--;
            /* Cierro el archivo */
            FSfclose(boot_file);
            /* Lo vuelvo a abrir */
            boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
            if( !boot_file) boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
            /* me paro donde se hab?a cortado */
            FSfseek(boot_file, bytecount, SEEK_SET);
        }

    }

    STATUS_LED = 0;

    /* Si el HEX no sirve */
    if(validHex == 0)
    {
        Log("[main] Archivo PGM.HEX invalido.");
        if(ValidAppPresent())
        {
            Log("[main] Iniciando programa previamente cargado.");
            JumpToApp();
        }
        else
        {
            Log("[main] No hay programa cargado.");
            //Indicate error and stay in while loop.
            Error(3);
        }
    }

    /* Cierro el archivo */
    FSfclose(boot_file);
    Log("[main] PGM.HEX verificado.");
    /* Lo vuelvo a abrir */
    boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
    if( !boot_file) boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
#endif /* VERIFY_PROGRAM */

    // Erase Flash (Block Erase the program Flash)
    EraseFlash();

    MODE_LED = 0;
    
    Log("[main] Cargando programa.");
    /* Un parche de reintentos para salvar errores de lectura de las SD */
    bytecount = 0;
    readretry = READ_RETRY;
    while(readretry && bytecount < boot_file->size)
    {
        while((byteread = readLine(ascii_buffer, 80, boot_file)) > 0)
        {
            /* Cada vez que leo bien reseteo el contador de errores */
            readretry = READ_RETRY;
            /* Voy contando lo bytes leidos */
            bytecount += byteread;
            /* Salt?o los ':' iniciales */
            ConvertAsciiToHex(&ascii_buffer[1],hex_rec);
            rc = WriteHexRecord2Flash(hex_rec);
            if(rc > 0)
            {
                /* Cuando encuentra el registro final sale con 1 */
                Log("[main] Fin de carga de programa.");
                JumpToApp();
            }
            else if(rc < 0)
            {
                Log((char*)ascii_buffer);
                Error(0-rc);
            }
            // Blink LED
            BlinkLed(300);
        }//while(1)

        /* Me fijo si ley? todo */
        if(readretry && bytecount < boot_file->size)
        {
            readretry--;
            /* Cierro el archivo */
            FSfclose(boot_file);
            /* Lo vuelvo a abrir */
            boot_file = FSfopen(PROGRAM_FILE_NAME, "r");
            if( !boot_file) boot_file = FSfopen(PROGRAM_FILE_NAME_BK, "r");
            /* me paro donde se hab?a cortado */
            FSfseek(boot_file, bytecount, SEEK_SET);
        }

    }
    /* Si pasa por ac? es un error en el archivo HEX */
    FSfclose(boot_file);
    Log("[main] El bootloader termino sin cargar programa.");
    Error(4);
    return 0;
}
