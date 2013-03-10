/*
 * WrtDataLogFile.c
 *
 *  Created on: 5 Mar 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WrtDataLogFile.h"

int openDataLogFile(struct struct_DataLog *pDataLog, char *flogname)
{
    FILE *fp;
    int fsize;

    if (strlen(flogname)>=sizeof(pDataLog->nameLogFile))
    {
    	printf ("ERROR: file name of data log file is too long (%s).\r\n", flogname);
    	return EXIT_FAILURE;
    }

    fp = fopen(flogname,"aw");
    if (fp == NULL)             // always check for success when opening a file
    {
        // perror("Open data log file fopen() failed\r\n"); //,flogname);
    	printf ("ERROR: Open data log file fopen(%s) failed\r\n", flogname);
    	return EXIT_FAILURE;
    }
    else
    {
    	fseek(fp, 0, SEEK_END); // seek to end of file
    	fsize = ftell(fp); // get current file pointer
    	if (fsize==0)
    	{ // new file, create the title line
    		printf("Empty file %s fsize=%d\r\n",flogname,fsize);

    		// fseek(fp, 0, SEEK_SET);
    		fprintf(fp, "%% Loadmon Data Log file\r\n");
    		fprintf(fp, "%% Created on     \r\n");
    		// fprintf(fp, "%%  \/\ /\ /   /  * _  '\r\n");
    		// fprintf(fp, "%% _/\ \/\/ __/__.'(_|_|\r\n");_
    		fprintf(fp, "%%hh min sec");
    		fprintf(fp, " IRnorm\t\tUVnorm");
    		fprintf(fp, "\t\tSS\t\t\tCOD");

    		fprintf(fp, "\t\t\tIRref\t\tUVref\t\tpkIRref\tpkUVref");
    		fprintf(fp, "\tIRwtr\t\tUVwtr\t\tpkIRwtr\tpkUVwtr");
    		fprintf(fp, "\tstdIRref\tstdUVref\tsigmaIRref\tsigmaUVref");
    		fprintf(fp, "\tstdIRwtr\tstdUVwtr\tsigmaIRwtr\tsigmaUVwtr");
    		fprintf(fp,"\tSSmA\t\tCODmA");
    		   fprintf(fp, "\t\ttempDeg\tAPDbv\tAOmv");
    		fprintf(fp,"\r\n");
    	}

    	strcpy(pDataLog->nameLogFile,flogname);
    	pDataLog->handleLogFile=fp;
    	return EXIT_SUCCESS;
    }
}


void closeDataLogFile(FILE *fhandle){
	   fclose(fhandle);

}

int appendDataLog(struct struct_DataLog *pDataLog)
{
	FILE *fp;
	struct struct_DataLog *pd;

	fp=pDataLog->handleLogFile;
	pd=pDataLog;
	// printf("appendDatalog: %02d  %02d  %02d",pdq->hour,pd->min,pd->sec);
	// printf("\t%08.3f\t%08.3f",pd->IRnorm, pd->UVnorm);
	fprintf(fp, " %02d\t%02d\t%02d",pd->hour,pd->min,pd->sec);
	fprintf(fp, "\t%08.5f\t%08.5f",pd->IRnorm, pd->UVnorm);
	fprintf(fp, "\t%08.3f\t%08.3f",pd->SS,pd->COD);
	fprintf(fp, "\t%08.3f\t%08.3f\t%04d\t%04d", pd->IRref, pd->UVref, pd->pkIRref, pd->pkUVref);
	fprintf(fp, "\t%08.3f\t%08.3f\t%04d\t%04d", pd->IRwtr, pd->UVwtr, pd->pkIRwtr, pd->pkUVwtr);
	fprintf(fp, "\t%08.3f\t%08.3f\t%08.3f\t%08.3f", pd->stdIRref, pd->stdUVref, pd->sigmaIRref, pd->sigmaUVref);
	fprintf(fp, "\t%08.3f\t%08.3f\t%08.3f\t%08.3f", pd->stdIRwtr, pd->stdUVwtr, pd->sigmaIRwtr, pd->sigmaUVwtr);
	fprintf(fp, "\t%08.3f\t%08.3f",pd->SSmA,pd->CODmA);
	fprintf(fp, "\t%07.3f\t%07.2f\t%04d", pd->tempDeg,pd->apdbv, pd->aomv);
	fprintf(fp,"\r\n");
	return EXIT_SUCCESS;
}

void initDataLog(struct struct_DataLog *pDataLog, int hh, int min, int sec)
{

		pDataLog->hour=hh;
		pDataLog->min=min;
		pDataLog->sec=sec;
		pDataLog->IRnorm=0;
		pDataLog->UVnorm=0;
		pDataLog->IRref=0;
		pDataLog->UVref=0;
		pDataLog->pkIRref=0;
		pDataLog->pkUVref=0;
		pDataLog->stdIRref=0; pDataLog->stdUVref=0;
		pDataLog->sigmaIRref=0;pDataLog->sigmaUVref=0;
		pDataLog->IRwtr=0; pDataLog->UVwtr=0; pDataLog->pkIRwtr=0; pDataLog->pkUVwtr=0;
		pDataLog->stdIRwtr=0;pDataLog->stdUVwtr=0;pDataLog->sigmaIRwtr=0;pDataLog->sigmaUVwtr=0;
		pDataLog->SS=0;pDataLog->COD=0;
		pDataLog->SSmA=0; pDataLog->CODmA=0;
		pDataLog->tempDeg=0; pDataLog->apdbv=0; pDataLog->aomv=0;
}
