/*
 * RdConfigFile.h
 *
 *  Created on: 1 Mar 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */


#ifndef RDCONFIGFILE_H_
#define RDCONFIGFILE_H_

#define CONFIGFILENAME "config.conf"
#define DELIM "="
#define MAXBUF 1024
#define LENT2VTABLE 41

struct scanArg
{
	int posA;
	int posB;
	int SpS;
	int sFs;
	int ampIR;
	int gainIR;
	int ampUV;
	int gainUV;
	float apdBV;

	int PkRange[2]; // if the peak location found by Gaussian fitting is over this range, it is an invalid location.
};

struct daqArg
{

	int nSam;
	int sFs;
	int ampIR;
	int gainIR;
	int ampUV;
	int gainUV;
	float apdBV;
};

struct config
{
   char UARTFile_mBed[100];
   char UARTFile_PC[100];
   struct scanArg refscan;
   struct scanArg wtrscan;
   struct daqArg refdaq;
   struct daqArg wtrdaq;
   int slowstartDelay;
   char temperature[MAXBUF];
   char APDBV[MAXBUF];
   char D2Avalue[MAXBUF];
   float T2VTable[2][LENT2VTABLE];
};

int get_config(char *filename, struct config *pconfig);
void trim(char *s);  /* Remove tabs/spaces/lf/cr  both ends */
int readT2VTable(char *cfline, struct config *pconfig, int lineID);

#endif /* RDCONFIGFILE_H_ */
