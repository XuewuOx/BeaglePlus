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

#define NAN 0x8000000
#include <math.h>

// #include "getRand.h"
// #include "gfit.h"
#include "gfit_rdfile.h"
#include "meanfile.h"

int procScanDataCore(char* fname, int* pkIR, int* pkUV, float *aIR,float *aUV,float *sigIR, float *sigUV )
{
	return EXIT_SUCCESS;
}
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
	    double stdIR, stdUV;
        return procDaqData2(fname, muIR, muUV, &stdIR, &stdUV);
}

int procDaqData2(char* fname, double* muIR, double* muUV, double* stdIR, double* stdUV)
{
	 double IRgf[3], UVgf[3]; // to store results fo Gaussian fitting
	 double IRms[2], UVms[2]; // to store results of mean() and std()
	 /* Return arguments of MATLAB generated codes meanfile()
	 % Return values:
	 %		IRgf=[muIR, sigIR, aIR] for mean, width and amplitude of Gaussian fitting over hist(IR)
	 %      UVgf=[muUV, sigUV, aUV] for mean, width and amplitude of Gaussian fitting over hist(IR)
	 %         IRavg=[meanIR, stdIR] for mean and std of IR by mean() and std()
	 %         UVavg=[meanUV, stdUV] for mean and std of UV by mean() and std()
	  */
	 printf("   Noise filtring at \"%s\" ...\r\n", fname);
     int fsize[2];
     IRgf[0]=-1.0; IRgf[1]= -2.0; IRgf[2]=-3; UVgf[0]=-3.0; UVgf[1]=-4.0; UVgf[2]=-5;
     fsize[0]=1; fsize[1]=strlen(fname);

     meanfile(fname, fsize, IRgf, UVgf, IRms, UVms);

     if (IRgf[0]<=0 ||IRgf[1]<=0 || isnan(IRgf[0]) || isnan(IRgf[1]))
     { // negative mean of Gaussian fitting, failed
    	 if (IRgf[0]==-1)
    		 { printf("    ERROR[%d]: failed to open data file\r\n", (int)IRgf[0]);
    		   return EXIT_FAILURE;
    		 }
    	 else if (IRgf[0]==-2|| IRgf[0]==-3)
    		 { printf("    ERROR[%d]: incorrect format of data file\r\n", (int)IRgf[0]);
    		   return EXIT_FAILURE;
    		 }
    	 else
    		 { printf("    ERROR[%d]: IR Gaussian fitting over failed. IRgfmu=%f, IRgfsig=%f\r\n", (int)IRgf[0], IRgf[0], IRgf[1]);
    		   if (IRms[0]<0 || IRms[1]<0 || isnan(IRms[0]) ||isnan(IRms[1]))
    			   return EXIT_FAILURE;
    		   else
    		      {// using mean() and std() method
    			   printf("     Using mean() and std() instead ");
    			   *muIR=IRms[0]; *stdIR=IRms[1];
    		      }
    		 }
     }
     else
     {
    	 printf("    IR Gaussian fitting OK.");
    	 *muIR=IRgf[0]; *stdIR=IRgf[1];
     }

     printf("     muIR=%4.2f, stdIR=%4.2f\r\n", *muIR, *stdIR);


     if (UVgf[0]<=0 ||UVgf[1]<=0 || isnan(UVgf[0]) || isnan(UVgf[1]))
     { // negative mean of Gaussian fitting, failed
    	 if (UVgf[0]==-1)
    		 { printf("    ERROR[%d]: failed to open data file.\r\n", (int)UVgf[0]);
    		   return EXIT_FAILURE;
    		 }
    	 else if (UVgf[0]==-2||UVgf[0]==-3)
    		 { printf("    ERROR[%d]: incorrect format of data file.r\n", (int)UVgf[0]);
    		   return EXIT_FAILURE;
    		 }
    	 else
    		 { printf("    ERROR[%d]: UV Gaussian fitting failed. UVgfmu=%f, UVgfsig=%f\r\n", (int)UVgf[0], UVgf[0],UVgf[1]);
    		   if (UVms[0]<0 || UVms[1]<0 || isnan(UVms[0]) ||isnan(UVms[1]))
				   return EXIT_FAILURE;
			   else
				  {// using mean() and std() method
				   printf("     Using mean() and std() instead");
				   *muUV=UVms[0]; *stdUV=UVms[1];
				  }

    		 }
     }
     else
     {
    	 printf("    UV Gaussian fitting OK.");
    	 *muUV=UVgf[0]; *stdUV=UVgf[1];
     }

     printf("      muUV=%4.2f, stdUV=%4.2f\r\n", *muUV, *stdUV);

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
	char *fname="./data/testfiles/Large_refscan.txt";
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


 	// char *fnameIR="./data/testfiles/refir.txt";
     // char *fnameIR="./data/testfiles/refir_test4.txt";
     char *fnameIR="./data/testfiles/wtrir_foam2000.txt";

	 double muIR=-1, stdIR=-1;
	 double muUV=-1, stdUV=-1;

/* call meanfile() directly
	 int fsize[2];
     double IRgf[3], UVgf[3]; // to store results fo Gaussian fitting
	 double IRms[2], UVms[2]; // to store results of mean() and std()
	 IRgf[0]=-1.0; IRgf[1]= -2.0; IRgf[2]=-3; UVgf[0]=-3.0; UVgf[1]=-4.0; UVgf[2]=-5;
	 fsize[0]=1; fsize[1]=strlen(fname);
     meanfile(fname, fsize, IRgf, UVgf, IRms, UVms);
     printf("   gfitting: IR=[%5.2f, %5.2f], UV=[%5.2f %5.2f].\r\n", IRgf[0], IRgf[1], UVgf[0], UVgf[1]);
     printf("   mean/std: IR=[%5.2f, %5.2f], UV=[%5.2f %5.2f].\r\n", IRms[0], IRms[1], UVms[0], UVms[1]);
*/

	 // call procDaqData2() to test meanfile()
     if (procDaqData2(fnameIR, &muIR, &muUV, &stdIR, &stdUV)==EXIT_FAILURE)
    	 {printf("   ERROR: Noise filtering at \"%s\" failed.\r\n", fnameIR); }
     else
     	 {printf("   Noise filtering at \"%s\" OK.\r\n",fnameIR); }
     printf("   IR=[%5.2f, %5.2f], UV=[%5.2f %5.2f].\r\n", muIR, stdIR, muUV, stdUV);

}
