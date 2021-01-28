#include <sys/types.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <zmq.h>
#include "hpParameters.h"

//extern struct hpStatusVariable getHPstatus();
extern struct pmacInfo getPMACinfo();

int pmacStringToInt(char* pmacString);

int main() {
 const char* sysStatusVar[] =
   {"STA_SD","STA_OL","STA_CL","STA_MV","STA_ER","STA_IN","STA_CM","STA_PN"};
 const char* cmdStatusVar[] =
   {"CMD_OK","CMD_ER","CMD_ST","CMD_TR","CMD_CE","CMD_CF","CMD_NA","CMD_NV"};
 int actStatusInt,csStatusInt;
 typedef struct hpStatusVariable hps;
 typedef struct pmacInfo pmacINFO;
 hps hp;
 pmacINFO pmac;
 char title[]="GLThpMon";
 int i=0,row,col,statusCol=75,statusRow=0;
 system("resize -s 41 110");
 initscr();
 getmaxyx(stdscr,row,col);

/*
 mvprintw(2,(col-strlen(title))/2,"%s",title);
*/
        
/*
 mvprintw(row-2,0,"This screen has %d rows and %d columns\n",row,col);
 printw("Try resizing your window(if possible) and then run this program again");
*/

 mvprintw(0,0,"%s",title);
 mvprintw(1,0,"Actual");
 mvprintw(2,0,"Commanded");
 mvprintw(3,0,"Target");

 pmac=getPMACinfo();

/* Locutus 2020.11.06 --
  Because getHPstatus() creates zmq connection everytime when called, the HP 
  monitor left tons of unclosed TIME_WAIT connections in the OS. I copied the 
  function here and separated the zmq comm in two parts.
*/
 int msgLength = 512; 
 int lengthSendMsg;
 char recvBuffer[msgLength];
 char sendBuffer[msgLength];
 char PMAChost[40];
 void *context = zmq_ctx_new ();
 void *requester = zmq_socket (context, ZMQ_REQ);
 sprintf(PMAChost,"tcp://%s:%d",PMAC_HOST_IP,PMAC_HOST_PORT);
 strcpy(sendBuffer,"g");
 lengthSendMsg=strlen(sendBuffer);
 zmq_connect (requester, PMAChost);

 while(1) {
 //hp = getHPstatus();
 zmq_send(requester, sendBuffer, lengthSendMsg, 0);
 zmq_recv(requester, recvBuffer, msgLength, 0);
 sscanf(recvBuffer,
  "%f %s %f %d %d %d %d %d %s %f \ 
  %f %f %f %f %f %f %f %f %f %f \
  %f %f %f %f %f %f %f %f %f %f \
  %f %f %f %f %f %f %f %f %f %f \
  %f %f %f %f %f %f %f %f %s %s \
  %s %s %s %s %f %f %f %f %f %f",
  &hp.sysUptime,
  hp.lastCmdCode,
  &hp.lastCmdUptime,
  &hp.lastSysErrorCode,
  &hp.sysStatus,
  &hp.errorCode,
  &hp.errorUptime,
  &hp.tempWarning,
  hp.coordSysStatus,
  &hp.chopIntegration,
  &hp.linearSpeed, &hp.rotSpeed,
  &hp.X,&hp.Y,&hp.Z,&hp.Rx,&hp.Ry,&hp.Rz,
  &hp.XCmd,&hp.YCmd,&hp.ZCmd,&hp.RxCmd,&hp.RyCmd,&hp.RzCmd,
  &hp.Xt,&hp.Yt,&hp.Zt,&hp.Rxt,&hp.Ryt,&hp.Rzt,
  &hp.act[0],&hp.act[1],&hp.act[2],&hp.act[3],&hp.act[4],&hp.act[5],
  &hp.MotorT[0],&hp.MotorT[1],&hp.MotorT[2],&hp.MotorT[3],&hp.MotorT[4],&hp.MotorT[5],
  &hp.actT[0],&hp.actT[1],&hp.actT[2],&hp.actT[3],&hp.actT[4],&hp.actT[5],
  hp.actStatus[0],hp.actStatus[1],hp.actStatus[2],hp.actStatus[3],hp.actStatus[4],hp.actStatus[5],&hp.actCmdCur[0],&hp.actCmdCur[1],&hp.actCmdCur[2],&hp.actCmdCur[3],&hp.actCmdCur[4],&hp.actCmdCur[5]);

 standout();
 mvprintw(0,15,"X (um)");
 mvprintw(0,25,"Y (um)");
 mvprintw(0,35,"Z (um)");
 mvprintw(0,45,"Rx (\")");
 mvprintw(0,55,"Ry (\")");
 mvprintw(0,65,"Rz (\")");
 standend();

 mvprintw(1,15,"%.1f",hp.X);
 mvprintw(1,25,"%.1f",hp.Y);
 mvprintw(1,35,"%.1f",hp.Z);
 mvprintw(1,45,"%.1f",hp.Rx);
 mvprintw(1,55,"%.1f",hp.Ry);
 mvprintw(1,65,"%.1f",hp.Rz);

 mvprintw(2,15,"%.1f",hp.XCmd);
 mvprintw(2,25,"%.1f",hp.YCmd);
 mvprintw(2,35,"%.1f",hp.ZCmd);
 mvprintw(2,45,"%.1f",hp.RxCmd);
 mvprintw(2,55,"%.1f",hp.RyCmd);
 mvprintw(2,65,"%.1f",hp.RzCmd);

 mvprintw(3,15,"%.1f",hp.Xt);
 mvprintw(3,25,"%.1f",hp.Yt);
 mvprintw(3,35,"%.1f",hp.Zt);
 mvprintw(3,45,"%.1f",hp.Rxt);
 mvprintw(3,55,"%.1f",hp.Ryt);
 mvprintw(3,65,"%.1f",hp.Rzt);

 standout();
 mvprintw(4,15,"Act 1");
 mvprintw(4,25,"Act 2");
 mvprintw(4,35,"Act 3");
 mvprintw(4,45,"Act 4");
 mvprintw(4,55,"Act 5");
 mvprintw(4,65,"Act 6");
 standend();

/* Matrix of Actuator status variables */

 mvprintw(5,0,"Encoder:");

 mvprintw(5,15,"%.1f",hp.act[0]);
 mvprintw(5,25,"%.1f",hp.act[1]);
 mvprintw(5,35,"%.1f",hp.act[2]);
 mvprintw(5,45,"%.1f",hp.act[3]);
 mvprintw(5,55,"%.1f",hp.act[4]);
 mvprintw(5,65,"%.1f",hp.act[5]);
 
 mvprintw(6,0,"TempMot:");

 mvprintw(6,15,"%.1f",hp.MotorT[0]);
 mvprintw(6,25,"%.1f",hp.MotorT[1]);
 mvprintw(6,35,"%.1f",hp.MotorT[2]);
 mvprintw(6,45,"%.1f",hp.MotorT[3]);
 mvprintw(6,55,"%.1f",hp.MotorT[4]);
 mvprintw(6,65,"%.1f",hp.MotorT[5]);

 mvprintw(7,0,"TempBody:");

 mvprintw(7,15,"%.1f",hp.actT[0]);
 mvprintw(7,25,"%.1f",hp.actT[1]);
 mvprintw(7,35,"%.1f",hp.actT[2]);
 mvprintw(7,45,"%.1f",hp.actT[3]);
 mvprintw(7,55,"%.1f",hp.actT[4]);
 mvprintw(7,65,"%.1f",hp.actT[5]);

 mvprintw(8,0,"Current:");

 mvprintw(8,15,"%.1f",hp.actCmdCur[0]);
 mvprintw(8,25,"%.1f",hp.actCmdCur[1]);
 mvprintw(8,35,"%.1f",hp.actCmdCur[2]);
 mvprintw(8,45,"%.1f",hp.actCmdCur[3]);
 mvprintw(8,55,"%.1f",hp.actCmdCur[4]);
 mvprintw(8,65,"%.1f",hp.actCmdCur[5]);

 mvprintw(9,0,"Status:");

 mvprintw(9,15,"%s",hp.actStatus[0]);
 mvprintw(9,25,"%s",hp.actStatus[1]);
 mvprintw(9,35,"%s",hp.actStatus[2]);
 mvprintw(9,45,"%s",hp.actStatus[3]);
 mvprintw(9,55,"%s",hp.actStatus[4]);
 mvprintw(9,65,"%s",hp.actStatus[5]);

 mvprintw(10,0,"HomeInProg:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_HOME_IN_PROG) {
        standout();
        mvprintw(10,15+i*10,"1");
        standend();
        }
    else mvprintw(10,15+i*10,"0");
  }

 mvprintw(11,0,"MinusLimit:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_MINUS_LIMIT) {
        standout();
        mvprintw(11,15+i*10,"1");
        standend();
        }
    else mvprintw(11,15+i*10,"0");
  }
 
 mvprintw(12,0,"PlusLimit:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_PLUS_LIMIT) {
        standout();
        mvprintw(12,15+i*10,"1");
        standend();
        }
    else mvprintw(12,15+i*10,"0");
  }

 mvprintw(13,0,"FeWarn:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_FE_WARN) {
        standout();
        mvprintw(13,15+i*10,"1");
        standend();
        }
    else mvprintw(13,15+i*10,"0");
  }

 mvprintw(14,0,"FeFatal:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_FE_FATAL) {
        standout();
        mvprintw(14,15+i*10,"1");
        standend();
        }
    else mvprintw(14,15+i*10,"0");
  }

 mvprintw(15,0,"LimitStop:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_LIMIT_STOP) {
        standout();
        mvprintw(15,15+i*10,"1");
        standend();
        }
    else mvprintw(15,15+i*10,"0");
  }

 mvprintw(16,0,"AmpFault:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_AMP_FAULT) {
        standout();
        mvprintw(16,15+i*10,"1");
        standend();
        }
    else mvprintw(16,15+i*10,"0");
  }

 mvprintw(17,0,"SoftMinusLimit:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_SOFT_MINUS_LIMIT) {
        standout();
        mvprintw(17,15+i*10,"1");
        standend();
        }
    else mvprintw(17,15+i*10,"0");
  }

 mvprintw(18,0,"SoftPlusLimit:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_SOFT_PLUS_LIMIT) {
        standout();
        mvprintw(18,15+i*10,"1");
        standend();
        }
    else mvprintw(18,15+i*10,"0");
  }

 mvprintw(19,0,"I2tFault:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_I2T_FAULT) {
        standout();
        mvprintw(19,15+i*10,"1");
        standend();
        }
    else mvprintw(19,15+i*10,"0");
  }

 mvprintw(20,0,"AmpWarn:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_AMP_WARN) {
        standout();
        mvprintw(20,15+i*10,"1");
        standend();
        }
    else mvprintw(20,15+i*10,"0");
  }

 mvprintw(21,0,"EncLoss:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_ENC_LOSS) {
        standout();
        mvprintw(21,15+i*10,"1");
        standend();
        }
    else mvprintw(21,15+i*10,"0");
  }

 mvprintw(22,0,"HomeComplete:");
  
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_HOME_COMPLETE) {
        standout();
        mvprintw(22,15+i*10,"1");
        standend();
        }
    else mvprintw(22,15+i*10,"0");
  }

 mvprintw(23,0,"DesVelZero:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_DES_VEL_ZERO) {
        standout();
        mvprintw(23,15+i*10,"1");
        standend();
        }
    else mvprintw(23,15+i*10,"0");
  }

 mvprintw(24,0,"ClosedLoop:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_CLOSED_LOOP) {
        standout();
        mvprintw(24,15+i*10,"1");
        standend();
        }
    else mvprintw(24,15+i*10,"0");
  }

 mvprintw(25,0,"AmpEnabled:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_AMP_ENA) {
        standout();
        mvprintw(25,15+i*10,"1");
        standend();
        }
    else mvprintw(25,15+i*10,"0");
  }

 mvprintw(26,0,"InPosition:");
 for(i=0;i<6;i++) {
 if(pmacStringToInt(hp.actStatus[i]) & ACT_IN_POS) {
        standout();
        mvprintw(26,15+i*10,"1");
        standend();
        }
    else mvprintw(26,15+i*10,"0");
  }

 mvprintw(27,0,"HPLinearSpeed:");
 mvprintw(27,15,"%.1f",hp.linearSpeed);
 mvprintw(28,0,"HPRotnlSpeed:");
 mvprintw(28,15,"%.1f",hp.rotSpeed);

 mvprintw(30,0,"ActuatorsOrder:");
 mvprintw(30,16,"%s",pmac.actuatorOrder);
 mvprintw(30,25,"PMAC:");
 mvprintw(30,30,"%s",pmac.softwareVersion);
/*
 mvprintw(30,45,"PMACfirmware:");
*/
 mvprintw(30,42,"%s",pmac.pmacFirmwareVer);

/* Right side General status variables */
 mvprintw(statusRow,statusCol,"Uptime:");
 mvprintw(statusRow,statusCol+8,"%.1f",hp.sysUptime);

 mvprintw(statusRow+1,statusCol,"LastCmd:");
 mvprintw(statusRow+1,statusCol+9,"%s",hp.lastCmdCode);

 mvprintw(statusRow+2,statusCol,"UptimeLastCmd:");
 mvprintw(statusRow+2,statusCol+14,"%.1f",hp.lastCmdUptime);

 mvprintw(statusRow+3,statusCol,"LastCmdExitStatus:");
 mvprintw(statusRow+3,statusCol+19,"%s",cmdStatusVar[-hp.lastSysErrorCode]);

 mvprintw(statusRow+4,statusCol,"ActualStatus:");
 mvprintw(statusRow+4,statusCol+14,"%s",sysStatusVar[hp.sysStatus]);

 mvprintw(statusRow+5,statusCol,"ErrorCode:");
 mvprintw(statusRow+5,statusCol+10,"%d",hp.errorCode);

 mvprintw(statusRow+6,statusCol,"ErrorUptime:");
 mvprintw(statusRow+6,statusCol+12,"%d",hp.errorUptime);
 mvprintw(statusRow+7,statusCol,"TempWarning:");
 mvprintw(statusRow+7,statusCol+12,"%d",hp.tempWarning);
 mvprintw(statusRow+8,statusCol,"CSstatusWord:");
 mvprintw(statusRow+8,statusCol+14,"%s",hp.coordSysStatus);
 mvprintw(statusRow+9,statusCol,"ChopIntegTime:");
 mvprintw(statusRow+9,statusCol+14,"%.3f",hp.chopIntegration);
 mvprintw(statusRow+10,statusCol,"HomeInProg:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_HOME_IN_PROG) {
        standout();
        mvprintw(statusRow+10,statusCol+11,"1");
        standend();
        }
    else mvprintw(statusRow+10,statusCol+11,"0");

 mvprintw(statusRow+11,statusCol,"MinusLimit:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_MINUS_LIMIT) {
        standout();
        mvprintw(statusRow+11,statusCol+11,"1");
        standend();
        }
    else mvprintw(statusRow+11,statusCol+11,"0");

 mvprintw(statusRow+12,statusCol,"PlusLimit:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_PLUS_LIMIT) {
        standout();
        mvprintw(statusRow+12,statusCol+11,"1");
        standend();
        }
    else mvprintw(statusRow+12,statusCol+11,"0");

 mvprintw(statusRow+13,statusCol,"FeWarn:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_FE_WARN) {
        standout();
        mvprintw(statusRow+13,statusCol+8,"1");
        standend();
        }
    else mvprintw(statusRow+13,statusCol+8,"0");

 mvprintw(statusRow+14,statusCol,"FeFatal:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_FE_FATAL) {
        standout();
        mvprintw(statusRow+14,statusCol+9,"1");
        standend();
        }
    else mvprintw(statusRow+14,statusCol+9,"0");

 mvprintw(statusRow+15,statusCol,"LimitStop:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_LIMIT_STOP) {
        standout();
        mvprintw(statusRow+15,statusCol+10,"1");
        standend();
        }
    else mvprintw(statusRow+15,statusCol+10,"0");

 mvprintw(statusRow+16,statusCol,"AmpFault:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_AMP_FAULT) {
        standout();
        mvprintw(statusRow+16,statusCol+10,"1");
        standend();
        }
    else mvprintw(statusRow+16,statusCol+10,"0");

 mvprintw(statusRow+17,statusCol,"SoftMinusLimit:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_SOFT_MINUS_LIMIT) {
        standout();
        mvprintw(statusRow+17,statusCol+16,"1");
        standend();
        }
    else mvprintw(statusRow+17,statusCol+16,"0");

 mvprintw(statusRow+18,statusCol,"SoftPlusLimit:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_SOFT_PLUS_LIMIT) {
        standout();
        mvprintw(statusRow+18,statusCol+16,"1");
        standend();
        }
    else mvprintw(statusRow+18,statusCol+16,"0");

 mvprintw(statusRow+19,statusCol,"I2tFault:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_I2T_FAULT) {
        standout();
        mvprintw(statusRow+19,statusCol+10,"1");
        standend();
        }
    else mvprintw(statusRow+19,statusCol+10,"0");

 mvprintw(statusRow+20,statusCol,"TriggerNotFound:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_TRIGGER_NOT_FOUND) {
        standout();
        mvprintw(statusRow+20,statusCol+17,"1");
        standend();
        }
    else mvprintw(statusRow+20,statusCol+17,"0");

 mvprintw(statusRow+21,statusCol,"AmpWarning:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_AMP_WARN) {
        standout();
        mvprintw(statusRow+21,statusCol+12,"1");
        standend();
        }
    else mvprintw(statusRow+21,statusCol+12,"0");

 mvprintw(statusRow+22,statusCol,"EncLoss:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_ENC_LOSS) {
        standout();
        mvprintw(statusRow+22,statusCol+9,"1");
        standend();
        }
    else mvprintw(statusRow+22,statusCol+9,"0");

 mvprintw(statusRow+23,statusCol,"TimerEnabled:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_TIMER_ENABLED) {
        standout();
        mvprintw(statusRow+23,statusCol+12,"1");
        standend();
        }
    else mvprintw(statusRow+23,statusCol+12,"0");

 mvprintw(statusRow+24,statusCol,"HomeComplete:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_HOME_COMPLETE) {
        standout();
        mvprintw(statusRow+24,statusCol+14,"1");
        standend();
        }
    else mvprintw(statusRow+24,statusCol+14,"0");

 mvprintw(statusRow+25,statusCol,"DesVelZero:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_DES_VEL_ZERO) {
        standout();
        mvprintw(statusRow+25,statusCol+12,"1");
        standend();
        }
    else mvprintw(statusRow+25,statusCol+12,"0");

 mvprintw(statusRow+26,statusCol,"ClosedLoop:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_CLOSED_LOOP) {
        standout();
        mvprintw(statusRow+26,statusCol+12,"1");
        standend();
        }
    else mvprintw(statusRow+26,statusCol+12,"0");

 mvprintw(statusRow+27,statusCol,"AmpEnable:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_AMP_ENA) {
        standout();
        mvprintw(statusRow+27,statusCol+11,"1");
        standend();
        }
    else mvprintw(statusRow+27,statusCol+11,"0");

 mvprintw(statusRow+28,statusCol,"InPos:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_IN_POS) {
        standout();
        mvprintw(statusRow+28,statusCol+7,"1");
        standend();
        }
    else mvprintw(statusRow+28,statusCol+7,"0");

 mvprintw(statusRow+29,statusCol,"TimersEnabled:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_TIMERS_ENABLED) {
        standout();
        mvprintw(statusRow+29,statusCol+15,"1");
        standend();
        }
    else mvprintw(statusRow+29,statusCol+15,"0");

 mvprintw(statusRow+30,statusCol,"SoftLimit:");
 if(pmacStringToInt(hp.coordSysStatus) & CS_SOFT_LIMIT) {
        standout();
        mvprintw(statusRow+30,statusCol+11,"1");
        standend();
        }
    else mvprintw(statusRow+30,statusCol+11,"0");


 refresh();
 sleep(0.5);
 i++;
 }

 getch();
 endwin();
 zmq_close(requester);
 zmq_ctx_destroy(context);

 return 0;
}

pmacStringToInt(char* pmacString){
    char *err,hexPrefix[64]="0x";
    int i;
    strcat(hexPrefix,pmacString);
    i=(int)strtod(hexPrefix,&err);
    return i;
}
