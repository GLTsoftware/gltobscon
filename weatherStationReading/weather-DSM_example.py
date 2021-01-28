#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
sys.path.append('/home/obscon/dsm-py/')
import dsm
DSM_HOST = b"gltacc"

dsm.open()

""" 
Current definitions for weather status:
   DSM_WEATHER_TEMP_C_F      # from Vaisala weather station
   DSM_WEATHER_PRESS_MBAR_F
   DSM_WEATHER_HUMIDITY_F
   DSM_WEATHER_WINDSPEED_MPS_F
   DSM_WEATHER_WINDDIR_AZDEG_F
"""

temp = 31.0
try:
   dsm.write(DSM_HOST, b"DSM_WEATHER_TEMP_C_F", temp)
except UserWarning as w:
    print(w)

temp, timestamp = dsm.read(DSM_HOST, b"DSM_WEATHER_TEMP_C_F")
print("Temperature = {} degC at sec {}".format(temp, timestamp))

dsm.close()
