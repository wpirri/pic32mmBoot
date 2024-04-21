/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/


#include "xc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "flash.h"
#include "../log.h"

#define PHYS_ADDR(x)  (x & 0x1FFFFFFF)

#define PH_FLASH_BASE_ADDRESS 	0x1D006000
#define PH_FLASH_END_ADDRESS    0x1D03FFFF
#define FLASH_UPPER_ADDRESS     PH_FLASH_END_ADDRESS

void delay_us(unsigned int us)
{
    us *= 10;
    while (us--) _nop();
}  

static uint32_t FLASH_NVMUnlock(uint32_t operation)
{
    unsigned int status;

    // Suspend or Disable all Interrupts
    asm volatile ("di %0" : "=r" (status));

    // Flash operation to perform
    NVMCON = operation;

    // Data sheet prescribes 6us delay for LVD to become stable.
    // To be on the safer side, we shall set 7us delay.
    delay_us(7);

    // Write Keys
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;

    // Start the operation using the Set Register
    NVMCONSET = 0x8000;

    // Wait for operation to complete
    while (NVMCON & 0x8000)
    {
    };

    // Restore Interrupts
    if ((status & 0x00000001) == 1)
    {
        asm volatile ("ei");
    }
    else
    {
        asm volatile ("di");
    }

    // Disable NVM write enable
    NVMCONCLR = 0x0004000;

    // Return WRERR and LVDERR Error Status Bits}
    return (NVMCON & 0x3000);
}
    
bool FLASH_ClearError()
{
    bool res;

    // Set NVMADDR to the Start Address of page to erase
    NVMADDR = (unsigned int) 0;
    
    // Unlock and Erase Page
    res = FLASH_NVMUnlock(0x4000);
    
    // Return Result
    return (res == 0);
}   


bool FLASH_ErasePage(uint32_t address)
{
    uint32_t phaddr = PHYS_ADDR(address);

    char str[256];
    sprintf(str, "[FLASH_ErasePage] PH: 0x%X\n", phaddr);
    Log(str);

    // Make sure request is page aligned.
    if ( (address & ((FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS)-1)) != 0 )
    {
        Log("[FLASH_ErasePage] Address Error\n");
        return false;
    }
    
    // Set NVMADDR to the Start Address of page to erase
    NVMADDR = phaddr;
    
    // Unlock and Erase Page
    if(FLASH_NVMUnlock(ERASE_PAGE_CODE))
    {
        Log("[FLASH_ErasePage] Page Erase Error\n");
        return false;
    }
    
    return true;
}


uint32_t FLASH_ReadWord(uint32_t address)
{
    return *(uint32_t*) address;
}


bool FLASH_WriteDoubleWord(uint32_t flashAddress, uint32_t Data0, uint32_t Data1  )
{
    uint32_t phaddr = PHYS_ADDR(flashAddress);

    char str[256];
    sprintf(str, "[FLASH_WriteDoubleWord] PH: 0x%X Data0 0x%08X Data1 0x%08X\n", phaddr, Data0, Data1);
    Log(str);

    // if address is not aligned, return.
    if (  (phaddr & 0x7) || phaddr < PH_FLASH_BASE_ADDRESS || phaddr > PH_FLASH_END_ADDRESS )
    {
        Log("[FLASH_WriteDoubleWord] Address Error\n");
        return false;
    }
    
    // Load data into NVMDATA register
    NVMDATA0 = Data0;
    NVMDATA1 = Data1;
    // Load address to program into NVMADDR register
    NVMADDR = phaddr;

    // Unlock and Write Word
    if(FLASH_NVMUnlock (WRITE_DWORD_CODE))
    {
        Log("[FLASH_WriteDoubleWord] Write Error\n");
        return false;
    }

    return true;
}

/* FLASH_WriteRow: Writes a single row of data from the location given in *data to
 *                   the flash location in address.  
 *                   The flashAddress will be converted to physical address of the Flash.
 *                   The source address  of the SRAM.  The below code will mask upper address bits to make this a physical address.
 *                    */
bool FLASH_WriteRow(uint32_t flashAddress, uint32_t * dataAddress)
{
    unsigned int res;

    // Validate address is row aligned and in flash area
    if (    (PHYS_ADDR(flashAddress) & (FLASH_WRITE_ROW_SIZE_IN_PC_UNITS-1)) 
         || (PHYS_ADDR(flashAddress) > FLASH_UPPER_ADDRESS) )
    {
        return false;
    }

    // Set NVMADDR to Start Address of row to program
    NVMADDR = (uint32_t)PHYS_ADDR(flashAddress);

    // Set NVMSRCADDR to the SRAM data buffer Address.  Need to give HW the physical SRAM Address
    NVMSRCADDR = PHYS_ADDR((uint32_t) dataAddress );
    
    // Unlock and Write Row
    res = FLASH_NVMUnlock(FLASH_WRITE_ROW_CODE);

    // Return Result
    return (res == 0);
}


uint16_t FLASH_GetErasePageOffset(uint32_t address)
{ 
    return (address & (FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS-1));
}

uint32_t FLASH_GetErasePageAddress(uint32_t address)
{
    return (address & ~(FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS-1));
}
