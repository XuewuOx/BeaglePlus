/*
 ============================================================================
 Name        : RdConfigFile.c
 Author      : Xuewu Daniel Dai
 Version     :
 Copyright   : Your copyright notice
 File		 : rdConfigFile.c
 Description : Simple program to read file into a config struct
               gcc -Wall reader.c -o reader
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RdConfigFile.h"




int get_config(char *filename, struct config *pconfig)
{
//      struct config configstruct;
	FILE *file = fopen (filename, "r");

	if (file != NULL)
	{
		char oneline[MAXBUF];
		int i = 1;
		int nArg;
		while(fgets(oneline, sizeof(oneline), file) != NULL)
		{
			trim(oneline);
			if (strlen(oneline)== 0)  /* Ignore empty lines */
				continue;
			if (oneline[0]==';' ||oneline[0]=='%' )  /* ignore lines starting with a ; or % - they are comments */
				continue;
			// 6-th line [T2VTable], ignore
			if (oneline[0]=='[' && oneline[1]=='T' && oneline[2]=='2' && oneline[3]=='V' &&
				oneline[4]=='T' && oneline[5]=='a' && oneline[6]=='b' && oneline[7]=='l' &&
				oneline[8]=='e' && oneline[9]==']')
			   { i=9;	continue;}

			char *cfline;
			cfline = strstr((char *)oneline,DELIM);
			cfline = cfline + strlen(DELIM);
			// printf("reading Line %d ... OK, %s\r\n", i, oneline);
			printf("Line %d OK \"%s\"\r\n", i, oneline);

			if (i == 1){
				memcpy(pconfig->UARTFile_mBed, cfline,strlen(cfline));
				i++; continue;
			}

			if (i == 2){
				memcpy(pconfig->UARTFile_PC,cfline,strlen(cfline));
				i++; continue;
			}

			if (i == 3){
				nArg=sscanf(cfline, "%d %d %d %d %d %d %d %d %f",&pconfig->refscan.posA, &pconfig->refscan.posB,
						&pconfig->refscan.SpS, &pconfig->refscan.sFs,
						&pconfig->refscan.ampIR, &pconfig->refscan.gainIR,
						&pconfig->refscan.ampUV,&pconfig->refscan.ampUV,
						&pconfig->refscan.apdBV);
				if (nArg!=9) {
					printf("ERROR: No enough parameters or wrong format at line \"%s\"\r\n", oneline);
					return EXIT_FAILURE;
					}
				// printf("posA=%d, posB=%d\r\n", pconfig->refscan.posA, pconfig->refscan.posB);
				i++; continue;}

			if (i == 4){
				nArg=sscanf(cfline, "%d %d %d %d %d %d %d %d %f",&pconfig->wtrscan.posA, &pconfig->wtrscan.posB,
						&pconfig->wtrscan.SpS, &pconfig->wtrscan.sFs,
						&pconfig->wtrscan.ampIR, &pconfig->wtrscan.gainIR,
						&pconfig->wtrscan.ampUV,&pconfig->wtrscan.ampUV,
						&pconfig->wtrscan.apdBV);
				if (nArg!=9) {
					printf("ERROR: No enough parameters or wrong format at line \"%s\"\r\n", oneline);
					return EXIT_FAILURE;
					}
				i++; continue;}
			if (i == 5){
				nArg=sscanf(cfline, "%d %d %d %d %d %d %f",
						&pconfig->refdaq.nSam, &pconfig->refdaq.sFs,
						&pconfig->refdaq.ampIR, &pconfig->refdaq.gainIR,
						&pconfig->refdaq.ampUV,&pconfig->refdaq.ampUV,
						&pconfig->refdaq.apdBV);
				if (nArg!=7) {
					printf("ERROR: No enough parameters or wrong format at line \"%s\"\r\n", oneline);
					return EXIT_FAILURE;
					}
				i++;continue;}

			if (i == 6){
				nArg=sscanf(cfline, "%d %d %d %d %d %d %f",
						&pconfig->wtrdaq.nSam, &pconfig->wtrdaq.sFs,
						&pconfig->wtrdaq.ampIR, &pconfig->wtrdaq.gainIR,
						&pconfig->wtrdaq.ampUV,&pconfig->wtrdaq.ampUV,
						&pconfig->wtrdaq.apdBV);
				if (nArg!=7) {
					printf("ERROR: No enough parameters or wrong format at line \"%s\"\r\n", oneline);
					return EXIT_FAILURE;
					}

				i++;continue;
				}
			if (i == 7){
				nArg=sscanf(cfline, "%ds", &pconfig->slowstartDelay);
				if (nArg!=1) {
					printf("ERROR: No enough parameters or wrong format at line \"%s\"\r\n", oneline);
					return EXIT_FAILURE;
					}
				i++;continue;
				}
			if (i==8)
				{ // line [T2VTable], ignore
					i++;continue;}
			if (i==9)
			{
				if (readT2VTable(cfline, pconfig, 0)==EXIT_FAILURE)
					return EXIT_FAILURE;
				else
				 { i++;continue;}
			}

			if (i==10){
				if (readT2VTable(cfline, pconfig, 1)==EXIT_FAILURE)
					return EXIT_FAILURE;
				else
				 { i++;continue;}
			}

			i++;
		} // End while

		fclose(file);
	}
	else
	{
		printf("opening file failed\r\n");
		return EXIT_FAILURE;
	}// End if file

	return EXIT_SUCCESS;

}


/*Functions */
void trim(char *s) /* Remove tabs/spaces/lf/cr  both ends */
{
	/* Trim from start */
	size_t i=0,j;
	while((s[i]==' ' || s[i]=='\t' || s[i] =='\n' || s[i]=='\r')) {
		i++;
	}
	if (i>0) {
		for( j=0; j < strlen(s);j++) {
			s[j]=s[j+i];
		}
	s[j]='\0';
	}

	/* Trim from end */
	i=strlen(s)-1;
	while((s[i]==' ' || s[i]=='\t'|| s[i] =='\n' || s[i]=='\r')) {
		i--;
	}
	if ( i < (strlen(s)-1)) {
		s[i+1]='\0';
	}
}

int readT2VTable(char *cfline, struct config *pconfig, int lineID)
{
	int k,nChar;
	int j=0;

	nChar=strlen(cfline);

	k=0;
	while(k<nChar)
	{
	while (k<nChar && (cfline[k]==' ' || cfline[k]=='\t' || cfline[k] =='\n' || cfline[k]=='\r'))
		{ k++; }
	sscanf(cfline+k, "%f", &pconfig->T2VTable[lineID][j++]);

	while (k<nChar && (cfline[k]!=' ' && cfline[k]!='\t' && cfline[k]!='\n' && cfline[k]!='\r'))
		 { k++;}
	}

	if (j!=LENT2VTABLE)
		{
		printf("ERROR: Reading T2VTable, line \"T=....\" failed. %d numbers have been read, stop at %d col.\r\n", j,k);
		return EXIT_FAILURE;
		}

/*// debug codes to display T2VTable
	if (lineID==0)
		printf("T= ");
	else
		printf("V= ");

	for (j=0;j<LENT2VTABLE; j++)
		{
		printf("%4.1f ",pconfig->T2VTable[lineID][j]);
		if (j%10==9)
			printf("\r\n");
		}
	printf("\r\n");
*/
return 	EXIT_SUCCESS;
}
