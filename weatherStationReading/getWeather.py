#!/usr/bin/env python3

import numpy as np, telnetlib, time, redis, socket, sys, argparse, json
from datetime import datetime as dt
sys.path.append('/home/obscon/common-py/')
import dsm

loop_t = 60.0   # sec
M = 180         # time axis = 180 min, in principle

ap = argparse.ArgumentParser(prog='cpy3 getWeather.py', description="Record the weather and plot its history.")
ap.add_argument("-p", "--plot", dest="plotflag", action="store_true", help="Plot the weather history.")
args = ap.parse_args()
if not args.plotflag:
    print('NOTE: You could specify "-p" or "--plot" to enable plotting. See --help.')

HOST = "192.168.1.13"
PORT = 50000
TIMEOUT = 30
#tn.set_debuglevel(0)

# Redis server on Obscon, using local IP address and default port and database number
r = redis.StrictRedis(host='192.168.1.11', port=6379, db=0)

DSM_HOST = b"gltacc"
dsm.open()

if args.plotflag:
    import matplotlib
    matplotlib.use("TkAgg")         # otherwise figure refreshing doesn't work!!
    import matplotlib.pyplot as plt
    #import matplotlib.animation as animation
    plt.ion()   # needed by TkAgg
    fig, ax = plt.subplots(5, sharex=True, figsize=(6,8))
    ylabels = ["Temp [C]", "Press [mb]", "Humid [%]", "WindSp [m/s]", "WindDr [deg]"]
    cs = 'rbgcy'
    lines = []
    for i in range(len(ax)):
        ax[i].set_ylabel(ylabels[i])
        ax[i].minorticks_on()
        lines.append(ax[i].plot([], [], 'o-'+cs[i])[0])
    ax[-1].set_xlabel("Time [min] (Last 3 hrs)")

aws = np.empty((6, M)) 
aws[:] = np.nan
# rows in minutes, temperature, pressure, humidity, windspeed, winddir

j = 0   # readout data array iterator
t0 = dt.utcnow()
print("Date Time Unixtime Temp(C) Pressure(mbar) Humidity(%) WindSpeed (m/s) WindDir (deg)")
try:
    while True:
        while True:
            try:
                tn = telnetlib.Telnet(HOST,PORT,TIMEOUT)
                time.sleep(1)
                break
            except socket.error:
                print("Could not telnet to AWS310... someone else may be connected to it at the moment... trying again in 1 minute.")
                time.sleep(60)

        while True:
            ''' TABLE command gets partial weather data, but we use it here only 
            to get the date and time. CSVREP command fetches a line of csv data. '''
            try:
                tn.write(("TABLE\n").encode('ascii'))
                td = tn.read_until(b"\r\n\r\n",timeout=1)
                tn.write(("CSVREP\n").encode('ascii'))
                wd = tn.read_until(b"\n")
                t1 = dt.utcnow()
                tn.close()
                break
            except ConnectionResetError:
                print("Connection Reset Error from AWS310... will try again in 1 minute")
                time.sleep(60)

        variables = td.decode('ascii').split(':')
        values=variables[2].split(' ')
        datestamp=values[1]
        timestamp=values[2]
        timestring = datestamp + " " + timestamp + " " + str(int(t1.timestamp()))
        aws[0, j] = (t1 - t0).total_seconds()/loop_t
        
        variables = wd.decode('ascii').split(',')
        wd_names = ['TAAVG1M', 'QFEAVG1M', 'RHAVG1M', 'WSAVG2M', 'WDAVG2M']
        try:
            wp = [float(variables[variables.index(n)+1]) for n in wd_names]
        except ValueError:
            print(timestring, ' Weather parameters retrieval is unsucceeded.')
            continue
        aws[1:, j] = wp
        print(timestring, *wp)

        # Write to Redis
        d_keys = ['datetime', 'temperature', 'pressure', 'humidity', 'windspeed', 'winddir']
        
        # redis-py 2.x interface
        #r.zadd('weatherData', t1.timestamp(), dict(zip(d_keys, [timestring, *wp])))

        # Note: non-json sorted-set elements can be restored to dict as:
        # json.loads(zrange('weatherData',-1, -1)[0].decode().replace("\'", "\""))
        
        # redis-py 3.0 interface + json
        r.zadd('weatherData', {json.dumps(dict(zip(d_keys, [timestring, *wp]))): t1.timestamp()})

        """ 
        Write weather information into DSM -
        Current definitions for weather status:
           DSM_WEATHER_TEMP_C_F      # from Vaisala weather station
           DSM_WEATHER_PRESS_MBAR_F
           DSM_WEATHER_HUMIDITY_F
           DSM_WEATHER_WINDSPEED_MPS_F
           DSM_WEATHER_WINDDIR_AZDEG_F
        """
        dsm_names = ['DSM_WEATHER_TEMP_C_F', 'DSM_WEATHER_PRESS_MBAR_F', 
        'DSM_WEATHER_HUMIDITY_F', 'DSM_WEATHER_WINDSPEED_MPS_F', 'DSM_WEATHER_WINDDIR_AZDEG_F']
        try:
            [dsm.write(DSM_HOST, dn.encode(), w) for dn, w in zip(dsm_names, wp)]
        except UserWarning as w:
            print(w)
        """
        # For checking:
        temp_dsm, timestamp_dsm = dsm.read(DSM_HOST, b"DSM_WEATHER_TEMP_C_F")
        pres_dsm, timestamp_dsm = dsm.read(DSM_HOST, b"DSM_WEATHER_PRESS_MBAR_F")
        humid_dsm, timestamp_dsm = dsm.read(DSM_HOST, b"DSM_WEATHER_HUMIDITY_F")
        winds_dsm, timestamp_dsm = dsm.read(DSM_HOST, b"DSM_WEATHER_WINDSPEED_MPS_F")
        windd_dsm, timestamp_dsm = dsm.read(DSM_HOST, b"DSM_WEATHER_WINDDIR_AZDEG_F")
        print("Temperature = {} degC, Pressure = {} mbar, Humidity = {} %, Windspeed = {} m/s, Wind Direction = {} deg at sec {}".format(temp_dsm, pres_dsm, humid_dsm, winds_dsm, windd_dsm, timestamp_dsm))
        """

        if args.plotflag:
            aws_p = np.roll(aws, -(j+1), axis=1)
            aws_p = aws_p[:, np.isfinite(aws_p[0, :])]  # remove NaN
            # need to set x, y limits
            ax[0].set_xlim(np.min(aws_p[0,:]), np.max(aws_p[0,:]))
            for i in range(len(ax)):
                ax[i].set_ylim(np.min(aws_p[i+1,:]), np.max(aws_p[i+1,:]))
                lines[i].set_data(aws_p[0,:], aws_p[i+1,:])
            fig.canvas.draw()
            fig.canvas.flush_events()

        # next cycle
        time.sleep(loop_t-1)    # 1 sec spent on establishing telnet connection above
        j = (j+1) % M

except KeyboardInterrupt:
    print("Closing connection to AWS310. Bye.")
    tn.close()
    dsm.close()
    sys.exit()

