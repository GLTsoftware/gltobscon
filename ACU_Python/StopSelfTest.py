import socket
import sys
import struct
import time

def checksum ( data):
    result = 0
    n=len(data)
    da = data[1:n-1]
    
    for k in da:
        result = result + k
    return result




def main ():
    print ("Tring to activate selftest\n")
    ACU_IP = "192.168.1.103"

    #                       STX "M" ,len, m, m,m,c,
    modecommand = bytearray([2, ord("M"),12,0, 0x22,0x22, 0, 0, 0])
    stopcommand = bytearray([2, ord("M"),12,0, 0x01,0x01, 0, 0, 0])
    SelftestRequest = bytearray([2,ord("z"),7,0])

    
    print (checksum( modecommand))

    #data = StatusRequest + bytearray( [ checksum(StatusRequest), 0, 0x03])
    data = modecommand + bytearray( [ checksum(modecommand), 0, 0x03])

    stopdata= stopcommand + bytearray( [ checksum(stopcommand), 0, 0x03])

    print (data)
    print ("---\n\n")
    print(len(data))

   
    s= socket.socket( socket.AF_INET, socket.SOCK_STREAM, 0)
    try:
        s.connect( (ACU_IP, 9010))
        print("\n\n")
        s.send( stopdata)
        print ("receive")
        buffer = s.recv( 100)
        print( buffer)

        
        return 
        time.sleep( 5)
        s.send( stopdata)
        buffer = s.recv( 100)
        print( buffer)
         
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
