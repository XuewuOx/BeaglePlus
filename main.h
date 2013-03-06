/*
 * main.h
 *
 *  Created on: 7 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */


#ifndef MAIN_H_
#define MAIN_H_

// #define DEBUG

#ifdef DEBUG
   #define DEBUGF printf
	#define DEBUG2PCUART daqModule.oPC.uartwriteStr
#else
    #define DEBUGF while(0)printf
	#define DEBUG2PCUART while(0)daqModule.oPC.uartwriteStr
#endif


#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

#endif /* MAIN_H_ */
