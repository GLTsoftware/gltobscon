/***************************************************************
*
* offsetUnit.c
*
*
* for GLT, DSM version. 4 September 2012. Satoki Matsushita
*              Revised. 7 December 2017. Satoki Matushita
*
*
* Original Notes:
*
* NAP 30 Dec 98
*
* Revised on 2 March 2000 for PowerPC and smapopt.
* Revised on 21 Feb 2001 apply to all antennas by default
* Revised on 26 Jun 2001 to command only those antennas which
*  are specified by the project command- if no antenna switch is given.
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <popt.h>
#include "dsm.h"
/* #include "commonLib.h" */

#define DSM_HOST "gltacc"

#define ERROR -1

 
void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: offsetUnit [options] --arcseconds ( or -s) <arcseconds>\n"
                "[options] include:\n"
                "  -h or --help    this help\n");
        exit(exitcode);
}

int main(int argc, char *argv[])  
{
	char c,command_n[30],*arcseconds;
	short pmac_command_flag=0;
	int i;
	short offset_unit;
	int gotit=0,dsm_status;
	poptContext optCon;
        int trackStatus=0;
        int tracktimestamp,timestamp;
	time_t dsmtimestamp;

	struct poptOption optionsTable[] = {
            {"help",'h',POPT_ARG_NONE,0,'h'},
            {"arcseconds",'s',POPT_ARG_STRING,&arcseconds,'s'},
            {NULL,0,0,NULL,0}
        };

	if(argc<2) usage(-1,"Insufficient number of arguments",
		"At least offsetUnits (arcseconds) required.");

        optCon = poptGetContext("offsetUnit", argc, argv, optionsTable,0);

        while ((c = poptGetNextOpt(optCon)) >= 0) {

            switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;

                case 's':
                gotit=1;
		offset_unit = atoi(arcseconds);
                break;
            }
        }

	for(i=0;i<30;i++) {
            command_n[i]=0x0;
        };
	pmac_command_flag=0;
	command_n[0]='U'; /* send 'U' command */

        if(gotit!=1) usage(-2,"No value given",
            "OffsetUnit (arcseconds) is required.\n");

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

#if 0
        /* check if track is running on this antenna */

        dsm_status=dsm_read(DSM_HOST,"DSM_UNIX_TIME_L",
    					&timestamp,&dsmtimestamp);
	dsm_status=dsm_read(DSM_HOST,"DSM_TRACK_TIMESTAMP_L",
					&tracktimestamp,&dsmtimestamp);

        printf("%d %d \n",timestamp,tracktimestamp);

        if(abs(tracktimestamp-timestamp)>3L) {
                trackStatus=0;
                printf("Track is not running.\n");
        }
        if(abs(tracktimestamp-timestamp)<=3L) trackStatus=1;

        if(trackStatus==1) {
#endif

	dsm_status=dsm_write(DSM_HOST,"DSM_OFFSET_UNIT_ARCSEC_S",
					&offset_unit);
	if(dsm_status != DSM_SUCCESS) {
    		dsm_error_message(dsm_status,"dsm_write()");
		exit(1);
       	}

	dsm_status=dsm_write(DSM_HOST,"DSM_COMMANDED_TRACK_COMMAND_C30",
                                        &command_n);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }

	/* the following dsm_write should actually be dsm_write_notify
	   - but due to a bug in gltTrack, dsm_write_notify does not work */
	dsm_status=dsm_write(DSM_HOST,"DSM_COMMAND_FLAG_S",
                                            &pmac_command_flag);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }

#if 0
		} /* if track is running */
#endif

return(0); 
}
