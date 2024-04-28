/* 
 * File:   config.h
 * Author: walte
 *
 * Created on 10 de abril de 2024, 13:42
 */

#ifndef CONFIG_H
#define	CONFIG_H

#define __DEBUG__
#define READ_RETRY 100
#define VERIFY_PROGRAM

#define PROGRAM_FILE_NAME       "PGM.HEX"
#define PROGRAM_FILE_NAME_BK    "PGMBK.HEX"

/* ************************************************************************** */
/* Estos valores dependen de lo definido en el Linker Script                  */
/* ************************************************************************** */
/* ParÃ¯Â¿Â½metros afectados en Linker file
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
#define BOOTLOADER_SIZE 0x6000
/* APP_FLASH_BASE_ADDRESS and APP_FLASH_END_ADDRESS reserves program Flash for the application*/
/* Rule:
 		1)The memory regions kseg0_program_mem, kseg0_boot_mem, exception_mem and
 		kseg1_boot_mem of the application linker script must fall with in APP_FLASH_BASE_ADDRESS
 		and APP_FLASH_END_ADDRESS

 		2)The base address and end address must align on  4K address boundary */

#define APP_FLASH_BASE_ADDRESS 	(0x9D000000 + BOOTLOADER_SIZE)
#define APP_FLASH_END_ADDRESS   0x9D03FFFF

/* Address of  the Flash from where the application starts executing */
/* Rule: Set APP_FLASH_BASE_ADDRESS to _RESET_ADDR value of application linker script*/

// For PIC32MX1xx and PIC32MX2xx Controllers only
#define USER_APP_RESET_ADDRESS 	(APP_FLASH_BASE_ADDRESS + 0x1000)

#define STATUS_LED      LATAbits.LATA0
#define MODE_LED        LATAbits.LATA1
#define AUX_LED         LATAbits.LATA4

// Clock frequency values
#define _XTAL_FREQ  24000000UL             // Hz
#define GetSystemClock()		(_XTAL_FREQ)
#define GetInstructionClock()	(GetSystemClock()/*/4*/)
#define GetPeripheralClock()	(GetSystemClock()/*/4*/)

#endif	/* CONFIG_H */

