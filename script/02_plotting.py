#!/usr/bin/env python
import pyclvhd
import matplotlib.pyplot as plt
#button to show/hide the data
from matplotlib.widgets import Button

import numpy as np
from collections import deque
import asyncio
from concurrent.futures import ThreadPoolExecutor
import time
import scipy.signal as signal
import scipy.interpolate as interpolate

#mutex to protect the data buffer
import threading

mutex = threading.Lock()

# Parameters
buffer_size = 600  # Maximum number of samples to store
fs = 500  # Sampling rate in Hz
fp = 200  # Plot update interval in milliseconds




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

# Initialize a deque for storing data with a maximum length of BUFFER_SIZE
data_buffer = [deque(maxlen=buffer_size) for i in range(3*nb)]
ts_buffer = deque(maxlen=buffer_size)
plot_visible = [True for i in range(3*nb)]
data_recording = True


# Function to handle incoming data and store it in the buffer
async def store_data():
    global clvhd, data_recording, mutex
    last_ts = time.time_ns()/1e3
    while data_recording:
        t_us = time.time_ns()/1e3
        print(f'Elapsed time: {t_us - last_ts}')
        if t_us - last_ts > 1000000./fs:    
            last_ts = t_us
            ts, fast, precise = clvhd.read_all()
            with mutex:
                ts_buffer.append(ts/1e6)# Convert timestamp to seconds
                for i in range(nb):
                    for j in range(3):
                        data_buffer[3*i+j].append(precise[i][j])
        else:
            await asyncio.sleep(0.0001)

# Function to plot the data
def update_plot(nb):
    global data_recording, mutex
    plt.ion()
    # Create a figure and axis
    fig, ax = plt.subplots(1, 1, figsize=(10, 6))
    lines = []
    with mutex:
        ts_buffer_copy = np.array(ts_buffer)
        data_buffer_copy = np.array(data_buffer)
    for i in range(nb):
        for j in range(3):
            line, = ax.plot(ts_buffer_copy, data_buffer_copy[3*i+j]+0.1*j, label=f'Module {i+1} Channel {j+1}')
            lines.append(line)
    ax.set_title(f'Module 1')
    ax.set_ylim(-1, 1)
    ax.legend()

    # Function to toggle the visibility of a plot
    def toggle_plot(i, j):
        if plot_visible[3*i+j]:
            lines[3*i+j].set_visible(False)
            plot_visible[3*i+j] = False
            buttons[3*i+j].color = 'lightcoral'
        else:
            lines[3*i+j].set_visible(True)
            plot_visible[3*i+j] = True
            buttons[3*i+j].color = 'lightgoldenrodyellow'

    #add a hide/show button for each channel
    button_axes = []
    buttons = []
    for i in range(nb):
        for j in range(3):
            #create button_axe for each channel
            button_axes.append(plt.axes([0.92, 0.9-0.055*(3*i+j), 0.06, 0.05]))
            #create button for each channel
            button = Button(button_axes[-1], f'M{i+1} Ch{j+1}', color='lightgoldenrodyellow', hovercolor='0.975')
            buttons.append(button)
            #add the event to the button
            button.on_clicked(lambda event, i=i, j=j: toggle_plot(i, j))



    # Update the plot while the window is open
    while plt.fignum_exists(fig.number):
        if len(ts_buffer) > 0:
            #copy the data and timestamp
            with mutex:
                ts_buffer_copy = np.array(ts_buffer)
                data_buffer_copy = np.array(data_buffer)

            for i in range(nb):
                for j in range(3):
                    lines[3*i+j].set_xdata(ts_buffer_copy)
                    lines[3*i+j].set_ydata(data_buffer_copy[3*i+j]+0.1*j)
            
            ax.relim() # Recalculate limits
            ax.autoscale_view() # Autoscale
            fig.canvas.draw()
            fig.canvas.flush_events()
        time.sleep(1/fp)
    data_recording = False

# Run the asyncio event loop
async def main():
    loop = asyncio.get_event_loop()
    data_task = asyncio.create_task(store_data())

    with ThreadPoolExecutor() as pool:
        plot_task = loop.run_in_executor(pool, update_plot, nb)
        await asyncio.gather(data_task, plot_task)

# Run the main function
if __name__ == "__main__":
    asyncio.run(main())





