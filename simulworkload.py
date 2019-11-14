import argparse

parser = argparse.ArgumentParser()
parser.add_argument("step_size", type=int, help = "number of steps to keep")
parser.add_argument("-f", '--filename', action='store', dest='filename', default='/media/onetbdrive/m.cap.out')
args = parser.parse_args()

writeto = '/media/onetbdrive/m.cap.short.out'
fw = open(writeto, 'w')

with open(args.filename) as f:
    for lineno, line in enumerate(f):
        if lineno % args.step_size == 0:
            fw.write(line)
