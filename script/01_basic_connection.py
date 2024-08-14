#!/usr/bin/env python
import pyclvhd
import time

clvhd = pyclvhd.Device(3)
clvhd.open_connection(path="/dev/ttyACM0")
clvhd.setup()
route_table = [[1,2], [3,4], [5,6]]
chx_enable = [True, True, True]
chx_high_res = [True, True, True]
chx_high_freq = [True, True, True]
R1 = [2, 4, 4]
R2 =  4 
R3 =  [4, 4, 4]

clvhd.setupADS1293(route_table, chx_enable, chx_high_res,
                chx_high_freq, R1, R2, R3)

clvhd.start_acquisition()

#run for 10s
t0 = time.time()
while time.time() - t0 < 10:
    l = clvhd.read_all()
    print(l)


