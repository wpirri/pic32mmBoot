/* 
 * File:   config.h
 * Author: walte
 *
 * Created on 10 de abril de 2024, 13:42
 */

#ifndef CONFIG_H
#define	CONFIG_H

// Clock frequency values
#define _XTAL_FREQ  24000000UL             // Hz
#define GetSystemClock()		(_XTAL_FREQ)
#define GetInstructionClock()	(GetSystemClock()/*/4*/)
#define GetPeripheralClock()	(GetSystemClock()/*/4*/)

#endif	/* CONFIG_H */

