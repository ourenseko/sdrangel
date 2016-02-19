![SDR Angel banner](/doc/img/sdrangel_banner.png)

**SDRangel** is an Open Source Qt5/OpenGL SDR and signal analyzer frontend to various hardware.

Although it keeps the same look and feel as its parent application **SDRangelove** it is a major redesign from it hitting more than half the lines of the code. Therefore the code base cannot be kept in sync anymore with its parent. It also contains enhancements and major differences. So it should now fly with its own wings and with its own name: **SDRangel**

<h1>Source code</h1>

<h2>Repository branches</h2>

- master: the production branch
- dev: the development branch
- fix: production fixes that can't wait
- legacy: the modified code from the parent application [hexameron rtl-sdrangelove](https://github.com/hexameron/rtl-sdrangelove) before a major redeisign of the code was carried out and sync was lost.

<h2>Untested plugins</h2>

These plugins come from the parent code base and have been maintained so that they compile but they are not being actively tested:

- Channels:
  - lora
  - tcpsrc (although it has evolved please use the udpsrc plugin instead)

<h2>Unsupported plugins</h2>

These plugins come from the parent code base and are still present in the source tree but are not part of the build:

- Channels:
  - tetra
- Sample sources:
  - gnuradio
  - osmosdr
  - v4l-msi
  - v4l-rtl

<h3>Gnuradio</h3>

The Gnuradio plugin source needs extra packages, including `liblog4cpp-dev libboost-system-dev gnuradio-dev libosmosdr-dev`

If you use your own location for Gnuradio install directory you need to specify library and include locations. Example with `/opt/install/gnuradio-3.7.5.1` with the following defines on `cmake` command line:

`-DGNURADIO_RUNTIME_LIBRARIES=/opt/install/gnuradio-3.7.5.1/lib/libgnuradio-runtime.so -DGNURADIO_RUNTIME_INCLUDE_DIRS=/opt/install/gnuradio-3.7.5.1/include`

<h3>osmosdr</h3>

If you use your own location for gr.osmocom install directory you need to specify library and include locations. Example with `/opt/install/gr-osmosdr` with the following defines on `cmake` command line:

`-DGNURADIO_OSMOSDR_LIBRARIES=/opt/install/gr-osmosdr/lib/libgnuradio-osmosdr.so -DGNURADIO_OSMOSDR_INCLUDE_DIRS=/opt/install/gr-osmosdr/include`

<h3>v4l-\*</h3>

Use `cmake ../ -DV4L-RTL=ON` to build the Linux kernel driver for RTL-SDR (Experimental). Needs a recent kernel and libv4l2. Will need extra work to support SDRPlay. Needs `cp KERNEL_SOURCE/include/linux/compiler.h /usr/include/linux/` and `cp KERNEL_SOURCE/include/uapi/linux/videodev2.h /usr/include/uapi/linux/` and package `libv4l-dev`.

<h1>Supported hardware</h1>

<h2>Airspy</h2>

Airspy is supported through the libairspy library that should be installed in your system for proper build of the software and operation support. Add `libairspy-dev` to the list of dependencies to install.

If you use your own location for libairspy install directory you need to specify library and include locations. Example with `/opt/install/libairspy` with the following defines on `cmake` command line:

`-DLIBAIRSPY_LIBRARIES=/opt/install/libairspy/lib/libairspy.so -DLIBAIRSPY_INCLUDE_DIR=/opt/install/libairspy/include`

Please note that if you are using a recent version of libairspy (>= 1.0.6) the dynamic retrieval of sample rates is supported. To benefit from it you should modify the `plugins/samplesource/airspy/CMakeLists.txt` and change line `add_definitions(${QT_DEFINITIONS})` by `add_definitions("${QT_DEFINITIONS} -DLIBAIRSPY_DYN_RATES")`. In fact both lines are present with the last one commented out.

Be also aware that the lower rates (2.5 MS/s or 5 MS/s with modified firmware) are affected by a noise artifact so 10 MS/s is preferable for weak signal work or instrumentation. A decimation by 64 was implemented to facilitate narrow band work at 10 MS/s input rate.

<h2>BladeRF</h2>

BladeRF is supported through the libbladerf library that should be installed in your system for proper build of the software and operation support. Add `libbladerf-dev` to the list of dependencies to install.

If you use your own location for libbladeRF install directory you need to specify library and include locations. Example with `/opt/install/libbladerf` with the following defines on `cmake` command line:

`-DLIBBLADERF_LIBRARIES=/opt/install/libbladeRF/lib/libbladeRF.so -DLIBBLADERF_INCLUDE_DIR=/opt/install/libbladeRF/include`

<h2>FunCube Dongle</h2>

Both Pro and Pro+ are supported with the plugins in fcdpro and fcdproplus respectively. For the Pro+ the band filter selection is not effective as it is handled by the firmware using the center frequency.

The control interface is based on qthid and has been built in the software in the fcdhid library. You don't need anything else than libusb support. Library fcdlib is used to store the constants for each dongle type.

The Pro+ has trouble starting. The sound card interface is not recognized when you just plug it in and start SDRAngel. The workaround is to start qthid then a recording program like Audacity and start recording in Audacity. Then just quit Audacity without saving and quit qthid. After this operation the Pro+ should be recognized by SDRAngel until you unplug it.

<h2>HackRF</h2>

HackRF is supported through the libhackrf library that should be installed in your system for proper build of the software and operation support. Add `libhackrf-dev` to the list of dependencies to install. Please note that you will need a recent version (2015.07.2 or 2015.07.1 at least) of libhackrf that supports the sequential listing of devices so you might need to build and install the Github version: `https://github.com/mossmann/hackrf.git`. Note also that the firmware must be updated to match the library version as per instructions found in the HackRF wiki.

If you use your own location for libhackrf install directory you need to specify library and include locations. Example with `/opt/install/libhackrf` with the following defines on `cmake` command line:

`-DLIBHACKRF_LIBRARIES=/opt/install/libhackrf/lib/libhackrf.so -DLIBHACKRF_INCLUDE_DIR=/opt/install/libhackrf/include`

HackRF is better used with a sampling frequency over 8 MS/s. You can use the 9.6Ms/s setting that decimates nicely into integer kS/s sample rates. There are quite a few problems with narrowband work with this hardware. You may try various amplifiers settings to limit images and I/Q imbalance with varying success... The cheap RTL-SDR dongles usually do better.

<h2>RTL-SDR</h2>

RTL-SDR based dongles are supported through the librtlsdr library that should be installed in your system for proper build of the software and operation support. Add `librtlsdr-dev` to the list of dependencies to install.

If you use your own location for librtlsdr install directory you need to specify library and include locations. Example with `/opt/install/librtlsdr` with the following defines on `cmake` command line:

`-DLIBRTLSDR_LIBRARIES=/opt/install/librtlsdr/lib/librtlsdr.so -DLIBRTLSDR_INCLUDE_DIR=/opt/install/librtlsdr/include`

<h1>Plugins for special devices</h1>

<h2>File input</h2>

The file input plugin allows the playback of a recorded IQ file. Such a file is obtained using the recording feature. Press F7 to start recording and F8 to stop. The file has a fixed name `test.sdriq` created in the current directory.

Note that this plugin does not require any of the hardware support libraries nor the libusb library. It is alwasys available in the list of devices as `FileSource[0]` even if no physical device is connected.

<h2>SDRdaemon input</h2>

Warning: this is experimental and not fulle debugged yet.

This is the client side of the SDRdaemon server. See the [SDRdaemon](https://github.com/f4exb/sdrdaemon) project in this Github repository. You must specify the address and UDP port to which the server connects and samples will flow into the SDRangel application (default is `127.0.0.1`port `9090`). It uses the meta data to retrieve the sample flow characteristics such as sample rate and receiveng center frequency.

Note that this plugin does not require any of the hardware support libraries nor the libusb library. It is alwasys available in the list of devices as `SDRdaemon[0]` even if no physical device is connected.

<h1>Software build</h1>

<h2>Ubuntu</h2>

<h3>Prerequisites for 14.04 LTS</h3>

Prerequisite to install Qt5 libraries properly:
`sudo apt-get install libgles2-mesa-dev`

Install cmake version 3:

  - `sudo apt-get install software-properties-common`
  - `sudo add-apt-repository ppa:george-edison55/cmake-3.x`
  - `sudo apt-get update`
  - `sudo apt-get remove cmake` (if already installed)
  - `sudo apt-get install cmake`

<h3>With newer versions just do:</h3>

  - `sudo apt-get install cmake g++ pkg-config libfftw3-dev libqt5multimedia5-plugins qtmultimedia5-dev qttools5-dev qttools5-dev-tools libqt5opengl5-dev qtbase5-dev libusb-1.0 librtlsdr-dev libboost-all-dev libasound2-dev pulseaudio`
  - `mkdir build && cd build && cmake ../ && make`

`librtlsdr-dev` is in the `universe` repo. (utopic 14.10 amd64.)

There is no installation procedure the executable is at the root of the build directory

<h2>Mint</h2>

Tested with Cinnamon 17.2. Since it is based on Ubuntu 14.04 LTS pleae follow instructions for this distribution (paragraph just above).

<h2>Debian</h2>

For any version of Debian you will need Qt5.

Debian 7 "wheezy" uses Qt4. Qt5 is available from the "wheezy-backports" repo, but this will remove Qt4. Debian 8 "jessie" uses Qt5.

For Debian Jessie or Stretch:

`sudo apt-get install cmake g++ pkg-config libfftw3-dev libusb-1.0-0-dev libusb-dev qt5-default qtbase5-dev qtchooser libqt5multimedia5-plugins qtmultimedia5-dev qttools5-dev qttools5-dev-tools libqt5opengl5-dev qtbase5-dev librtlsdr-dev libboost-all-dev libasound2-dev pulseaudio`

`mkdir build && cd build && cmake ../ && make`

<h2>openSUSE</h2>

This has been tested with the bleeding edge "Thumbleweed" distribution:

`sudo zypper install cmake fftw3-devel gcc-c++ libusb-1_0-devel libqt5-qtbase-devel libQt5OpenGL-devel libqt5-qtmultimedia-devel libqt5-qttools-devel libQt5Network-devel libQt5Widgets-devel boost-devel alsa-devel pulseaudio`

Then you should be all set to build the software with `cmake` and `make` as discussed earlier.

  - Note1 for udev rules: installed udev rules for BladeRF and HackRF are targetted at Debian or Ubuntu systems that have a plugdev group for USB hotplug devices. This is not the case in openSUSE. To make the udev rules file compatible just remove the `GROUP` parameter on all lines and change `MODE` parameter to `666`.
  - Note2: A package has been created (thanks Martin!), see: [sdrangel](http://software.opensuse.org/download.html?project=home%3Amnhauke%3Asdr&package=sdrangel). It is based on the 1.0.1 release.

<h2>Fedora</h2>

This has been tested with Fedora 23 and 22:

  - `sudo dnf groupinstall "C Development Tools and Libraries"`
  - `sudo dnf install mesa-libGL-devel`
  - `sudo dnf install cmake gcc-c++ pkgconfig fftw-devel libusb-devel qt5-qtbase-devel qt5-qtmultimedia-devel qt5-qttools-devel boost-devel pulseaudio alsa-lib-devel`

Then you should be all set to build the software with `cmake` and `make` as discussed earlier.

  - Note for udev rules: the same as for openSUSE applies. This is detailed in the previous paragraph for openSUSE.

<h2>Manjaro</h2>

Tested with the 15.09 version with LXDE desktop (community supported). The exact desktop environment should not matter anyway. Since Manjaro is Arch Linux based prerequisites should be similar for Arch and all derivatives.

`sudo pacman -S cmake pkg-config fftw qt5-multimedia qt5-tools qt5-base libusb boost boost-libs pulseaudio`

Then you should be all set to build the software with `cmake` and `make` as discussed earlier.

  - Note1 for udev rules: the same as for openSUSE and Fedora applies.
  - Note2: A package has been created in the AUR (thanks Mikos!), see: [sdrangel-git](https://aur.archlinux.org/packages/sdrangel-git). It is based on the `205fee6` commit of 8th December 2015.

<h1>Known Issues</h1>

  - The message queuing model supports a n:1 connection to an object (on its input queue) and a 1:1 connection from an object (on its output queue). Assuming a different model can cause insidious disruptions.
  - As the objects input and output queues can be publicly accessed there is no strict control of which objects post messages on these queues. The correct assumption is that messages can be popped from the input queue only by its holder and that messages can be pushed on the output queue only by its holder.
  - Objects managing more than one message queue (input + output for example) do not work well under stress conditions. Output queue removed from sample sources but this model has to be revised throughout the application.

<h1>Limitations</h1>

  - Tabbed panels showing "X0" refer to the only one selected device it is meant to be populated by more tabs when it will support more than one device possibly Rx + Tx.

<h1>Features</h1>

<h2>Changes from SDRangelove</h2>

See the v1.0.1 first official relase [release notes](https://github.com/f4exb/sdrangel/releases/tag/v1.0.1)

<h2>To Do</h2>

  - UDP source plugin: add the possibility to launch an external command that will process the samples like a GNUradio headless flowgraph
  - Allow the handling of more than one device at the same time. For Rx/Tx devices like the BladeRF Rx and Tx appear as two logical devices with two plugin instances and a common handler for the physical device services both plugins. This effectively opens Tx support.
  - Tx channels
  - Possibility to connect channels for example Rx to Tx or single Rx channel to dual Rx channel supporting MI(MO) features like 360 degree polarization detection.
  - Specialize plugins into channel and sample source plugins since both have almost complete different requirements and only little in common
  - 32 bit samples for the Channel Analyzer
  - Enhance presets management (Edit, Move, Import/Export from/to human readable format like JSON).
  - Headless mode based on a saved configuration in above human readable form
  - Allow arbitrary sample rate for channelizers and demodulators (not multiple of 48 kHz). Prerequisite for polyphase channelizer
  - Implement polyphase channelizer
  - Level calibration 
  - Even more demods ...

<h1>Developper's notes</h1>

<h2>Build options</h2>

The release type can be specified with the `-DBUILD_TYPE` cmake option. It takes the following values:

  - `RELEASE` (default): produces production release code i.e.optimized and no debug symbols
  - `RELEASEWITHDBGINFO`: optimized with debug info
  - `DEBUG`: unoptimized with debug info

You can specify whether or not you want to see debug messages printed out to the console with the `-DDEBUG_OUTPUT` cmake option:

  - `OFF` (default): no debug output
  - `ON`: debug output

You can add `-Wno-dev` on the `cmake` command line to avoid warnings.

<h2>Code organization</h2>

At the first subdirectory level `indclude` and `sdrbase` contain the common core components include and source files respectively. They are further broken down in subdirectories corresponding to a specific area:

  - `audio` contains the interface with the audio device(s)
  - `dsp` contains the common blocks for Digital Signal Processing like filters, scope and spectrum analyzer internals
  - `gui` contains the common Graphical User Interface components like the scope and spectrum analyzer controls and display
  - `plugin` contains the common blocks for managing plugins
  - `settings` contains components to manage presets and preferences
  - `util` contains common utilities such as the message queue

The `plugins` subdirectory contains the associated plugins used to manage devices and channel components. Naming convention of various items depend on the usage and Rx (reception side) or Tx (transmission side) affinity. Transmission side is yet to be created.

  - Receiver functions (Rx):
    - `samplesource`: Device managers:
      - `xxx` : Device manager (e.g. xxx = airspy)
        - `xxxinput.h/cpp` : Device interface
        - `xxxgui.h/cpp` : GUI
        - `xxxplugin.h/cpp` : Plugin interface
        - `xxxsettings.h/cpp` : Configuration manager
        - `xxxthread.h/cpp` : Reading samples
    - `channel`: Channel handlers:
      - `xxx` : Demodulator internal handler (e.g xxx = am)
        - `xxxdemod.h/cpp` : Demodulator core
        - `xxxdemodgui.h/cpp` : Demodulator GUI
        - `xxxplugin.h/cpp` : Plugin interface
      - `xxxanalyzer` : Analyzer internal handler (e.g xxx = channel)
        - `xxxanalyzer.h/cpp` : Analyzer core
        - `xxxanalyzergui.h/cpp` : Analyzer GUI
        - `xxxanalyzerplugin.h/cpp` : Analyzer plugin manager
      - `xxxsrc` : Interface to the outside (e.g xxx = udp):
        - `xxxsrc.h/cpp` : Inteface core
        - `xxxsrcgui.h/cpp` : Interface GUI
        - `xxxsrcplugin/h/cpp` : Interface plugin manager
