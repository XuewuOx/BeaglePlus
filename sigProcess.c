/*
 * sigProcess.c
 *
 *  Created on: 18 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */

#include "sigProcess.h"

// #include "getRand.h"
// #include "gfit.h"
#include "test.h"
#include "gfit_rdfile.h"
#include "meanfile.h"

int procScanData(char* fname, int* optIR, int* optUV)
{
	// Gaussian fitting to find the peak location
	  char temStr[200];
	  int fsize[2];
	  fsize[0]=1; fsize[1]=strlen(fname);
		double gfIR[3], gfUV[3];

	  gfit_rdfile(fname, fsize, gfIR, gfUV); // commented for debug

		 if (round(gfIR[2])==-1)
		 {
			 printf("  opening file %s failed.returned value =%2f \n",fname,gfIR[2]);
		     sprintf(temStr,"\r\nreading file '%s' failed. Returned value =%d\r\n",
									fname,gfIR[2]);
			 // PC.uartwriteStr(temStr);
			 return -1;
		 }
		 if (gfIR[2]<=-2)
		 {
			printf("  Gaussian fitting at %s failed. returned value =%2f\n",fname,gfIR[2]);
			sprintf(temStr,"\r\nGaussian fitting at '%s' failed. Returned value =%d\r\n",
												fname,gfIR[2]);
			 // PC.uartwriteStr(temStr);
			return -1;
		 }
	  int msPkIR, msPkUV;
      msPkIR=round(gfIR[0]); msPkUV=round(gfUV[0]);
      printf("Gaussian fitting at '%s' successes and the optIR=%d, optUV=%d\n",fname,msPkIR, msPkUV);

	// sprintf(temStr,"\r\nreading %s successes and the opt step =%d\r\n",fname,optstep);
	// PC.uartwriteStr(temStr);
	 *optIR=msPkIR; // gfIR[0];
	 *optUV=msPkUV; //gfUV[0];
	 return EXIT_SUCCESS;
}

int procDaqData(char* fname, double* muIR, double* muUV)
{
	   double EIR[2], EUV[2];
        int fsize[2];
        EIR[0]=-1.0; EIR[1]= -2.0; EUV[0]=-3.0; EUV[1]=-4.0;
        fsize[0]=1; fsize[1]=strlen(fname);
        meanfile(fname, fsize, EIR, EUV);
        printf(" Averaging at \"%s\" OK. IR[mu, sigma]=[%5.2f, %5.2f], UV=[%5.2f %5.2f]\r\n",
            		fname, EIR[0], EIR[1], EUV[0], EUV[1]);
        *muIR=EIR[0];
        *muUV=EUV[0];
        return EXIT_SUCCESS;
}

int procDaqData2(char* fname, double* muIR, double* muUV, double* stdIR, double* stdUV)
{
	 double EIR[2], EUV[2];
     int fsize[2];
     EIR[0]=-1.0; EIR[1]= -2.0; EUV[0]=-3.0; EUV[1]=-4.0;
     fsize[0]=1; fsize[1]=strlen(fname);
     meanfile(fname, fsize, EIR, EUV);
     printf(" Averaging at \"%s\" OK. IR[mu, sigma]=[%5.2f, %5.2f], UV=[%5.2f %5.2f]\r\n",
         		fname, EIR[0], EIR[1], EUV[0], EUV[1]);
     *muIR=EIR[0]; *stdIR=EIR[1];
     *muUV=EUV[0]; *stdUV=EUV[1];
     return EXIT_SUCCESS;
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
	// char *fname="wtrscan.txt";
	//  char *fname="refscan_20130205_16h42m02s.txt";
	char *fname="Large_refscan.txt";
	//  char *fname="refscan.txt";
	  int fsize[2];
	  fsize[0]=1; fsize[1]=strlen(fname);
	  double gfIR[3], gfUV[3];
      gfIR[0]=-3; gfIR[1]=-3; gfIR[2]=-3;
      gfUV[0]=-4; gfUV[1]=-4; gfUV[2]=-4;

      // gfIR[0]=gfit_rdfile(fname, fsize); // gfit_SingleGfit_OK, .\work_readfile\readfile_v4.m
     gfit_rdfile(fname, fsize, gfIR, gfUV);

	 // printf("gfIR=[%f, %f, %f]\r\n", gfIR[0],gfIR[1],gfIR[2]);
	 // printf("gfUV=[%f, %f, %f]\r\n", gfUV[0],gfUV[1],gfUV[2]);
	 if (gfIR[2]<0 && gfUV[2]<0)
	 {
		printf("  reading \"%s\" or the Gaussian fitting failed.returned value =%f\n",fname,gfIR[0]);
		return;
	 }
     int pkIR, pkUV;

     pkIR=round(gfIR[0]); pkUV=round(gfUV[0]);
     printf("   [optIR=%d, optUV=%d]. Gaussian fitting at \"%s\" OK. \n",pkIR, pkUV, fname);


 	char *fnameIR="refir.txt";
	  fsize[0]=1; fsize[1]=strlen(fnameIR);
	  double dIR[2], dUV[2];
    dIR[0]=-3; dIR[1]=-3;
    dUV[0]=-4; dUV[1]=-4;

    meanfile(fname, fsize, dIR, dUV);
    printf("   IR[mu, std]=[%5.2f, %5.2f], UV=[%5.2f %5.2f]. Averaging at \"%s\" OK. \r\n",
    		dIR[0], dIR[1], dUV[0], dUV[1], fnameIR);

}
