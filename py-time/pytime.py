import serial
import sys
import datetime
import time

def write_line(ser, s, delay=1.5):
    s += "\r"
    total = 0
    for c in s:
        total += ser.write(bytes(c, 'utf-8'))
        time.sleep(float(delay)/1000.0)
    return total

def write_time(ser, dt):
    print(dt)
    w = 't {}:{}:{}'.format(dt.hour, dt.minute, dt.second)
    print("sending: " + w)
    write_line(ser, w)

def write_date(ser, dt):
    print(dt.date())
    w = 'd {}/{}/{}-{}'.format(dt.month, dt.day, (dt.year-2000), dt.weekday())
    print("sending: " + w)
    write_line(ser, w, 5)

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("USAGE: {} <serial dev>".format(sys.argv[0]))
        exit(1)

    ser = serial.Serial(sys.argv[1], 57600)
    last_sec = -1
    last_minute = -1

    while True:

        dt = datetime.datetime.today()
        if int(dt.second) != last_sec:
            write_time(ser, dt)
            last_sec = int(dt.second)

            if dt.minute != last_minute:
                time.sleep(0.02)
                write_date(ser, dt)
                last_minute = dt.minute

            time.sleep(0.800)


        if int(dt.minute) != last_minute:
            write_date(ser, dt)
            last_minute = int(dt.minute)
            #print(ser.readline())


        time.sleep(0.005)

        #time.sleep(0.01)
