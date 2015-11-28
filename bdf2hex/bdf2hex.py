import os
import sys

chars = {}

with open("font.bdf",'r') as f:
    while True:
        l = f.readline()
        if not l: break

        if l.startswith("STARTCHAR"):
            name = l.split(" ")[1].strip()
            l = f.readline()
            encoding = int(l.split(" ")[1].strip())

            while(not l.startswith("BBX")):
                l = f.readline()

            _, w, h, x, y = l.strip().split(" ")

            while(not l.startswith("BITMAP")):
                l = f.readline()

            bitmap = []
            while(not l.startswith("ENDCHAR")):
                l = f.readline()
                if (not l.startswith("ENDCHAR")):
                    bitmap.append(int(l.strip(), 16)>>(4+int(x)))

            for i in range(int(y)+1):
                bitmap.append(0)
            for i in range(6-len(bitmap)):
                bitmap.insert(0, 0)

#            print("{}: {}".format(encoding,name))
#
#            for i in range(6):
#                print("{:04b}".format(bitmap[i]))


            bitmap2 = [0,0,0]
            for i in range(6):
                bitmap2[0] |= ((bitmap[i] & 8) >> 3) << i
                bitmap2[1] |= ((bitmap[i] & 4) >> 2) << i
                bitmap2[2] |= ((bitmap[i] & 2) >> 1) << i

#            print(bitmap2)
#            for i in range(4):
#                print("{:06b}".format(bitmap2[i]))

            bitmap2hex = ["0x{:02x}".format(x) for x in bitmap2]

            #print("{{ {} }}".format(",".join(bitmap2hex)))

            chars[encoding] = {'name': name, 'bitmap': bitmap, 'bitmap2': bitmap2, 'hex': bitmap2hex }

for i in range(256):
    if i in chars:
        print(", ".join(chars[i]['hex']), end='')
        print( ("," if i < 255 else ""), end='' )
        print(" // {:3} - {}".format(i, chars[i]['name']))
    else:
        print("0x00, 0x00, 0x00", end='')
        print( ("," if i < 255 else ""), end='' )
        print(" // {:3} - empty".format(i))


#for enc in sorted(chars.keys()):
#    print("{}: {}".format(enc, chars[enc]['name']))

