- [Flashing the WhiteFox](#flashing-the-whitefox)
    + [Entering Flash Mode.](#entering-flash-mode)
    + [Configure Your Layout](#configure-your-layout)
    + [Flashing - macOS](#flashing---macos)
    + [Flashing - linux](#flashing---linux)
    + [Flashing - Windows](#flashing---windows)
      - [Command Prompt Flashing](#command-prompt-flashing)

![Whitefox](https://cdn.shopify.com/s/files/1/1994/3097/products/WhiteFox_Product_Image_2048x2048.png?v=1517883254)

# Flashing the WhiteFox (or NightFox)

### Entering Flash Mode.

 1) Flip your WhiteFox over.
 2) With the keys down you should see a small hole in the middle of the board.
 3) While the keyboard is plugged in, use a paperclip or other small object to press the button inside the case.
    - TIP: you can also configure a key combination to enter flash mode. On the default layout it's Fn-ESC.

#### Failsafe mode

In case the keyboard cannot load the flashed firmware it will go into failsafe mode. In this state the keyboard is unresponsive, but can still enter flash mode:
1) Unplug an replug the USB cable
2) Press the flash button (as described above) twice in a row

### Exiting Flash Mode

Press ESC or unplug the keyboard.

The keyboard exists flash mode automatically after a successful flash.
 
#### [WhiteFox Video Flashing Tutorial](https://www.youtube.com/watch?v=okFwGmpq70Y)

### Configure Your Layout
 1) Configure your layout as you see fit using the configurator - [web version](https://configurator.input.club) or [client side](https://github.com/kiibohd/configurator/releases/latest)
 2) Download the firmware zip file.
 3) Unzip it to the location of your choice.

### Flashing - macOS

 1) Using [homebrew](https://brew.sh/) install [dfu-util](http://dfu-util.sourceforge.net/releases/) - `brew install dfu-util`
 2) Navigate to the directory you unzipped the firmware to
 3) With the WhiteFox in flash mode enter the command `dfu-util -D kiibohd.dfu.bin`
 4) You're done, enjoy your keyboard
 
### Flashing - linux

 1) Using your system package manager install [dfu-util](http://dfu-util.sourceforge.net/releases/), e.g.:
    - on Debian, Ubuntu and Mint: `sudo apt-get install dfu-util`
    - on Arch Linux `pacman -S dfu-util`
    - on Fedora `sudo dnf install dfu-util`
 2) Navigate to the directory you unzipped the firmware to
 3) With the WhiteFox in flash mode enter the command `dfu-util -D kiibohd.dfu.bin`
    - If the above fails, try running the command with `sudo` (i.e. `sudo dfu-util -D kiibohd.dfu.bin`)
    
 4) You're done, enjoy your keyboard
 
### Flashing - Windows
 1) (First time only) Download [Zadig](http://zadig.akeo.ie/)
	1) Select "Options", and then select "List All Devices"
	2) From the dropdown, select "API Interface (Interface 5)
	3) Using the arrows below, select the driver libusbK (Version number will update) 
	4) Click the large button that says "Install Driver", "Reinstall Driver", or "Replace Driver" (this depends on the devices you have used with this computer)
	Note: The above steps are to guarantee that this process works on all Windows machines. 
	
	After you have installed libusbK on the API Interface, we are also going to install it onto the WhiteFox. This is also only to be done the first time you use a computer with an Input Club keyboard.
	1) Put the WhiteFox in flash mode by pressing the button on the back of the keyboard. 
    2) With your WhiteFox in Flash mode you should see an entry in the Zadig dropdown entitled simply "WhiteFox"
    3) Select the same libusbK driver from the available options and press either "Install Driver", "Reinstall Driver", or "Replace Driver". 
	
 2) Download [dfu-util](http://dfu-util.sourceforge.net/releases/) and unzip to a folder that you know the location of.
 3) Download and install the current [Input Club Configurator](https://github.com/kiibohd/configurator/releases/latest).
 4) Make your adjustments to your layout or your visual animations and click "Download Firmware"
 5) Click the button labeled "Flash". If you accidentally close it, there is an icon of a lightning bolt in the top right that you can press instead. 
 6) Click the three dots next to the file labeled "dfu-util command" and navigate to where you installed dfu-util. Select the application dfu-util.exe so that the configurator can use the software.
 7) Your layout should be auto-loaded into the .bin file field, but if you are flashing another file you acquired elsewhere, you can navigate to where that .bin file is and load it. 
 8) Put the K-Type into "Flash Mode" by pressing the button on the back with a paperclip or other thin device.
 9) Press Flash.
 
 If you see a FAILED message, it didn't work and you should join our [Discord Channel](https://discordapp.com/invite/9tpgDGS) and we will try and figure out what is going on. 
 
 If you see a SUCCESS message, congratulations on flashing your K-Type, and feel free to also join our Discord Channel to visit or say hello!
 
 #### Command Prompt Flashing
 1) Create a layout using either the [web configurator](https://input.club/configurator/) or the [client side configurator](https://github.com/kiibohd/configurator/releases/latest) and download it somewhere you know the location of. 
 2) Copy the `kiibohd.dfu.bin` file from the unpacked firmware zip file into the same directory you installed `dfu-util`
 3) Open a command prompt (Start => cmd.exe)
 4) Change directories to the `dfu-util` directory - `cd "C:\<path>\<to>\<dir>"`
 5) Execute the command `dfu-util -D kiibohd.dfu.bin`
 6) You're done, enjoy your keyboard
 
