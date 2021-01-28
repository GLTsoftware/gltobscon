/* hexapod single-key command */
/*
s: shutdown - switch off the +65V dc power, open loops
n: on procedure - switch on the +65V power 
e: enable
o: disable (open loop)
h: stop
i: reply with general system info
w: warmup procedure (long)
z: chopping warmup procedure (short)
t: start autotest
r: reset
*/
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
  int msgLength = 10; 
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
  char HCUsoftwareVersion[32],PMACfirmwareVersion[32];
  int actuatorsOrder;
/*
  int sysUptime,lastCmdUptime,lastSysErrorCode,errorCode;
*/

  if(argc<2) {
  printf("Usage: hexapodCommand <char>  \n");
  printf("Where <char> is one of the following:\n");
  printf("s: shutdown - switch off the +65V dc power, open loops\n");
  printf("n: on procedure - switch on the +65V power \n");
  printf("e: enable\n");
  printf("o: disable (open loop)\n");
  printf("h: stop\n");
  printf("i: reply with general system info\n");
  printf("w: warmup procedure (long)\n");
  printf("z: chopping warmup procedure (short)\n");
  printf("t: start autotest\n");
  printf("r: reset\n");
  
  exit(0);
  }


  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);

  sprintf(PMAChost,"tcp://%s:%d",PMAC_HOST_IP,PMAC_HOST_PORT);

  strcpy(sendBuffer,argv[1]);

/*
  sprintf(sendBuffer,"o 0.0");
*/

  printf("sending... %s\n",sendBuffer);

  zmq_connect (requester, PMAChost);
  lengthSendMsg=strlen(sendBuffer);
  zmq_send(requester,sendBuffer,lengthSendMsg,0);
  zmq_recv (requester, recvBuffer, msgLength, 0);
  printf("PMAC response: %s\n",recvBuffer);

  if(strcmp("i",argv[1])==0){
  sscanf(recvBuffer,"%d %s %s", &actuatorsOrder,HCUsoftwareVersion,PMACfirmwareVersion);

  printf("Actuators order inside the hexapod: %d\n",actuatorsOrder);
  printf("HCU software version: %s\n",HCUsoftwareVersion);
  printf("PMAC firmware version: %s\n",PMACfirmwareVersion);
  } else {
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

  }

  zmq_close(requester);
  zmq_ctx_destroy(context);
  return 0;
}
