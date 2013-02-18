/*
 * sigProcess.h
 *
 *  Created on: 18 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */


#ifndef SIGPROCESS_H_
#define SIGPROCESS_H_

void testMatabCode();
int procScanData(char* fname, int* optIR, int* optUV);
int procDaqData(char* fname, double* muIR, double* muUV);

#endif /* SIGPROCESS_H_ */
