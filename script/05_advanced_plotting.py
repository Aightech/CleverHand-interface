import pyqtgraph as pg
from qtpy import QtWidgets, QtCore
import numpy as np
import pyclvhd
import sys
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


class ChannelControlWindow(QtWidgets.QWidget):
    channel_visibility_changed = QtCore.Signal(int, bool)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("Channel Control")

        layout = QtWidgets.QVBoxLayout()

        self.checkboxes = []

        for device_idx in range(nb):  # Loop over each device
            # Add a horizontal layout for device label and RGB button
            device_layout = QtWidgets.QHBoxLayout()

            # Add a label for each device
            device_label = QtWidgets.QLabel(f"Device {device_idx + 1}")
            device_layout.addWidget(device_label)

            # Add RGB button next to the device label
            button = QtWidgets.QPushButton(f"Set RGB")
            button.clicked.connect(self.create_button_clicked_handler(device_idx))  # Bind to the specific device
            device_layout.addWidget(button)

            # Add device layout (label and button) to the main layout
            layout.addLayout(device_layout)

            # Add checkboxes for each channel of the device (horizontally)
            hbox = QtWidgets.QHBoxLayout()
            for ch_idx in range(3):  # Each device has 3 channels
                channel_index = device_idx * 3 + ch_idx
                checkbox = QtWidgets.QCheckBox(f"Ch {channel_index + 1}")
                checkbox.setChecked(True)  # All channels visible by default
                checkbox.stateChanged.connect(self.checkbox_state_changed)
                hbox.addWidget(checkbox)
                self.checkboxes.append(checkbox)

            layout.addLayout(hbox)

            # Add a horizontal line to separate devices
            line = QtWidgets.QFrame()
            line.setFrameShape(QtWidgets.QFrame.HLine)
            line.setFrameShadow(QtWidgets.QFrame.Sunken)
            layout.addWidget(line)

        self.setLayout(layout)

    def checkbox_state_changed(self, state):
        
        sender = self.sender()
        channel_index = self.checkboxes.index(sender)
        is_checked = sender.isChecked()
        # Emit signal with the channel index and its visibility state
        self.channel_visibility_changed.emit(channel_index, is_checked)

    def create_button_clicked_handler(self, device_idx):
        def handler():
            # When the button is clicked, call clvhd.setRGB for the device
            print(f"Set RGB for Device {device_idx + 1}")
            clvhd.setRGB(device_idx, 0, [0, 0, 0])
        return handler

    def closeEvent(self, event):
        # When any window is closed, the entire application will exit
        QtWidgets.QApplication.instance().quit()
        event.accept()


class ScaleWindow(QtWidgets.QWidget):
    scale_changed = QtCore.Signal(float)
    offset_changed = QtCore.Signal(float)

    def __init__(self, main_window):
        super().__init__()
        self.main_window = main_window  # Reference to the main window
        self.setWindowTitle("Scale & Offset Adjustment")

        layout = QtWidgets.QVBoxLayout()

        # Y-scale selection
        self.scale_label = QtWidgets.QLabel("Y-axis scale:")
        self.scale_combo = QtWidgets.QComboBox()
        self.scale_combo.addItems(["0.1", "0.5", "1", "2"])  # Add scale options
        self.scale_combo.currentIndexChanged.connect(self.scale_changed_handler)
        layout.addWidget(self.scale_label)
        layout.addWidget(self.scale_combo)

        # Y-offset selection
        self.offset_label = QtWidgets.QLabel("Y-axis offset:")
        self.offset_combo = QtWidgets.QComboBox()
        self.offset_combo.addItems(["-0.5", "0", "0.5"])  # Add offset options
        self.offset_combo.currentIndexChanged.connect(self.offset_changed_handler)
        layout.addWidget(self.offset_label)
        layout.addWidget(self.offset_combo)

        self.setLayout(layout)

    def scale_changed_handler(self, index):
        # Get the selected scale value and emit it as a float
        scale_value = float(self.scale_combo.currentText())
        self.scale_changed.emit(scale_value)

    def offset_changed_handler(self, index):
        # Get the selected offset value and emit it as a float
        offset_value = float(self.offset_combo.currentText())
        self.offset_changed.emit(offset_value)

    def closeEvent(self, event):
        # When any window is closed, the entire application will exit
        QtWidgets.QApplication.instance().quit()
        event.accept()


class RealTimePlot(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        # Create a pyqtgraph plot window
        self.graphWidget = pg.GraphicsLayoutWidget()
        self.setCentralWidget(self.graphWidget)

        self.plots = []
        self.curves = []
        self.curve_visibility = [True] * (3 * nb)  # Store visibility of each curve
        self.time_window = 10  # Set time window to 10 seconds
        self.y_range = 1  # Initial Y-axis range
        self.y_offset = 0  # Initial Y-axis offset

        # Initialize the plots for each module and channel
        for i in range(nb):
            plot = self.graphWidget.addPlot(row=i, col=0, title=f'Module {i+1}')
            plot.setYRange(-self.y_range + self.y_offset, self.y_range + self.y_offset)  # Set initial Y range with offset
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
                    curve_index = 3 * i + j
                    if len(data_buffer_copy[curve_index]) > 0 and self.curve_visibility[curve_index]:
                        self.curves[curve_index].setData(ts_buffer_copy, data_buffer_copy[curve_index] + 0.1 * j)
                        self.plots[i].setXRange(start_time, latest_time)  # Adjust the X range to keep data within the window

    def update_y_range(self, scale_value):
        self.y_range = scale_value
        for plot in self.plots:
            plot.setYRange(-self.y_range + self.y_offset, self.y_range + self.y_offset)  # Adjust Y range based on scale_value

    def update_y_offset(self, offset_value):
        self.y_offset = offset_value
        for plot in self.plots:
            plot.setYRange(-self.y_range + self.y_offset, self.y_range + self.y_offset)  # Adjust Y range with offset

    def set_channel_visibility(self, channel_index, is_visible):
        self.curve_visibility[channel_index] = is_visible
        self.curves[channel_index].setVisible(is_visible)

    def closeEvent(self, event):
        # When any window is closed, the entire application will exit
        QtWidgets.QApplication.instance().quit()
        event.accept()


def main():
    app = QtWidgets.QApplication(sys.argv)

    # Create main plot window
    window = RealTimePlot()

    # Create scale and offset adjustment window
    scale_window = ScaleWindow(main_window=window)
    scale_window.scale_changed.connect(window.update_y_range)
    scale_window.offset_changed.connect(window.update_y_offset)

    # Create channel control window
    channel_control_window = ChannelControlWindow()
    channel_control_window.channel_visibility_changed.connect(window.set_channel_visibility)

    # Show main window
    window.show()

    # Set positions and sizes of the side windows (scale and channel control)
    window_x, window_y, window_width, window_height = window.geometry().getRect()

    # Adjust scale window (left side)
    # scale_window.setGeometry(window_x - 200, window_y, 200, window_height)
    scale_window.show()
    scale_window.move(window_x - scale_window.frameGeometry().width(), window_y)

    # Adjust channel control window (right side)
    # channel_control_window.setGeometry(window_x + window_width, window_y, 200, window_height)
    channel_control_window.show()
    channel_control_window.move(window_x + window_width, window_y)

    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
