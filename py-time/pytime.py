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
    print(dt.time())
    w = 't {}:{}:{}'.format(dt.hour, dt.minute, dt.second)
    print("sending: " + w)
    write_line(ser, w, 3)

def write_date(ser, dt):
    print(dt.date())
    w = 'd {}/{}/{}-{}'.format(dt.month, dt.day, (dt.year-2000), dt.weekday())
    print("sending: " + w)
    write_line(ser, w, 6)

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("USAGE: {} <serial dev>".format(sys.argv[0]))
        exit(1)

    ser = serial.Serial(sys.argv[1], 38400)

    while True:
        cmd = ser.readline().decode().strip()

        print(cmd)
        if cmd == "gtd":
            dt = datetime.datetime.today()
            write_time(ser, dt)

            time.sleep(0.050) # wait 50 ms so as to not overload the pic

            write_date(ser, dt)

