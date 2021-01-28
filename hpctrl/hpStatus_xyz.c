/* hpStatus command */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "hpParameters.h"

extern struct hpStatusVariable getHPstatus();

int main(int argc, char *argv[]) {
  int msgLength = 512; 
  int lengthSendMsg;
  char recvBuffer[msgLength];
  char sendBuffer[msgLength];

  char PMAChost[40];

  typedef struct hpStatusVariable hps;
  hps hp;

  hp = getHPstatus();

  printf("%f %f %f",hp.X,hp.Y,hp.Z);

  return 0;
}
