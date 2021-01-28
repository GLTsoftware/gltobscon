#!/usr/bin/python
import os, datetime

def MJD(year=-1, month=-1, day=-1):
  """
  Calculate the Julian Date from the calendar date and (optionally)
  UT.
  """
  if year == -1:
    now = datetime.datetime.utcnow()
    year = now.year
    month = now.month
    day = now.day
    hour = now.hour
    minute = now.minute
    second = now.second
  y = year
  m = month
  d = float(day + hour/24.0 + minute/1440.0 + second/86400.0)
  if m < 3:
    y = y-1
    m = m+12
  a  = int(y//100)
  b  = int(2 - a + a//4)
  c  = int(365.25 * float(y))
  dd = int(30.6001 * float(m+1))
  tJD = float(b+c+dd) + d + 1720994.5
  return int(tJD-2400000)

os.chdir('/global/polar/')
os.system('rm ser7.dat')
#os.system('wget ftp://maia.usno.navy.mil/ser7/ser7.dat')
os.system('wget ftp://toshi.nofs.navy.mil/ser7/ser7.dat')
outFile = open('results.ser7', 'w')
for line in open('ser7.dat'):
    tok = line.split()
    if len(tok) == 7:
        try:
            year = int(tok[0])
            month = int(tok[1])
            day = int(tok[2])
            if (2012 < year < 2050) and (0 < month < 13) and (0 < day < 32):
                print >>outFile, '%s    %s    %s    %s' % (tok[3], tok[4], tok[5], tok[6])
        except ValueError:
            continue
outFile.close()
mJDNow = MJD()
outFile = open('polar.dat', 'w')
for line in open('results.ser7'):
    tok = line.split()
    if int(tok[0]) >= mJDNow:
        print >>outFile, line,
        break
