/*
 * WrtDataLogFile.h
 *
 *  Created on: 5 Mar 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */


#ifndef WRTDATALOGFILE_H_
#define WRTDATALOGFILE_H_



/* Members of struct_DataLog
 * 		IRnorm, UVnorm:  normalised means of IR and UV. negative number indicates invalid values
 * 		IRref, UVref: 		means of IR and UV reference measurements at pkIRref, pkUVref
 * 		pkIRref, pkUvref:	peak positions (in motor steps) of IR and UV at reference scanning
 * 		stdIRref, stdUVref: deviation of IR and UV reference measurements at pkIRref, pkUVref
 * 		sigmaIRref, sigmaUVref: width of GAussain fitting for IR, UV reference scanning
 *
 * 		IRwtr, UVwtr: 		   means of IR and UV reference measurements at pkIRwtr, pkUVwtr
 * 		pkIRwtr, pkUVwtr: 		peak positions (in motor steps) of IR and UV at water (height) scanning
 * 		stdIRwtr, stdUVwtr: deviation of IR and UV water measurements at pkIRwtr, pkUVwtr
 * 		sigmaIRwtr, sigmaUVwtr: width of GAussain fitting for IR, UV water scanning
 *
 * 		SS, COD:		estimates of SS and COD
 * 		SSmA, CODmA;    4-20mA outputs of SS and COD values
 *
 */
struct struct_DataLog
{
	unsigned int yy, mm, dd;
   unsigned int hour, min, sec;
   float IRnorm, UVnorm; // standardised  IR/UV measurements
   float IRref, UVref;  // averaged IR/UV values at the peaks of reference scanning
   int pkIRref, pkUVref; // peak of IR/UV's reference scanning

   //standard deviation of IRref and UVref. And the width of the scanning shape
   float stdIRref, stdUVref, sigmaIRref, sigmaUVref;

   float IRwtr, UVwtr; // averaged IR/UV values at the peaks of water scanning
   int pkIRwtr, pkUVwtr; // peak position in motor steps of IR/UV's water scanning

   //standard deviation of IRwtr and UVwtr
   float stdIRwtr, stdUVwtr, sigmaIRwtr, sigmaUVwtr;

   float SS, COD; // estimated SS and COD
   float SSmA, CODmA; // 4-20mA output for SS and COD
   float tempDeg, apdbv; //temperature and corresponding APD's bias voltage
   int aomv; // corresponding mBed analogue output controlling APD's bias voltage

   char nameLogFile[100]; // name of data log file
   FILE *handleLogFile; // file handle of data log file
};


/* Open data log file specified by flogname
 *
 * If file does not exist, create one and set the title
 * If file open or create successes, store the file name and handle to
 * pDataLog->nameLogFile and pDAtaLog->handleLogFile
 *
 */
 int openDataLogFile(struct struct_DataLog *pDataLog, char *flogname);
 void closeDataLogFile(FILE *fhandle);
 int appendDataLog(struct struct_DataLog *pDataLog);
/*
 * Initialise dataLog structure with specified time
 *  and clean other data fields except file name and handle
 */
void initDataLog(struct struct_DataLog *pDataLog, int hh, int min, int sec);




#endif /* WRTDATALOGFILE_H_ */
