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

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0LoadmonDriver::
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

	int initDriver(char *uartName_mBed, char *uartName_PC);
	int initDriver(int uartID_mBed, int uartID_PC);

	void finishDriver();
	int moveMotor2Switch();
	int moveMotor2Dest(int dest);

	int scanIRUV(int posA, int posB, int nSam, int sFs, int ampIR, int gainIR, int ampUV, int gainUV, float apdBias, char *fnamebase);
	int scanIRUVcore(int a, int nDataSteps_a2b, int nSperS, int sFs, char *fleadnamescan);

	int daqIRUV(int posA, int nSam, int sFs, int ampIR, int gainIR, int ampUV, int gainUV, float apdBias, char *fnamebase_daq);
	int daqIRUVcore(int posA, int nSamples, int Fs, char * fleadnamemeas);


	void sourceON(int ampIR, int gainIR, int ampUV, int gainUV, float apdBias);
	void sourceOFF();

private:
	void initBeagle();
	int init_uarts(int uartID_mBed, int uartID_PC);
	int init_uarts(char *uartName_mBed, char *uartName_PC);
	void finish_uarts();


};

#endif /* LOADMONDRIVER_H_ */
