/**************************************************************
*
* observe.c
* 
* SMAsh command for tracking an astronomical source
* NAP 
* 1 March 2000
*
* 5 June 2001: added inputs for source specs and offsets
* This version is for the PowerPC
* Command line arguments are now parsed using the popt library.
* Revised on 26 Jun 2001 to command only those antennas which
*  are specified by the project command- if no antenna switch is given.
* 22 oct 2003: added additional command line arguments for input
* of source name and coordinates- allowing non-catalog sources.
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <popt.h>
#include <ctype.h>
#include <time.h>
#include "dsm.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

#define DSM_HOST "gltacc"

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: observe [options] --source <sourcename>\n"
                "[options] include:\n"
		"[--ra or -r <hh:mm:ss.sss> --dec or -d <+/-dd:mm:dd.ddd>\n"
		"--epoch -e <1950./2000.>\n"
		"--velocity or -v <velocity km/s (only vlsr for now)>]\n"
                "  -h or --help    this help\n");
        exit(exitcode);
}

int main(int argc, char *argv[])  {

	char c,*source, Source[34], command_n[30], *nameIt, fileSource[34], oldSource[34];
        char SourceCheck[34];
	short i, pmac_command_flag=0; 
	short  sourcelength;
	int gotsource=0;
	int gotpmra=0,gotpmdec=0;
	int gotra=0,gotdec=0,gotepoch=0,gotvel=0;
	int newSourceFlag=0;
	int dsm_status;
	int sourceType=0;
	poptContext optCon;	
	int tracktimestamp,timestamp;
	int trackStatus=0;
	double ra=0.0,dec=0.0,velocity=0.0,epoch=2000.;
        char *raStr,*decStr;
        int rah,ram,decd,decm;
        double ras,decs;
        int decsign;
	double pmra=0.0,pmdec=0.0;
	time_t timeStamp;
        int iSource;

	 struct  poptOption optionsTable[] = {
                {"source",'s',POPT_ARG_STRING,&source,'s',
		"source name."},
                {"name_it",'n',POPT_ARG_STRING,&nameIt,'n',
		"use this name in data file."},
                {"ra",'r',POPT_ARG_STRING,&raStr,'r',
		"RA in hours  (decimal or HH:MM:SS.SSS format"},
                {"dec",'d',POPT_ARG_STRING,&decStr,'d',
		"DEC in degrees (decimal or +-DD:MM:SS.SSS"},
                {"velocity",'v',POPT_ARG_DOUBLE,&velocity,'v',
		"source velocity in km/s, VLSR"},
                {"epoch",'e',POPT_ARG_DOUBLE,&epoch,'e',
		"Epoch of input coordinates 1950 or 2000."},
                {"pmra",'p',POPT_ARG_DOUBLE,&pmra,'p',
		"Proper motion along RA in mas/yr."},
                {"pmdec",'q',POPT_ARG_DOUBLE,&pmdec,'q',
		"Proper motion along DEC in mas/yr."},
		POPT_AUTOHELP	
                {NULL,0,0,NULL,0}
        };

 if(argc<2) usage(-1,"Insufficient number of arguments","At least source- name required.");
 
        optCon = poptGetContext("observe", argc, argv, optionsTable,0);
 
        while ((c = poptGetNextOpt(optCon)) >= 0) {
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
                case 's':
                gotsource=1;
                break;

                case 'r':
                gotra=1;
                break;

                case 'd':
                gotdec=1;
                break;

                case 'p':
                gotpmra=1;
                break;

                case 'q':
                gotpmdec=1;
                break;

                case 'e':
                gotepoch=1;
                break;

                case 'v':
                gotvel=1;
                break;
 
                }
 
        }
 
        if(gotsource!=1) usage(-2,"No source specified","Source name is required .\n");

	if((gotpmra==1)||(gotpmdec==1)) {
		if(!((gotpmra==1)&&(gotpmdec==1))) {
		usage(-5,"Insufficient arguments."," when giving proper motion values, both ra and dec pm are required.\n");
		}
	}

	if((gotra==1)||(gotdec==1)) {
		if(!((gotra==1)&&(gotdec==1)&&(gotsource==1))) {
		usage(-4,"Insufficient arguments.",
	"if coordinates are specified, source-name, ra, dec, epoch and propermotions  are all required; velocity is optional.\n");
		} else  {
                /* check if ra/dec are specified in sexagesimal or decimal
                   format and parse accordingly */
                if(strchr(raStr,':')==NULL) {
                ra=atof(raStr);
                } else {
                sscanf(raStr,"%d:%d:%lf",&rah,&ram,&ras);
                if((rah<0)||(rah>24) || (ram<0)||(ram>60)||(ras<0)||(ras>60.)) 
                    usage(-6,"Invalid RA.",NULL);
                ra=(double)rah+(double)ram/60.+ras/3600.;
                }
                if ((ra<0.0)||(ra>24.0)) usage(-6,"Invalid RA.",NULL);
                if(strchr(decStr,':')==NULL) {
                dec=atof(decStr);
                } else {
                sscanf(decStr,"%d:%d:%lf",&decd,&decm,&decs);
                if((decd<-90)||(decd>90)||(decm<0)||(decm>60)||(decs<0)||(decs>60.)) 
                    usage(-6,"Invalid DEC.",NULL);
                if(decd<0) {decsign=-1;} else {decsign=+1;}
                decd=abs(decd);
                dec=(double)decd+(double)decm/60.+decs/3600.;
                if(decsign==-1) dec=-dec;
                }
                if ((dec<-90.0)||(dec>90.0)) usage(-6,"Invalid DEC.",NULL);
                
		newSourceFlag=1;
		if(gotepoch==0) epoch=2000.;
		}
	}

 
        if(c<-1) {
        fprintf(stderr, "%s: %s\n",
                poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                poptStrerror(c));
        }
	/* initialize dsm */
	  dsm_status = dsm_open();
	  if(dsm_status!= DSM_SUCCESS) {
	    dsm_error_message(dsm_status, "dsm_open");
	    exit(-1);
	  }

	poptFreeContext(optCon);


	sourcelength=strlen(source);
	strcpy(Source,source);
	for(i=0;i<30;i++) {
	command_n[i]=0x0;
	}
	command_n[0]='n';/*0x6e;*/ /* send 'n' command */
	command_n[1]=0x0;
	pmac_command_flag=0;


#if 0

	  /* check if track is running on this antenna */
        
        rm_status=rm_read(antenna,"DSM_UNIX_TIME_L",&timestamp);
	rm_status=rm_read(antenna,"DSM_TRACK_TIMESTAMP_L",&tracktimestamp);
        if((abs(tracktimestamp-timestamp)>3L) && (antenna < 9)) {
	trackStatus=0;
	printf("Track is not running on antenna %d.\n",antenna);
	}
        if((abs(tracktimestamp-timestamp)<=3L) || (antenna > 8)) trackStatus=1;

	if(trackStatus==1) {
#endif

/*debug*/

	
	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_SOURCE_FLAG_L",
				&newSourceFlag);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }
	
	if(newSourceFlag==1) {

	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_SOURCE_C34",
				&Source);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }

	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_RA_HOURS_D", &ra);
	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_DEC_DEG_D", &dec);
	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_EPOCH_YEAR_D", &epoch);
	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_PMRA_MASPYEAR_D", &pmra);
	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_PMDEC_MASPYEAR_D", &pmdec);
	if(gotvel==0) velocity=0.0;
	dsm_status=dsm_write(DSM_HOST,"DSM_CMD_SVEL_KMPS_D", &velocity);
	} /* if new source flag =1 */

	dsm_status=dsm_write(DSM_HOST,"DSM_SOURCE_LENGTH_S",
				&sourcelength);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }

	dsm_status=dsm_read(DSM_HOST,"DSM_SOURCE_C34",SourceCheck,&timestamp);
        while(strcmp(Source,SourceCheck)!=0) {

/*
        for(iSource=0;iSource<100;iSource++) {
*/
         dsm_status=dsm_write(DSM_HOST,"DSM_SOURCE_C34",Source);
         usleep(100000);
/*
        }
*/

	dsm_status=dsm_read(DSM_HOST,"DSM_SOURCE_C34",SourceCheck,&timestamp);
        }


	dsm_status=dsm_write(DSM_HOST,"DSM_COMMANDED_TRACK_COMMAND_C30",
					command_n);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }
	
	dsm_status=dsm_write(DSM_HOST,"DSM_COMMAND_FLAG_S",
					&pmac_command_flag);
        if(dsm_status != DSM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write(DSM_COMMAND_FLAG)");
                exit(1);
        }


#if 0
	} /* if track is running */
#endif
	
return(0);
}
