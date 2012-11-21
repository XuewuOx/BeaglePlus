//============================================================================
// Name        : main.cpp
// Author      : Xuewu Daniel Dai
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
//#include <math.h>
#include <time.h>
// #include <ctime.h>

#include "uartBeagle.h"
#include "getRand.h"
#include "gfit.h"
#include "test.h"

// Change the PORT_NAME for proparait  serial port
// static const char *PORT_NAME = "/dev/ttyUSB0"; // for AirLink USB-232 converter
static const char *PORTMBED_NAME0 = "/dev/ttyACM0"; // for mBed USB-232
// static const char *PORTMBED_NAME1 = "/dev/ttyACM1"; // for mBed USB-232
static const char *PORTPC_NAME = "/dev/ttyO2";  // for default RS232 console at BB
uartBeagle mBed("mBed");
uartBeagle PC("PC");

#define MAXROW_DATA 20
#define MAXCOL_DATA 500

int dataUV[MAXROW_DATA][MAXCOL_DATA];
int dataIR[MAXROW_DATA][MAXCOL_DATA];
int *pdataUV;
int *pdataIR;
int nline;

// int statemain; // 0 init; 1 IDLE; 2 SWN  3 DAQ
enum {
	INIT =0,
	IDLE =1,
	MOVEMOTOR =2,
	SWN =3,
	DAQ =4
}statemain;

struct MOTORSTATUS{
	int nOrigin;
	int nNow;
	double motorSpd; //steps/s,
	int fullStep;
	int statusLEDMotor;
	int uSW;
};

MOTORSTATUS motorLED;

void setBeagleRTC(void);
void process_UART();
void  sigint_handler(int sig);
void testMatabCode();
void init_main(char *pNamemBed);
void moveMotor2Switch();
int moveMotor2Dest(int dest);

int daqBySWN(int a, int b, int nSperS, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA]);
int daqUVIR(int Fs, int nSamples, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA]);


int main(int argc, char* argv[]) {

	int i;
	unsigned int nDAQ;
    double elapsed_secs;
    char *pNamemBed;
    int nChars;
    char tempStr[300];

    cout << "\n!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    cout <<argv[0];
    if (argc<=1)
    	{ pNamemBed=(char*)PORTMBED_NAME0; cout<<endl;}
    else
    	{ pNamemBed=argv[1]; cout <<argv[1]<<endl;}

    double ax[5];
    int n;

    // string tempStr2("dir%d=[%d %d %d %d %d ]");

    // string strTest("dir02=[0001 0002 0003 0004 0005 ]");
    // sscanf(strTest.c_str(),"dir%d=[%f %f %f %f %f ]",&n, &ax[0], &ax[1], &ax[2], &ax[3], &ax[4]);
    // sscanf(strTest.c_str(),tempStr2.c_str(),&n, ax, ax+1, ax+2, ax+3, ax+4);
    // cout<<"dir"<<n<<"=["<<ax[0]<<ax[1]<<ax[2]<<ax[3]<<ax[4]<<" ]"<<endl;

    // Initialise global variables, display time, open UART ports, etc.
    statemain=INIT; // 0;
    init_main(pNamemBed);

    // Test Matlab Code Generation
	testMatabCode();


	usleep(1000);
    PC.uartwriteStr("PC.uartwriteStr() success\n");
    usleep(1000);
    //----------------------------------
	PC.uartwriteStr("Beagle starts\r\n");
	PC.uartwriteStr("resetmbed\r\n");
	usleep(1000);

	// Assign a handler to close the serial port on Ctrl+C.
	signal (SIGINT, &sigint_handler);
	printf("Press Ctrl+C to exit the program.\n");

	mBed.uartwriteStr("setm 1 0 30 100 1\r\n");
	usleep(1000);
	while (mBed.readline()==0){}
	// mBed.uartwriteStr("setm 1 0 0 100 1\r\n");
	statemain=IDLE; // 1
	// The main program loop:
	time_t t1, t2;
	time(&t1);
	t1-=60;
	nDAQ=0;
	while (1)
	{
		// tNow=clock();
		// elapsed_secs=(double) (tNow - tLast)/(CLOCKS_PER_SEC);
		time(&t2);
		elapsed_secs=difftime(t2,t1);
		// cout<<"elapsed_secs="<<elapsed_secs<<endl;
		if (elapsed_secs>=30)
		{
			nDAQ++;
			struct tm * timeinfo= localtime(&t2);
			printf("[%d/%d/%d %02d:%02d:%02d] ",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
				    		    timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			printf("start %d-th UV/IR measurement\n",nDAQ);
			t1=t2;

			statemain=MOVEMOTOR; //2
			// moveMotor2Switch();
			// move motor for reference measurement
			moveMotor2Dest(-80);
			statemain=SWN; //3
			mBed.uartwriteStr("uvs00\r\n");
			// fgets(tempbuff, 100, uart_mBed);
			usleep(100);
			mBed.uartwriteStr("irs00\r\n");
			usleep(100);
			mBed.uartread();
			int nChn, Fs, nSample;
			nChn=0; Fs=1000; nSample=1;
			sprintf(tempStr,"a2d %d %d %d\r\n",nChn, Fs, nSample);
			// mBed.uartwriteStr(tempStr);
			usleep(100);
			mBed.uartread();

		    int posA, posB, nSam;
			posA=91; posB=posA+MAXROW_DATA-1; nSam=MAXCOL_DATA;
			if (daqBySWN(posA, MAXROW_DATA, nSam, dataIR, dataUV)==-1)
			{ // something wrong
				cout<<"WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop."<<endl;
				continue; // ignore the data and continue the main loop
			}

		    // printf("SWN done! Press any key to resume\r\n");
			// getchar();

			// Data processing
			double sigmaUV, muUV, aUV;
			double sigmaIR, muIR, aIR;
			// int xx[MAXROW_DATA];
			double xx[MAXROW_DATA];
			double yyIR[MAXROW_DATA], yyUV[MAXROW_DATA];
			for (int i=0;i<MAXROW_DATA;i++)
				{ xx[i]=posA+i;
				  yyIR[i]=0;
				  for (int j=0;j<MAXCOL_DATA;j++)
					  yyIR[i]+=dataIR[i][j];
				  yyIR[i]=yyIR[i]/MAXCOL_DATA;

				  yyUV[i]=0;
				  for (int j=0;j<MAXCOL_DATA;j++)
				  	  yyUV[i]+=dataUV[i][j];
				  yyUV[i]=yyUV[i]/MAXCOL_DATA;
				}
		   for (int i=0; i<MAXROW_DATA;i++)
			   printf("xx[%d]-yyIR[%f]-yyUV[%f]\r\n", xx[i],yyIR[i], yyUV[i]);

			gfit(xx,yyIR,0.2, &sigmaIR, &muIR, &aIR);
		    gfit(xx,yyUV,0.2, &sigmaUV, &muUV, &aUV);
		    int posOptIR=round(muIR); // aligned by IR

		    sprintf(tempStr,"move -d 1 %d\r\n",posOptIR);
		    mBed.uartwriteStr(tempStr);
            // TODO:
		    string strPkt;
		    mBed.readPkt("MOTORxxx","xxxx",strPkt);
		    daqUVIR(Fs, nSample, dataIR, dataUV);

		} // end of if (elapsed_secs>=10)
		process_UART();
		if (strncmp(PC.rxbuf, "quit",4)==0)
		{   // quit the programme
			PC.uartwriteStr("Program is going to kill itself, good bye\n");
			sigint_handler(15);
			return 0;
		}
		if (strncmp(PC.rxbuf,"settime",7)==0)
		{
			printf("Do you want to set a new time? (Y/N)\n");
			char chgTime;
			scanf("%s",&chgTime);
			if (chgTime=='Y' || chgTime=='y')
			{ setBeagleRTC();
						}
		}
		// TODO: The application can perform other tasks here.

	}
	cout<<"=== main loop exit! ==="<<endl;
	return 0;

    //----------------------------------
    PC.uartclose();
    mBed.uartclose();

    return 0;
}


int daqUVIR(int Fs, int nSamples, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA])
{
	int nChn=0;
	char tempStr[30];
	sprintf(tempStr,"a2d %d %d %d\r\n",nChn, Fs, nSamples);
    mBed.uartwriteStr(tempStr);
    string strData;
    // TODO:
    // mBed.readPkt("MOTORxxx","xxxx", strData);
}

// int daqBySWN(int a, int b, int nSperS)
// return value:
//     -1  invalid data acquisition. dataIR and dataUV must be ignored
//      0   valid data acquisition. nDataSteps_a2b rows, nSperS column data has been collected
int daqBySWN(int a, int nDataSteps_a2b, int nSperS, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA])
{
	int i,j;
	char dataStr[MAXCOL_DATA*MAXROW_DATA*5*2];
	string strRx;

	sprintf(dataStr,"swn %d %d %d\r\n",a,a+nDataSteps_a2b-1,nSperS);
	mBed.flushrxbuf();
	mBed.uartwriteStr(dataStr);
	cout<<"==================================="<<endl;
	cout<<"send '"<<dataStr<<"' down to mBed"<<endl;
	size_t found0, found2;

	// mBed.readPkt("M posA=","DATAIRUVEND",strRx);
	mBed.readPktTimeout("M posA=","DATAIRUVEND",strRx,10000);
	/*
	cout<<"========================================"<<endl;
	cout<<"After packet check, strRx.length()="<<strRx.length()<<" chars, strRx="<<endl;
	cout<<strRx<<endl;
	cout<<"========================================"<<endl;

	printf("\r\n----------------------------------------\r\n");
	printf("printf full data packet found: %s",dataStr);
	*/
	int posA, nSteps, nSam, nFs, nArg;
	// char charLE="\r\n";
	string firstLine;
	size_t posLE;
	firstLine=strRx.substr(1,(int)(strRx.find("\r\n")));
	strRx.erase(0,(int)(strRx.find("\r\n"))+1);
	// cout<< "first line: "<<firstLine<<endl;
	printf("desired values: posA=%d, nSteps=%d, nSam=%d\r\n",a,nDataSteps_a2b,nSperS);
	nArg=sscanf(firstLine.c_str(),"M posA=%d, nSteps=%d, nSam=%d, Fs=%d",&posA,&nSteps, &nSam, &nFs);
	printf("decaped values: posA=%d, nSteps=%d, nSam=%d\r\n",posA,nSteps,nSam);
	if(nArg!=4 || posA!=a || nSteps!=nDataSteps_a2b || nSam!=nSperS )
		{ printf("ERROR: wrong M posA=%d, nSteps=%d  nSam=%d from mBed\r\n", posA, nSteps, nSam);
		  return -1; // return an ERROR
		  // continue; // continue the main loop
		}

	// convert ASCII packet into int array
	char leadStrIR[10], leadStrUV[10];
	char *pStart, *pEnd;
	size_t pNextLine;
	int nDataChar;

	for (i=0;i<nSteps;i++)
		for (j=0; j<nSam; j++)
			{ dataIR[i][j]=0xFFFF; dataUV[i][j]=0xFFFF;}

	for (i=0;i<nSteps;i++)
	{   sprintf(leadStrIR,"dir%03d=[ \0",i); //'\0' to terminate the string
		found0=strRx.find(leadStrIR);
		found2=strRx.find("\r\n");
		nDataChar=found2-(found0+strlen(leadStrIR))-1;
	    int lencpy=strRx.copy(dataStr,nDataChar,found0+strlen(leadStrIR));
	    dataStr[lencpy]='\0'; // terminate the string
		strRx.erase(0,found2+1); // remove one line
		// cout<< "dataStr contains: "<<dataStr<<" OK "<<endl;
	    //  printf("%d-th line IR: %s\r\n", i, dataStr);
	    // printf("dataIR[%d][]=",i);
	    pEnd=dataStr;
	    for (int j=0;j<nSam;j++)
		 	{ dataIR[i][j]=strtol(pEnd, &pEnd,10);
		 	  // printf(" %d",dataIR[i][j]);
		 	}
	    // printf("\r\n");


	    sprintf(leadStrUV,"duv%03d=[ \0",i);
		found0=strRx.find(leadStrUV);
		found2=strRx.find("\r\n");
		nDataChar=found2-(found0+strlen(leadStrIR))-1;
		// cout<<"found0="<<found0<<", found2="<< found2<<" nDataChar="<<nDataChar<<endl;
		// TODO: check if nDataChar == nSam*(4+1)+2
		// nDataChar=nSam*(4+1)+2; // how many chars in one data line
		strRx.copy(dataStr,nDataChar,(int)found0+strlen(leadStrUV));
	    dataStr[nDataChar]='\0'; // terminate the string
		strRx.erase(0,found2+1); // remove one line
		// printf("dataUV[%d][]=",i);
	    pEnd=dataStr;
		for (int j=0;j<nSam;j++)
			dataUV[i][j]=strtol(pEnd, &pEnd,10);

	 }
	// printf the dataIR and dataUV int array
	printf("dataIR and dataUV are:\r\n");
	for (int i=0;i<nSteps;i++)
		{  int j;
		   printf("IR03%d=[",i);
		   for (j=0; j<nSam;j++)
			   printf(" %04d",dataIR[i][j]);
		   printf("]\r\n");

		   printf("UV03%d=[",i);
		   for (j=0; j<nSam;j++)
		  	   printf(" %04d",dataUV[i][j]);
		   printf("]\r\n");
		}
	return 0;
}

// Test Matlab Code Generation
void testMatabCode()
{
	double xx[100];
	double yy[100];

	time_t rawt = time(NULL); // /* Seconds since the Epoch. 1970-01-01  */
	time(&rawt);

	// getRand(rawt,yy);
	getRand(10393,yy);
	printf("[");
	 for (int i=0;i<100;i++)
		{	xx[i]=i;
		    printf("%f, ", yy[i]);
		    // printf("testfunc=%d\n",testfunc(i));
		}
	 printf("]\n");

	 // function [sigma, mu, A] = gfit(x,y,h) %#codegen
	//  gfit(const real_T x[100], const real_T y[100], real_T h, real_T *sigma, real_T *mu, real_T *A);
	 double sigma, mu, A;
	 gfit(xx,yy,0.2, &sigma, &mu, &A);
	 printf("sigma=%f, mu=%f, A=%f\n",sigma, mu, A);
}
// process UART strings
void process_UART()
{

	int chars_read;

		// Read received mBed data and forward to PC
		// up to but not including the buffer's terminating null.
		// chars_read=mBed.uartread();
		chars_read=mBed.readline();
		if (chars_read > 0)
		{
			// Data was read.
			// mBed.rxbuf[chars_read]=0x00; // terminate string
			printf("mBed->BB: %s", mBed.rxbuf);
			printf("\n"); //mBed does not send \n, manually add \n to stdio
	//		PC.uartwriteChar(mBed.rxbuf,chars_read);
	//		respond_to_command(read_buffer, chars_read);
	//		mBed.flushrxbuf();
		}


	    // Read received command from PC
		// chars_read = PC.uartread();
		int line_read=PC.readline();
		// printf("chars_read PC =%d\n", chars_read);
		if (line_read > 0)
		{
			    // PC.flushrxbuf();
				// PC.rxbuf[chars_read]=0x00; // terminate string
				// printf("chars_read=%d, %s",chars_read, read_buffer);
				printf("PC->mBed (line=%d): %s", line_read, PC.rxbuf);
				{
				/*
				    pdataUV=dataUV+nline*5;
					sscanf(PC.rxbuf,"%d %d %d %d %d", pdataUV, pdataUV+1, pdataUV+2, pdataUV+3, pdataUV+4);
					nline++;
					if (nline==4)
						{ printf("received data are:\n");
						  for (int i=1; i<20;i++)
							{ printf("%d ", dataUV[i]);
							  if (i%5==0) printf("\n");
							}
						  printf("\n");
						  nline=0;
						}
				*/
				}

				//PC.rxbuf[chars_read-2]=0x30;
				// PC.uartwriteChar(PC.rxbuf, chars_read);
				mBed.uartwriteChar(PC.rxbuf, chars_read);
		}

}

// move Motor to destination
// Input: destination in steps
// return: motor's position after complete the move command
int moveMotor2Dest(int dest)
{
	int nLines, nPars;
	char cmdMove[20];
	sprintf(cmdMove, "move -d 1 %d\r\n",dest);
	// motorLED.uSW=0;
	mBed.uartwriteStr(cmdMove);
	do{
			nLines=mBed.readline();
	}while(nLines==0);
	// cout << "received " <<nChars <<"characters"<<endl;
	nPars=sscanf(mBed.rxbuf,
			"motor[1] is motorLED: nOrigin=%d, nNow=%d, motorSpd=%f steps/s, fullStep=%d, statusLEDMotor=%d, uSW(p29)=%d",
			&motorLED.nOrigin, &motorLED.nNow,&motorLED.motorSpd, &motorLED.fullStep, &motorLED.statusLEDMotor, &motorLED.uSW);
	// cout<<"received "<<nChars<<"parameters of motroLED"<<endl;
	printf("moveMotor2Dest done! (statemain=%d): ",statemain);
	// cout<<mBed.rxbuf<<endl;
	printf("motorLED: nOrigin=%d, nNow=%d, motorSpd=%f, fullStep=%d, statusMotor=%d, uSW=%d\r\n",
			motorLED.nOrigin, motorLED.nNow, motorLED.motorSpd, motorLED.fullStep, motorLED.statusLEDMotor,
			motorLED.uSW);

   return motorLED.nNow;
	/*	if (motorLED.nNow==dest)
		{
			printf("moterLED arrives at nNow=%d\r\n",motorLED.nNow);
			return motorLED.nNow;
		}
		else
			{ 	printf("moveMotor2Dest failed. motorLED now is at %d\r\n",motorLED.nNow);
				return -1;
			}
	*/
}


void moveMotor2Switch()
{
	int nChars;

	motorLED.uSW=0;
	while(motorLED.uSW==0)
	{	mBed.uartwriteStr("move -s 1 100\r\n");
		do{
			nChars=mBed.readline();
		}while(nChars==0);
		cout<<mBed.rxbuf;
		nChars=sscanf(mBed.rxbuf,
			"motor[1] is motorLED: nOrigin=%d, nNow=%d, motorSpd=%f steps/s, fullStep=%d, statusLEDMotor=%d, uSW(p29)=%d",
			&motorLED.nOrigin, &motorLED.nNow,&motorLED.motorSpd, &motorLED.fullStep, &motorLED.statusLEDMotor, &motorLED.uSW);
	}
}

// Executes when the user presses Ctrl+C.
// Closes the port, resets the terminal, and exits the program.

void  sigint_handler(int sig)
{   printf("\n======================================================\n");
	printf("Please remember restore file /etc/init/ttyO2.conf \n");
	printf("   $ sudo cp ttyO2.conf.save ttyO2.conf\n");
	printf("======================================================\n");
	printf("GOOD LUCK\n\n");

	mBed.uartclose();
	PC.uartclose();
	exit (sig);
}

void init_main(char *pNamemBed)
{
	nline=0;

	time_t rawt = time(NULL); // /* Seconds since the Epoch. 1970-01-01  */
	struct tm * timeinfo;
	printf("\n=================================================\n");
	printf("    Loadmon Beagle main() starts at \n");
	/* get current timeinfo and modify it to the user's choice */
	time(&rawt);
	timeinfo= localtime(&rawt);
	printf("   (Beagle Time) %d/%02d/%02d %02d:%02d:%02d\n",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
		    timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	printf("=================================================\n");

	char *portName;
	// portName=(char*)PORTMBED_NAME0;
	if (mBed.uartopen(pNamemBed)!=-1)
		cout<<"  UART port"<<mBed.portName<<" (uartID="<<mBed.uartID<<") OK"<<endl;

	portName=(char *)PORTPC_NAME;
	if (PC.uartopen(portName)!=-1)
		cout<<"  UART port"<<PC.portName<<" (uartID="<<PC.uartID<<") OK"<<endl;


}


void setBeagleRTC(void)
{
	time_t mytime;
	// struct timespec rawtime;
	struct timeval rawtime;
	 struct tm * timeinfo;
	 int year, month ,day, hh, mm, ss;
	 char * weekday[] = { "Sunday", "Monday",
	                      "Tuesday", "Wednesday",
	                      "Thursday", "Friday", "Saturday"};

	 /* prompt user for date */
	 printf("Enter day/month/year: "); scanf("%d/%d/%d",&day, &month, &year);
	 printf("Enter hour:minute:second "); scanf("%d:%d:%d",&hh, &mm, &ss);

	 /* get current timeinfo and modify it to the user's choice */
	 time ( &mytime ); // get mytime in seconds
	 //  When interpreted as an absolute time value, it represents  the number of
	 // seconds elapsed since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
	 timeinfo = localtime ( &mytime );

	 timeinfo->tm_year = year - 1900;
	 timeinfo->tm_mon = month - 1;
	 timeinfo->tm_mday = day;
timeinfo->tm_hour=hh;
timeinfo->tm_min=mm;
timeinfo->tm_sec=ss;
time ( &mytime );
	 /* call mktime: timeinfo->tm_wday will be set */
	// rawtime=mktime ( timeinfo );
	rawtime.tv_sec=mktime ( timeinfo );
	rawtime.tv_usec=0;
	 settimeofday(&rawtime,NULL);
	 printf("Set Beagle Time to: %s %d/%d/%d %02d:%02d:%02d\n", weekday[timeinfo->tm_wday],
			 timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
			 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

}



