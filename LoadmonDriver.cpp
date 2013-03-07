/*
 * LoadmonDriver.cpp
 *
 *  Created on: 15 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */
#include <iostream>
using namespace std;

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))


#include "RdConfigFile.h"
#include "WrtDataLogFile.h"

struct config configstruct;
struct struct_DataLog currentdatalog;


#include "LoadmonDriver.h"

// uartBeagle mBed2((char *)"mBed2");
// uartBeagle PC2((char *)"PC2");
#include "main.h"

struct MOTORSTATUS{
	int nOrigin;
	int nNow;
	double motorSpd; //steps/s,
	int fullStep;
	int statusLEDMotor;
	int uSW;
};

MOTORSTATUS motorLED;



/*
LoadmonDriver::LoadmonDriver(): omBed("mBed"), oPC("PC")
 {
	// TODO Auto-generated constructor stub

	// mBed("mBed");
	// PC("PC");
}
*/
LoadmonDriver::~LoadmonDriver()
{
	// TODO Auto-generated destructor stub
}


void LoadmonDriver::initBeagle()
{
	int nline=0;

	time_t rawt = time(NULL); // /* Seconds since the Epoch. 1970-01-01  */
	struct tm * timeinfo;

	time(&rawt);
	timeinfo=localtime(&rawt);

   printf("Enter  $ sudo /home/ubuntu/restore_DHCPconsole\r\n");
   printf("     to reboot the Beagle with DHCP and console over ttyO2\r\n");
   printf("Enter  $ sudo /home/ubuntu/restore_IP10.2.1.3ttyO2\r\n");
   printf("     to reboot the Beagle with IP=10.2.1.3 and without console over ttyO2\r\n");


	printf("\n=================================================\n");
	printf("    Loadmon Beagle starts\n");
	printf("=================================================\n");
	// printf("[yyy/mm/dd hh:mm:ss]\r\n");
	printf("[%d/%02d/%02d %02d:%02d:%02d]\r\n",timeinfo->tm_year+1900, timeinfo->tm_mon+1,
		    timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

int LoadmonDriver::initDriver(char *fileconfig)
{
	int uartID1, uartID2;

		initBeagle();

		if (get_config(fileconfig, &configstruct)==EXIT_FAILURE)
		{
			printf("Reading config file %s failed",fileconfig);
			return EXIT_FAILURE;
		}
		uartID1=omBed.uartopen(configstruct.UARTFile_mBed);
			if (uartID1!=-1)
				cout<<"  UART port"<<omBed.portName<<" (uartID="<<omBed.uartID<<") OK"<<endl;

			uartID2=oPC.uartopen(configstruct.UARTFile_PC);
			if (uartID2!=-1)
				cout<<"  UART port"<<oPC.portName<<" (uartID="<<oPC.uartID<<") OK"<<endl;

			if (uartID1==-1 || uartID2==-1)
				return -1;
			else
			    return EXIT_SUCCESS;

}


int LoadmonDriver::initDriver(char *fileconfig, char *uartName_mBed, char *uartName_PC)
{
	int uartID1, uartID2;

	initBeagle();

	if (get_config(fileconfig, &configstruct)==EXIT_FAILURE)
	{
		printf("Reading config file %s failed",fileconfig);
		return EXIT_FAILURE;
	}

	strcpy(configstruct.UARTFile_mBed, uartName_mBed);
	strcpy(configstruct.UARTFile_PC, uartName_PC);

	uartID1=omBed.uartopen(uartName_mBed);
	if (uartID1!=-1)
		cout<<"  UART port"<<omBed.portName<<" (uartID="<<omBed.uartID<<") OK"<<endl;

	uartID2=oPC.uartopen(uartName_PC);
	if (uartID2!=-1)
		cout<<"  UART port"<<oPC.portName<<" (uartID="<<oPC.uartID<<") OK"<<endl;

	if (uartID1==-1 || uartID2==-1)
		return -1;
	else
	    return EXIT_SUCCESS;

}

int LoadmonDriver::initDriver(char *fconfig, int uartID_mBed, int uartID_PC)
{
	initBeagle();
	if (get_config(fconfig, &configstruct)==EXIT_FAILURE)
	{
		printf("Reading config file %s failed",fconfig);
		return EXIT_FAILURE;
	}

	omBed.uartID=uartID_mBed;
	oPC.uartID=uartID_PC;
	return EXIT_SUCCESS;

}

int LoadmonDriver::selftest()
{
	// PC.uartwriteStr("resetmbed\r\n");
		usleep(1000);

		oPC.uartwriteStr("  Testing the motor ...");
		printf("  Testing the motor ...");
		// PC.uartwriteStr("% setm 1 0 30 100 1\r\n");
		// printf("setm 1 0 30 100 1\r\n");
		// mBed.uartwriteStr("setm 1 0 30 100 1\r\n");
		// usleep(100);

		// while (mBed.readline()==0){}
		omBed.flushrxbuf();
		DEBUGF((char *)"move -s 1 100\r\n");
		omBed.uartwriteStr((char *)"move -s 1 100\r\n");
		usleep(1000);
		// while (mBed.readline()==0){}
		if (omBed.readlineTimeOut(100000)==-1)
			{
			cout<<"Failed to communicate with mBed to move motor\r\n";
			return -1;
			}

		oPC.uartwriteStr((char *)" ... ");
		DEBUGF("move -s 1 -100\r\n");
		omBed.uartwriteStr((char *)"move -s 1 -100\r\n");
		usleep(1000);
		while (omBed.readline()==0){}
		// sleep one second and readline to clear the rx buffer
		// This is to avoid confusing strings from mbed uart.
		sleep(1);
		omBed.readline();
		oPC.uartwriteStr(" OK\r\n");
		printf(" OK\r\n");
		return EXIT_SUCCESS;
}

void LoadmonDriver::finishDriver()
{
		omBed.uartclose();
		oPC.uartclose();
}


float LoadmonDriver::readTemperature()
{
	int nChars;
	float temp;
    char *pStr;


	omBed.readline();
    omBed.flushrxbuf();
    omBed.uartwriteStr("rdtemp\r\n");
	if (omBed.readlineTimeOut(30000)<=0)
	{  // no response from mbed is received
		return -999.9;
	}
	// printf("received %s", omBed.rxbufptr);
	if (strlen(omBed.rxbufptr)>100)
	{
		return -999.9;
	}
	pStr = strstr((char *)omBed.rxbufptr,"temperature=");
	sscanf(pStr, "temperature=%f\r\n", &temp);
    // cout<<"get tempearture="<<temp<<endl;
	return temp;
}

int LoadmonDriver::setAPDBV(float apdbv)
{
	char cmdStr[20];

	sprintf(cmdStr,"apdbv %7.2fv\r\n", apdbv);
	omBed.flushrxbuf();
	omBed.uartwriteStr(cmdStr);
	if (omBed.readlineTimeOut(30000)<=0)
		{  // no response from mbed is received
			return -1;
		}
	else
		return EXIT_SUCCESS;
}

int LoadmonDriver::setAPDBV(float apdbv, int *aomv)
{
	char cmdStr[20];
	float aomv2;

	sprintf(cmdStr,"apdbv %7.2fv\r\n", apdbv);
	omBed.flushrxbuf();
	omBed.uartwriteStr(cmdStr);
	if (omBed.readlineTimeOut(30000)<=0)
		{  // no response from mbed is received
			*aomv=-1;
			return -1;
		}
	else
	{
		char *pStr;
		pStr = strstr((char *)omBed.rxbufptr,"analog output=");
		if (sscanf(pStr,"analog output=%fmv", &aomv2)!=1)
			*aomv=-1;
		else
			*aomv=round(aomv2);
		return EXIT_SUCCESS;
	}
}

int LoadmonDriver::readTsetV()
{
	float temperature;
	float IntervalTgrid;
	int i;

	temperature=readTemperature();
	if (temperature<configstruct.T2VTable[0][0] || temperature>configstruct.T2VTable[0][LENT2VTABLE])
		return -1;

	IntervalTgrid=abs(configstruct.T2VTable[0][1]-configstruct.T2VTable[0][0])/2;
	for (i=0; i<LENT2VTABLE;i++)
	{
		if (abs(temperature-configstruct.T2VTable[0][i])<=IntervalTgrid)
		{
			printf("T=%7.2fdegC, V=%7.2fv\r\n", temperature, configstruct.T2VTable[1][i]);
			setAPDBV(configstruct.T2VTable[1][i]);
			return EXIT_SUCCESS;
		}
	}
	return -1;
}

int  LoadmonDriver::readTsetV(float *tempDeg, float *apdbv, int *aomv)
{
	float IntervalTgrid;
	// float temp2, bv2;
	int i;

	*tempDeg=readTemperature();
	// printf("temp2=%f\r\n",temp2);
	if (*tempDeg<configstruct.T2VTable[0][0] || *tempDeg>configstruct.T2VTable[0][LENT2VTABLE])
		return -1;

	IntervalTgrid=abs(configstruct.T2VTable[0][1]-configstruct.T2VTable[0][0])/2;
	for (i=0; i<LENT2VTABLE;i++)
	{
		if (abs(*tempDeg-configstruct.T2VTable[0][i])<=IntervalTgrid)
		{
			// printf("T=%7.2fdegC, V=%7.2fv\r\n", temp2, configstruct.T2VTable[1][i]);
			*apdbv=configstruct.T2VTable[1][i];
			setAPDBV(*apdbv, aomv);
			// *apdbv=bv2;
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;

}

int LoadmonDriver::moveMotor2Switch()
{
	int nChars;
	time_t t1, t2;
	double wait_secs;
	double Upbound_secs;
    bool timeover;

    Upbound_secs=30;
    timeover=0;
	time(&t1);
	cout<<"% move motor to switch position"<<endl;
	motorLED.uSW=0;
	omBed.flushrxbuf();
	while(motorLED.uSW==0)
	{	omBed.uartwriteStr("move -s 1 -2000\r\n");
		oPC.uartwriteStr("\% move -s 1 -2000\r\n");
		do{
			nChars=omBed.readline();
			time(&t2);
			wait_secs=difftime(t2,t1);
			if (wait_secs>Upbound_secs)
			{
				timeover=1;
				break;
			}
		}while(nChars==0);

		if (timeover==0)
		{
			cout<<omBed.rxbuf;
			nChars=sscanf(omBed.rxbuf,
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

void LoadmonDriver::sourceOFF()
{
	omBed.uartwriteStr("irt\r\n");
	printf("irt\r\n"); // PC.uartwriteStr("\% irt\r\n");
	omBed.uartwriteStr("uvt\r\n");
	printf("uvt\r\n"); // PC.uartwriteStr("\% uvt\r\n");
	printf("IR and UV are off \r\n");

}

void LoadmonDriver::sourceON(int ampIR, int gainIR, int ampUV, int gainUV, float apdBias)
{
	 char tempStr[500];

	if (ampUV<=0)
	 			sprintf(tempStr,"uvt\r\n");
	else if (ampUV>=100)
		sprintf(tempStr,"uvs00\r\n");
	else
	 	sprintf(tempStr,"uvs%02d\r\n",ampUV);

	// printf(" !NOTE!: UV is forced off for debugging!\r\n");
	// sprintf(tempStr,"uvt\r\n"); // NOTE: let UV off for debugging

	omBed.uartwriteStr(tempStr);
	printf(tempStr); // PC.uartwriteStr(tempStr);
	usleep(10000);
	omBed.readlineTimeOut(1000);
	//omBed.readline();

	if (ampIR<=0)
	 	sprintf(tempStr,"irt\r\n");
	else if (ampIR>=100)
		sprintf(tempStr,"irs00\r\n");
	else
	 	sprintf(tempStr,"irs%02d\r\n",ampIR);

	omBed.uartwriteStr(tempStr);
	printf(tempStr); // PC.uartwriteStr(tempStr);
	usleep(10000);
	omBed.readline();

	sprintf(tempStr,"irg%2d\r\n",gainIR);
	omBed.uartwriteStr(tempStr);
	printf(tempStr); // PC.uartwriteStr("\% irg1 \r\n");
	usleep(10000);
	omBed.readline();


	sprintf(tempStr,"uvg%2d\r\n",gainUV);
	omBed.uartwriteStr(tempStr);
	printf(tempStr); // PC.uartwriteStr("\% irg1 \r\n");
	usleep(10000);
	omBed.readline();


	sprintf(tempStr,"apdbv %3.2fv\r\n",apdBias);

	omBed.uartwriteStr(tempStr);
	printf(tempStr); //PC.uartwriteStr("\% apdbv 141v\r\n");
	usleep(10000);
	omBed.readline();
	sleep(configstruct.slowstartDelay); // wait for UV/IR reaches expected strength
	return;
}

int LoadmonDriver::scanIRUV(int posA, int posB, int nSam, int sFs, int ampIR, int gainIR, int ampUV, int gainUV, float apdBias, char *fnamebase)
{
	 char tempStr[500];
	 char fnamescan[20];

	// printf("start UV/IR measurements\n");
	// oPC.uartwriteStr("\% start UV/IR measurements\n");
	// Skip moving motor to origin

	int scanSteps=posB-posA+1;
	int scanOK;

	omBed.flushrxbuf();
	sourceON(ampIR, gainIR, ampUV, gainUV, apdBias);
	scanOK=scanIRUVcore(posA, scanSteps,nSam,sFs, fnamebase);
	sourceOFF();

	return scanOK;
}


// int scanIRUV(int a, int b, int nSperS)
// return value:
//     -1  invalid data acquisition. dataIR and dataUV must be ignored
//      0   valid data acquisition. nDataSteps_a2b rows, nSperS column data has been collected
int LoadmonDriver::scanIRUVcore(int a, int nDataSteps_a2b, int nSperS, int sFs, char *fleadnamescan)
{
	int i,j, nChar;
		char fname[100];
		char fname2[100];

		char temStr[100];

		sprintf(fname,"%s.txt",fleadnamescan);

		string strRx;
		time_t tnow;

		// set mBed's sampling rate to 500
		sprintf(temStr,"a2d s %d %d\r\n", sFs, 0);
	    omBed.uartwriteStr(temStr);
		uint ttimeout; // ms, threshold of waiting time for readPktTimeout()
		ttimeout=nSperS*abs(nDataSteps_a2b)*(2.0*1000*1/sFs+1)+10*3000+40000; // assume Fs Hz sampling rate
												// 50 ms for sending each data via UART
		                                        // motor spd: 100 steps per second
		                                        // maximum 3000 steps
		// ttimeout=120000;
		printf("a2d s %d %d  to set mBed's sampling rate to %d\r\n", sFs, 0, sFs);
		usleep(10000);
		omBed.readlineTimeOut(2000); //ms


		sprintf(temStr,"swn %d %d %d\r\n",a,a+nDataSteps_a2b-1,nSperS);
		omBed.flushrxbuf();
		omBed.uartwriteStr(temStr);
		cout<<"send 'swn "<< a <<" "<< a+nDataSteps_a2b-1 <<" "<< nSperS<< "' down to mBed"<<endl;
		cout<<"==================================="<<endl;
		size_t found0, found2;


		// mBed.readPkt("M posA=","DATAIRUVEND",strRx);
		nChar=omBed.readPktTimeout("%SWN posA=","DATAIRUVEND",strRx,ttimeout);
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



		fdata=fopen(fname,"w"); //Create an empty file
		if (fdata==NULL)
		  { cout<<"fopen() to creat a data file "<<fname<<" failed."<<endl;
			return -1; // return an ERROR
		  }
		fwrite(strRx.c_str(),1,strRx.length(), fdata);
		fclose (fdata);
		printf("save data to %s OK\n", fname);

	    // backup the data file

		sprintf(fname2,"%s%s_%04d%02d%02d_%02dh%02dm%02ds.txt",DUMPPATH,fleadnamescan, timeinfo->tm_year+1900, timeinfo->tm_mon+1,
					timeinfo->tm_mday,currentdatalog.hour, currentdatalog.min, currentdatalog.sec);
		sprintf(temStr, "cp %s %s", fname, fname2);
		system(temStr);
		return EXIT_SUCCESS;
}


int LoadmonDriver::daqIRUV(int posA, int nSam, int sFs, int ampIR, int gainIR, int ampUV, int gainUV, float apdBias, char *fnamebase_daq)
{
		char tempStr[100];
//		char fnamedaq[100];
//		 sprintf(fnamedaq, "%s.txt", fnamebase_daq);

		sprintf(tempStr,"move -d 1 %d\r\n",posA);
		omBed.uartwriteStr(tempStr);
		// PC.uartwriteStr(tempStr);
		printf(tempStr);
		usleep(10000);
		omBed.readlineTimeOut(30000); // wait until motor arrives at optimal position
		usleep(500000);

		omBed.flushrxbuf();
		sourceON(ampIR, gainIR, ampUV, gainUV, apdBias);
		int daqOK;
    	daqOK=daqIRUVcore(posA, nSam, sFs,fnamebase_daq);
    	sourceOFF();
    	return daqOK;

}

int LoadmonDriver::daqIRUVcore(int posA, int nSamples, int Fs, char * fleadnamemeas)
{
	int nChn=0;
	char fname[100];
	char tempStr[30];
    int twait=0;
    int mBedReply=0;

	FILE *fdata;
	sprintf(fname, "%s.txt", fleadnamemeas);
	// set mBed's sampling rate to 500
		sprintf(tempStr,"a2d s %d %d\r\n", Fs, 0);
		omBed.uartwriteStr(tempStr);

		uint ttimeout; // ms, threshold of waiting time for readPktTimeout()
		ttimeout=nSamples*(1000*1/Fs+2)+10*2000+20000; // assume Fs Hz sampling rate
		                                        // motor spd: 100 steps per second
		                                        // maximum 2000 steps
		printf("a2d s %d %d  to set mBed's sampling rate to %d\r\n", Fs, 0, Fs);
		usleep(10000);
		mBedReply=omBed.readlineTimeOut(2000);

	sprintf(tempStr,"swn %d %d %d\r\n", posA, posA, nSamples);
    omBed.uartwriteStr(tempStr);
    cout<<"send 'swn "<< posA <<" "<< posA <<" "<< nSamples<< "' down to mBed"<<endl;
	string strRx;
	string strData;
    // TODO:
    // mBed.readPkt("MOTORxxx","xxxx", strData);
    // mBed.readPkt("M posA=","DATAIRUVEND",strRx);
    int nChar=omBed.readPktTimeout("%SWN posA=","DATAIRUVEND",strRx,ttimeout);
    if (nChar<=0)
    	{ // nothing valid received from mBed.
    		cout<<"Nothing valid received from mBed. "<<endl;
    		return -1; // return an ERROR
    	}
    	// valid packet received
    	// save it to a txt file

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
    	sprintf(fname2,"%s%s_%04d%02d%02d_%02dh%02dm%02ds.txt",DUMPPATH, fleadnamemeas, timeinfo->tm_year+1900, timeinfo->tm_mon+1,
    						timeinfo->tm_mday,currentdatalog.hour, currentdatalog.min, currentdatalog.sec);
    	sprintf(cmdline,"cp %s %s",fname, fname2);
    	system(cmdline);
    	printf("backup data in %s to %s OK\n", fname, fname2);

    return 0;
}



// move Motor to destination
// Input: destination in steps
// return: motor's position after complete the move command
int LoadmonDriver::moveMotor2Dest(int dest)
{
	int nLines, nPars;
	char cmdMove[20];
	sprintf(cmdMove, "move -d 1 %d\r\n",dest);
	// motorLED.uSW=0;
	omBed.uartwriteStr(cmdMove);
	do{
			nLines=omBed.readline();
	}while(nLines==0);
	// cout << "received " <<nChars <<"characters"<<endl;
	nPars=sscanf(omBed.rxbuf,
			"motor[1] is motorLED: nOrigin=%d, nNow=%d, motorSpd=%f steps/s, fullStep=%d, statusLEDMotor=%d, uSW(p29)=%d",
			&motorLED.nOrigin, &motorLED.nNow,&motorLED.motorSpd, &motorLED.fullStep, &motorLED.statusLEDMotor, &motorLED.uSW);
	// cout<<"received "<<nChars<<"parameters of motroLED"<<endl;
	printf("moveMotor2Dest done!");
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

