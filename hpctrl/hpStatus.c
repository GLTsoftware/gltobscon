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

  printf("General status of the Hexapod...\n\n");
  printf("System uptime: %.1f\n",hp.sysUptime);
  printf("Last command code: %s\n",hp.lastCmdCode);
  printf("Last command uptime: %.1f\n",hp.lastCmdUptime);
  printf("Last system error code: %d\n\n",hp.lastSysErrorCode);
  printf("System Status: %d\n",hp.sysStatus);
  printf("Error code: %d\n",hp.errorCode);
  printf("Coordinate system status: %s\n\n",hp.coordSysStatus);
  printf("Hexapod position...\n\n");
  printf("Hexapod X, Y, Z (um): %f %f %f\n",hp.X,hp.Y,hp.Z);
  printf("Target X, Y, Z (um): %f %f %f\n",hp.Xt,hp.Yt,hp.Zt);
  printf("Hexapod Rx, Ry, Rz (arcsec): %f %f %f\n",hp.Rx,hp.Ry,hp.Rz);
  printf("Target Rx, Ry, Rz (arcsec): %f %f %f\n\n",hp.Rxt,hp.Ryt,hp.Rzt);
  printf("Actuator positions...\n\n");
  printf("%f %f %f %f %f %f\n",hp.act[0],hp.act[1],hp.act[2],hp.act[3],hp.act[4],hp.act[5]);
  printf("Actuator temperatures...(C)\n\n");
  printf("%f %f %f %f %f %f\n",hp.actT[0],hp.actT[1],hp.actT[2],hp.actT[3],hp.actT[4],hp.actT[5]);
  printf("Motor temperatures...(C)\n\n");
  printf("%f %f %f %f %f %f\n",hp.MotorT[0],hp.MotorT[1],hp.MotorT[2],hp.MotorT[3],hp.MotorT[4],hp.MotorT[5]);
  printf("Actuator status: %s %s %s %s %s %s\n",hp.actStatus[0],hp.actStatus[1],hp.actStatus[2], hp.actStatus[3],hp.actStatus[4],hp.actStatus[5]);
  printf("Actuator commanded currents:\n");
  printf("%f %f %f %f %f %f\n",hp.actCmdCur[0],hp.actCmdCur[1],hp.actCmdCur[2],hp.actCmdCur[3],hp.actCmdCur[4],hp.actCmdCur[5]);

  return 0;
}
