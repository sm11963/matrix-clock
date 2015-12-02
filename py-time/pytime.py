import serial
import sys
import datetime
import time


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("USAGE: {} <serial dev>".format(sys.argv[0]))
        exit(1)

    ser = serial.Serial(sys.argv[1], 9600)

    while True:

        dt = datetime.datetime.today()

        print(dt)
        w = 'sec {}'.format(dt.second)#, dt.minute, dt.second)
        print("sending: " + w)

        ser.write(bytes(w, 'utf-8')+b'\r')

        #ser.write(bytes('{}sdfd'.format(1),'utf-8')+b'\r')

        print(ser.readline())
        print(ser.readline())

        w = 'min {}'.format(dt.minute)#, dt.minute, dt.second)
        print("sending: " + w)

        ser.write(bytes(w, 'utf-8')+b'\r')

        #ser.write(bytes('{}sdfd'.format(1),'utf-8')+b'\r')

        print(ser.readline())
        print(ser.readline())

        w = 'hr {}'.format(dt.hour)#, dt.minute, dt.second)
        print("sending: " + w)

        ser.write(bytes(w, 'utf-8')+b'\r')

        #ser.write(bytes('{}sdfd'.format(1),'utf-8')+b'\r')

        print(ser.readline())
        print(ser.readline())

        time.sleep(0.1)
