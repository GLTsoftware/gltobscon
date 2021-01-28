#!/usr/bin/python3

import socket
import sys
import struct
import time

ACU_IP="192.168.1.103"


def decodeselftest( data):
    if (data[0] == 0x02):
        

        count = len( data)
        results= data[9:count-3]

        header= data[4:9]
        print ( "Header: ", hex(header[0]), hex(header[1]), hex(header[2]), hex(header[3]), hex(header[4]))
        print ( "Status: {0:c}".format( header[0]) )
        print ( "Current Test: ", header[1]+header[2]*256)
        print ( "Failed Tests: ", header[3]+header[4]*256)
        while( len(results) >0):
            singleresult = results[0:6]
            results = results[6:]
            id, a, b, c, d =struct.unpack("hcccc",singleresult)
            print (id,a, b ,c, d)


def checksum ( data):
    result = 0
    n=len(data)
    da = data[1:n-1]
    for k in da:
        result = result + k
    return result

def main ():
    print ("Client for Selftest Results")

    StatusRequest = bytearray([2,ord("q"),7,0])
    SelftestRequest = bytearray([2,ord("z"),7,0])

    
    print (checksum( StatusRequest))

    #data = StatusRequest + bytearray( [ checksum(StatusRequest), 0, 0x03])
    data = SelftestRequest + bytearray( [ checksum(SelftestRequest), 0, 0x03])


    print (data)
    print ("---\n\n")





    while True:
        s= socket.socket( socket.AF_INET, socket.SOCK_STREAM, 0)
        try:
            s.connect( (ACU_IP, 9110))
            print("\n\n")
            s.send( data)
            print ("receive")
            buffer = s.recv( 100)
            print( buffer)
            if( buffer[0] == 6):
                buffer = s.recv( 4000)
                print( buffer)

            length = len( buffer)
            print( "len")
            print( length)
            print("\n")
            
            decodeselftest( buffer)

            s.close()
            return 
        except socket.error as e:
            print (e)
            s.close()
            return

        except:
            print ("exception:")
            print (sys.exc_info()[0])
            s.close()
            return
main()
