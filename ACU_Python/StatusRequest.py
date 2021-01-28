import socket
import sys
import struct
import time

ACU_IP="192.168.1.103"


def decodeselftest( data):
    if (data[0] == 0x02):
        print ("ok\n");
        count = len( data)
        results= data[9:count-3]
        print(len(results))
        print("\n")
        print (results)

        while( len(results) >0):
            singleresult = results[0:5]
            results = results[6:]
            print( "::")
            print( singleresult)



def checksum ( data):
    result = 0
    n=len(data)
    da = data[1:n-1]
    
    for k in da:
        result = result + k
    return result




def main ():
    print ("Client")

    StatusRequest = bytearray([2,ord("q"),7,0])
    data = StatusRequest + bytearray( [ checksum(StatusRequest), 0, 0x03])
    #data = SelftestRequest + bytearray( [ checksum(SelftestRequest), 0, 0x03])


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
                buffer = s.recv( 500)
                print( buffer)

            length = len( buffer)
            print( "len")
            print( length)
            print("\n")
            
            print( buffer[34])
            print( buffer[35] ) 

 
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
