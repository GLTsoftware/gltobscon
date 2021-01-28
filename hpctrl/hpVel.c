/* velocity command */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <zmq.h>
/*
#include "HPData.h"
#include "HPCommands.h"
*/
#include "hpParameters.h"

short checkSum (char *buff, int size);

int main(int argc, char *argv[]) {
  int msgLength = 512; 
  int lengthSendMsg;
  char recvBuffer[msgLength];
  char sendBuffer[msgLength];
  char sentCmdCode[10];
  int timestamp,exitCode;

/*
  HPStatus HPStatusResp;
  HPCmd HPCommand;
  HPXYZCmd HPXYZCommand;
*/

  char PMAChost[40];

  float linearVel,rotVel;
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);

  /* check validity of requested positions */
 
  if(argc!=4) {
  printf("Usage: setAxisVel <linear micron/s> <rotational arcsec/s> \n");
  printf("Usage: setAxisVel 100.0 100.0\n");
  exit(0);
  }

  sprintf(PMAChost,"tcp://%s:%d",PMAC_HOST_IP,PMAC_HOST_PORT);

  linearVel = atof(argv[2]);
  rotVel = atof(argv[3]);

  if((linearVel<1.0)||(linearVel>500.0)||(rotVel<1.0)||(rotVel>560.0)) {
  printf("Invalid command; linear velocity should be between 1 and 500 microns/sec and rotational velocity between 1.0 and 560.0 arcsec/sec.\n");
  exit(0);
  }
 
  sprintf(sendBuffer,"v %f %f",linearVel,rotVel);

  zmq_connect (requester, PMAChost);
  lengthSendMsg=strlen(sendBuffer);
  zmq_send(requester,sendBuffer,lengthSendMsg,0);
  zmq_recv (requester, recvBuffer, msgLength, 0);
  printf("PMAC response: %s\n",recvBuffer);
  sscanf(recvBuffer,"%s %d %d",sentCmdCode,&timestamp,&exitCode);
  switch (exitCode) {
       case 0:
       printf("Command parsed OK\n");
       break;
   
       case -1:
       printf("System error. Fix error before issuing another command.\n");
       break;
 
       case -2:
       printf("Command cannot be sent in the actual status\n");
       break;

       case -3: 
       printf("Target position outside boundaries\n");
       break;

       case -4:
       printf("Chopping target position outside boundaries\n");
       break;

       case -5:
       printf("Chopping target frequency too high\n");
       break;

       case -6:
       printf("Command not recognized\n");
       break;
  }
  zmq_close(requester);
  zmq_ctx_destroy(context);
  return 0;
}
