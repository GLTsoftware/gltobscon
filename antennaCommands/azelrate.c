/***************************************************************
*
* azelRate.c
* 
* NAP 13 Sep 2018
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
#define AZ_NEG_LIM -167. /*deg*/
#define AZ_POS_LIM 353. 
#define EL_NEG_LIM 2.1
#define EL_POS_LIM 90.1
#define MAXRATE 6.0 /* deg per sec */


void usage(int exitcode, char *error, char *addl) {

        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: azelRate [options] --azrate (or -a) <az degrees/s rate> --elrate (or -e) <el degrees>\n"
                "[options] include:\n"
                "  -h or --help    this help\n");
        exit(exitcode);
}

int main(int argc, char *argv[])  
{

	char c,*azrate,*elrate,command_n[30];
/*      char messg[100]; */
	short  pmac_command_flag=0;
	int gotazrate=0,gotelrate=0,dsm_status;
	double commanded_az_rate,commanded_el_rate;
	float actual_az,actual_el;
	poptContext optCon;
	int i;

	int trackStatus=0;
	int tracktimestamp,timestamp;
	time_t dsmtimestamp;

        struct  poptOption optionsTable[] = {
                {"help",'h',POPT_ARG_NONE,0,'h'},
                {"azrate",'a',POPT_ARG_STRING,&azrate,'a'},
                {"elrate",'e',POPT_ARG_STRING,&elrate,'e'},
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
                gotazrate=1;
		commanded_az_rate = atof(azrate);
                break;

                case 'e':
                gotelrate=1;
		commanded_el_rate = atof(elrate);
                break;

                }


        }

 if((gotazrate!=1)&&(gotelrate!=1)) usage(-2,"No rate specified","Azrate (deg/s) and Elrate (deg/s) are both required .\n");

 /* check limits for commanded az */
  if ((commanded_az_rate < -MAXRATE) || (commanded_az_rate > MAXRATE)) {
  printf("Commanded azrate is out of allowed range of -%f to %f\n",MAXRATE,MAXRATE);
  exit(-1);
  }

 /* check limits for commanded el */
  if ((commanded_el_rate < -MAXRATE) || (commanded_el_rate > MAXRATE)) {
  printf("Commanded elrate is out of allowed range of %f to %f\n",MAXRATE,MAXRATE);
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



        command_n[0]='S'; /* send 'S' command */


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

	dsm_status=dsm_write(DSM_HOST, "DSM_CMD_AZRATE_D",&commanded_az_rate);
	dsm_status|=dsm_write(DSM_HOST, "DSM_CMD_ELRATE_D",&commanded_el_rate);
	 if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write(DSM_CMD_AZRATE_D)");
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
