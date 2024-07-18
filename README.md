# CleverHand-library
This is the CleverHand library, a library for the CleverHand interface.


## Serial protocol
Each request from the computer to the controller is a 8 bytes long frame. The first byte is always the command, the rest of the bytes depend on the command. 

### Read 'r' request
Request to read a register from a module attached to the controller. 
1. 'r': read commad
2. id : A mask indicating which module to address. **Note**: 0b0110 address module 1 and 2
3. reg: register to read
4. n  : number of bytes to read starting from reg

**Note**: The number of bytes replyed by the controller `len` is equal to `n`*N with N the number of modules addressed by the id mask.

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x==|==xxxxxxx',data:['r','id','reg','n']},{name:'Rx',wave:'xxxxxx=|==.|x',data:['ts','len','val']},{node:'..E.F.A.BC..D'}],head:{text:'Read command'},edge:['A+B 8bytes','C+D len bytes','E+F 4 bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x==|==xxxxxxx', data: ['r', 'id', 'reg', 'n']},
  { name: 'Rx', wave: 'xxxxxx=|==.|x', data: ['ts', 'len', 'val']},
  {                              node: '..E.F.A.BC..D'}
],
    head: { text: 'Read command' },
    edge: [ 'A+B 8bytes', 'C+D len bytes' , 'E+F 4 bytes']
}
```

### Write 'w' request
Request to write a register from a module attached to the controller. There is no response to this command.
1. 'w': write command
2. id : A mask indicating which module to address. **Note**: 0b0110 address module 1 and 2
3. reg: register to write
4. n: number of bytes to write starting from reg
5. val: value to write


<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x==|===|x',data:['w','id','reg','n','val']},{node:'..A.B.C.D'}],head:{text:'Write command'},edge:['A+B 4 bytes','C+D n bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x==|===|x', data: ['w', 'id', 'reg', 'n', 'val'] },
  {                              node: '..A.B.C.D'}
],
    head: { text: 'Write command' },
    edge: [ 'A+B 4 bytes', 'C+D n bytes' ]
}
```

### Setup 's' request
Request to setup the controller. Expected response is the number of modules attached to the controller.
1. 's': setup command

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x=xxxxx',data:['s']},{name:'Rx',wave:'xx=|==x',data:['ts','1','nb']},{node:'..A.B'}],head:{text:'Setup command'},edge:['A+B 8bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x=xxxxx', data: ['s'] },
  { name: 'Rx', wave: 'xx=|==x', data: ['ts', '1', 'nb']},
  {                              node: '..A.B'}
],
    head: { text: 'Setup command' },
    edge: [ 'A+B 8bytes' ]
}
```

### Number of modules 'n' request
Request to know the number of modules attached to the controller.
1. 'n': number of modules command

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x=xxxxx',data:['n']},{name:'Rx',wave:'xx=|==x',data:['ts','1','nb']},{node:'..A.B'}],head:{text:'Number of modules command'},edge:['A+B 8bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x=xxxxx', data: ['n'] },
  { name: 'Rx', wave: 'xx=|==x', data: ['ts', '1', 'nb']},
  {                              node: '..A.B'}
],
    head: { text: 'Number of modules command' },
    edge: [ 'A+B 8bytes' ]
}
```

### Mirror 'm' request
Request to test the communication with the controller. The expected response is the same frame sent by the computer.
1. 'm': mirror command
2. v1 : value 1
3. v2 : value 2
4. v3 : value 3

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x====xxxxxxx',data:['m','v1','v2','v3']},{name:'Rx',wave:'xxxxx=|====x',data:['ts','3','v1','v2','v3']},{node:'.....A.B.C.D'}],head:{text:'Mirror command'},edge:['A+B 8bytes']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x====xxxxxxx', data: ['m', 'v1', 'v2', 'v3'] },
  { name: 'Rx', wave: 'xxxxx=|====x', data: ['ts', '3', 'v1', 'v2', 'v3']},
  {                              node: '.....A.B.C.D'}
],
    head: { text: 'Mirror command' },
    edge: [ 'A+B 8bytes' ]
}
```

### Version 'v' request
Request to know the version of the controller.
1. 'v': version command

<img src="https://svg.wavedrom.com/{signal:[{name:'Tx',wave:'x=xxxxxx',data:['v']},{name:'Rx',wave:'xx=|===x',data:['ts','2','V.M','V.m']},{node:'..A.BCDE'}],head:{text:'Version command'},edge:['A+B 8bytes','C+D Major','D+E Minor']}"/>

```wavedrom
{ signal: [
  { name: 'Tx', wave: 'x=xxxxxx', data: ['v'] },
  { name: 'Rx', wave: 'xx=|===x', data: ['ts', '2', 'V.M', 'V.m']},
  {                              node: '..A.BCDE'}
],
    head: { text: 'Version command' },
    edge: [ 'A+B 8bytes', 'C+D Major', 'D+E Minor' ]
}
```

## Cpp library
The library is written in C++ and is composed of a namespace `ClvHd`, a class `Device` , a class `Controller` and a class abstract class `Module`.
The `Device` class is the main class of the library and contains a `Controller` object and a list of `Module` objects. The `Controller` class is used to communicate with the controller and the `Module` class is an abstract class that represents a module attached to the controller.

As there is an infinite possible type of module, the `Module` class is abstract and the user can create a new class that inherits from `Module` to represent a new type of module. The user must implement static methods to enable the user to use access the module from the `Device` class.

```mermaid
classDiagram
    class Device {
        Controller controller
        Module* modules[]
        +Device(int n)
        +~Device()
        +setup()
    }
    class Controller {
        +open_connection()
        +close_connection()
        +read()
        +write()
    }
    class Module {
        +read()
        +write()
    }
    class EMG_ADS1293 {
        +setup()
        +start()
        +getModule()
        +read()
    }
    Device --> Controller
    Device --> Module
    EMG_ADS1293 --> Module
```

### Device

The `Device` class is the main class of the library and contains a `Controller` object and a list of `Module` objects pointer.
Initially, module array is filled with `nullptr`. When the `setup()` static method of a specific type (e.g. `EMG_ADS1293`) is called, test are made to identify which module have this type, then an instance of the module is created and stored in the module array at the index corresponding to the module id.
**Example**: A controller with 3 ADS1293 modules attached and 1 LIS3DH module attached. When the `setup()` method of the `EMG_ADS1293` class is called, the function will read the RevID register of the 4 modules, the 3 first modules will reply with the correct value and the last module will reply with an error. The `Device` class will create 3 instances of the `EMG_ADS1293` class and store them in the module array at the index corresponding to the module id. Then the `setup()` method of the `LIS3DH` class is called, the function will read the WHO_AM_I register of the remaining uninstanciated module and create an instance of the `LIS3DH` class and store it in the module array at the index corresponding to the module id.

### Controller
The `Controller` class is used to communicate with the controller board. It provides basic functionalities to read/write registers of the modules attached to the controller.

### Module

The `Module` class is an abstract class that represents a module attached to the controller.

### EMG_ADS1293

The `EMG_ADS1293` class is used to read EMG data from the CleverHand board.

## Creating a New Module Type

To create a new module type, follow these steps:

1. **Inherit from the `Module` class**:
    ```cpp
    class NewModuleType : public ClvHd::Module {
    public:
        NewModuleType() { /* Constructor implementation */ }
        ~NewModuleType() { /* Destructor implementation */ }
        // Implement specific methods for the new module
        static void setup(ClvHd::Device &device);
        static void start(ClvHd::Device &device);
        static NewModuleType* getModule(ClvHd::Device &device, int id);
        static uint64_t read(ClvHd::Device &device, double *sample);
    };
    ```

2. **Implement the required static methods**:
    - `setup()`: Detect, instanciate and configure modules of this type attached to the controller.
    - `foo()`: any other methods required to interact with the module.

3. **Use the `Controller` class for communication**: Ensure the new module interacts with the `Device` class to use the `Controller` class for communication with the controller board and the modules array.


## Building the library
### Linux
### Requirements
- CMake
- Make
- build-essential

### Building
```bash
mkdir build
cd build
cmake ..
make
```

## Windows
### Requirements
- CMake

### Building     
```bash
mkdir build
cd build
cmake ..
cmake --build .
```



## Usage
Here's an example of how to use the library in a `main.cpp` file:

```cpp
#include <iostream>
#include <vector>
#include <iomanip>
#include <unistd.h>
#include "ClvHd.h"

int main()
{
    std::cout << "CleverHand Serial Interface:" << std::endl;
    try
    {
        ClvHd::Device device(3);

        // Open the serial connection between the computer and the controller board
        device.controller.open_connection("/dev/ttyACM0", 500000, O_RDWR | O_NOCTTY);
        usleep(500000);

        // Setup the device (count and find the type of modules attached)
        device.setup();

        // Setup the EMG modules
        bool chx_enable[3] = {true, false, false}; // Enable channel 1, disable channels 2 and 3
        int route_table[3][2] = {{0, 1}, {0, 1}, {0, 1}}; // Electrodes configuration
        bool chx_high_res[3] = {true, true, true}; // Enable high resolution mode
        bool chx_high_freq[3] = {true, true, true}; // Enable high frequency mode
        int R1[3] = {4, 4, 4}; // Gain R1 of the INA channels
        int R2 = 4; // Gain R2 of the INA channels
        int R3[3] = {4, 4, 4}; // Gain R3 of the INA channels

        // Create and setup the EMG modules in the device
        ClvHd::EMG_ADS1293::setup(device, chx_enable, route_table, chx_high_res, chx_high_freq, R1, R2, R3);

        // Start the EMG modules
        ClvHd::EMG_ADS1293::start_acquisition(device);

        std::vector<double> sample(6);
        for(int t = 0;; t++)
        {
            uint64_t timestamp = ClvHd::EMG_ADS1293::read_all(device, sample.data());
            std::cout << "timestamp: " << timestamp << " ";
            for(int i = 0; i < 6; i++)
                std::cout << std::fixed << std::setprecision(2) << sample[i] << " ";
            std::cout << std::endl;
            usleep(1000);
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
    }
    catch(std::string str)
    {
        std::cerr << "[ERROR] Got an exception: " << str << std::endl;
    }

    return 0; // success
}
```
