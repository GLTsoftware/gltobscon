#define XNEGLIM -5000
#define XPOSLIM 5000
#define YNEGLIM -5000
#define YPOSLIM 5000
#define ZNEGLIM -10000
#define ZPOSLIM 10000
#define TIPNEGLIM -5400
#define TIPPOSLIM 5400
#define TILTNEGLIM -5400
#define TILTPOSLIM 5400
#define SPINNEGLIM 0
#define SPINPOSLIM 0

#define PMAC_HOST_IP "192.168.1.105"
#define PMAC_HOST_PORT 6660

#define ACTLOWLIM -10000
#define ACTHIGHLIM 10000

/* Actuator general status bits */
#define ACT_TRIGGER_MOVE 0x80000000
#define ACT_HOME_IN_PROG 0x40000000
#define ACT_MINUS_LIMIT 0x20000000
#define ACT_PLUS_LIMIT 0x10000000
#define ACT_FE_WARN 0x8000000
#define ACT_FE_FATAL 0x4000000
#define ACT_LIMIT_STOP 0x2000000
#define ACT_AMP_FAULT 0x1000000
#define ACT_SOFT_MINUS_LIMIT 0x800000
#define ACT_SOFT_PLUS_LIMIT  0x400000
#define ACT_I2T_FAULT 0x200000
#define ACT_TRIGGER_NOT_FOUND 0x100000
#define ACT_AMP_WARN 0x80000
#define ACT_ENC_LOSS 0x40000
#define ACT_HOME_COMPLETE 0x8000
#define ACT_DES_VEL_ZERO 0x4000
#define ACT_CLOSED_LOOP 0x2000
#define ACT_AMP_ENA 0x1000
#define ACT_IN_POS 0x800
#define ACT_BLOCK_REQUEST 0x200
#define ACT_PHASE_FOUND 0x100
#define ACT_TRIGGER_SPEED_SEL 0x80
#define ACT_GANTRY_HOMED 0x40
#define ACT_SPINDLE_MOTOR_BIT1 0x20
#define ACT_SPINDLE_MOTOR_BIT0 0x10

/* Coordinate system status word bits */
/*
#define CS_TRIGGER_MOVE 0x1F
#define CS_HOME_IN_PROG 0x1E
#define CS_MINUS_LIMIT 0x1D
#define CS_PLUS_LIMIT 0x1C
#define CS_FE_WARN 0x1B
#define CS_FE_FATAL 0x1A
#define CS_LIMIT_STOP 0x19
#define CS_AMP_FAULT 0x18
#define CS_SOFT_MINUS_LIMIT 0x17
#define CS_SOFT_PLUS_LIMIT 0x16
#define CS_I2T_FAULT 0x15
#define CS_TRIGGER_NOT_FOUND 0x14
#define CS_AMP_WARN 0x13
#define CS_ENC_LOSS 0x12
#define CS_TIMER_ENABLED 0x10
#define CS_HOME_COMPLETE 0xF 
#define CS_DES_VEL_ZERO 0xE 
#define CS_CLOSED_LOOP 0xD
#define CS_AMP_ENA 0xC 
#define CS_IN_POS 0xB 
#define CS_BLOCK_REQUEST 0x9
#define CS_TIMERS_ENABLED 0x8
#define CS_RADIUS_ERROR_BIT1 0x7
#define CS_RADIUS_ERROR_BIT0 0x6
#define CS_SOFT_LIMIT 0x5
#define CS_RUN_TIME_ERROR 0x4
#define CS_PVT_ERROR 0x3
#define CS_LIN_TO_PVT_ERROR 0x2
#define CS_ERROR_STATUS_BIT1 0x1
#define CS_ERROR_STATUS_BIT0 0x0
*/

#define CS_HOME_IN_PROG 0x40000000
#define CS_MINUS_LIMIT 0x20000000
#define CS_PLUS_LIMIT 0x10000000
#define CS_FE_WARN 0x8000000
#define CS_FE_FATAL 0x4000000
#define CS_LIMIT_STOP 0x2000000
#define CS_AMP_FAULT 0x1000000
#define CS_SOFT_MINUS_LIMIT 0x800000
#define CS_SOFT_PLUS_LIMIT 0x400000
#define CS_I2T_FAULT 0x200000
#define CS_TRIGGER_NOT_FOUND 0x100000
#define CS_AMP_WARN 0x80000
#define CS_ENC_LOSS 0x40000
#define CS_HOME_COMPLETE 0x8000
#define CS_DES_VEL_ZERO  0x4000
#define CS_CLOSED_LOOP  0x2000
#define CS_AMP_ENA  0x1000
#define CS_IN_POS  0x800
#define CS_TIMER_ENABLED 0x10000
#define CS_TIMERS_ENABLED 0x100
#define CS_SOFT_LIMIT 0x20
/*
#define CS_BLOCK_REQUEST 0x200 
#define CS_PHASE_FOUND  0x100
#define CS_TRIGGER_SPEED_SEL 0x80
#define CS_SOFT_LIMIT 
#define CS_RUN_TIME_ERROR 
#define CS_PVT_ERROR 
#define CS_LIN_TO_PVT_ERROR 
#define CS_ERROR_STATUS_BIT1 
#define CS_ERROR_STATUS_BIT0 
*/


struct hpStatusVariable {
  char lastCmdCode[5];
  char coordSysStatus[64];
  char actStatus[6][64];
  int lastSysErrorCode;
  int errorCode;
  int sysStatus;
  int errorUptime;
  int tempWarning;
  float sysUptime;
  int lastCmdUptime;
  float chopIntegration;
  float linearSpeed,rotSpeed;
  float X,Y,Z,Rx,Ry,Rz,Xt,Yt,Zt,Rxt,Ryt,Rzt;
  float XCmd,YCmd,ZCmd,RxCmd,RyCmd,RzCmd;
  float act[6];
  float MotorT[6];
  float actT[6];
  float actCmdCur[6];
};

struct pmacInfo {
  char actuatorOrder[32];
  char softwareVersion[32];
  char pmacFirmwareVer[32];
};

typedef struct hpStatusVariable hps;
typedef struct pmacInfo pmacINFO;

/*
enum sysStatusVar {"STA_SD","STA_OL","STA_MV","STA_ER","STA_IN","STA_CM","STA_PN"};
*/
