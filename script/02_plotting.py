#!/usr/bin/env python
import pyclvhd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from collections import deque
import random  # For simulating incoming data
import time

# Parameters
BUFFER_SIZE = 1000  # Maximum number of samples to store
PLOT_UPDATE_INTERVAL = 100  # Plot update interval in milliseconds

# Initialize a deque for storing data with a maximum length of BUFFER_SIZE
data_buffer = deque(maxlen=BUFFER_SIZE)

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



# Function to handle incoming data and store it in the buffer
def store_data():
    global clvhd
    while True:
        sample = clvhd.read_all()
        data_buffer.extend(sample)
        #time.sleep(0.01)  # Adjust this sleep time based on your data rate

# Function to plot the data
def update_plot(frame):
    plt.cla()
    plt.plot(data_buffer)
    plt.ylim(-1000, 1000)  # Adjust Y-axis limits as needed

# Set up the plot
fig, ax = plt.subplots()
ani = animation.FuncAnimation(fig, update_plot, interval=PLOT_UPDATE_INTERVAL)

# Start the data storing process in a separate thread
import threading
data_thread = threading.Thread(target=store_data, daemon=True)
data_thread.start()

# Display the plot
plt.show()

