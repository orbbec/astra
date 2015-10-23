Astra SDK v0.4.0 Preview
Copyright (c) 2015 Orbbec
www.orbbec3d.com

For help and support, check https://3dclub.orbbec3d.com.

What's New
==========

v0.4.0 2015/10/14
* Add official support for Win64 and OS X 10.8+
* Updated SFML to 2.3.2
* Added features to SimpleDepthViewer-SFML and SimpleStreamViewer-SFML: pausing, overlay color on depth, display depth data under the mouse in text overlay. (See keyboard shortcut section below.)
* Minor internal bug fixes

v0.3.0 2015/09/14
* Rename to Astra SDK.
* Rename Sensor to StreamSet in C++ API.
* Various bug fixes and internal enhancements.
* New samples:  SimpleStreamViewer-SFML, SimpleColorViewer-SFML, ColorReaderEvent, ColorReaderPoll
* Samples have improved performance.
* Add IR stream, mirrored depth, and registered depth support.
* VS2013 samples solution no longer requires copying files - compiles and runs out of the box.
* StreamReader start() and stop() are functional now. See SimpleStreamViewer-SFML.
* Add initial getting started documentation

v0.2.1 2015/07/06 Updated Android and Windows drivers for new sensor USB IDs. Add Android test app .apk.

v0.2.0 2015/07/03 First version ready for external use.

Pre-built samples
==========

Pre-built samples are included in the bin/ directory.
Simply plug in your sensor and then run any of the executable files in the bin/ directory.

We recommend starting with SimpleStreamViewer-SFML and SimpleHandViewer-SFML.
In the hand viewer, wave left and right at the sensor a few times to start hand tracking.

OS-specific prerequisites instructions:

OS X:
=======
Requires:
* OS X 10.8+
* Xcode 6.2+
* homebrew from http://brew.sh/

At the terminal command line:
$ brew install sfml libusb

Now you can run the pre-built binaries from the Astra SDK bin/ directory.

Windows:
=======
If you want to run the pre-compiled samples and don't have VS2013 installed,
you must install the Visual C++ Redistributable Packages for Visual Studio 2013 from
http://www.microsoft.com/en-us/download/details.aspx?id=40784
(Not required if VS2013 is installed.)

Sample keyboard shortcuts
==========
SimpleStreamViewer-SFML:
* F - toggle fullscreen
* R - toggle registered depth
* M - toggle mirrored streams
* I - enable IR (RGB mode)
* G - enable IR (Gray16 mode)
* C - enable color
* P - toggle pausing the streams
* O - toggle overlay color stream on depth stream

SimpleDepthViewer-SFML:
* F - toggle fullscreen
* R - toggle registered depth
* M - toggle mirrored streams
* P - toggle pausing the streams
* Space bar - toggle text overlay with the depth data under the mouse cursor

Building the samples from source
==========

OS X:
=======

Make sure you have already run the commands above to install sample prerequisites.
You will also need to install CMake 3.2+ with this command:
$ brew install cmake

Then, in the terminal, change directories to the Astra SDK samples/ directory, then:
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j8

Now you can run the samples in the samples/bin/ directory.

Windows:
=======
Requirements:
* Visual Studio 2013 or later. The VS 2013 Community version is a free download:
https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx
* Windows 7 or later. Tested on Windows 10, Windows 8.1, Windows 7.

The provided Visual Studio 2013 solution is already configured to run out of the box.

In the sdk/samples/vs2013 folder, open astra-samples.sln. Change the configuration to Release. Build the solution.

The samples compile to sdk/samples/vs2013/bin/Debug/ or Release/, depending upon your build configuration.

Tips:
=========

You can exit samples by pressing Control-C. They will catch this signal and exit cleanly. To exit samples with a GUI window, press Control-C, escape, or simply close the window.

When you start development, we highly recommend using the C++ API (or a higher level language wrapper.) The C API is meant for compatibility, so the C++ API is much more pleasant development experience.

Documentation
==========

Preliminary documentation in HTML format can be found in the sdk/docs directory.

Known issues
==========
1) There is no error message if no sensor is found or plugged in. (But, it doesn't crash!) Hotplugging (plugging in or removing the sensor while an app is running) is not currently supported.

2) If a sample crashes or you stop debugging before it cleans itself up nicely, the sensor driver may be put into a weird state. Before the next run you may need to replug the sensor or turn the sensor off and then on first. (This is primarily an OpenNI driver issue.)

3) If you close a sample's console window without cleanly exiting the program, it may crash. Press control-c to exit cleanly.

4) If any samples run slow on your computer, try building them in release mode with optimizations turned on.
