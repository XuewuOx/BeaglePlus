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
#include "gfit_rdfile.h"

// #define FALSE 0
// #define TRUE 1

// Change the PORT_NAME for proparait  serial port
// static const char *PORT_NAME = "/dev/ttyUSB0"; // for AirLink USB-232 converter
static const char *PORTMBED_NAME0 = "/dev/ttyACM0"; // for mBed USB-232
// static const char *PORTMBED_NAME1 = "/dev/ttyACM1"; // for mBed USB-232
static const char *PORTPC_NAME = "/dev/ttyO2";  // for default RS232 console at BB
uartBeagle mBed("mBed");
uartBeagle PC("PC");

#define MAXROW_DATA 500
#define MAXCOL_DATA 500

int dataUV[MAXROW_DATA][MAXCOL_DATA];
int dataIR[MAXROW_DATA][MAXCOL_DATA];
int *pdataUV;
int *pdataIR;
int nline;

// int statemain; // 0 init; 1 IDLE; 2 SWN  3 DAQ
enum MAINSTATUS{
	INIT =0,
	IDLE =1,
	MOVEMOTOR =2,
	SWN =3,
	DAQ =4,
	MBEDONLY =5
};

int statemain;

struct MOTORSTATUS{
	int nOrigin;
	int nNow;
	double motorSpd; //steps/s,
	int fullStep;
	int statusLEDMotor;
	int uSW;
};

MOTORSTATUS motorLED;


// void process_UART();
void process_UART(int *pstatemain);
void  sigint_handler(int sig);
void testMatabCode();
void init_main(char *pNamemBed);
int moveMotor2Switch();
int moveMotor2Dest(int dest);

int scanIRUV(int a, int b, int nSperS, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA],char *fleadname);
int daqIRUV(int Fs, int nSamples, int posMS, char * fname);
int scanProcData(char* fname, int* optIR, int* optUV);

int manual_scandaq(int posA, int posB, int nSam, int ampIR, int ampUV, char *fleadname);
// int daqUVIR(int Fs, int nSamples, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA]);

void setBeagleRTC(void);
void setBeagleRTC2(int yy, int mm, int dd, int hh, int min, int ss);
void setBeagleRTC3(int yy, int mm, int dd, int hh, int min, int ss);



int main(int argc, char* argv[]) {

	int i;
	unsigned int nDAQ;
    double elapsed_secs;
    char *pNamemBed;
    int nChars;
    char tempStr[500];

    cout << "\n!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    cout <<argv[0];
    if (argc<=1)
    	{ pNamemBed=(char*)PORTMBED_NAME0; cout<<endl;}
    else
    	{ pNamemBed=argv[1]; cout <<argv[1]<<endl;}

    double ax[5];
    int n;
    int returnstate; // a variable returned by a function

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
    // PC.uartwriteStr("PC.uartwriteStr() success\n");
    usleep(1000);
    //----------------------------------
	PC.uartwriteStr("Beagle starts\r\n");

	// PC.uartwriteStr("resetmbed\r\n");
	usleep(1000);

	// Assign a handler to close the serial port on Ctrl+C.
	signal (SIGINT, &sigint_handler);
	printf("Press Ctrl+C to exit the program.\n");


	PC.uartwriteStr("  Testing the motor ...");
	// PC.uartwriteStr("% setm 1 0 30 100 1\r\n");
	// printf("setm 1 0 30 100 1\r\n");
	// mBed.uartwriteStr("setm 1 0 30 100 1\r\n");
	// usleep(100);

	// while (mBed.readline()==0){}
	mBed.flushrxbuf();
	printf("move -s 1 -100\r\n");
	mBed.uartwriteStr("move -s 1 -300\r\n");
	usleep(1000);
	while (mBed.readline()==0){}

	PC.uartwriteStr(" ... ");
	printf("move -s 1 100\r\n");
	mBed.uartwriteStr("move -s 1 300\r\n");
	usleep(1000);
	while (mBed.readline()==0){}

	PC.uartwriteStr(" OK\r\n");

	PC.uartwriteStr("  Set Beagle's time and date ... ");
	setBeagleRTC();
	struct timeval systime;
	gettimeofday(&systime, NULL);
	char * text_time = ctime(&systime.tv_sec);
	PC.uartwriteStr("  OK! ");
	PC.uartwriteStr(text_time);

	statemain=MBEDONLY; // 1
	PC.uartwriteStr("\r\n  Enter mbed mode by default for controlling mbed manually\r\n");
	printf("Enter mbed mode by default for controlling mbed manually\r\n");

	PC.uartwriteStr("  Waiting for commands from USB/RS232 accessport\r\n");
	printf("  Waiting for commands from USB/RS232 accessport (for command information, please reset mbed\r\n");

	PC.uartwriteStr("      to see the mbed command, please reset mbed\r\n");
	printf("      to see the mbed command, please reset mbed\r\n");

	printf("      to switch to Beagle mode for automatic data collection, please type idle followed by ENTER\r\n");
	printf("      to stop program, type quit followed by ENTER. Wait for a few seconds\r\n");
	PC.uartwriteStr("      to switch to Beagle mode for automatic data collection, please type idle followed by ENTER\r\n");
	PC.uartwriteStr("      to stop program, type quit followed by ENTER. Wait for a few seconds\r\n");

	// The main program loop:
	time_t t1, t2;
	time(&t1);
	t1-=60; // for debug purpose: start first DAQ immediately
	nDAQ=0;
printf("main loop starts\r\n");
while (1)
{
	switch (statemain) {
	case IDLE:
	  {
		process_UART(&statemain);
		if (statemain!=IDLE)
		{ // the statemain has been changed by process_UART under the request of user
		  break;
		}
		time(&t2);
		// printf("check new time t2\r\n");
		elapsed_secs=difftime(t2,t1);
		// cout<<"elapsed_secs="<<elapsed_secs<<endl;
		if (elapsed_secs>=10)
			{
			t1=t2;
			printf("skip %d-th DAQ for debuging \r\n", ++nDAQ);
			// statemain=DAQ; // to start data acquisition
			}
		break;
		}
	case MBEDONLY:
	  {	process_UART(&statemain);
	  	break;
	  }
	case DAQ:
	{
		nDAQ++;
		struct tm * timeinfo= localtime(&t2);
		printf("[%d/%d/%d %02d:%02d:%02d] ",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
							timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		printf("start %d-th UV/IR measurement\n",nDAQ);

		sprintf(tempStr,"[%d/%d/%d %02d:%02d:%02d] ",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
				timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		PC.uartwriteStr(tempStr);
		sprintf(tempStr,"start %d-th UV/IR measurement\n",nDAQ);
		PC.uartwriteStr(tempStr);

		manual_scandaq(100, 300,10,25,00,"wtr");

//=====================================================
		// dataIR and dataUV now is saved as a string in *.txt file
		// and processed fully by Matlab functions
		// dataIR and dataUV is not accessed by main()
/*
*		// Data processing
*		double sigmaUV, muUV, aUV;
*		double sigmaIR, muIR, aIR;
*		// int xx[MAXROW_DATA];
*		double xx[MAXROW_DATA];
*		double yyIR[MAXROW_DATA], yyUV[MAXROW_DATA];
*		for (int i=0;i<MAXROW_DATA;i++)
*			{ xx[i]=posA+i;
*			  yyIR[i]=0;
*			  for (int j=0;j<MAXCOL_DATA;j++)
*				  yyIR[i]+=dataIR[i][j];
*			  yyIR[i]=yyIR[i]/MAXCOL_DATA;
*
*			  yyUV[i]=0;
*			  for (int j=0;j<MAXCOL_DATA;j++)
*				  yyUV[i]+=dataUV[i][j];
*			  yyUV[i]=yyUV[i]/MAXCOL_DATA;
*			}
*	   for (int i=0; i<MAXROW_DATA;i++)
*		   printf("xx[%d]-yyIR[%f]-yyUV[%f]\r\n", (int)round(xx[i]),yyIR[i], yyUV[i]);
*
*		// gfit(xx,yyIR,0.2, &sigmaIR, &muIR, &aIR);
*		// gfit(xx,yyUV,0.2, &sigmaUV, &muUV, &aUV);
*		int posOptIR=round(muIR); // aligned by IR
*/


		statemain=IDLE;
		break;
	} // end of case DAQ
	//-------------------------------------------

	default:
	{   statemain=IDLE;
		printf("WARN: case statemain = default. reset statemain=IDLE\r\n");
		break;
	}

  }  // end of switch (statemain)
	//-------------------------------------------


  // TODO: The application can perform other tasks here.


} // end of while (1)
//----------------------------------


    cout<<"=== EXCEPTION main loop exit! ==="<<endl;
    PC.uartwriteStr("EXCEPTION main loop exit\r\n");
    usleep(1000);
	sigint_handler(15);
    return 0;
}


int daqIRUV(int Fs, int nSamples, int posMS, char * fleadnamemeas)
{
	char dataStr[MAXCOL_DATA*MAXROW_DATA*5*2];
	int nChn=0;
	char fname[100];
	char tempStr[30];
	// sprintf(tempStr,"a2d s %d %d\r\n", Fs, nSamples);
	sprintf(tempStr,"swn %d %d %d\r\n", posMS, posMS, nSamples);
    mBed.uartwriteStr(tempStr);
	string strRx;
	string strData;
    // TODO:
    // mBed.readPkt("MOTORxxx","xxxx", strData);
    // mBed.readPkt("M posA=","DATAIRUVEND",strRx);
    int nChar=mBed.readPktTimeout("%SWN posA=","DATAIRUVEND",strRx,40000);
    if (nChar<=0)
    	{ // nothing valid received from mBed.
    		cout<<"Nothing valid received from mBed. "<<endl;
    		return -1; // return an ERROR
    	}
    	// valid packet received
    	// save it to a txt file
    	FILE *fdata;
    	sprintf(fname, "%s.txt", fleadnamemeas);
    	fdata=fopen(fname,"w"); //Create an empty file
    	if (fdata==NULL)
    	  { cout<<"fopen() to create a data file "<<fname<<" failed."<<endl;
    		return -1; // return an ERROR
    	  }
    	fwrite(strRx.c_str(),1,strRx.length(), fdata);
    	fclose (fdata);
    	printf("save data to %s OK\n", fname);

    	// backup file
    	time_t tnow;
		struct tm * timeinfo;
    	char fname2[100];
    	char cmdline[150];
    	time(&tnow);
    	timeinfo= localtime(&tnow);
    	sprintf(fname2,"%s_%04d%02d%02d_%02dh%02dm%02ds.txt",fleadnamemeas,timeinfo->tm_year+1900, timeinfo->tm_mon+1,
    	    	    			timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    	sprintf(cmdline,"cp %s %s",fname, fname2);
    	system(cmdline);
    	printf("backup data in %s to %s OK\n", fname, fname2);

    return 0;
}

// int scanIRUV(int a, int b, int nSperS)
// return value:
//     -1  invalid data acquisition. dataIR and dataUV must be ignored
//      0   valid data acquisition. nDataSteps_a2b rows, nSperS column data has been collected
int scanIRUV(int a, int nDataSteps_a2b, int nSperS, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA], char *fleadnamescan)
{
	int i,j, nChar;
	char fname[100];
	char fname2[100];

	char temStr[100];
	string strRx;
	time_t tnow;

	sprintf(temStr,"swn %d %d %d\r\n",a,a+nDataSteps_a2b-1,nSperS);
	mBed.flushrxbuf();
	mBed.uartwriteStr(temStr);
	cout<<"==================================="<<endl;
	cout<<"send 'swn "<< a <<" "<< a+nDataSteps_a2b-1 <<" "<< nSperS<< "' down to mBed"<<endl;
	size_t found0, found2;

	int ttimeout; // ms, threshold of waiting time for readPktTimeout()
	ttimeout=nSperS*nDataSteps_a2b+10*2000; // assume 1000kHz sampling rate
	                                        // motor spd: 100 steps per second
	                                        // maximum 2000 steps
	// mBed.readPkt("M posA=","DATAIRUVEND",strRx);
	nChar=mBed.readPktTimeout("%SWN posA=","DATAIRUVEND",strRx,ttimeout);
	if (nChar<=0)
	{ // nothing valid received from mBed.
		cout<<"Nothing valid received from mBed. "<<endl;

		return -1; // return an ERROR
	}
	// valid packet received
	// save it to a txt file
	FILE *fdata;
	time(&tnow);
	struct tm * timeinfo= localtime(&tnow);


	sprintf(fname,"%s.txt",fleadnamescan);
	fdata=fopen(fname,"w"); //Create an empty file
	if (fdata==NULL)
	  { cout<<"fopen() to creat a data file "<<fname<<" failed."<<endl;
		return -1; // return an ERROR
	  }
	fwrite(strRx.c_str(),1,strRx.length(), fdata);
	fclose (fdata);
	printf("save data to %s OK\n", fname);

    // backup the data file
	sprintf(fname2,"%s_%04d%02d%02d_%02dh%02dm%02ds.txt",fleadnamescan, timeinfo->tm_year+1900, timeinfo->tm_mon+1,
				timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	sprintf(temStr, "cp %s %s", fname, fname2);
	system(temStr);
/*
	fdata=fopen(fname2,"w"); //Create an empty file
	if (fdata==NULL)
	  { cout<<"fopen() failed to create a data file ref.txt."<<endl;
		return -1; // return an ERROR
		  }
	fwrite(strRx.c_str(),1,strRx.length(), fdata);
	fclose (fdata);
*/
	printf("backup data in '%s' to '%s' OK\n", fname, fname2);
	return nChar;

//==================================================================================
	// The following codes convert the received string into arrays dataIR and dataUV
	// then pass dataIR and dataUV to a Matlab generated function for Guassian fitting
	// Note: now we save string to *.txt file and have the Matlab function load data from
	//       *.txt file, rather than directly send data arrays.
/*
* 	char dataStr[MAXCOL_DATA*MAXROW_DATA*5*2];
* 	dataStr=strRx.c_str();
*	cout<<"========================================"<<endl;
*	cout<<"After packet check, strRx.length()="<<strRx.length()<<" chars, strRx="<<endl;
*	cout<<strRx<<endl;
*	cout<<"========================================"<<endl;
*
*	printf("\r\n----------------------------------------\r\n");
*	printf("printf full data packet found: %s",dataStr);
*
*	int posA, nSteps, nSam, nFs, nArg;
*	// char charLE="\r\n";
*	string firstLine;
*	size_t posLE;
*	firstLine=strRx.substr(1,(int)(strRx.find("\r\n")));
*	strRx.erase(0,(int)(strRx.find("\r\n"))+1);
*	// cout<< "first line: "<<firstLine<<endl;
*	printf("desired values: posA=%d, nSteps=%d, nSam=%d\r\n",a,nDataSteps_a2b,nSperS);
*	nArg=sscanf(firstLine.c_str(),"M posA=%d, nSteps=%d, nSam=%d, Fs=%d",&posA,&nSteps, &nSam, &nFs);
*	printf("decaped values: posA=%d, nSteps=%d, nSam=%d\r\n",posA,nSteps,nSam);
*	if(nArg!=4 || posA!=a || nSteps!=nDataSteps_a2b || nSam!=nSperS )
*		{ printf("ERROR: wrong M posA=%d, nSteps=%d  nSam=%d from mBed\r\n", posA, nSteps, nSam);
*		  return -1; // return an ERROR
*		  // continue; // continue the main loop
*		}
*
*	// convert ASCII packet into int array
*	char leadStrIR[10], leadStrUV[10];
*	char *pStart, *pEnd;
*	size_t pNextLine;
*	int nDataChar;
*
*	for (i=0;i<nSteps;i++)
*		for (j=0; j<nSam; j++)
*			{ dataIR[i][j]=0xFFFF; dataUV[i][j]=0xFFFF;}
*
*	for (i=0;i<nSteps;i++)
*	{   sprintf(leadStrIR,"dir%03d=[ \0",i); //'\0' to terminate the string
*		found0=strRx.find(leadStrIR);
*		found2=strRx.find("\r\n");
*		nDataChar=found2-(found0+strlen(leadStrIR))-1;
*	    int lencpy=strRx.copy(dataStr,nDataChar,found0+strlen(leadStrIR));
*	    dataStr[lencpy]='\0'; // terminate the string
*		strRx.erase(0,found2+1); // remove one line
*		// cout<< "dataStr contains: "<<dataStr<<" OK "<<endl;
*	    //  printf("%d-th line IR: %s\r\n", i, dataStr);
*	    // printf("dataIR[%d][]=",i);
*	    pEnd=dataStr;
*	    for (int j=0;j<nSam;j++)
*		 	{ dataIR[i][j]=strtol(pEnd, &pEnd,10);
*		 	  // printf(" %d",dataIR[i][j]);
*		 	}
*	    // printf("\r\n");
*
*
*	    sprintf(leadStrUV,"duv%03d=[ \0",i);
*		found0=strRx.find(leadStrUV);
*		found2=strRx.find("\r\n");
*		nDataChar=found2-(found0+strlen(leadStrIR))-1;
*		// cout<<"found0="<<found0<<", found2="<< found2<<" nDataChar="<<nDataChar<<endl;
*		// TODO: check if nDataChar == nSam*(4+1)+2
*		// nDataChar=nSam*(4+1)+2; // how many chars in one data line
*		strRx.copy(dataStr,nDataChar,(int)found0+strlen(leadStrUV));
*	    dataStr[nDataChar]='\0'; // terminate the string
*		strRx.erase(0,found2+1); // remove one line
*		// printf("dataUV[%d][]=",i);
*	    pEnd=dataStr;
*		for (int j=0;j<nSam;j++)
*			dataUV[i][j]=strtol(pEnd, &pEnd,10);
*
*	 }
*	// printf the dataIR and dataUV int array
*	printf("dataIR and dataUV are:\r\n");
*	for (int i=0;i<nSteps;i++)
*		{  int j;
*		   printf("IR03%d=[",i);
*		   for (j=0; j<nSam;j++)
*			   printf(" %04d",dataIR[i][j]);
*		   printf("]\r\n");
*
*		   printf("UV03%d=[",i);
*		   for (j=0; j<nSam;j++)
*		  	   printf(" %04d",dataUV[i][j]);
*		   printf("]\r\n");
*		}
*/
//==================================================================================
	return -1;  // must never reach here
}

int scanProcData(char* fname, int* optIR, int* optUV)
{
	// Gaussian fitting to find the peak location
	  char temStr[200];
	  int fsize[2];
	  fsize[0]=1; fsize[1]=strlen(fname);
	 int optstep=gfit_rdfile(fname, fsize);
	 if (optstep<=0)
	 {
		printf("reading file '%s' or the Gaussian fitting failed. returned value =%d\n",fname,optstep);
		sprintf(temStr,"\r\nreading file '%s' or the Gaussian fitting failed. Returned value =%d\r\n",
						fname,optstep);
		// PC.uartwriteStr(temStr);
		return -1; // return an ERROR
	 }
	 printf("reading file '%s' successes and the opt step =%d\n",fname,optstep);

	// sprintf(temStr,"\r\nreading %s successes and the opt step =%d\r\n",fname,optstep);
	// PC.uartwriteStr(temStr);
	 *optIR=0;
	 *optUV=optstep;
	 return 1;
}

// manually collect data and save data into ref.txt
int manual_scandaq(int posA, int posB, int nSam, int ampIR, int ampUV, char *fleadname)
{    char tempStr[500];
	char fnamescan[20];
	char fnamemeas[20];
	time_t t2;
	int pre_statemain;

	pre_statemain=statemain;

	sprintf(fnamescan, "%sscan", fleadname);
	sprintf(fnamemeas, "%smeas", fleadname);

	struct tm * timeinfo= localtime(&t2);
	sprintf(tempStr,"[%d/%d/%d %02d:%02d:%02d] ",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
					timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	printf("%s start UV/IR measurements\n",tempStr);
	PC.uartwriteStr(tempStr);
	PC.uartwriteStr("\% start UV/IR measurements\n");
	// Skip moving motor to origin
/*
	statemain=MOVEMOTOR; //2
	if (moveMotor2Switch()<=0)
		{  // moveMotor2Switch failed
			cout<<"moveMotor2Switch failed. Cancel data collection"<<endl;
			statemain=pre_statemain;
			return -1;
		}
	mBed.uartwriteStr("setm 1 0 0 100 1\r\n");
	PC.uartwriteStr("setm 1 0 0 100 1\r\n");
	usleep(100);
	mBed.readline();
*/

	statemain=SWN; //3

	if (ampUV<0)
		sprintf(tempStr,"uvt\r\n");
	else
		sprintf(tempStr,"uvs%02d\r\n",ampUV);
	mBed.uartwriteStr(tempStr);
	printf(tempStr); // PC.uartwriteStr(tempStr);
	usleep(100);
	mBed.readline();

	if (ampIR<0)
			sprintf(tempStr,"irt\r\n");
	else
			sprintf(tempStr,"irs%02d\r\n",ampIR);

	mBed.uartwriteStr(tempStr);
	printf(tempStr); // PC.uartwriteStr(tempStr);
	usleep(100);
	mBed.readline();

	mBed.uartwriteStr("irg1\r\n");
	printf("irg1 \r\n"); // PC.uartwriteStr("\% irg1 \r\n");
	usleep(100);
	mBed.readline();


	mBed.uartwriteStr("apdbv 141v\r\n");
	printf("apdbv 141v\r\n"); //PC.uartwriteStr("\% apdbv 141v\r\n");
	usleep(100);
	mBed.readline();

	mBed.flushrxbuf();

		// posA=1601; posB=1900; nSam=MAXCOL_DATA;
		int scanSteps=posB-posA+1;
		int scanOK;
		// swnOK=daqBySWN(posA, scanSteps, nSam, dataIR, dataUV, fleadname);
		scanOK=scanIRUV(posA, scanSteps, nSam, dataIR, dataUV, fnamescan); // for debug

		if (scanOK==-1)
		{ // something wrong
			cout<<"WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop."<<endl;
			PC.uartwriteStr("WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop.\r\n");
			cout<<"============================="<<endl<<endl;
			PC.uartwriteStr("% =======================\r\n\r\n");
        	goto Label_ReturnFailure;
		}


		int optIR, optUV;
		int gfitOK;
		sprintf(tempStr,"%s.txt", fnamescan);
		gfitOK=scanProcData(tempStr, &optIR, &optUV);
        if (gfitOK==-1)
        {   cout<<"WARN: scanProcData() for Gaussian fitting returns WRONG result. Continue main loop."<<endl;
    	goto Label_ReturnFailure;
        }



		sprintf(tempStr,"move -d 1 %d\r\n",optUV);
		mBed.uartwriteStr(tempStr);
		// PC.uartwriteStr(tempStr);
		printf(tempStr);
		usleep(1000);
		mBed.readline(); // wait until motor arrives at optimal position

		// Collect UV IR data at the optimal position

		int daqOK;
		int Fs, nSample;
		Fs=1000;
    	nSample=10*nSam;
    	daqOK=daqIRUV(Fs, nSample, optUV,fnamemeas);
        if(daqOK==-1)
        {
        	cout<<"WARN: daqIRUV() fails. Continue main loop."<<endl;
        	goto Label_ReturnFailure;
        }
		// Process data in refmeas.txt
        printf("daqIRUV() successed. meanIR=, meanUV=\r\n");



//Label_ReturnSuccess:
    	mBed.uartwriteStr("uvt\r\n");
        printf("uvt\r\n"); // PC.uartwriteStr("\% uvt\r\n");
        mBed.uartwriteStr("irt\r\n");
        printf("irt\r\n"); // PC.uartwriteStr("\% irt\r\n");
        cout<<"turn off lights, \r\n   irt\r\n  irs\r\n";
        statemain=pre_statemain;
        return 0;


Label_ReturnFailure:
		mBed.uartwriteStr("irt\r\n");
		printf("irt\r\n"); // PC.uartwriteStr("\% irt\r\n");
		mBed.uartwriteStr("uvt\r\n");
		printf("uvt\r\n"); // PC.uartwriteStr("\% uvt\r\n");

	    cout<<"turn off lights, \r\n   irt\r\n  irs\r\n";
		statemain=pre_statemain;
		return -1; // ignore the data and continue the main loop
}

// Test Matlab Code Generation
void testMatabCode()
{
	/*	// Test codes for getRand() and gfit()

	double xx[100];
	double yy[100];

	time_t rawt = time(NULL); // Seconds since the Epoch. 1970-01-01
	time(&rawt);

	getRand(rawt,yy);
    // getRand(10393,yy);
	printf("[");
	 for (int i=0;i<100;i++)
		{	xx[i]=i;
		    printf("%f, ", yy[i]);
		    // printf("testfunc=%d\n",testfunc(i));
		}
	 printf("]\n");

	 double sigma, mu, A;

	  // // function [sigma, mu, A] = gfit(x,y,h) %#codegen
	  // //  gfit(const real_T x[100], const real_T y[100], real_T h, real_T *sigma, real_T *mu, real_T *A);
	  gfit(xx,yy,0.2, &sigma, &mu, &A);
	  printf("sigma=%f, mu=%f, A=%f\n",sigma, mu, A);
*/

	// test codes for gfit_rdfile
	  char *fname="refscan.txt";
	  int fsize[2];
	  fsize[0]=1; fsize[1]=strlen(fname);
	 int optstep=gfit_rdfile(fname, fsize);
	 if (optstep<=0)
	 {
		printf("reading ref_test.txt or the Gaussian fitting failed.returned value =%d\n",optstep);
		return;
	 }
	 printf("Gaussian fitting at ref_test.txt successes and the opt step =%d\n",optstep);

}
// process UART strings
void process_UART(int *pstatemain)
{

	int chars_read;
	int recvLines_mBed, recvLines_PC;

	recvLines_mBed=0;
	recvLines_PC=0;
	if (*pstatemain!=IDLE && *pstatemain!=MBEDONLY)
		{
		printf("We only process UART when statemain==IDLE || MBEDONLY \r\n");
		PC.uartwriteStr("We only process UART when statemain==IDLE || MBEDONLY \r\n");
		return;
		}

	// statemain is IDLE, process UART and change statemain accordingly
    // Read received mBed data and forward to PC
	// chars_read=mBed.readline();// read characters
	recvLines_mBed=mBed.readline(); // returns how many lines have been received
	if (recvLines_mBed > 0)
	{
		// printf("mBed->BB: %s", mBed.rxbuf);
		// printf("\n"); //mBed may not send \n, manually add \n to stdio
		mBed.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
	}

    // Read received command from PC
	// chars_read = PC.uartread(); // read characters
	recvLines_PC=PC.readline();
	if  (recvLines_PC>0)
	{
		// cout<<"received string from PC statemain="<<*pstatemain<<endl;
		PC.uartwriteStr(PC.rxbuf);
		/*  process special command from PC */
		// NOTE: Putty console does not send \n at the end of the string
		// therefore, not check \n as the end of the string
		if (strncmp(PC.rxbuf, "quit\r",5)==0)
		{   // quit the programme
			PC.uartwriteStr("% Program is going to kill itself, good bye\n");
			sigint_handler(15); // program terminates here
			return;
	     }
		if (strncmp(PC.rxbuf,"settime",7)==0)
		{
			int nArg;
			int yy, mm ,dd, hh, min, ss;
			nArg=sscanf(PC.rxbuf, "settime %d/%d/%d %d:%d:%d",&dd,&mm, &yy, &hh, &min, &ss);
			if (nArg==6)
				setBeagleRTC2(yy, mm, dd, hh, min, ss);
			else
			{PC.uartwriteStr("% settime command has incorrect parameters, Ignored.\r\n");
			printf("%% \"%s\" has incorrect parameters, Ignored.\r\n", PC.rxbuf);
			}
			PC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			return;
		}
		if (strncmp(PC.rxbuf,"mbed\r",5)==0)
		{
			printf("SWITCH from %d to mBed mode (statemain=%d MBEDONLY). Connect to mBed directly\r\n",
					*pstatemain, MBEDONLY);
			PC.uartwriteStr("SWITCH to mBed mode (MBEDONLY). Connect to mBed directly for manual control\r\n");
			*pstatemain=MBEDONLY;
			PC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			return;
		}

		if (strncmp(PC.rxbuf, "idle\r",5)==0)
		{  // quit the programme
		   printf("SWITCH from %d to IDLE mode (statemain=%d IDLE). \r\n",
							*pstatemain, IDLE);
		   PC.uartwriteStr("%switch back to IDLE\r\n");
		   *pstatemain=IDLE;
			PC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			return;
		}

		if (strncmp(PC.rxbuf, "swn&daq\r",5)==0)
				{  // quit the programme

				   int nArg;
				   int pos1, pos2,nS, ampIR, ampUV;
				   char fname_prefix[20];

				   nArg=sscanf(PC.rxbuf, "swn&daq %d %d %d %d %d %s",&pos1,&pos2, &nS, &ampIR, &ampUV, fname_prefix);
				   if (nArg!=6)
				   	{  PC.uartwriteStr("% Incorrect arguments for swn&daq pos1 pos2 nSam ampIR ampUV fname, Ignored.\r\n");
				   		printf("%% \"%s\" has incorrect parameters, Ignored.\r\n", PC.rxbuf);
				   		PC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
				   		return;
				   	}
				   printf("manually collect data and save to %s*.txt at statemain=%d. \r\n",
				   									fname_prefix, *pstatemain);

				   PC.uartwriteStr("%manually collect data and save to");
				   PC.uartwriteStr(fname_prefix);
				   PC.uartwriteStr("*.txt \r\n");

                   printf("posA=%d, posB=%d, nSam=%d, ampIR=%d, ampUV=%d\n", pos1, pos2, nS, ampIR, ampUV);
                   PC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
                   // int manual_daqswn(int posA, int posB, int nSam, int ampIR, int ampUV);
				   manual_scandaq(pos1, pos2, nS,ampIR,ampUV,fname_prefix);

				   // *pstatemain=DAQ;
				   return;
				}


		//Otherwise, forward recieved string to console and mBed
		printf("PC->mBed (line=%d): %s", recvLines_PC, PC.rxbuf);
		// mBed.uartwriteChar(PC.rxbuf, chars_read);
		mBed.uartwriteStr(PC.rxbuf);
        PC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
	} // end of if (PC.readline())
	    /*  DEBUG code: simulate the string received from PC or mBed

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

}  // end of process_UART()

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


int moveMotor2Switch()
{
	int nChars;
	time_t t1, t2;
	double wait_secs;
	double Upbound_secs;
    bool timeover;

    Upbound_secs=30;
    timeover=FALSE;
	time(&t1);
cout<<"move motor to switch position"<<endl;
	motorLED.uSW=0;
	mBed.flushrxbuf();
	while(motorLED.uSW==0)
	{	mBed.uartwriteStr("move -s 1 -2000\r\n");
		PC.uartwriteStr("move -s 1 -2000\r\n");
		do{
			nChars=mBed.readline();
			time(&t2);
			wait_secs=difftime(t2,t1);
			if (wait_secs>Upbound_secs)
			{
				timeover=TRUE;
				break;
			}
		}while(nChars==0);

		if (timeover==FALSE)
		{
			cout<<mBed.rxbuf;
			nChars=sscanf(mBed.rxbuf,
						"%% motor[1] is motorLED: nOrigin=%d, nNow=%d, motorSpd=%f steps/s, fullStep=%d, statusLEDMotor=%d, uSW(p29)=%d",
						&motorLED.nOrigin, &motorLED.nNow,&motorLED.motorSpd, &motorLED.fullStep, &motorLED.statusLEDMotor, &motorLED.uSW);
			// cout<<"movemotor2switch() receives  "<< mBed.rxbuf<<endl;
			// cout<<"nChars="<<nChars<<", ";
			cout<<"nNow="<<motorLED.nNow<<", return nChars="<<nChars<<endl;
			return nChars;
			// return 1;
		}
		else
		{ // over time, no response
			cout<<"moveMotor2Switch() time over. give up moving motor"<<endl;
			return -1;
		}

	}

	// should never reach here
	return -1;

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

   printf("Enter  $ sudo /home/ubuntu/restore_DHCPconsole\r\n");
   printf("     to reboot the Beagle with DHCP and console over ttyO2\r\n");
   printf("Enter  $ sudo /home/ubuntu/restore_IP10.2.1.3ttyO2\r\n");
   printf("     to reboot the Beagle with IP=10.2.1.3 and without console over ttyO2\r\n");

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
	 int year, month ,day, hh, min, ss;

	 printf("Do you want to set a new time? (Y/N)\r");
	 char chgTime;
	 // getc("%c",&chgTime);
	 chgTime=getchar();
	 if (chgTime=='Y' || chgTime=='y')
	 {	 /* prompt user for date */
	 	 printf("Enter day/month/year: "); scanf("%d/%d/%d",&day, &month, &year);
	     printf("Enter hour:minute:second "); scanf("%d:%d:%d",&hh, &min, &ss);
	     // setBeagleRTC2(year, month, day, hh, min, ss);
	    setBeagleRTC3(year, month, day, hh, min, ss);
	 }
}

void setBeagleRTC2(int yy, int mm, int dd, int hh, int min, int ss)
{
	time_t mytime, t2;
    char tempStr[500];
	// struct timespec rawtime;
	struct timeval systime;
	 struct tm * timeinfo_ptr;
	 struct tm timeinfo;

	 char * weekday[] = { "Sunday", "Monday",
	                      "Tuesday", "Wednesday",
	                      "Thursday", "Friday", "Saturday"};

	 //  When interpreted as an absolute time value, it represents  the number of
	 // seconds elapsed since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
	 timeinfo.tm_year = yy - 1900;
	 timeinfo.tm_mon = mm - 1;
	 timeinfo.tm_mday = dd;
	 timeinfo.tm_hour=hh;
	 timeinfo.tm_min=min;
	 timeinfo.tm_sec=ss;

	 printf("To set Beagle Time to: %d/%d/%d %02d:%02d:%02d\n",
			 timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday,
			 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);


    /* call mktime: timeinfo->tm_wday will be set */
	systime.tv_sec=mktime ( &timeinfo );
	systime.tv_usec=0;
	settimeofday(&systime,NULL);

	 /* Time of day is set.  Now, let's get it, and verify that it looks right */
    printf("Read new time from Beagle (by time()): ");   //Confirmation!!
	time(&t2);
	timeinfo_ptr= localtime(&t2);
	sprintf(tempStr,"[%d/%d/%d %02d:%02d:%02d] ",timeinfo_ptr->tm_year+1900, timeinfo_ptr->tm_mon+1,
	 	    					timeinfo_ptr->tm_mday,timeinfo_ptr->tm_hour, timeinfo_ptr->tm_min, timeinfo_ptr->tm_sec);
	cout<<tempStr<<endl;


	char * text_time;
	gettimeofday(&systime, NULL);
	text_time = ctime(&systime.tv_sec);
    printf("The system time has been set to %s\n", text_time);

    cout<<"setBeagleRTC2 done"<<endl<<endl;
}

void setBeagleRTC3(int yy, int mm, int dd, int hh, int min, int ss)
{
	    //MMDDhhmmYY.ss
	    char dTime[26] = "sudo date 012414272013.30";   //24 Jan 2013 14:27:30
	    int n;

	    sprintf(dTime,"sudo date %02d%02d%02d%02d%04d.%02d",mm,dd,hh,min,yy, ss);
	    system(dTime);

	    printf("Date and time has been changed to: ");   //Confirmation!!
        // read system time
	        char tempStr[500];
	    	time_t t2;

	    	time(&t2);
	       	struct tm * timeinfo= localtime(&t2);
	    	sprintf(tempStr,"[%04d/%02d/%02d %02d:%02d:%02d] ",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
	    					timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	    	cout<<tempStr<<endl<<endl;

}


