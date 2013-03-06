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

#include "main.h"

#include "uartBeagle.h"
#include "LoadmonDriver.h"
#include "sigProcess.h"
// #include "test.h"

#include "RdConfigFile.h"
#include "WrtDataLogFile.h"

extern struct config configstruct;
extern struct struct_DataLog currentdatalog;


// Now file names of UART to mBed and PC are stored in config.conf file
// and system parameters are read from  CONFIGFILENAME and converted into struct config

LoadmonDriver daqModule;


#define MAXROW_DATA 500
#define MAXCOL_DATA 500

// default sampling rate for scanning
#define FS_SCAN 500

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





// void process_UART();
void process_UART(int *pstatemain);
void  sigint_handler(int sig);
void init_main(char *pNamemBed);

void readTime(int *yy, int *mm, int *dd, int *hh, int *min, int *ss);
void readTime2(time_t tnow, int *yy, int *mm, int *dd, int *hh, int *min, int *ss);
int recordData(time_t tlastrecord, time_t tnow, struct struct_DataLog *pDataLog);

int manual_scandaq(int posA, int posB, int nSam, int ampIR, int ampUV, char *fleadname, int nSmeas);
int auto_scandaq(struct scanArg *pscanArg, struct daqArg* pdaqArg, char *fleadname);
// int daqUVIR(int Fs, int nSamples, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA]);

void setBeagleRTC(void);
void setBeagleRTC2(int yy, int mm, int dd, int hh, int min, int ss);
void setBeagleRTC3(int yy, int mm, int dd, int hh, int min, int ss);

#define UARTNAME_PC "/dev/ttyO2"
#define DATAFILEPATH "./data/"
#define DAQINTERVAL_s 60

int main(int argc, char* argv[]) {

	int i;
	unsigned int nDAQ;
    double elapsed_secs;
    char *pNamemBed;
    char pNamePC[20]="/dev/ttyO2";
    int nChars;
    char tempStr[500];
    int returnstate; // a variable returned by a function
    int yy,mm, dd, hh, min, ss;

    cout << "\n!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    // cout<<"testfunc()="<<testfunc(5)<<endl;

    // Initialise global variables, display time, open UART ports, etc.
    statemain=INIT; // 0;

	// read configuration iile and  initialise londmon driver
    if (argc<=1)
    	{ returnstate=daqModule.initDriver(CONFIGFILENAME); }
    else
    	{   pNamemBed=argv[1]; cout <<argv[1]<<endl;
    	returnstate=daqModule.initDriver(CONFIGFILENAME, pNamemBed, UARTNAME_PC);
    	}

    if (returnstate!=EXIT_SUCCESS)
    {
    	cout<<"LoadmonDriver::initDriver() failed. Exit.\r\n";
    	return EXIT_FAILURE;
    }

    /* Check struct members */
	int posA, posB;
	printf("\r\nposA=%d, posB=%d, SpS=%d, apdBV=%f\r\n", configstruct.refscan.posA, configstruct.refscan.posB,
		configstruct.refscan.SpS, configstruct.refscan.apdBV);
	daqModule.selftest();
	sleep(1);

	// Set BB's clock
	setBeagleRTC();

	// init and test datalog.txt file

	readTime(&yy, &mm, &dd, &hh, &min, &ss);
	char datafilename[100];
	sprintf(currentdatalog.nameLogFile,"%sdatalog_%04d%02d%02d.txt",DATAFILEPATH, yy, mm, dd);
	if (openDataLogFile(&currentdatalog, currentdatalog.nameLogFile)==EXIT_FAILURE)
	{
		perror("Open data log file datalog.txt failed\r\n");
		return EXIT_FAILURE;
	}
	initDataLog(&currentdatalog, hh, min, ss);
	daqModule.readTsetV(&(currentdatalog.tempDeg), &(currentdatalog.apdbv), &(currentdatalog.aomv));
	appendDataLog(&currentdatalog);

	daqModule.oPC.uartwriteStr("Beagle starts...\r\n");
    // string tempStr2("dir%d=[%d %d %d %d %d ]");
    // string strTest("dir02=[0001 0002 0003 0004 0005 ]");
    // sscanf(strTest.c_str(),"dir%d=[%f %f %f %f %f ]",&n, &ax[0], &ax[1], &ax[2], &ax[3], &ax[4]);
    // sscanf(strTest.c_str(),tempStr2.c_str(),&n, ax, ax+1, ax+2, ax+3, ax+4);
    // cout<<"dir"<<n<<"=["<<ax[0]<<ax[1]<<ax[2]<<ax[3]<<ax[4]<<" ]"<<endl;


	// Test Matlab Code Generation
    printf(" Testing file reading & Gaussian fitting\r\n");
	testMatabCode();

	// Assign a handler to close the serial port on Ctrl+C.
	signal (SIGINT, &sigint_handler);
	printf("Press Ctrl+C to exit the program.\n");



	statemain=MBEDONLY; // 1
	daqModule.oPC.uartwriteStr("\r\n  Enter mbed mode by default for controlling mbed manually\r\n");
	printf("Enter mbed mode by default for controlling mbed manually\r\n");

	daqModule.oPC.uartwriteStr("  Waiting for commands from USB/RS232 accessport\r\n");
	printf("  Waiting for commands from USB/RS232 accessport (for command information, please reset mbed\r\n");

	daqModule.oPC.uartwriteStr("      to see the mbed command, please reset mbed\r\n");
	printf("      to see the mbed command, please reset mbed\r\n");

	printf("      to switch to Beagle mode for automatic data collection, please type idle followed by ENTER\r\n");
	printf("      to stop program, type quit followed by ENTER. Wait for a few seconds\r\n");
	daqModule.oPC.uartwriteStr("      to switch to Beagle mode for automatic data collection, please type idle followed by ENTER\r\n");
	daqModule.oPC.uartwriteStr("      to stop program, type quit followed by ENTER. Wait for a few seconds\r\n");

	// The main program loop:
	time_t t1, t2;
	time(&t1);
	t1-=60; // for debug purpose: start first DAQ immediately
	nDAQ=0;
	printf("main loop starts\r\n");
	daqModule.oPC.uartwriteStr(" Main loop starts \r\n");

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
		if (elapsed_secs>=DAQINTERVAL_s)
			{
			nDAQ++;
			readTime2(t2, &yy, &mm, &dd, &hh, &min, &ss);
			sprintf(tempStr,"\r\n[%d/%d/%d %02d:%02d:%02d] start %d-th UV/IR measurements\n",yy,mm,dd,hh,min,ss, nDAQ);
			printf("%s", tempStr);
			daqModule.oPC.uartwriteStr(tempStr);
			 // collect data
			 initDataLog(&currentdatalog, hh, min, ss);
			 daqModule.readTsetV(&(currentdatalog.tempDeg), &(currentdatalog.apdbv), &(currentdatalog.aomv));

			 //reference scanning and daq
			 auto_scandaq(&(configstruct.refscan), &(configstruct.refdaq), "ref");
			 // manual_scandaq(1750, 1900,10,25,00,"ref", 2000); // for debug

			 // water scanning and daq
			 // auto_scandaq(&(configstruct.wtrscan), &(configstruct.wtrdaq),"wtr");

			 printf("%d-th reference measurement DONE \r\n\r\n",nDAQ);
			 // printf("skip %d-th DAQ for debugging \r\n\r\n", nDAQ);
			 // statemain=DAQ; // to start data acquisition

			 // record data
			 recordData(t1, t2,&currentdatalog);
			 t1=t2;
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
		daqModule.oPC.uartwriteStr(tempStr);
		sprintf(tempStr,"start %d-th UV/IR measurement\n",nDAQ);
		daqModule.oPC.uartwriteStr(tempStr);

		auto_scandaq(&(configstruct.wtrscan), &(configstruct.wtrdaq),"wtr");

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
    daqModule.oPC.uartwriteStr("EXCEPTION main loop exit\r\n");
    sleep(1);
    // force terminate program
   	sigint_handler(15); // program terminates here
    return 0;
}

int recordData(time_t tlastrecord, time_t tnow, struct struct_DataLog *pDataLog)
{
    int y0,m0, d0, hh0, min0, ss0;
    int yn,mn, dn, hhnow, minnow, ssnow;
	struct tm * timeinfo= localtime(&tlastrecord);
	 y0=timeinfo->tm_year+1900;	 m0=timeinfo->tm_mon+1;
	 d0=timeinfo->tm_mday;	 hh0=timeinfo->tm_hour;
	 min0=timeinfo->tm_min;	 ss0=timeinfo->tm_sec;

	 timeinfo= localtime(&tnow);
	 yn=timeinfo->tm_year+1900;	 mn=timeinfo->tm_mon+1;
	 dn=timeinfo->tm_mday;	 hhnow=timeinfo->tm_hour;
	 minnow=timeinfo->tm_min;	 ssnow=timeinfo->tm_sec;

	// if (hhnow!=hh0)
	if(minnow!=min0)
	 { //different hour, close file to save data and reopen it

		closeDataLogFile(pDataLog->handleLogFile);

		if(dn!=d0)
		{ // new day, create new file
			sprintf(pDataLog->nameLogFile,"%sdatalog_%04d%02d%02d.txt",DATAFILEPATH, yn, mn, dn);
		}
		else
		{// same day

		}




		if (openDataLogFile(pDataLog, pDataLog->nameLogFile)==EXIT_FAILURE)
		{
			perror("Open data log file datalog.txt failed\r\n");
			return EXIT_FAILURE;
		}
	 }

	 appendDataLog(pDataLog);
		printf("save currentDatalog to %s ... OK\r\n",pDataLog->nameLogFile);
}


int auto_scandaq(struct scanArg *pscanArg, struct daqArg* pdaqArg, char *fleadname)
{
	int posA, posB, nSam, ampIR, ampUV;
	int nSmeas;
	posA=pscanArg->posA; posB=pscanArg->posB;
	nSam=pscanArg->SpS; ampIR=pscanArg->ampIR; ampUV=pscanArg->ampUV;

	nSmeas=pdaqArg->nSam;
	return manual_scandaq(posA, posB, nSam, ampIR, ampUV, fleadname, nSmeas);
}

// manually scan and collect IR/UV data and save data into ???scan.txt, ???ir.txt, ???uv.txt, etc.
int manual_scandaq(int posA, int posB, int nSam, int ampIR, int ampUV, char *fleadname, int nSmeas)
{    char tempStr[500];
	char fnamescan[20];
	char fnameIR[20];
	char fnameUV[20];
	time_t t2;
	int pre_statemain;

	pre_statemain=statemain;
	statemain=SWN; //3

	bool refModel; // 1 for reference model; 0 for water sample

	struct scanArg *pscanArg;

	if (strstr(fleadname,"ref")==NULL)
		{//no ref in the filename, not ref model
			refModel=0;
			pscanArg=&(configstruct.wtrscan);
		}
	else
	{// find ref in the filename, ref model
		refModel=1;
		pscanArg=&(configstruct.refscan);
	}

	int pkUP, pkLOW;
	pkLOW=pscanArg->PkRange[0]; pkUP=pscanArg->PkRange[1];

	sprintf(fnamescan, "%sscan", fleadname);
	sprintf(fnameIR, "%sir", fleadname);
	sprintf(fnameUV, "%suv", fleadname);

	// 1. scan
	// daqModule.readTsetV(&(currentdatalog.tempDeg), &(currentdatalog.apdbv), &(currentdatalog.aomv));
	if (daqModule.scanIRUV(posA, posB,nSam,FS_SCAN,ampIR, 1, ampUV, 1, currentdatalog.apdbv, fnamescan)==-1)
		{ // something wrong
			cout<<"WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop."<<endl;
			daqModule.oPC.uartwriteStr("WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop.\r\n");
			cout<<"============================="<<endl<<endl;
			daqModule.oPC.uartwriteStr("% =======================\r\n\r\n");
        	goto Label_ReturnFailure;
		}

    // 2. Gaussian alignment
		int optIR, optUV;
        double muIR, muUV;
        double stdIR, stdUV;

		int gfitOK;
		sprintf(tempStr,"%s.txt", fnamescan);
		gfitOK=procScanData(tempStr, &optIR, &optUV);

        if (gfitOK!=EXIT_SUCCESS)
        {   cout<<"WARN: procScanData() for Gaussian fitting returns WRONG result. Continue main loop."<<endl;
    	goto Label_ReturnFailure;
        }

        if (refModel)
        	{
        	  currentdatalog.pkIRref=optIR;  currentdatalog.sigmaIRref=INVALIDVALUE;
        	  currentdatalog.pkUVref=optUV;  currentdatalog.sigmaUVref=INVALIDVALUE;
        	}
        else
        	{
        	currentdatalog.pkIRwtr=optIR;  	currentdatalog.sigmaIRwtr=INVALIDVALUE;
        	currentdatalog.pkUVwtr=optUV; 	currentdatalog.sigmaUVwtr=INVALIDVALUE;
            }
/*
        if ( (optIR<pkLOW || optIR>pkUP) && (optUV<pkLOW || optUV>pkUP) )
        	{ cout << "WARN: Both peaks of IR and UV are invalid. Skip daq"<<endl;
            if (refModel)
            	{ currentdatalog.IRref=INVALIDVALUE; currentdatalog.stdIRref=INVALIDVALUE;
            	  currentdatalog.UVref=INVALIDVALUE; currentdatalog.stdUVref=INVALIDVALUE;}
            else
            	{ currentdatalog.IRwtr=INVALIDVALUE; currentdatalog.stdIRwtr=INVALIDVALUE;
            	  currentdatalog.UVwtr=INVALIDVALUE; currentdatalog.stdUVwtr=INVALIDVALUE;
            	}
        	goto Label_ReturnFailure;
        	}
*/

    // Collect UV IR data at the optimal position
    // 3. measure IR first
    	if (refModel)
		{ currentdatalog.IRref=INVALIDVALUE; currentdatalog.stdIRref=INVALIDVALUE;}
		else
		{ currentdatalog.IRwtr=INVALIDVALUE; currentdatalog.stdIRwtr=INVALIDVALUE; }

        if (optIR>=pkLOW && optIR<=pkUP)
        { // Valid IR peak
        	if(daqModule.daqIRUV(optIR, nSmeas, FS_SCAN, ampIR, 1, ampUV, 1, currentdatalog.apdbv, fnameIR)==-1)
             {
        		cout<<"WARN: daqIRUV() for IR ("<<fnameIR<<".txt) fails. Continue main loop."<<endl;
        		goto Label_ReturnFailure;
             }

        	//TODO: Possible bug: If nSmeas is over 5000 in above daqIRUV() call,
        	//                    an error of segmentation will occur in the following codes
        	//

        	// TODO: call matlab function calculate mean EIR
        	sprintf(tempStr,"%s.txt", fnameIR);
        	procDaqData2(tempStr,&muIR, &muUV, &stdIR, &stdUV);

        	printf("daqIRUV() for IR at MS=%d successes (%s.txt). muIR=%f\r\n", optIR, fnameIR,muIR);

        	if (refModel)
        	{ currentdatalog.IRref=muIR; currentdatalog.stdIRref=stdIR;	}
        	else
        	{ currentdatalog.IRwtr=muIR; currentdatalog.stdIRwtr=stdIR; }
        }
        else
        { // Invalid IR peak
        	printf("WARN: Invalid IR peak %d out of [%d,%d]. Skip daq\r\n",optIR,pkLOW, pkUP);
        }
        // end of IR measurement if


   // 4. measure UV second
    	if (refModel)
		{ currentdatalog.UVref=INVALIDVALUE; currentdatalog.stdUVref=INVALIDVALUE;}
		else
		{ currentdatalog.UVwtr=INVALIDVALUE; currentdatalog.stdUVwtr=INVALIDVALUE; }

        if (optUV>=pkLOW && optUV<=pkUP)
        { // Valid UV peak
          if(daqModule.daqIRUV(optUV, nSmeas, FS_SCAN, ampIR, 1, ampUV, 1, currentdatalog.apdbv, fnameUV)==-1)
        	{      	cout<<"WARN: daqIRUV() for UV ("<<fnameUV<<".txt) fails. Continue main loop."<<endl;
                	goto Label_ReturnFailure;
        	}
		  sprintf(tempStr,"%s.txt", fnameUV);
		  // procDaqData(tempStr,&muIR, &muUV);
		  procDaqData2(tempStr,&muIR, &muUV, &stdIR, &stdUV);
          printf("daqIRUV() for UV at MS=%d successes (%s.txt). muUV=%f\r\n", optUV, fnameUV, muUV);
          if (refModel)
        	{ currentdatalog.UVref=muUV; currentdatalog.stdUVref=stdUV;	}
          else
        	{ currentdatalog.UVwtr=muUV;currentdatalog.stdUVwtr=stdUV;}
        }
        else
        { // Invalid UV peak
        	printf("WARN: Invalid UV peak %d out of [%d,%d]. Skip daq\r\n",optUV, pkLOW, pkUP);
        }


  // 5. return
//Label_ReturnSuccess:
        cout<<"SUCCESS: Both scan and daq are OK.\r\n";
        statemain=pre_statemain;
        return EXIT_SUCCESS;


Label_ReturnFailure:
	    cout<<"FAILED: turn off lights.\r\n";
		statemain=pre_statemain;
		return -1; // ignore the data and continue the main loop
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
		daqModule.oPC.uartwriteStr("We only process UART when statemain==IDLE || MBEDONLY \r\n");
		return;
		}

	// statemain is IDLE, process UART and change statemain accordingly
    // Read received mBed data and forward to PC
	// chars_read=mBed.readline();// read characters
	recvLines_mBed=daqModule.omBed.readline(); // returns how many lines have been received
	if (recvLines_mBed > 0)
	{
		// printf("mBed->BB: %s", mBed.rxbuf);
		// printf("\n"); //mBed may not send \n, manually add \n to stdio
		daqModule.omBed.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
	}

    // Read received command from PC
	// chars_read = PC.uartread(); // read characters
	recvLines_PC=daqModule.oPC.readline();
	if  (recvLines_PC>0)
	{
		// cout<<"received string from PC statemain="<<*pstatemain<<endl;
		DEBUG2PCUART("% PC->BB: ");
		DEBUG2PCUART(daqModule.oPC.rxbuf);

		// PC.uartwriteStr(PC.rxbuf);
		/*  process special command from PC */
		// NOTE: Putty console does not send \n at the end of the string
		// therefore, not check \n as the end of the string
		if (strncmp(daqModule.oPC.rxbuf, "quit\r",5)==0)
		{   // quit the programme
			daqModule.oPC.uartwriteStr("% Program is going to kill itself, good bye\n");
			sigint_handler(15); // program terminates here
			return;
	     }
		if (strncmp(daqModule.oPC.rxbuf,"settime",7)==0)
		{
			int nArg;
			int yy, mm ,dd, hh, min, ss;
			nArg=sscanf(daqModule.oPC.rxbuf, "settime %d/%d/%d %d:%d:%d",&dd,&mm, &yy, &hh, &min, &ss);
			if (nArg==6)
				setBeagleRTC2(yy, mm, dd, hh, min, ss);
			else
			{daqModule.oPC.uartwriteStr("% settime command has incorrect parameters, Ignored.\r\n");
			printf("%% \"%s\" has incorrect parameters, Ignored.\r\n", daqModule.oPC.rxbuf);
			}
			daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			return;
		}
		if (strncmp(daqModule.oPC.rxbuf,"mbed\r",5)==0)
		{
			printf("SWITCH from %d to mBed mode (statemain=%d MBEDONLY). Connect to mBed directly\r\n",
					*pstatemain, MBEDONLY);
			daqModule.oPC.uartwriteStr("SWITCH to mBed mode (MBEDONLY). Connect to mBed directly for manual control\r\n");
			*pstatemain=MBEDONLY;
			daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			return;
		}

		if (strncmp(daqModule.oPC.rxbuf, "idle\r",5)==0)
		{  // quit the programme
		   printf("SWITCH from %d to IDLE mode (statemain=%d IDLE). \r\n",
							*pstatemain, IDLE);
		   daqModule.oPC.uartwriteStr("%switch back to IDLE\r\n");
		   *pstatemain=IDLE;
		   daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			return;
		}

		if (strncmp(daqModule.oPC.rxbuf, "mv2swt\r",7)==0 && *pstatemain==MBEDONLY)
		{

			int pre_statemain;
			pre_statemain=*pstatemain;
			statemain=MOVEMOTOR; //2
			// if (moveMotor2Switch()<=0)
			if (daqModule.moveMotor2Switch()<=0)
				{  // moveMotor2Switch failed
					cout<<"daqModule.moveMotor2Switch failed."<<endl;
					*pstatemain=pre_statemain;
					return;
				}
			cout<<"daqModule.moveMotor2Switch OK."<<endl;
			daqModule.omBed.uartwriteStr("setm 1 0 0 100 1\r\n");
			DEBUGF("BB->mBed: setm 1 0 0 100 1\r\n");
			usleep(10000);
			daqModule.omBed.readline();
			*pstatemain=pre_statemain;
			return;
		}

		if (strncmp(daqModule.oPC.rxbuf, "swn&daq",7)==0)
				{  // quit the programme

				   int nArg;
				   int pos1, pos2,nS, ampIR, ampUV, nSmeas;
				   char fname_prefix[20];

				   nArg=sscanf(daqModule.oPC.rxbuf, "swn&daq %d %d %d %d %d %s %d",&pos1,&pos2, &nS, &ampIR, &ampUV, fname_prefix, &nSmeas);
				   if (nArg!=7)
				   	{  daqModule.oPC.uartwriteStr("% Incorrect arguments for swn&daq pos1 pos2 nSam ampIR ampUV fname, Ignored.\r\n");
				   		printf("%% \"%s\" has incorrect parameters, Ignored.\r\n", daqModule.oPC.rxbuf);
				   		daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
				   		return;
				   	}
				   printf("manually collect data and save to %s*.txt at statemain=%d. \r\n",
				   									fname_prefix, *pstatemain);

				   daqModule.oPC.uartwriteStr("%manually collect data and save to");
				   daqModule.oPC.uartwriteStr(fname_prefix);
				   daqModule.oPC.uartwriteStr("*.txt \r\n");

                   printf("posA=%d, posB=%d, nSam=%d, ampIR=%d, ampUV=%d\n", pos1, pos2, nS, ampIR, ampUV);
                   daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
                   // int manual_daqswn(int posA, int posB, int nSam, int ampIR, int ampUV);
				   manual_scandaq(pos1, pos2, nS,ampIR,ampUV,fname_prefix, nSmeas);

				   // *pstatemain=DAQ;
				   return;
				}

		if (strncmp(daqModule.oPC.rxbuf, "daq",3)==0)
			{
			   int nArg;
			   int pos1, ampIR, gainIR, ampUV, gainUV, nSmeas;
			   char fname_prefix[20];
               char tempStr[100];
               double muIR, muUV;
			   nArg=sscanf(daqModule.oPC.rxbuf, "daq %d %d %d %d %d %s %d",&pos1, &ampIR, &gainIR, &ampUV, &gainUV,fname_prefix, &nSmeas);
			   if (nArg!=7)
			      	{  daqModule.oPC.uartwriteStr("% Incorrect arguments for daq pos1 ampIR gainIR ampUV gainUV fname nSmeas, Ignored.\r\n");
			   		   		printf("%% \"%s\" has incorrect parameters, Ignored.\r\n", daqModule.oPC.rxbuf);
			   		   		daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
			   		   		return;
			      	}

				if(daqModule.daqIRUV(pos1, nSmeas, FS_SCAN, ampIR, gainIR, ampUV, gainUV, 141, fname_prefix)==-1)
				{
							cout<<"WARN: daqIRUV() for UV ("<<fname_prefix<<".txt) fails. Continue main loop."<<endl;
			   		   		return;
				}
				sprintf(tempStr,"%s.txt", fname_prefix);
				procDaqData(tempStr,&muIR, &muUV);
				printf("daqIRUV() for UV at MS=%d successes (%s.txt). meanUV=%f\r\n", pos1, fname_prefix, muUV);
				return;
			}
		if (strncmp(daqModule.oPC.rxbuf, "readTsetV",9)==0)
			{
				if (daqModule.readTsetV()!=EXIT_SUCCESS)
					cout<<"WARN: readTsetV() failed. "<<endl;
				return;
			}


		//Otherwise, forward recieved string to console and mBed
		printf("PC->BB->mBed (line=%d): %s", recvLines_PC, daqModule.oPC.rxbuf);
		// mBed.uartwriteChar(PC.rxbuf, chars_read);
		daqModule.omBed.uartwriteStr(daqModule.oPC.rxbuf);
		daqModule.oPC.rxbuf[0]='\0'; // To enhance safety, make sure there is no string in the rxbuf
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




// Executes when the user presses Ctrl+C.
// Closes the port, resets the terminal, and exits the program.

void  sigint_handler(int sig)
{   printf("\n======================================================\n");
	printf("Please remember restore file /etc/init/ttyO2.conf \n");
	printf("   $ sudo cp ttyO2.conf.save ttyO2.conf\n");
	printf("======================================================\n");
	printf("GOOD LUCK\n\n");

	daqModule.omBed.uartclose();
	daqModule.oPC.uartclose();
	closeDataLogFile(currentdatalog.handleLogFile);
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

}

void readTime(int *yy, int *mm, int *dd, int *hh, int *min, int *ss)
{
	 time_t tnow;
	 time(&tnow);
	 struct tm * timeinfo= localtime(&tnow);
	 *yy=timeinfo->tm_year+1900;
	 *mm=timeinfo->tm_mon+1;
	 *dd=timeinfo->tm_mday;
	 *hh=timeinfo->tm_hour;
	 *min=timeinfo->tm_min;
	 *ss=timeinfo->tm_sec;
}

void readTime2(time_t tnow, int *yy, int *mm, int *dd, int *hh, int *min, int *ss)
{

	 time(&tnow);
	 struct tm * timeinfo= localtime(&tnow);
	 *yy=timeinfo->tm_year+1900;
	 *mm=timeinfo->tm_mon+1;
	 *dd=timeinfo->tm_mday;
	 *hh=timeinfo->tm_hour;
	 *min=timeinfo->tm_min;
	 *ss=timeinfo->tm_sec;
}


void setBeagleRTC(void)
{
	 int year, month ,day, hh, min, ss;

	 daqModule.oPC.uartwriteStr("  Set Beagle's time and date ... ");

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

	 // read system time
	 char tempStr[500];
	 time_t t2;

	 time(&t2);
	 struct tm * timeinfo= localtime(&t2);
	 readTime(&year, &month, &day, &hh, &min, &ss);
	 sprintf(tempStr,"[%04d/%02d/%02d %02d:%02d:%02d]\r\n",year, month,day,hh,min, ss);
	 cout<<tempStr;

	 sprintf(tempStr,"[%04d/%02d/%02d %02d:%02d:%02d] ",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
				timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	 daqModule.oPC.uartwriteStr("  OK! ");
	 daqModule.oPC.uartwriteStr(tempStr);
	 daqModule.oPC.uartwriteStr("\r\n");
	 cout<<tempStr;


	 struct timeval systime;
	 gettimeofday(&systime, NULL);
	 char * text_time = ctime(&systime.tv_sec);
	 printf(text_time);


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


