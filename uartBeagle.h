/*
 * uartBeagle.h
 *
 *  Created on: 17 Nov 2012
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */

#ifndef UARTBEAGLE_H_
#define UARTBEAGLE_H_

#include "stddef.h"  // for size_t
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>


#define MAX_COMMAND_LENGTH 50000

class uartBeagle {
public:
	char portName[50];
	int uartID;
	char rxbuf[MAX_COMMAND_LENGTH + 1];
	char txbuf[MAX_COMMAND_LENGTH + 1];
	unsigned int lineReceived; // a state variable indicating a line terminated by
		                             // '\r' or '\n' has been received
		                             // 0   have not received \r or \n
		                             // non zero:  a full line st
	unsigned int nTotalChar;
	char *rxbufptr;
private:
	char InstName[10]; // name of the instance, either "mBed" or "PC"

	char *txbufptr;
	struct termios options_original;


public:
	uartBeagle(char *piName);
	virtual ~uartBeagle();

	int readUntilStr(char *strEnd);
	int readPkt(char *strHead, char *strTail,string &strRx);
	int readPktTimeout(char *strHead, char *strTail,string &strPkt, double timeout_ms);

	int init_uart(void);
	void uartclose();
	int uartopen(char* portName);
	int uartread();
	void uartwriteStr(char *write_buffer);
	int uartwriteChar(char *write_buffer, int nChar);

	void flushrxbuf(); // flush rxbuf and reset rxbufptr to rxbuf
	int readline(); // read uart until \r or \n is received
// 	int readlineOnce();

};

#endif /* UARTBEAGLE_H_ */
