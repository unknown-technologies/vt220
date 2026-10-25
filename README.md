DEC VT220
=========

This is a somewhat accurate emulation of a DEC VT220 terminal with fancy
graphics.

Special care was taken to create a visually appealing appearance which
explicitly tries to look very similar to a real VT220 instead of inventing
random "graphics effects" (more like "distortions") like certain other terminal
emulators with "fancy graphics" do.


Hardware Requirements
---------------------

- Graphics card with OpenGL 3.2+


Build Requirements
------------------

- Linux with git, gcc and make
- glfw3
- glslang
- [bin2o](https://github.com/hackyourlife/bin2o)


Building
--------

Just run `make` to get the native binary.


Usage
-----

`vt220 -l` launches the VT220 in "local mode", that is, the RX and TX pin on
the virtual terminal is connected and everything you type is directly
displayed. This is useful if you want to manually design UI prototypes.

`vt220` without arguments starts the default shell,
`vt220 -s /path/to/the/binary` runs an arbitrary program in the VT220. The
`TERM` environment variable is set to `vt220` and if the `LANG` environment
variable is set to an UTF-8 language, it is reset to `LANG=C` to avoid
problems.

`vt220 -t host port` connects to the telnet server `host` at port `port`.


Keyboard
--------

A VT220 does not have all the keys you usually find on a PC keyboard and some
keys have a very different meaning. In particular, the function keys F1-F5 are
"local" keys which configure the terminal and have no associated sequence that
could be sent to the host.

The VT220 emulator uses the following map for local keys:
- F1 = hold screen
- F2 = print screen
- F3 = setup
- F4 = toggle fullscreen
- F5 = send BREAK (TELNET) / SIGINT (PTY)
- CTRL-F5 = send answerback message

In addition, ALT-F1 to ALT-F10 is interpreted as F11 to F20.


Completeness
------------

Feel free to test this emulator with
[vttest](https://invisible-island.net/vttest/) to see how good this emulator is.

Almost all features of the real VT220 are implemented already. Compared to a
real VT220, the following features are currently missing:
- Keyclick and margin/warning bell

The following features are currently incomplete or differ from the real VT220:
- Complete keyboard emulation (dead keys and compose keys are missing)
- Printer support (missing character set behavior)
- 7-bit comm mode (completely missing)
- national mode (currently this is ignored and the keyboard always sends multinational codes)
- NRCS (currently all NRCS can be selected regardless of DECNRCM / keyboard)
- DRCS (currently all parameters are ignored, hard character sets cannot be shadowed)

This VT220 emulator is implemented using the VT220 reference manual as well as
testing against a real VT220. If the emulator behaves differently than a real
VT220, even if it is just some edge case, it is a bug. Please report such bugs
with the necessary input and a photo of how the result should look like as well
as a short explanation of what is wrong, in case this is not directly obvious
from the input/photo.


Raspberry Pi Version
--------------------

A version for Raspberry Pi 4+ is implemented but not yet complete. This
implementation is useful to create dedicated "VT220 appliances" without relying
on X11 / Wayland. Instead, the graphics context is created via DRM/EGL and
keyboard events are directly handled via the appropriate /dev/input device.

An advanced "Connection Manager" supports configuration of local PTY sessions,
TTY sessions (RS232, UART) in case TTY lines are available, TELNET, and SSH
connections. This Connection Manager automatically shows a menu of available
connections in case no connection is marked as "autoconnect". By invoking a
disconnect sequence (SHIFT-F5) or pressing the DATA/TALK key (F4), you can
always enter the Connection Manager at any time.

Any hardware variant of the Raspberry Pi 4+ works, even the smallest 1GB
version is sufficient. The only hardware requirements are a keyboard and a
screen. Ideally the screen has a resultion of 800x480 pixels, like the official
Raspberry Pi foundation 7" touch display. Development happens on a Raspberry Pi
4 running [ALARM](https://archlinuxarm.org/).

To build the Raspberry Pi version, you need a Raspberry Pi 4+ with libdrm, EGL,
glslang, libssh, and [bin2o](https://github.com/hackyourlife/bin2o). GLFW is
_not_ required nor used. Once the prerequisites are installed, run the
following command on the Raspberry Pi:

```
make -f Makefile.rpi -j4 NDEBUG=1
```

**IMPORTANT**: the VT220 emulator takes exclusive control of the keyboard
device which means you have _no_ way to exit the VT220 emulator once it is
running. This is intended behavior, since the Raspberry Pi version is meant to
be used as the only application running on an "appliance" built around the
Raspberry Pi, with the VT220 emulator started as a system service. The intended
method to stop the VT220 emulator is by sending a SIGINT or SIGTERM signal to
the process.


Windows Version
---------------

There is a mostly untested Windows version which only supports loopback mode and
the TELNET client. Since Windows lacks a sane equivalent to a PTY, you _cannot_
run any Windows console programs within this VT220.

To build this Windows version, you need mingw-w64 and mingw-w64-glfw. Once both
are installed, you can run the following command to build the VT220 binary:

```
make -f Makefile.win32 NDEBUG=1
```


Screenshots
-----------

vim in a screen session with green phosphor:

![VIM in a screen session](https://raw.githubusercontent.com/unknown-technologies/vt220/refs/heads/master/doc/vt220-vim.png)

setup screen with amber phosphor:

![Setup screen](https://raw.githubusercontent.com/unknown-technologies/vt220/refs/heads/master/doc/vt220-setup.png)
