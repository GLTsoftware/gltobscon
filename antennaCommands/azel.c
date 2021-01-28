/***************************************************************
*
* azel.c
* 
* NAP 18 FEB 99.
*
* Revised on 2 March 2000 for PowerPC and popt.
* Revised on 27 Feb 2001 to command all antennas on default
* Revised on 26 Jun 2001 to command only those antennas which
*  are specified by the project command- if no antenna switch is given.
****************************************************************/

#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
/*#include <popt.h>*/
#include <popt.h>
#include "dsm.h"
/*#include "commonLib.h"*/

#define DSM_HOST "gltacc"

#define OK     0
#define ERROR -1
#define RUNNING 1
#define AZ_NEG_LIM -167.
#define AZ_POS_LIM 353. 
#define EL_NEG_LIM 1.89
#define EL_POS_LIM 90.1


void usage(int exitcode, char *error, char *addl) {

        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: azel [options] --azdegrees (or -a) <az degrees> --eldegrees (or -e) <el degrees>\n"
                "[options] include:\n"
                "  -h or --help    this help\n");
        exit(exitcode);
}

int main(int argc, char *argv[])  
{

	char c,*azdegrees,*eldegrees,command_n[30];
/*      char messg[100]; */
	short  pmac_command_flag=0;
	int gotaz=0,gotel=0,dsm_status;
	double commanded_az,commanded_el;
	float actual_az,actual_el;
	poptContext optCon;
	int i;

	int trackStatus=0;
	int tracktimestamp,timestamp;
	time_t dsmtimestamp;

        struct  poptOption optionsTable[] = {
                {"help",'h',POPT_ARG_NONE,0,'h'},
                {"azdegrees",'a',POPT_ARG_STRING,&azdegrees,'a'},
                {"eldegrees",'e',POPT_ARG_STRING,&eldegrees,'e'},
                {NULL,0,0,NULL,0}
        };

   for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };

 if(argc<2) usage(-1,"Insufficient number of arguments","At least az(deg) required.");

        optCon = poptGetContext("az", argc, argv, optionsTable,0);

        while ((c = poptGetNextOpt(optCon)) >= 0) {

        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;

                case 'a':
                gotaz=1;
		commanded_az = atof(azdegrees);
                break;

                case 'e':
                gotel=1;
		commanded_el = atof(eldegrees);
                break;

                }


        }

 if((gotaz!=1)&&(gotel!=1)) usage(-2,"No position specified","Az (deg) and El (deg) are both required .\n");

 /* check limits for commanded az */
  if ((commanded_az < AZ_NEG_LIM) || (commanded_az > AZ_POS_LIM)) {
  printf("Commanded az is out of allowed range of %f to %f\n",AZ_NEG_LIM,AZ_POS_LIM);
  exit(-1);
  }

 /* check limits for commanded el */
  if ((commanded_el < EL_NEG_LIM) || (commanded_el > EL_POS_LIM)) {
  printf("Commanded az is out of allowed range of %f to %f\n",EL_NEG_LIM,EL_POS_LIM);
  exit(-1);
  }



        if(c<-1) {
        fprintf(stderr, "%s: %s\n",
                poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                poptStrerror(c));
        }
         poptFreeContext(optCon);

       /* initializing ref. mem. */
        dsm_status=dsm_open();
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_open()");
                exit(1);
        }



        command_n[0]='T'; /* send 'T' command */


          /* check if track is running on this antenna */

       dsm_status=dsm_read(DSM_HOST,"DSM_UNIX_TIME_L",&timestamp,&dsmtimestamp);
        dsm_status=dsm_read(DSM_HOST,"DSM_TRACK_TIMESTAMP_L",&tracktimestamp,&dsmtimestamp);

/*
printf("%d %d \n",timestamp,tracktimestamp);
*/

#if 0
        if(abs(tracktimestamp-timestamp)>3L) {
        trackStatus=0;
        printf("Track is not running.\n");
        }
        if(abs(tracktimestamp-timestamp)<=3L) trackStatus=1;

      if(trackStatus==1) {
#endif

	dsm_status=dsm_write(DSM_HOST, "DSM_COMMANDED_AZ_DEG_D",&commanded_az);
	dsm_status|=dsm_write(DSM_HOST, "DSM_COMMANDED_EL_DEG_D",&commanded_el);
	 if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write(DSM_COMMANDED_AZ_DEG_D)");
                exit(1);
        }


        dsm_status=dsm_write(DSM_HOST,"DSM_COMMANDED_TRACK_COMMAND_C30",
                                        &command_n);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write(DSM_COMMANDED_TRACK_COMMAND_C30)");
                exit(1);
        }



/* the following dsm_write should actually be dsm_write_notify- but due
to a bug in gltTrack, dsm_write_notify does not work */
     dsm_status=dsm_write_notify(DSM_HOST,"DSM_COMMAND_FLAG_S",
                                        &pmac_command_flag);

     if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write_notify(DSM_COMMAND_FLAG_S)");
                exit(1);
        }




#if 0
	} /* if track is running */
#endif
return (0);
}
