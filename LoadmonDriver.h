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



#define uSW_OpticRef 1
#define uSW_WaterMes 0

#define uSW_POSITION  uSW_OpticRef
// uSW_POSITION   1 when the uSW is at the optic reference end
//                 0 when the uSW at the water measurement end


#define UVIRALWAYSON
// #define UVALWAYSOFF


#ifndef TRUE
	# define TRUE 1
#endif

#ifndef FALSE
	# define FALSE 0
#endif

struct STRUCT_Temp2APDbv
{
	float tempFiltered; // temperature after smoothing
	unsigned int ind_now; // current index of configstruct.T2VTable[k][ind_now], where k=0,1 for temperature and APDbv respectively
	unsigned int ind_last; // last index of configstruct.T2VTable[k][ind_last],#
};

class LoadmonDriver {

private:
	struct STRUCT_Temp2APDbv t2apdbv;


public:
	uartBeagle *pmBed;//="mBed";//((char *)"mBed"); // ("mBed");
	uartBeagle *pPC; // ("PC");

	uartBeagle omBed;//="mBed";//((char *)"mBed"); // ("mBed");
	uartBeagle oPC; // ("PC");

	bool lightIsOFF; // =1 true by default after initialisation

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
	void init_t2apdbv();
	int init_uarts(int uartID_mBed, int uartID_PC);
	int init_uarts(char *uartName_mBed, char *uartName_PC);
	void finish_uarts();
	int daqIRUVcore(int posA, int nSamples, int Fs, char * fleadnamemeas);
	int scanIRUVcore(int a, int nDataSteps_a2b, int nSperS, int sFs, char *fleadnamescan);

};

#endif /* LOADMONDRIVER_H_ */
