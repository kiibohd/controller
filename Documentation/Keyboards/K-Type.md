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
    1) With your K-Type in Flash mode you should see an entry in the Zadig dropdown entitled simply "K-Type"
    2) With the "WinUSB" driver selected; click the "Install Driver" button
 2) Download [dfu-util](http://dfu-util.sourceforge.net/) and unzip to a folder
 3) Copy the `kiibohd.dfu.bin` file from the unpacked firmware zip file into the `dfu-util` directory
 4) Open a command prompt (Start => cmd.exe)
 5) Change directories to the `dfu-util` directory - `cd "C:\<path>\<to>\<dir>"`
 6) Execute the command `dfu-util -D kiibohd.dfu.bin`
 4) You're done, enjoy your keyboard
