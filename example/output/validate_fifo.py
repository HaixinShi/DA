#!/usr/bin/env python3

import argparse
import os, atexit
import textwrap

import signal
import random
import time
from enum import Enum

from collections import defaultdict, OrderedDict

def check_positive(value):
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("{} is an invalid positive int value".format(value))
    return ivalue

def checkProcess(filePath):
    i = 1
    filename = os.path.basename(filePath)
    nextMessage = defaultdict(lambda : 1)
    with open(filePath) as f:
        for lineNumber, line in enumerate(f):
            tokens = line.split()

            # Check broadcast
            if tokens[0] == 'b':
                msg = int(tokens[1])
                if msg != i:
                    print("File {}, Line {}: Messages broadcast out of order. Expected message {} but broadcast message {}".format(filename, lineNumber, i, msg))
                    return False
                i += 1

            # Check delivery
            if tokens[0] == 'd':
                sender = int(tokens[1])
                msg = int(tokens[2])
                if msg != nextMessage[sender]:
                    print("File {}, Line {}: Message delivered out of order. Expected message {}, but delivered message {}".format(filename, lineNumber, nextMessage[sender], msg))
                    return False
                else:
                    nextMessage[sender] = msg + 1
    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--proc_num",
        required=True,
        type=check_positive,
        dest="proc_num",
        help="Total number of processes",
    )
    results = parser.parse_args()

    for i in range(1, results.proc_num+1):
        print("Checking {}".format(i))
        if i < 10:
            path = "./proc0" + str(i) + ".output";
        else:
            path = "./proc" + str(i) + ".output";
        if checkProcess(path):
            print("Correct!")
        else:
            print("Fail!")


