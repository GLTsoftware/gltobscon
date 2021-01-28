#!/usr/bin/env python3
# @author: Locutus
# Last Update 2019 September

from pymodbus.client.sync import ModbusTcpClient as ModbusClient
import numpy as np, sys, time
sys.path.append('/home/obscon/common-py/')
import hexapod
#%%
class CSRB:
    #pin2lim = np.array((5, 11, 10, 12, 9, 16, 14, 15, 13, 3, 2, 4, 1, 7, 6, 8)) -1     # incorrect
    pin2lim = np.array((13, 15, 14, 16, 9, 12, 10, 11, 5, 8, 6, 7, 1, 4, 2, 3)) -1     # 2a/pin 13 as +X axis
    sys_status = {0:'Normal', 1:'Collision Triggered', 2:'In Recovery (Masked)', 
                  3:'Recovered (Exiting Recovery)', 4:'Unknown Failure'}
    
    def __init__(self):
        self.client = ModbusClient('192.168.1.106', port=502, timeout=10)
        self.client.connect()
    def __del__(self):
        self.client.close()
    def __str__(self):
        s = ('========= CSRB Report ========\n'
             'Number of triggered limit switches = {}, at {} degrees.\n'
             'Panic output relay normally-closed? {}\n'
             'System status = {}\n'
             'Recovery timeout code = {}\n'
             '==============================').format(np.sum(self.sensors), 
            np.rad2deg(self.sen_ang), self.no_panic, self.status, self.timeout)
        return s
    
    @property
    def sensors(self):
        di = np.array(self.client.read_coils(0x8000, 16).bits[0:16])      # read coils
        return di[self.pin2lim]       # boolean array, re-ordered to match HP coord.
    @property
    def sen_ang(self):
        lim_angle = np.pi/8 * np.arange(16)
        return lim_angle[self.sensors]
    @property
    def no_panic(self):
        do = self.client.read_coils(0x8010, 1)      # read coil
        return do.bits[0]
    @property
    def status(self):
        sta = self.client.read_holding_registers(0x8000, 1) # read holding register
        sr = sta.registers[0]
        return sr, self.sys_status[sr]
    @property
    def timeout(self):
        to = self.client.read_holding_registers(0x8001, 1) # read holding register
        return to.registers[0]
    
    def recovery(self, hp):
        hstep = 10   # μm, hexapod retract step
        haxis = 10   # μm/s, arcsec/s; axis velocity default 500 μm/s, 540 arcsec/s
        hstep_max = 3000    # μm, max retract distance. (XY software limit = 5000 μm)
        re_addr = 0x8012
        self.client.write_coil(re_addr, True)    # write coil
        time.sleep(1)
        # check if panic is released
        if self.no_panic and self.status[0]==2:
            hp_orig = hp.abs_position + hp.axis_velocity
            if np.abs(hp_orig[5]) > 0.5:    # Rz must be close to zero
                sys.exit('The Spin (Rz) is not zero! Exit.')
            ''' If collision is trigged when STA_CL (-> STA_ER|STA_PN), and then bypassed or restored,
                status will NOT come back to STA_CL or STA_OL, but STA_ER. Have to power-cycle 
                HCU and it will be STA_PN and then STA_OL.
            '''
            hp.send_cmd('e')	# STA_CL -> STA_PN -> STA_ER -> power cycle -> STA_PN (STA_OL)
            time.sleep(1)
            hp.send_cmd('v', haxis, haxis)        # slow down axis velocities; unable to perform at STA_PN
            print('Start retracting with each step = {} μm.'.format(hstep))
            rad_axis_sen = csrb.sensors.reshape(2, -1, order='F')
            if rad_axis_sen[0, :].any():
                print('radial (XY) sensor is triggered.')
                x_decl = hp_orig[0] + np.arange(1, int(hstep_max/hstep)+1) * -np.cos(csrb.sen_ang)*hstep
                y_decl = hp_orig[1] + np.arange(1, int(hstep_max/hstep)+1) * -np.sin(csrb.sen_ang)*hstep
                if x_decl.size != y_decl.size:      # equalize the length of steps
                    xyl = max(len(x_decl), len(y_decl))
                    tmp = np.zeros(xyl)
                    tmp[:len(x_decl)] = x_decl
                    x_decl = tmp
                    tmp = np.zeros(xyl)
                    tmp[:len(y_decl)] = y_decl
                    y_decl = tmp
                z_decl = hp_orig[2] * np.ones(x_decl.size)
            elif rad_axis_sen[1, :].any():
                print('axial (Z) sensor is triggered.')
                # outward motion only because the co-plane axial sensors. Z soft limit 10,000 μm
                z_decl = np.arange(hp_orig[2]+hstep, min(hp_orig[2]+hstep_max, 1e4), hstep)
                x_decl = hp_orig[0] * np.ones(z_decl.size)
                y_decl = hp_orig[1] * np.ones(z_decl.size)
            else:
                raise RuntimeError('Collision sensor type cannot be determined!')

            x_di = iter(x_decl);    y_di = iter(y_decl);    z_di = iter(z_decl)
            while self.status[0] != 3:
                print('Hexapod position X={}, \tY={}, \tZ={}'.format(*hp.abs_position[0:3]))
                try:
                    ns = next(x_di), next(y_di), next(z_di), *hp_orig[3:5], 0   # Rz must be 0
                except StopIteration:
                    print('No more available steps!', file=sys.stderr)
                    break
                print('Next step at coord. =', ns)
                input('--------- Press any key to move HP ---------')
                hp.send_cmd('m', *ns)
                hp.wait_in_position()
        else:
            raise RuntimeError('Not able to switch to the recovery mode.')
        # completion check
        if self.status[0] == 3:
            hp.send_cmd('v', *hp_orig[6:])
            time.sleep(1)
            hp.send_cmd('o')                # open loop
            print('Seems recovery success. Quit the recovery mode.')
            self.client.write_coil(re_addr, False)    # write coil
            time.sleep(1)
            print('The CSRB status changes to {}.'.format(self.status[1]))
        else:
            raise RuntimeError('CSRB cannot get a clean recovery-exit state. Abort.')
#%%
csrb = CSRB()
hp = hexapod.Connection()
# display the panic situation
print(csrb)
if np.any(csrb.sensors) and (not csrb.no_panic) and (csrb.status[0]==1):
    print('Collision accident confirmed. The triggered switches are at {}'
          'degrees.'.format(np.rad2deg(csrb.sen_ang)))
else:
    sys.exit('Auto-recovery can not be applied on the circumstance. Exit.')

# exam if the triggered sensors are next to each other
#angles = csrb.sen_ang
#if np.amax(angles) != np.amin(angles) + (len(angles) -1) * np.pi/8:
#    sys.exit('Multiple switches are touched but no optimal maneuver can be solved. Exit.')
#
# cancel the support of multiple triggers because of the different types of sensors 
if len(csrb.sen_ang) > 1:
    sys.exit('Multiple collision switches triggered. Exit.')

# try homing & exam if it is away from the touched area
## XYZ translation only, no tilts
xy_ang = np.arctan2(*hp.abs_position[1::-1])
if xy_ang < 0:
    xy_ang += 2*np.pi
print('HP position angle = {} degrees'.format(np.rad2deg(xy_ang)))
if np.abs(xy_ang - csrb.sen_ang) > np.pi/8/2:
    sys.exit('HP position mismatches with the collision location. Exit.')

# enter recovery
c = input('CSRB is going to enter the recovery mode to retract the HP. Proceed (y/n)? [n]')
if len(c) == 1 and c in 'yY':
    try:
        csrb.recovery(hp)
    except Exception as e:
        print('Exception *** {} *** occured during recovery.'.format(e), file=sys.stderr)
        print(csrb)
        print('Please examine again the collision accident carefully. ',
              'The program ends here.', file=sys.stderr)
else:
    sys.exit('Chose to do nothing. Exit.')
