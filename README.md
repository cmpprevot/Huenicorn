# Huenicorn

A free ambilight driver for your Philips Hue™ devices


## Description

Huenicorn allows you to stream the dominant colors of your computer monitor to your Philips Hue lightbulbs. It provides an easy user interface to setup and manage the lights you want to use.

### Screenshot

![Screenshot](screenshots/HuenicornFullWebUI.png)
*<center>Huenicorn Light Manager user interface</center>*

## Project status
Huenicorn 1.0.0 is now ready and available.
This initial version powers your lights through the Philips Hue V2 entertainment API, solving the latency issues of the (now) legacy version.
Good old HTTP-based version has been moved to Legacy branch.

Thank you very much for your patience, comments and contributions.


## Getting Started

* Before using Huenicorn, you need to define some entertainment area on your Hue bridge through the official application provided by Philips.

### Requirements

* Gnu/Linux system running X11 graphics session
* Philips Hue bridge with registered lamps

### Dependencies

* X11
* [OpenCV](https://github.com/opencv/opencv)
* [CurlPP](https://github.com/jpbarrette/curlpp)
* [Restbed](https://github.com/Corvusoft/restbed)
* [Mbed-TLS](https://github.com/Mbed-TLS/mbedtls)

### Compiling Huenicorn

Make sure the abovementionned are installed (preferably through your system's package manager)

```
git clone https://gitlab.com/openjowelsofts/huenicorn.git
cd huenicorn
mkdir build && cd build
cmake ..
make
```

The output is a standalone executable called "huenicorn".
When running Huenicorn, make sure that the "webroot" directory is in the cwd (current working directory).

### Executing program

* Run the executable named "huenicorn" in the way you prefer (Terminal if you want some text feedback)
* Open your favorite web browser at http://127.0.0.1:8080
Browser will spawn automatically for the initial setup and as long as no light profile has been saved

#### Initial setup

When executed for the first time, Huenicorn spawns the setup page in your favorite browser. It consists of a step-by-step wizard to locate your Hue bridge on the network and register an user on it.

Upon setup proper setup termination, Huenicorn will resume in its ordinary execution workflow.

The webpage can then be refreshed to reach the Light Manager interface.

#### Ordinary execution worflow

At startup, Huenicorn will look for a saved profile and start its color streaming execution.

If you want to assign lights to some portions of your screen, open the web interface and drag'n'drop the available lights from the left list to the right one.

When a synced light is selected in the list, its portion of allocated screen can be adjusted in the screen partitioning section.

Once you are satisfied with the layout, you can save it as a profile and it will be automatically reloaded for further executions.

As long as no profile has been saved, light management interface will spawn automatically at startup.

#### Configuration files
The configuration files can be found in your home directory at ~/.config/huenicorn/
config.json contains the bridge-related configuration.
profile.json contains the saved channels geometry for an enteraintment configuration.

The data structure of these files is JSON.


#### Shutting down

Huenicorn can be shut down through the web interface or by sending a termination signal.

## Website
Additionnal information and news can be found on [Huenicorn.org](http://huenicorn.org), the official website of the project.


## Version history

* 1.0.0
  * First stable release
* 1.0.0-rc1
  * Implements real-time color streaming through Hue API V2
* 0.0.0 (Legacy)
  * Initial implementation using Hue HTTP API

## Authors

OpenJowel

## License

Huenicorn is licensed under the GNU GPLv3 License. see the [LICENSE.txt](LICENSE.txt) file for details.

## Acknowledgments

* [RGB D65 conversion] https://gist.github.com/popcorn245/30afa0f98eea1c2fd34d
* [Huestacean] https://github.com/BradyBrenot/huestacean
