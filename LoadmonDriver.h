/*
 * LoadmonDriver.h
 *
 *  Created on: 15 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */

#ifndef LOADMONDRIVER_H_
#define LOADMONDRIVER_H_

#include "main.h"
#include "uartBeagle.h"

/*
#ifndef FS_SCAN
 #define FS_SCAN 500
#endif
*/

#define DUMPPATH "./dump/"

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

class LoadmonDriver {

public:
	uartBeagle *pmBed;//="mBed";//((char *)"mBed"); // ("mBed");
	uartBeagle *pPC; // ("PC");

	uartBeagle omBed;//="mBed";//((char *)"mBed"); // ("mBed");
	uartBeagle oPC; // ("PC");

	//LoadmonDriver();
	LoadmonDriver(): omBed("mBed"), oPC("PC"){	}
	virtual ~LoadmonDriver();

	int initDriver(char *fconfig);
	int initDriver(char *fconfig,char *uartName_mBed, char *uartName_PC);
	int initDriver(char *fconfig,int uartID_mBed, int uartID_PC);

	int selftest();

	void finishDriver();
	int moveMotor2Switch();
	int moveMotor2Dest(int dest);

	int scanIRUV(int posA, int posB, int nSam, int sFs, int ampIR, int gainIR, int ampUV, int gainUV, float apdBias, char *fnamebase);


	int daqIRUV(int posA, int nSam, int sFs, int ampIR, int gainIR, int ampUV, int gainUV, float apdBias, char *fnamebase_daq);



	void sourceON(int ampIR, int gainIR, int ampUV, int gainUV, float apdBias);
	void sourceOFF();

	float readTemperature();
	int setAPDBV(float apdbv);
	int setAPDBV(float apdbv, int *aomv);
	int readTsetV();
	int readTsetV(float *tempDeg, float *apdbv, int *aomv);

private:
	void initBeagle();
	int init_uarts(int uartID_mBed, int uartID_PC);
	int init_uarts(char *uartName_mBed, char *uartName_PC);
	void finish_uarts();
	int daqIRUVcore(int posA, int nSamples, int Fs, char * fleadnamemeas);
	int scanIRUVcore(int a, int nDataSteps_a2b, int nSperS, int sFs, char *fleadnamescan);

};

#endif /* LOADMONDRIVER_H_ */
