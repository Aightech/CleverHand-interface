# CleverHand-library
This is library is used to communicate with the CleverHand device. It is written in C++ and integrates a Python wrapper. The library is meant to be modular and easy to use. It enables the user to communicate with the CleverHand device and its different type of modules (read sensors, write actuators, configure parameters, etc.).

> [!WARNING]
> This library is still in development. It is not fully tested and some features may not be available yet. Please raise an issue if you encounter any problem. 


> [!NOTE] 
> The library is originally written for Ubuntu but should work on other OS as well. 


## Architecture
```mermaid
classDiagram
direction LR
	namespace CLvHd {
        class Device {
            initSerial(portpath, baudrate)
            initWifi(ip, port)
        }

        class Controller {
        }

        class MonoController {
        }

        class SerialController {
        }

        class ADS1293 {
        }

        class ADS1293Config {
            gain, route, etc
        }

        class ADS1293Pack {
            configure(config)
            start_acquisition()
            config
        }

        class ADS1298 {
        }


        class ModulePack {
            setup()
        }

        class ADS1298Pack {
        }

        class Module {
            Value sensor
            Value actuator
        }

        class Value {
            Type //sensor, actuator
            uint64 time_ns
            uint64 time_s
            Vector<double> data

        }

	}
	namespace Communication {
        class TCPserver {
        }

        class UDPserver {
        }

        class Serial {
        }

	}

    ADS1293 --|> Module : Inheritance
    ADS1298 --|> Module : Inheritance
    ADS1298Pack --|> ModulePack : Inheritance
    ADS1293Pack --|> ModulePack : Inheritance
    Module ..> Controller : pointer
    Device ..> Controller : pointer
    Device --> Module : vector of
    ADS1293Pack --> ADS1293 : vector of
    ADS1298Pack --> ADS1298 : vector of
    Controller <|-- MonoController : Inheritance
    Controller <|-- SerialController : Inheritance
    MonoController -- TCPserver
    MonoController -- UDPserver
    SerialController -- Serial
    ModulePack ..> Device : pointer
    Module --> Value : owns
    ADS1293Pack --> ADS1293Config : uses
```


### Device

The `Device` class is the main interface with the cleverhand device; it contains a `Controller` object and a a vector of `Module` objects.
To initialize the `Device` class, you need to call the method `initSerial(portpath)` or `initWifi(ip, port)` to open the serial or TCP connection with the adequate controller. During this initialization, the `Device` class will read the version of the controller and the number of modules attached to it. The `Device` class will then create a vector of `Module` objects, one for each module attached to the controller. 

> [!TIP]
> ```cpp
> ClvHd::Device device();
> device.initSerial("/dev/ttyACM0");
> ```

### Module Pack

The `ModulePack` class is used to group and communicate with multiple modules of the same type attached to the controller. Each type of module has its own class that inherits from the `Module` class. For example, the `EMG_ADS1293Pack`  represents a pack of `EMG_ADS1293` modules. When the device is initialized, you can create a `ModulePack` object for each type of module attached to the controller and call the `setup()` method of each pack to detect the modules of this type attached to the controller. You can then use these pack to configure and read data from the modules.

> [!TIP]
> ```cpp
> ClvHd::Device device;
> device.initSerial("/dev/ttyACM0");
> ClvHd::EMG_ADS1293Pack emg_pack(&device);
> emg_pack.setup();// Detect ADS1293 modules attached to the controller
> ClvHd::EMG_ADS1293Config config;// Create a configuration object for the ADS1293 modules
> emg_pack.configure(config);// Configure the ADS1293 modules with the given configuration
> emg_pack.start_acquisition();// Start the acquisition of the ADS1293 modules
> while(true)
> {
>     vector<ClvHd::Value *> values = emg_pack.read_all();
>     double timestamp = values[0]->time_s + values[0]->time_ns / 1000000.0;// Convert to seconds
>     values[0]->data[0]; // Access the first value of the first
> }
> ```





## Building the library
### Requirements
- CMake
- Make
- build-essential
- (optional: [liblsl](https://github.com/sccn/liblsl))

### Building
```bash
mkdir build
cd build
cmake .. -DBUILD_EXAMPLES=1 -DBUILD_PYTHON=1
make
```

## Usage

### C++
There are several examples in the `src` folder. The examples start with `main_` and showcase different features of the library.

### Python
The library integrates a Python wrapper. To use the library in Python, you need to build the library with the `-DBUILD_PYTHON=1` option. This will create a `clvhd.so` file in the `build` folder. You can then use this file in your Python code.

> [!WARNING]
> The Python wrapper is not fully implemented yet. The library is still in development and some features may not be available in the Python wrapper.




