Bootloader para PIC32MM

Carga el programa desde pgm.hex desde una memoria SD o MicroSD.

El programa a cargar debe ser linkeado utilizando el archivo app_p32MM0256GPM028.ld como linkfile y renombrar como pgm.hex el HEX file generado.

El archivo pgm.hex del programa a ser cargado debe generarse con la opción de normalizado habilitada (Properties->Building->Normalize hex file)

El programa a cargar debe tener los mismo bits de configuración que el bottloader (ver inicio de main.c)

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

#pragma config FWDTEN = ON    //Watchdog Timer Enable bit->WDT is enabled



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
