import pyqtgraph as pg
from qtpy import QtWidgets, QtCore
import numpy as np
import pyclvhd
import sys
import time
from collections import deque

# Parameters
buffer_size = 5000  # （500Hz * 10 sec ）

verbosity = 3  # Amount of information to print in the console (0: no information)
path_dev = "/dev/ttyACM0"  # Path to the serial port
clvhd = pyclvhd.Device(verbosity)  # Create a cleverHand device
clvhd.open_connection(path=path_dev)  # Open a connection to the device
nb = clvhd.setup()  # Setup the device (returns the number of modules connected)

# ADS1293 configuration
route_table = [[1, 2], [3, 4], [5, 6]]  # Pair of channels to route the ADCs to
chx_enable = [True, True, True]
chx_high_res = [True, True, True]
chx_high_freq = [True, True, True]
R1 = [2, 2, 2]
R2 = 4
R3 = [4, 4, 4]
clvhd.setupADS1293(route_table, chx_enable, chx_high_res, chx_high_freq, R1, R2, R3)

# Start the acquisition
clvhd.start_acquisition()

# Initialize a deque for storing data with a maximum length of BUFFER_SIZE
data_buffer = [deque(maxlen=buffer_size) for i in range(3 * nb)]
ts_buffer = deque(maxlen=buffer_size)

class RealTimePlot(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        # Create a pyqtgraph plot window
        self.graphWidget = pg.GraphicsLayoutWidget()
        self.setCentralWidget(self.graphWidget)

        self.plots = []
        self.curves = []
        self.time_window = 10  # Set time window to 10 seconds

        # Initialize the plots for each module and channel
        for i in range(nb):
            plot = self.graphWidget.addPlot(row=i, col=0, title=f'Module {i+1}')
            plot.setYRange(-1, 1)
            plot.setXRange(0, self.time_window)  # Set the X range to 10 seconds
            plot.setLabel('left', 'mV')  # Add "mV" label on the left side
            self.plots.append(plot)
            colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]  # Set colors for 3 channels
            for j in range(3):
                curve = plot.plot(pen=pg.mkPen(colors[j], width=2), name=f'Channel {j+1}')
                self.curves.append(curve)

        # Timer to update the plot
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(2)  # Update every 2 ms, equivalent to 500 Hz

    def update_plot(self):
        # Read data from the device
        ts, fast, precise = clvhd.read_all()
        ts_buffer.append(ts / 1e6)  # Convert timestamp to seconds
        for i in range(nb):
            for j in range(3):
                data_buffer[3 * i + j].append(precise[i][j]*1e3)

        # Update the curves
        ts_buffer_copy = np.array(ts_buffer)
        data_buffer_copy = np.array(data_buffer)

        if len(ts_buffer_copy) > 0:
            latest_time = ts_buffer_copy[-1]
            start_time = max(0, latest_time - self.time_window)  # Keep the time window at 10 seconds
            for i in range(nb):
                for j in range(3):
                    if len(data_buffer_copy[3 * i + j]) > 0:
                        self.curves[3 * i + j].setData(ts_buffer_copy, data_buffer_copy[3 * i + j] + 0.1 * j)
                        self.plots[i].setXRange(start_time, latest_time)  # Adjust the X range to keep data within the window

def main():
    app = QtWidgets.QApplication(sys.argv)
    window = RealTimePlot()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
