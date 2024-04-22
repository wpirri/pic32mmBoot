/**
  FLASH Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    flash.h

  @Summary
    This is the generated header file for the FLASH driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for FLASH.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  PIC32MM0256GPM028
        Driver Version    :  1.00
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/

#ifndef FLASH_H
#define FLASH_H


#include <stdint.h>
#include <stdbool.h>
//-------------------------------

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

#define FLASH_PC_UNITS_PER_INSTRUCTION 4
#define FLASH_WORD_WRITE_SIZE_IN_INSTRUCTIONS 2

#define  WRITE_DWORD_CODE     0x4002
#define  ERASE_PAGE_CODE      0x4004
#define  FLASH_WRITE_ROW_CODE 0x4003
#define  FLASH_NOP            0x4000

#define FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS  64
#define FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS (FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS*8)

#define FLASH_WRITE_ROW_SIZE_IN_PC_UNITS (FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS*4)
#define FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS  (FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*4)

#define FLASH_ERASE_PAGE_MASK    (~((FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS) - 1)) 

/* FLASH_ErasePage:   Erases a single page of flash.
 *       Parameter address:  Address of the page, must be page aligned.
 * 
 *       returns    true   for success
 *                  false for failure
 *                   */
bool     FLASH_ErasePage(uint32_t address);    

/* FLASH_ReadWord:   Reads a single word of flash from memory.
 *       Parameter address:  Address of the flash.  Must be word aligned.
 * 
 *       returns    value of the flash. */
uint32_t FLASH_ReadWord(uint32_t address);

/* FLASH_WriteDoubleWord:   Writes two words of data to flash.
 *       Parameter address:  Address of the flash.  Must be double word aligned.
 * 
 *                  data0, data1:  The two words of flash to write.
 * 
  *       returns   true   for success
 *                  false for failure
 *                   */

bool     FLASH_WriteDoubleWord(uint32_t address, uint32_t Data0, uint32_t Data1  );

/* FLASH_WriteRow:   Writes a single row of data from the location given in *data to
 *                   the flash location in address.  
 *                   The address in *data must be row aligned.
  *       returns    true   for success
 *                   false for failure
 *                   */
bool     FLASH_WriteRow(uint32_t address, uint32_t *data);

/* FLASH_ClearError:   Clears any pending error on the flash controller.
 *                     returns true if successful */
bool     FLASH_ClearError();
uint16_t FLASH_GetErasePageOffset(uint32_t address);
uint32_t FLASH_GetErasePageAddress(uint32_t address);


#endif	/* FLASH_H */
