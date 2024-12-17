import pyqtgraph as pg
from qtpy import QtWidgets, QtCore
import numpy as np
import pyclvhd
import sys
from collections import deque
from scipy.signal import butter, lfilter

# Parameters
buffer_size = 5000  # (500Hz * 10 sec)
verbosity = 3  # Amount of information to print in the console (0: no information)
path_dev = "/dev/ttyACM0"  # Path to the serial port
clvhd = pyclvhd.Device(verbosity)  # Create a cleverHand device
clvhd.open_connection(path=path_dev)  # Open a connection to the device
nb = clvhd.setup()  # Setup the device (returns the number of modules connected)

# ADS1293 configuration
route_table = [[3, 4], [2, 5], [5, 6]]  # Pair of channels to route the ADCs to
chx_enable = [True, True, False]
chx_high_res = [True, True, True]
chx_high_freq = [True, True, True]
R1 = [2, 2, 2]
R2 = 4
R3 = [4, 4, 4]
clvhd.setupADS1293(route_table, chx_enable, chx_high_res, chx_high_freq, R1, R2, R3)

# Start the acquisition
clvhd.start_acquisition()

# Initialize a deque for storing data with a maximum length of BUFFER_SIZE
data_buffer = [deque(maxlen=buffer_size) for _ in range(3 * nb)]
ts_buffer = deque(maxlen=buffer_size)

# Band-pass filter configuration
def butter_bandpass(lowcut, highcut, fs, order=4):
    nyquist = 0.5 * fs
    low = lowcut / nyquist
    high = highcut / nyquist
    b, a = butter(order, [low, high], btype='band')
    return b, a

def apply_bandpass_filter(data, lowcut, highcut, fs, order=4):
    b, a = butter_bandpass(lowcut, highcut, fs, order=order)
    return lfilter(b, a, data)

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
        self.channel_visibility_changed.emit(channel_index, is_checked)

    def create_button_clicked_handler(self, device_idx):
        def handler():
            print(f"Set RGB for Device {device_idx + 1}")
            clvhd.setRGB(device_idx, 0, [0, 0, 0])
        return handler

    def closeEvent(self, event):
        QtWidgets.QApplication.instance().quit()
        event.accept()

class ScaleWindow(QtWidgets.QWidget):
    scale_changed = QtCore.Signal(float)
    offset_changed = QtCore.Signal(float)

    def __init__(self):
        super().__init__()
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
        scale_value = float(self.scale_combo.currentText())
        self.scale_changed.emit(scale_value)

    def offset_changed_handler(self, index):
        offset_value = float(self.offset_combo.currentText())
        self.offset_changed.emit(offset_value)

    def closeEvent(self, event):
        QtWidgets.QApplication.instance().quit()
        event.accept()

class FilterWindow(QtWidgets.QWidget):
    filter_parameters_changed = QtCore.Signal(float, float)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("Filter Settings")

        layout = QtWidgets.QVBoxLayout()

        self.lowcut_label = QtWidgets.QLabel("Low Cutoff Frequency (Hz):")
        self.lowcut_spinbox = QtWidgets.QDoubleSpinBox()
        self.lowcut_spinbox.setRange(0.1, 250.0)
        self.lowcut_spinbox.setValue(1.0)
        layout.addWidget(self.lowcut_label)
        layout.addWidget(self.lowcut_spinbox)

        self.highcut_label = QtWidgets.QLabel("High Cutoff Frequency (Hz):")
        self.highcut_spinbox = QtWidgets.QDoubleSpinBox()
        self.highcut_spinbox.setRange(0.1, 250.0)
        self.highcut_spinbox.setValue(50.0)
        layout.addWidget(self.highcut_label)
        layout.addWidget(self.highcut_spinbox)

        self.apply_button = QtWidgets.QPushButton("Apply Filter")
        self.apply_button.clicked.connect(self.apply_filter_settings)
        layout.addWidget(self.apply_button)

        self.setLayout(layout)

    def apply_filter_settings(self):
        lowcut = self.lowcut_spinbox.value()
        highcut = self.highcut_spinbox.value()
        self.filter_parameters_changed.emit(lowcut, highcut)

    def closeEvent(self, event):
        QtWidgets.QApplication.instance().quit()
        event.accept()

class RealTimePlot(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        self.graphWidget = pg.GraphicsLayoutWidget()
        self.setCentralWidget(self.graphWidget)

        self.plots = []
        self.curves = []
        self.curve_visibility = [True] * (3 * nb)
        self.time_window = 10
        self.y_range = 1
        self.y_offset = 0
        self.lowcut = 1.0
        self.highcut = 50.0
        self.filter_enabled = False

        for i in range(nb):
            plot = self.graphWidget.addPlot(row=i, col=0, title=f'Module {i+1}')
            plot.setYRange(-self.y_range + self.y_offset, self.y_range + self.y_offset)
            plot.setXRange(0, self.time_window)
            plot.setLabel('left', 'mV')
            self.plots.append(plot)
            colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
            for j in range(3):
                curve = plot.plot(pen=pg.mkPen(colors[j], width=2), name=f'Channel {j+1}')
                self.curves.append(curve)

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(2)

    def update_plot(self):
        ts, fast, precise = clvhd.read_all()
        ts_buffer.append(ts / 1e6)
        for i in range(nb):
            for j in range(3):
                data = precise[i][j] * 1e3
                if self.filter_enabled:
                    data = apply_bandpass_filter([data], self.lowcut, self.highcut, 500)[0]
                data_buffer[3 * i + j].append(data)

        ts_buffer_copy = np.array(ts_buffer)
        data_buffer_copy = np.array(data_buffer)

        if len(ts_buffer_copy) > 0:
            latest_time = ts_buffer_copy[-1]
            start_time = max(0, latest_time - self.time_window)
            for i in range(nb):
                for j in range(3):
                    curve_index = 3 * i + j
                    if len(data_buffer_copy[curve_index]) > 0 and self.curve_visibility[curve_index]:
                        self.curves[curve_index].setData(ts_buffer_copy, data_buffer_copy[curve_index] + 0.1 * j)
                        self.plots[i].setXRange(start_time, latest_time)

    def update_y_range(self, scale_value):
        self.y_range = scale_value
        for plot in self.plots:
            plot.setYRange(-self.y_range + self.y_offset, self.y_range + self.y_offset)

    def update_y_offset(self, offset_value):
        self.y_offset = offset_value
        for plot in self.plots:
            plot.setYRange(-self.y_range + self.y_offset, self.y_range + self.y_offset)

    def set_channel_visibility(self, channel_index, is_visible):
        self.curve_visibility[channel_index] = is_visible
        self.curves[channel_index].setVisible(is_visible)

    def apply_filter(self, lowcut, highcut):
        self.lowcut = lowcut
        self.highcut = highcut
        self.filter_enabled = True

    def closeEvent(self, event):
        QtWidgets.QApplication.instance().quit()
        event.accept()

def main():
    app = QtWidgets.QApplication(sys.argv)

    window = RealTimePlot()

    scale_window = ScaleWindow()
    scale_window.scale_changed.connect(window.update_y_range)
    scale_window.offset_changed.connect(window.update_y_offset)

    filter_window = FilterWindow()
    filter_window.filter_parameters_changed.connect(window.apply_filter)

    channel_control_window = ChannelControlWindow()
    channel_control_window.channel_visibility_changed.connect(window.set_channel_visibility)

    window.show()

    window_x, window_y, window_width, window_height = window.geometry().getRect()

    scale_window.show()
    scale_window.move(window_x - scale_window.frameGeometry().width(), window_y)

    filter_window.show()
    filter_window.move(window_x, window_y + window_height)

    channel_control_window.show()
    channel_control_window.move(window_x + window_width, window_y)

    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
