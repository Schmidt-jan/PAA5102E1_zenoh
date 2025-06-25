# Zenoh

## Adding a New Device to Zenoh

## Use Serial Interface for Communication

1. Add the following to `board_build.cmake_extra_args`:
   -DZ_FEATURE_LINK_SERIAL=1

2. In the `lib` folder, create a directory named `<your_platform>` and add the following files:

   - `network.cpp`  
     Implementation of the network layer (WiFi, Serial).  
     Use an existing implementation, e.g. [ESP32](), and adapt it to work with your target platform.

   - `system.c`  
     Copy the configuration from an already implemented platform, e.g. [ESP32](), and fix any broken functions.

   - `zenoh_generic_config.h`  
     You can set all Zenoh flags here. The default configuration is available [here]().

   - `zenoh_generic_platform.h`  
     Copy the configuration from an already implemented platform, e.g. [ESP32](), and fix any broken functions.

3. Create an additional script that sets the `ZENOH_GENERIC` environment variable.  
   `set_library_vars.py`:
    ``` python
    Import("env")
    env.Append(ZENOH_GENERIC="1")
    ```
    
4. Add the following line to your `platform.ini`:
    extra_scripts = pre:set_library_vars.py