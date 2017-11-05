# Flashing the K-Type

### Entering Flash Mode.

 1) Flip your K-Type over.
 2) With the keys down you should see a small hole on the left side in the middle of the board.
 3) While the keyboard is plugged in, use a paperclip or other small object to press the button inside the case.
 4) If the LEDs turn off and you see the amber light in the diffuser on the lower right (when looking at the top) it is now in flash mode.

### Configure Your Layout
 1) Configure your layout as you see fit using the configurator - [web version](https://configurator.input.club) or [client side](https://github.com/kiibohd/configurator)
 2) Download the firmware zip file.
 3) Unzip it to the location of your choice.

### Flashing - macOS

 1) Using [homebrew](https://brew.sh/) install [dfu-util](http://dfu-util.sourceforge.net/) - `brew install dfu-util`
 2) Navigate to the directory you unzipped the firmware to
 3) With the K-Type in flash mode enter the command `dfu-util -D kiibohd.dfu.bin`
 4) You're done, enjoy your keyboard
 
### Flashing - linux

 1) Using your system package manager install [dfu-util](http://dfu-util.sourceforge.net/) (e.g. on arch linux `pacman -S dfu-util`)
 2) Navigate to the directory you unzipped the firmware to
 3) With the K-Type in flash mode enter the command `dfu-util -D kiibohd.dfu.bin`
 4) You're done, enjoy your keyboard
 
### Flashing - Windows
 1) (First time only) Download [Zadig](http://zadig.akeo.ie/)
    1) Put the K-Type in [Flash mode](#entering-flash-mode).
    2) In the Zadig dropdown list, select "K-Type". If you do not see "K-Type" in the list then your board isn't in flash mode.
    3) With the "WinUSB" driver selected, click the "Install Driver" button.
 2) Download [dfu-util](http://dfu-util.sourceforge.net/) and unzip to a folder
 3) Copy the `kiibohd.dfu.bin` file from the unpacked firmware zip file into the `dfu-util` directory
 4) Open a command prompt (Start => cmd.exe)
 5) Change directories to the `dfu-util` directory - `cd "C:\<path>\<to>\<dir>"`
 6) Execute the command `dfu-util -D kiibohd.dfu.bin`
 4) You're done, enjoy your keyboard

# Changing the switches on the K-Type
The keyboard has hotswap support via switch sockets allowing you to change out and replace the keyswitches with different ones as long as:
1. The switches have the same pinspacing - for example Alps switches are not compatible
2. The switches are plate mount. If you have PCB mount switches, you may, at your discretion, cut off the two small plastic pegs on the bottom of the switch to have them be the same as plate mount.
3. The switches were not desoldered and/or were never soldered. This is due to desoldered pins having a coat of solder on them, which varies in thickness depending on desoldering quality. This additional thickness may damage the sockets or detach them from the circuit board due to the extra insertion force. If you are attempting to use desoldered switches, proceed at your own risk as this may void your warranty.

To replace the switches, use the following procedure:

1. Remove the keycaps on the switches that you want to replace using the provided keycap puller.
2. Using the provided switch removal tool, remove the switches from the switch plate by pushing the locking arms inward, then lifting the switch straight up.
3. Ensure that the replacement switch has undamaged pins that are straight and not bent. Some switches have very soft pins that bend easily, so caution should be observed.
4. Insert the new switch, positioning of the pins to match to the socket. Ensure that all sides of the switch are firmly on the plate.
5. Put the keycaps back on the replaced switches.

Note that the sockets are rated for 100 insertion cycles. While the sockets themselves may be replaced as well, the procedure requires good soldering skills and may void your warranty.
