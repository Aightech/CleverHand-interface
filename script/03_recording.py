#!/usr/bin/env python
import pyclvhd
import time
import sys

duration = 5  # duration of the recording in seconds
if len(sys.argv) > 1:
    try:
        duration = int(sys.argv[1])
    except:
        print("Invalid duration")
        print("Usage: python 03_recording.py [duration]")    
print(f"Recording duration: {duration}s")

verbosity = 3  # amount of information to print in the console (0: no information)
path_dev = "/dev/ttyACM0"  # path to the serial port
clvhd = pyclvhd.Device(verbosity)  # create a cleverHand device
clvhd.open_connection(path=path_dev)  # open a connection to the device
nb = (
    clvhd.setup()
)  # setup the device (the device will count the number of modules connected and return it)

# ADS1293 configuration
# Each module has 3 ADC which can internally be routed to 2 of the 6 channels available on the board
route_table = [[1, 2], [3, 4], [5, 6]]  # Pair of channels to route the ADCs to
chx_enable = [True, False, True]  # Select which ADCs to enable
chx_high_res = [True, True, True]  # Select the resolution of the ADCs
chx_high_freq = [True, True, True]  # Select low (102.8KHz) or high (204.8KHz) frequency
# The Output Data Rate (ODR) of the ADCs can be calculated as follows:
# ODR_1 = fs / (R1*R2*R3) where fs is the sampling frequency of the ADCs (102.8KHz or 204.8KHz)
# ODR_2 = fs / (R1*R2) where fs is the sampling frequency of the ADCs (102.8KHz or 204.8KHz)
# Where ODR_1 is the ODR of the slower but more accurate readings and ODR_2 is the ODR of the faster but less accurate readings
R1 = [2, 2, 2]
R2 = 4
R3 = [4, 4, 4]

# Configure all the ADS1293 modules
clvhd.setupADS1293(route_table, chx_enable, chx_high_res, chx_high_freq, R1, R2, R3)

# Start the acquisition
clvhd.start_acquisition()

# open a file "data.csv" in write mode
f = open("data.txt", "w")

# run for 10s
t0 = time.time()
printed_ts = 0
while time.time() - t0 < duration:
    ts, fast_list, precise_list = clvhd.read_all()  #read all the data from the device
    s = str(ts) + ", " 
    for i in range(nb):
        for j in range(3):
            s += str(fast_list[i][j]) + ", "
    for i in range(nb):
        for j in range(3):
            s += str(precise_list[i][j]) + ", "
    s = s[:-2] + "\n"
    f.write(s)  #write the data to the file
    if time.time() - printed_ts > 1:
        print(int(time.time()-t0)+1, "/", duration, "s")
        printed_ts = time.time()

f.close()  #close the file
