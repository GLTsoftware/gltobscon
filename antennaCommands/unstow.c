/***************************************************************
*
* unstow.c
*
*
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <popt.h>
#include "dsm.h"

#define OK 0
#define RUNNING 1
#define ERROR -1

#define DSM_HOST "gltacc"

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: stop [options] \n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
          "                  (default: all antennas)\n");
        exit(exitcode);
}


int main(int argc, char *argv[])  {

	char c,command_n[30];

	short pmac_command_flag=0;
	int dsm_status;
	poptContext optCon;
	int i ;

        int trackStatus=0;
        int tracktimestamp,timestamp;
	time_t dsmtimestamp;

	 struct  poptOption optionsTable[] = {
                {"help",'h',POPT_ARG_NONE,0,'h'},
                {NULL,0,0,NULL,0}
        };


        optCon = poptGetContext("stop", argc, argv, optionsTable,0);
 
        while ((c = poptGetNextOpt(optCon)) >= 0) {
 
            switch(c) {
                    case 'h':
                    usage(0,NULL,NULL);
                    break;
            }
        }


	for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };
	command_n[0]='9'; /* send the command */
        pmac_command_flag=0;

 
        if(c<-1) {
        fprintf(stderr, "%s: %s\n",
                poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                poptStrerror(c));
        }
         poptFreeContext(optCon);
 
              /* initializing DSM */
        dsm_status=dsm_open();
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_open()");
                exit(1);
        }


#if 0
          /* check if track is running on this antenna */

        dsm_status=dsm_read(DSM_HOST,"DSM_UNIX_TIME_L",&timestamp,&dsmtimestamp);
	dsm_status=dsm_read(DSM_HOST,"DSM_TRACK_TIMESTAMP_L",&tracktimestamp,&dsmtimestamp);
        if(abs(tracktimestamp-timestamp)>3L) {
        trackStatus=0;
        printf("Track is not running.\n");
        }
        if(abs(tracktimestamp-timestamp)<=3L) trackStatus=1;

        if(trackStatus==1) {
        dsm_status=dsm_write(DSM_HOST,"DSM_COMMANDED_TRACK_COMMAND_C30",
                                        &command_n);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }
#endif
 
        dsm_status=dsm_write(DSM_HOST,"DSM_COMMANDED_TRACK_COMMAND_C30",
                                        &command_n);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }
 
/* the following dsm_write should actually be dsm_write_notify- but due 
to a bug in gltTrack, dsm_write_notify does not work */
     dsm_status=dsm_write(DSM_HOST,"DSM_COMMAND_FLAG_S",
                                        &pmac_command_flag);
 
     if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }
 
#if 0
 
	} /* if track is running */
#endif

	return 0;
 
}
