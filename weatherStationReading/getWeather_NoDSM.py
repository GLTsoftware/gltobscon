#!/usr/bin/python3

from numpy import savetxt
import matplotlib
from pylab import *
matplotlib.use('GTKAgg')
import matplotlib.pyplot as plt
import numpy as np
import telnetlib
import time
import redis
import sys
import socket

HOST = "192.168.1.15"
PORT = 50000
TIMEOUT = 30
M = 180
#tn.set_debuglevel(0)

# Redis server on obscon, using local IP address
# and default port and database number
r = redis.StrictRedis(host='192.168.1.11',port=6379,db=0)


f,(ax1,ax2,ax3,ax4,ax5)= plt.subplots(5,sharex=True)

ax1.set_xlim([0,M])
ax2.set_xlim([0,M])
ax3.set_xlim([0,M])
ax4.set_xlim([0,M])
ax5.set_xlim([0,M])

plt.ion()

timearray=[]
temperature=[]
pressure=[]
humidity=[]
windspeed=[]
winddir=[]
t0=0
timestring=""

j=0
shiftflag=0
while True:
    try:
        while True:
            try:
                tn = telnetlib.Telnet(HOST,PORT,TIMEOUT)
                time.sleep(1)
            except socket.error:
                print("Could not telnet to AWS310... someone else may be connected to it at the moment... trying again in 1 minute.")
                time.sleep(60)
                break

            #TABLE command gets partial weather data
            #but we use it here only to get the date and time
            try:
                tn.write(("TABLE\n").encode('ascii'))
                break
            except ConnectionResetError:
                print("Connection Reset Error from AWS310... will try again in 1 minute")
                time.sleep(60)

        if (j==0):
            print("Date Time Unixtime Temp(C) Humidity(%) Pressure(mbar) WindSpeed (m/s) WindDir (deg)")
            t0=time.time()
        t1 = time.time()
        if (shiftflag==0):
            timearray.append(int((t1-t0)/60.))
        else:
            timearray[:-1]=timearray[1:];timearray[-1]=int((t1-t0)/60.)

        data = tn.read_until(b"\r\n\r\n",timeout=1)
        line=data.decode('ascii')
        variables=line.split(':')
        values=variables[2].split(' ')
        datestamp=values[1]
        timestamp=values[2]
        timestring = datestamp + " " + timestamp + " " + str(int(t1))
        #CSVREP command fetches a line of csv data
        tn.write(("CSVREP\n").encode('ascii'))
        data = tn.read_until(b"\n")
        line=data.decode('ascii')
        variables=line.split(',')
        for i in range(1,len(variables)-1,2):
            if "TAAVG1M" in variables[i]:
                temp=float(variables[i+1])
                if (shiftflag==0):
                    temperature.append(temp)
                else:
                    temperature[:-1]=temperature[1:];temperature[-1]=temp
            if "QFEAVG1M" in variables[i]:
                pres=float(variables[i+1])
                if (shiftflag==0):
                    pressure.append(pres)
                else:
                    pressure[:-1]=pressure[1:];pressure[-1]=pres
            if "RHAVG1M" in variables[i]:
                humid=float(variables[i+1])
                if (shiftflag==0):
                    humidity.append(humid)
                else:
                    humidity[:-1]=humidity[1:];humidity[-1]=humid
            if "WDAVG2M" in variables[i]:
                windd=float(variables[i+1])
                if (shiftflag==0):
                    winddir.append(windd)
                else:
                    winddir[:-1]=winddir[1:];winddir[-1]=windd
            if "WSAVG2M" in variables[i]:
                winds=float(variables[i+1])
                if (shiftflag==0):
                    windspeed.append(winds)
                else:
                    windspeed[:-1]=windspeed[1:];windspeed[-1]=winds
        if (shiftflag==1):
            ax1.set_xlim([min(timearray),max(timearray)])
            ax2.set_xlim([min(timearray),max(timearray)])
            ax3.set_xlim([min(timearray),max(timearray)])
            ax4.set_xlim([min(timearray),max(timearray)])
            ax5.set_xlim([min(timearray),max(timearray)])
        ax1.plot(timearray,temperature,"o-",color="red")
        ax1.set_ylabel("Temp [C]")
        ax2.plot(timearray,humidity,"o-",color="blue")
        ax2.set_ylabel("Humid [%]")
        ax3.plot(timearray,pressure,"o-",color="green")
        ax3.set_ylabel("Press [mb]")
        ax4.plot(timearray,windspeed,"o-",color="cyan")
        ax4.set_ylabel("WindSp [m/s]")
        ax5.plot(timearray,winddir,"o-",color="orange")
        ax5.set_ylabel("WindDr [deg]")
        ax5.set_xlabel("Time [min] (Last 3 hrs)")
        print(timestring,temp,humid,pres,winds,windd)
        r.zadd('weatherData',t1,{"datetime":timestring,"temperature":temp,"humidity":humid,"pressure":pres,"windspeed":winds,"winddir":windd})
        tn.close()
        time.sleep(59)
        j=j+1
        if (j>M):
            shiftflag=1
        plt.pause(0.05) 
    except KeyboardInterrupt:
        print("Closing connection to AWS310. Bye.")
        tn.close()
        sys.exit()

while True:
    plt.pause(0.05)
