# Huenicorn

Free screen synchronizer for your Philips Hue™ devices


## Description

Huenicorn allows you to stream the dominant colors of your computer monitor to your Hue lightbulbs.

## Getting Started
  * Before using Huenicorn, you need to register your Hue devices on your Hue Bridge through the official applications provided by Philips

### Requirements
  * Gnu/Linux system running through X11 graphics session
  * Philips Hue Bridge
  * Philips Hue lamps

### Dependencies
* X11
* [OpenCV](https://github.com/opencv/opencv)
* [CurlPP](https://github.com/jpbarrette/curlpp)
* [Restbed](https://github.com/Corvusoft/restbed)

### Compiling Huenicorn
Make sure the abovementionned are installed (preferably through your system's package manager)

```
git clone https://gitlab.com/openjowelsofts/huenicorn.git
cd huenicorn
mkdir build && cd build
cmake ..
make
```

The output is a standalone executable called huenicorn

### Executing program
* Run the executable in the way you prefer (Terminal if you want some textual feedback)
* Open your favorite web browser at http://127.0.0.1:8080 (Browser will spawn automatically for the initial setup and as long as no light profile has been saved)

#### Initial setup
* Follow the web wizard to register Huenicorn on the Hue Bridge

Upon setup proper termination, Huenicorn will resume in its ordinary execution workflow.
<br>
The webpage can then be refreshed to reach the Light Manager interface.

#### Ordinary execution worflow
At startup, Huenicorn will look for a saved profile and compute the assigned colors to lights.

If you want to assign lights to some portions of your screen, open the web interface and drag'n'drop the available lights from the left list to the right one

When a synced light is selected in the list, its portion of allocated screen can be adjusted in the screen partitioning section.

Once you are satisfied with the layout, you can save it as a profile and it will be automatically reloaded for further executions.

#### Shutting down
Huenicorn can be shut down through the web interface or by sending a termination signal.


## Authors
OpenJowel


## Version History
  * Initial Release

## License

This project is licensed under the GNU GPLv3 License - see the LICENSE.txt file for details

## Acknowledgments
* https://gist.github.com/popcorn245/30afa0f98eea1c2fd34d
* https://github.com/BradyBrenot/huestacean
