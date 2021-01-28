/* Move an actuator to absolute position */
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

  float targetPosition;
  int actuatorID;
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);

  /* check validity of requested positions */
 
  if(argc<2) {
  printf("Usage: moveActuator <actuator ID (1-6)> <microns> \n");
  printf("Usage: moveActuator 1 100.0\n");
  exit(0);
  }

  sprintf(PMAChost,"tcp://%s:%d",PMAC_HOST_IP,PMAC_HOST_PORT);

  actuatorID = atoi(argv[1]);
  targetPosition = atof(argv[2]);

  if((actuatorID<1)||(actuatorID>6)||(targetPosition<ACTLOWLIM)||(targetPosition>ACTHIGHLIM)) {
  printf("Invalid command; actuatorID betn 1 and 6; check position value.\n");
  exit(0);
  }
 
  sprintf(sendBuffer,"a %d %f",actuatorID,targetPosition);

  zmq_connect (requester, PMAChost);
  lengthSendMsg=strlen(sendBuffer);
  zmq_send(requester,sendBuffer,lengthSendMsg,0);
  zmq_recv (requester, recvBuffer, msgLength, 0);
  printf("PMAC response: %s",recvBuffer);
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
