/*
 * uartBeagle.cpp
 *
 *  Created on: 17 Nov 2012
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */

 #include <iostream>
using namespace std;

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "uartBeagle.h"

 // extern PC;
extern uartBeagle PC;

uartBeagle::uartBeagle(char *piName) {
	nTotalChar=0;
	lineReceived=0;
	strcpy(InstName,piName);
	for (int i=0; i<MAX_COMMAND_LENGTH;i++)
	{	rxbuf[i]=0;
		txbuf[i]=0;
	}
	rxbufptr=rxbuf;
	txbufptr=txbuf;
	// TODO Auto-generated constructor stub

}

uartBeagle::~uartBeagle() {
	// TODO Auto-generated destructor stub
}



void uartBeagle::uartclose(void)
{
	if (uartID!=-1)
		{ tcsetattr(uartID,TCSANOW,&options_original);
		 close(uartID);
		}
}

int uartBeagle::uartopen(char* pName)
{
    // int uartid;
	struct termios options;

	printf("Opening Serial Port %s ... \n", pName);
	// cout<<pName<<endl;
  uartID = open(pName, O_RDWR | O_NONBLOCK);

  if (uartID != -1)
  {
	  strcpy(portName, pName);
	  // printf("baud=%d\n", getbaud(uartid));
	  // printf("baud=%d\n", getbaud(uartid));

	  tcgetattr(uartID,&options_original);
 	  tcgetattr(uartID, &options);
	  cfsetispeed(&options, B115200);
	  cfsetospeed(&options, B115200);
	  /*
	  options.c_cflag |= (CLOCAL | CREAD);
	  options.c_lflag |= ICANON; // ECHO;
	  options.c_oflag |= 0;
*/
	  //
	  // Input flags - Turn off input processing
	  // convert break to null byte, no CR to NL translation,
	  // no NL to CR translation, don't mark parity errors or breaks
	  // no input parity check, don't strip high bit off,
	  // no XON/XOFF software flow control
	  //
	  options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
	                      INLCR | PARMRK | INPCK | ISTRIP | IXON);
	  //
	  // Output flags - Turn off output processing
	  // no CR to NL translation, no NL to CR-NL translation,
	  // no NL to CR translation, no column 0 CR suppression,
	  // no Ctrl-D suppression, no fill characters, no case mapping,
	  // no local output processing
	  //
	  // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
	  //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
	  options.c_oflag |= 0;
	  //
	  // No line processing:
	  // echo off, echo newline off, canonical mode off,
	  // extended input processing off, signal chars off
	  //
	  options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	  //
	  // Turn off character processing
	  // clear current char size mask, no parity checking,
	  // no output processing, force 8 bit input
	  //
	  options.c_cflag &= ~(CSIZE | PARENB);
	  options.c_cflag |= CS8;
	  //
	  // One input byte is enough to return from read()
	  // Inter-character timer off
	  //
	  options.c_cc[VMIN]  = 1;
	  options.c_cc[VTIME] = 0;

	  if (tcsetattr(uartID, TCSANOW, &options)!=0)
	  {
		  printf("error %d from tcsetattr", errno);
		  return (-1);
	  }


  }
  else
  {
	  printf("  Unable to open %s \n",pName);
  	  printf("  Error %d when opening %s: %s\n",errno, pName, strerror(errno));
  }
  return (uartID);


}

int uartBeagle::uartread()
{
	int bufsize;
	bufsize=MAX_COMMAND_LENGTH;
	int chars_read = read(uartID, rxbuf, bufsize);
	if (chars_read>=0)
	{   rxbuf[chars_read]='\0'; // printf("rcv: %s",rxbuf);
		if (strcmp(InstName,"mBed")==0)
			PC.uartwriteChar(rxbuf,chars_read);

	}
	return chars_read;
}

void uartBeagle::flushrxbuf()
{
	int bufsize;
	bufsize=MAX_COMMAND_LENGTH;
	for(int i=0; i<bufsize;i++)
		rxbuf[i]=0;
	rxbufptr=rxbuf;
	lineReceived=0;
	nTotalChar=0;
}


// read uart, copy the rxbuf to the string until finding the terminate string steEnd
int uartBeagle::readUntilStr(char *strEnd)
{
	int bufsize,nChar, lenStrEnd;
	char *pChar;
	bool findEndStr=0;

	flushrxbuf();
	lenStrEnd=strlen(strEnd);
	while(1)
	{
		if ((nChar=read(uartID, rxbufptr, rxbuf+bufsize-rxbufptr-1))>0)
		{
			// printf("receiv@readline(): %s",rxbuf);
			if (strcmp(InstName,"mBed")==0)
		    	PC.uartwriteChar(rxbuf-nChar, nChar);

			// scan rxbuf to find how many lines have been received
			// lineReceived=0;
			if (rxbufptr-rxbuf<lenStrEnd)
				pChar=rxbuf;
			else
				pChar=rxbufptr-lenStrEnd;
			for (;pChar<rxbuf+nChar-lenStrEnd;pChar++)
			   {
					findEndStr=1;
					for (int i=0;i<lenStrEnd;i++)
						if (*(pChar+i)!=strEnd[i]) findEndStr=0;
					if (findEndStr==1)
					{ // the terminate string is found
						break;
					}
			   }
			if (findEndStr==1)
				{  nTotalChar=pChar+lenStrEnd-rxbuf;
				   *(pChar+lenStrEnd)='\0';
				   return 	nTotalChar;
				}
			else
				usleep(1000);

		}
	} // end while (1)
	// never reach here unless something wrong
	printf("WRONG: Codes should never reach here.\r\n");
	return 0;
}


int uartBeagle::readPktTimeout(char *strHead, char *strTail,string &strRx, double timeout_ms)
{

	size_t found0, found2;
    int n_usleeps;
    cout<<"readPktTimeOut("<<(int)timeout_ms<<"ms) is waiting for data "<<endl;
    n_usleeps=int(timeout_ms);
		while(1)
		{   usleep(1000);
			if (uartread()>0)
			{
			// cout<<"mBed.rxbuf length="<<strRx.length()<<"nTotoalChar="<<nTotalChar;
			// cout<<"lineReceived="<<mBed.lineReceived<<endl;
			strRx.append(rxbuf);

			// mBed.rxbufptr=mBed.rxbuf;
			found0=strRx.find(strHead);
			found2=strRx.find(strTail);
			// cout<<"found0("<<strHead<<")="<<(int)found0<<", found2("<<strTail<<")="<<(int)found2<<endl;
			if (found0!=string::npos && found2!=string::npos)
				{cout <<"first '"<<strHead<<"' found at: "<<int(found0)<<endl;
				cout <<"first '"<<strTail<< "' found at: "<<int(found2)<<endl;
				cout << "remove chars before "<<int(found0)<<"and after"<<int(found2)<<endl;
				strRx.erase(found2+strlen(strTail));
				strRx.erase(0,found0-1);
				cout <<"Now strRx.length()="<<strRx.length()<<endl;
				// strRx.copy(strPkt, 0, strRx.length());
				// strPkt[strRx.length()]='\0';
				break;
				}
			}
			if (n_usleeps<0)
				continue;
			else if(n_usleeps==0)
				break;
			else
				n_usleeps--;

		}
		cout<<"uartBeagle::readPktTimeout() returns"<<endl;
		return strRx.length();
}

// read a string beginning with strHead and terminated by strTail
// save the string into strPkt and return the length of the valid string
int uartBeagle::readPkt(char *strHead, char *strTail,string &strRx)
{
	// string strRx;
	size_t found0, found2;
	cout<<"uartBeagle::readPkt() is wating for data"<<endl;
	while(1)
	{
		if (uartread()>0)
		{
		// cout<<"mBed.rxbuf length="<<strRx.length()<<"nTotoalChar="<<nTotalChar;
		// cout<<"lineReceived="<<mBed.lineReceived<<endl;
		strRx.append(rxbuf);

		// mBed.rxbufptr=mBed.rxbuf;
		found0=strRx.find(strHead);
		found2=strRx.find(strTail);
		// cout<<"found0("<<strHead<<")="<<(int)found0<<", found2("<<strTail<<")="<<(int)found2<<endl;
		if (found0!=string::npos && found2!=string::npos)
		{	cout <<"first '"<<strHead<<"' found at: "<<int(found0)<<endl;
			cout <<"first '"<<strTail<< "' found at: "<<int(found2)<<endl;
			cout << "remove chars before "<<int(found0)<<"and after"<<int(found2)<<endl;
			strRx.erase(found2+strlen(strTail));
			strRx.erase(0,found0-1);
			cout <<"Now strRx.length()="<<strRx.length()<<endl;
			// strRx.copy(strPkt, 0, strRx.length());
			// strPkt[strRx.length()]='\0';
			break;
		}
		}
		usleep(1000);
	}
	cout<<"uartBeagle::readPkt() is returning"<<endl;
	return strRx.length();
}


// read uart until \r or \n is received
int uartBeagle::readline()
{
	int nChar,i;
	int bufsize;
	char *pChar;

	bufsize=MAX_COMMAND_LENGTH;

	if ((nChar=read(uartID, rxbufptr, rxbuf+bufsize-rxbufptr-1))>0)
	  {
    	nTotalChar+=nChar;
    	rxbufptr+=nChar;
		if(rxbufptr[-1]=='\n' || rxbufptr[-1]=='\r')
		{
			/* null terminate the string*/
			*rxbufptr='\0';
			// scan rxbuf to find how many lines have been received
			lineReceived=1;
			for (pChar=rxbuf+1;pChar<rxbufptr;pChar++)
					{   if (pChar[-1]==0x0D && pChar[0]==0x0A)
						lineReceived++;
					}
				// printf("receiv@readline(): %s",rxbuf);
				if (strcmp(InstName,"mBed")==0)
				    	PC.uartwriteChar(rxbuf, rxbufptr-rxbuf);

				if (rxbufptr-rxbuf>(int)nTotalChar)
				{  // there are some char at the end of \r or \n, which will be lost
					printf("WARN: rxbufptr-rxbuf=%d, nTotalChar=%d, There are chars at the end of \\r or \\n, which will be lost\r\n",
							rxbufptr-rxbuf, nTotalChar);
					nTotalChar=rxbufptr-rxbuf;
				}
				cout<<"string ended with \\n or \\r received, lineReceived="<<lineReceived<<endl;
				cout<<rxbuf<<endl;
				rxbufptr=rxbuf; // reset the pointer to the beginning of rxbuf
				                // for receiving next command line
				nTotalChar=0;

		 	return lineReceived;
		}
	  }

	return 0; // zero line is received
}

void uartBeagle::uartwriteStr(char *write_buffer)
{
	int bytes_written;
	size_t len = 0;

	len = strlen(write_buffer);
	bytes_written = write(uartID, write_buffer, len);
	if (bytes_written < len)
	{
		printf("Write serial(uartID=%d) failed \n", uartID);
	}
}

int uartBeagle::uartwriteChar(char *write_buffer, int nChar)
{
	int bytes_written;
	size_t len = 0;

	len = (size_t) nChar;
	bytes_written = write(uartID, write_buffer, len);
	if (bytes_written < len)
	{
		printf("Write searial (portName=%s) failed \n", portName);
		return -1;
	}
	return bytes_written;
}

