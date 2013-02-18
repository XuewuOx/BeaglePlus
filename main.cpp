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

// Change the PORT_NAME for proparait  serial port
// static const char *PORT_NAME = "/dev/ttyUSB0"; // for AirLink USB-232 converter
static const char *PORTMBED_NAME0 = "/dev/ttyACM0"; // for mBed USB-232
// static const char *PORTMBED_NAME1 = "/dev/ttyACM1"; // for mBed USB-232
static const char *PORTPC_NAME = "/dev/ttyO2";  // for default RS232 console at BB
// uartBeagle mBed("mBed");
// uartBeagle PC("PC");

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
void testMatabCode();
void init_main(char *pNamemBed);
int moveMotor2Switch();
int moveMotor2Dest(int dest);

int scanIRUV(int a, int b, int nSperS, int dataIR[][MAXCOL_DATA], int dataUV[][MAXCOL_DATA],char *fleadname);
int daqIRUV(int Fs, int nSamples, int posMS, char * fname);
int procScanData(char* fname, int* optIR, int* optUV);

int manual_scandaq(int posA, int posB, int nSam, int ampIR, int ampUV, char *fleadname, int nSmeas);
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
    // cout<<"testfunc()="<<testfunc(5)<<endl;

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
    daqModule.initDriver(pNamemBed,(char *)PORTPC_NAME);
    daqModule.oPC.uartwriteStr("Beagle starts...\r\n");

    setBeagleRTC();
    printf(" Testing file reading & Gaussian fitting\r\n");
	// Test Matlab Code Generation
	testMatabCode();

	usleep(1000);
    // PC.uartwriteStr("PC.uartwriteStr() success\n");
    usleep(1000);
    //----------------------------------


	// PC.uartwriteStr("resetmbed\r\n");
	usleep(1000);

	// Assign a handler to close the serial port on Ctrl+C.
	signal (SIGINT, &sigint_handler);
	printf("Press Ctrl+C to exit the program.\n");


	daqModule.oPC.uartwriteStr("  Testing the motor ...");
	// PC.uartwriteStr("% setm 1 0 30 100 1\r\n");
	// printf("setm 1 0 30 100 1\r\n");
	// mBed.uartwriteStr("setm 1 0 30 100 1\r\n");
	// usleep(100);

	// while (mBed.readline()==0){}
	daqModule.omBed.flushrxbuf();
	printf("move -s 1 100\r\n");
	daqModule.omBed.uartwriteStr("move -s 1 100\r\n");
	usleep(1000);
	// while (mBed.readline()==0){}
	if (daqModule.omBed.readlineTimeOut(100000)==-1)
		{
		cout<<"Failed to communicate with mBed to move motor\r\n";
		return -1;
		}

	daqModule.oPC.uartwriteStr(" ... ");
	printf("move -s 1 -100\r\n");
	daqModule.omBed.uartwriteStr("move -s 1 -100\r\n");
	usleep(1000);
	while (daqModule.omBed.readline()==0){}

	daqModule.oPC.uartwriteStr(" OK\r\n");





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
		daqModule.oPC.uartwriteStr(tempStr);
		sprintf(tempStr,"start %d-th UV/IR measurement\n",nDAQ);
		daqModule.oPC.uartwriteStr(tempStr);

		manual_scandaq(100, 300,10,25,00,"wtr", 10000);

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
    usleep(1000);
	sigint_handler(15);
    return 0;
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

	sprintf(fnamescan, "%sscan", fleadname);
	sprintf(fnameIR, "%sir", fleadname);
	sprintf(fnameUV, "%suv", fleadname);

	// 1. scan
	if (daqModule.scanIRUV(posA, posB,nSam,FS_SCAN,ampIR, 1, ampUV, 1, 141, fnamescan)==-1)
		{ // something wrong
			cout<<"WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop."<<endl;
			daqModule.oPC.uartwriteStr("WARN: SOMTHING WRONG in executing daqBySWN(). Skip and continue main loop.\r\n");
			cout<<"============================="<<endl<<endl;
			daqModule.oPC.uartwriteStr("% =======================\r\n\r\n");
        	goto Label_ReturnFailure;
		}

    // 2. Gaussian alignment
		int optIR, optUV;
		int gfitOK;
		sprintf(tempStr,"%s.txt", fnamescan);
		gfitOK=procScanData(tempStr, &optIR, &optUV);

        if (gfitOK==-1)
        {   cout<<"WARN: procScanData() for Gaussian fitting returns WRONG result. Continue main loop."<<endl;
    	goto Label_ReturnFailure;
        }


	// Collect UV IR data at the optimal position
    // 3. measure IR first
        if(daqModule.daqIRUV(optIR, nSmeas, FS_SCAN, ampIR, 1, ampUV, 1, 141, fnameIR)==-1)
        {
        	cout<<"WARN: daqIRUV() for IR ("<<fnameIR<<".txt) fails. Continue main loop."<<endl;
        	goto Label_ReturnFailure;
        }
        // TODO: call matlab function calculate mean EIR
        double muIR, muUV;
		sprintf(tempStr,"%s.txt", fnameIR);
		procDaqData(tempStr,&muIR, &muUV);
        printf("daqIRUV() for Ir at MS=%d successes (%s.txt). meanIr=%f\r\n", optIR, fnameIR,muIR);


   // 4. measure UV second
        if(daqModule.daqIRUV(optUV, nSmeas, FS_SCAN, ampIR, 1, ampUV, 1, 141, fnameUV)==-1)
        {
                	cout<<"WARN: daqIRUV() for UV ("<<fnameUV<<".txt) fails. Continue main loop."<<endl;
                	goto Label_ReturnFailure;
        }
		sprintf(tempStr,"%s.txt", fnameUV);
		procDaqData(tempStr,&muIR, &muUV);
        printf("daqIRUV() for UV at MS=%d successes (%s.txt). meanUV=%f\r\n", optUV, fnameUV, muUV);

  // 5. return
//Label_ReturnSuccess:
        cout<<"SUCCESS: Both scan and daq are OK. turn off lights. \r\n";
        statemain=pre_statemain;
        return 0;


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


