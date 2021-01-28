/* getHPstatus function */
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

typedef struct pmacInfo pmacINFO;

pmacINFO getPMACinfo() {
  int msgLength = 512; 
  int lengthSendMsg;
  char recvBuffer[msgLength];
  char sendBuffer[msgLength];

  char PMAChost[40];

  pmacINFO pmac;

  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);

  sprintf(PMAChost,"tcp://%s:%d",PMAC_HOST_IP,PMAC_HOST_PORT);

  strcpy(sendBuffer,"i");

  zmq_connect (requester, PMAChost);
  lengthSendMsg=strlen(sendBuffer);
  zmq_send(requester,sendBuffer,lengthSendMsg,0);
  zmq_recv (requester, recvBuffer, msgLength, 0);
/*
  printf("PMAC response: %s",recvBuffer);
*/

sscanf(recvBuffer,"%s %s %s",
pmac.actuatorOrder,
pmac.softwareVersion,
pmac.pmacFirmwareVer);

  zmq_close(requester);
  zmq_ctx_destroy(context);
  return pmac;
}
