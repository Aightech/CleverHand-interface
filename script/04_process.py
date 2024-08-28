import sys
import numpy as np

# filtering
import scipy.signal as signal

# resampling
import scipy.interpolate as interpolate

import matplotlib.pyplot as plt


def usage():
    print("Usage: python 04_process.py [file_path]")
    print(
        "file_path: path to the file containing the data to process (default: data.txt)"
    )
    exit(1)


file_path = "data.txt"
if len(sys.argv) > 1:
    file_path = sys.argv[1]
    try:
        data = np.loadtxt(file_path, delimiter=",")
        nb = int((data.shape[1] - 1) / 2)
        ts = (data[:, 0] - data[0, 0]) / 1e6
        data = data[:, 1 :]
        print(f"Data loaded from {file_path}")
        print(f"Nb of signal: {nb}")
        print(f"Nb of samples: {len(ts)}")
        for i in range(nb*2):
            is_only_nan = np.all(np.isnan(data[:, i]))
            if is_only_nan:
                # replace all nan by 0
                data[:, i] = 0
            else:
                # replace nan by previous value if previous value
                for ts_i in range(len(ts)):
                    if np.isnan(data[ts_i, i]):
                        if ts_i == 0:
                            data[ts_i, i] = 0
                        else:
                            data[ts_i, i] = data[ts_i - 1, i]
        
        fast_data = data[:, :nb]
        precise_data = data[:, nb:]
    except Exception as e:
        print(f"Error: {e}")
        usage()
else:
    usage()

fs = 500  # Sampling rate in Hz
new_ts = np.arange(ts[0], ts[-1], 1 / fs) 
new_fast_data = np.zeros((len(new_ts), nb))
new_precise_data = np.zeros((len(new_ts), nb))

fig, ax = plt.subplots()
for i in range(nb):
    # resampling
    fast_interp = interpolate.interp1d(ts, fast_data[:, i], kind="linear")
    precise_interp = interpolate.interp1d(ts, precise_data[:, i], kind="linear")
    new_fast_data[:, i] = fast_interp(new_ts)
    new_precise_data[:, i] = precise_interp(new_ts)

    # filtering
    # bandpass filter between 0.5Hz and 200Hz
    b, a = signal.butter(4, [0.5, 200], btype="bandpass", fs=fs)
    new_fast_data[:, i] = signal.filtfilt(b, a, new_fast_data[:, i])
    new_precise_data[:, i] = signal.filtfilt(b, a, new_precise_data[:, i])

    # notch filter at 50Hz
    b, a = signal.iirnotch(50, Q=10, fs=fs)
    new_fast_data[:, i] = signal.filtfilt(b, a, new_fast_data[:, i])
    new_precise_data[:, i] = signal.filtfilt(b, a, new_precise_data[:, i])

    # rectify
    new_fast_data[:, i] = np.abs(new_fast_data[:, i])
    new_precise_data[:, i] = np.abs(new_precise_data[:, i])

    # low pass filter at 10Hz
    b, a = signal.butter(4, 10, btype="low", fs=fs)
    new_fast_data[:, i] = signal.filtfilt(b, a, new_fast_data[:, i])
    new_precise_data[:, i] = signal.filtfilt(b, a, new_precise_data[:, i])

    # normalize
    new_fast_data[:, i] = (new_fast_data[:, i] - np.mean(new_fast_data[:, i])) / np.std(
        new_fast_data[:, i]
    )
    new_precise_data[:, i] = (
        new_precise_data[:, i] - np.mean(new_precise_data[:, i])
    ) / np.std(new_precise_data[:, i])

    # plot
    ax.plot(new_ts, new_fast_data[:, i], label=f"fast_{i}")
    ax.plot(new_ts, new_precise_data[:, i], label=f"precise_{i}")

ax.set_xlabel("Time (s)")
ax.set_ylabel("Normalized amplitude")
ax.set_title("Processed data")
ax.set_xlim(new_ts[0], new_ts[-1])
ax.legend()
plt.show()
