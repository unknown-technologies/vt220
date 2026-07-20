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
- F2 = toggle fullscreen
- F3 = setup
- F5 = send BREAK (TELNET) / SIGINT (PTY)
- CTRL-F5 = send answerback message

In addition, ALT-F1 to ALT-F10 is interpreted as F11 to F20.


Completeness
------------

Feel free to test this emulator with
[vttest](https://invisible-island.net/vttest/) to see how good this emulator is.

Almost all features of the real VT220 are implemented already. Compared to a
real VT220, the following features are currently missing:
- Printer support
- RS232 support
- Complete keyboard emulation including compose keys

This VT220 emulator is implemented using the VT220 reference manual as well as
testing against a real VT220. If the emulator behaves differently than a real
VT220, even if it is just some edge case, it is a bug. Please report such bugs
with the necessary input and a photo of how the result should look like as well
as a short explanation of what is wrong, in case this is not directly obvious
from the input/photo.


Raspberry Pi Version
--------------------

A version for Raspberry Pi 4+ is planned, but not yet implemented. This will be
useful to create dedicated "VT220 appliances" without relying on X11 / Wayland.


Windows Version
---------------

Although there are some code fragments which look like there might be Windows
support, there is currently no such support. This is simply caused by the fact
that no Windows based machine with C compiler and graphics card is currently
available to the developer. If you are bored and want to provide proper Windows
support, feel free to open a pull request.

Only the TELNET library supports Windows at the moment since it was written
on a Windows computer many years ago, but even this is currently untested.


Screenshots
-----------

vim in a screen session with green phosphor:

![VIM in a screen session](https://raw.githubusercontent.com/unknown-technologies/vt220/master/doc/vt220-vim.png)

setup screen with amber phosphor:

![Setup screen](https://raw.githubusercontent.com/unknown-technologies/vt220/master/doc/vt220-setup.png)
