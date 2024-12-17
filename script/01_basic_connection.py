#!/usr/bin/env python
import pyclvhd
import time

verbosity = 3  # amount of information to print in the console (0: no information)
path_dev = "/dev/ttyACM0"  # path to the serial port
clvhd = pyclvhd.Device(verbosity)  # create a cleverHand device
clvhd.open_connection(path=path_dev)  # open a connection to the device
nb = (
    clvhd.setup()
)  # setup the device (the device will count the number of modules connected and return it)

# ADS1293 configuration
# Each module has 3 ADC which can internally be routed to 2 of the 6 channels available on the board
route_table = [[3, 4], [3, 4], [5, 6]]  # Pair of channels to route the ADCs to
chx_enable = [True, False, False]  # Select which ADCs to enable
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

# run for 10s
t0 = time.time()
while time.time() - t0 < 4:
    ts, fast_list, precise_list = clvhd.read_all()  #
    print("timestamp (us): ", ts)
    # print(
    #     "fast readings: ", fast_list
    # )  # list of list (nb_modules x 3) of the fast readings (nan for the readings that are not enabled or not ready)
    
    print("v: [", end="")
    for i in range(len(fast_list)):
        print(fast_list[i][0], end=", ")
    print("]")

    # print(
    #     "precise readings: ", precise_list
    # )  # list of list (nb_modules x 3) of the precise readings (nan for the readings that are not enabled or not ready)
    time.sleep(0.01)
