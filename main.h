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

#define DEBUG

#ifdef DEBUG
   #define DEBUGF printf
	#define DEBUG2PCUART PC.uartwriteStr
#else
    #define DEBUGF while(0)printf
	#define DEBUG2PCUART while(0)PC.uartwriteStr
#endif

// #define FALSE 0
// #define TRUE 1


#endif /* MAIN_H_ */
