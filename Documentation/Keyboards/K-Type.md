- [Flashing the K-Type](#flashing-the-k-type)
    + [Entering Flash Mode.](#entering-flash-mode)
    + [Configure Your Layout](#configure-your-layout)
    + [Flashing - macOS](#flashing---macos)
    + [Flashing - linux](#flashing---linux)
    + [Flashing - Windows](#flashing---windows)
      - [Command Prompt Flashing](#command-prompt-flashing)
- [Adding a static color map on the K-Type](#adding-a-static-color-map-on-the-k-type)
- [Adding an pre-built animation to the K-Type](#adding-an-pre-built-animation-to-the-k-type)
- [Changing the switches on the K-Type](#changing-the-switches-on-the-k-type)

# Flashing the K-Type

### Entering Flash Mode.

 1) Flip your K-Type over.
 2) With the keys down you should see a small hole on the left side in the middle of the board.
 3) While the keyboard is plugged in, use a paperclip or other small object to press the button inside the case.
    - TIP: you can also configure a key combination to enter flash mode. On the default layout it's Fn-ESC.
 4) If the LEDs turn off and you see the amber light in the diffuser on the lower right (when looking at the top) it is now in flash mode.
    - Note: on some keyboards there is no amber light indication (there are no leds at all in flash mode)

#### Failsafe mode

In case the keyboard cannot load the flashed firmware it will go into failsafe mode. In this state the keyboard is unresponsive, but can still enter flash mode:
1) Unplug an replug the USB cable
2) Press the flash button (as described above) twice in a row

### Exiting Flash Mode

Press ESC or unplug the keyboard.

The keyboard exists flash mode automatically after a successful flash.
 
#### [K-Type Flashing Video Tutorial](https://youtu.be/i5wFVnEJcok)

### Configure Your Layout
 1) Configure your layout as you see fit using the configurator - [web version](https://configurator.input.club) or [client side](https://github.com/kiibohd/configurator/releases/latest)
 2) Download the firmware zip file.
 3) Unzip it to the location of your choice.

### Flashing - macOS

 1) Using [homebrew](https://brew.sh/) install [dfu-util](http://dfu-util.sourceforge.net/releases/) - `brew install dfu-util`
 2) Navigate to the directory you unzipped the firmware to
 3) With the K-Type in flash mode enter the command `dfu-util -D kiibohd.dfu.bin`
 4) You're done, enjoy your keyboard
 
### Flashing - linux

 1) Using your system package manager install [dfu-util](http://dfu-util.sourceforge.net/releases/), e.g.:
    - on Debian, Ubuntu and Mint: `sudo apt-get install dfu-util`
    - on Arch Linux `pacman -S dfu-util`
    - on Fedora `sudo dnf install dfu-util`
 2) Navigate to the directory you unzipped the firmware to
 3) With the K-Type in flash mode enter the command `dfu-util -D kiibohd.dfu.bin`
    - If the above fails, try running the command with `sudo` (i.e. `sudo dfu-util -D kiibohd.dfu.bin`)
    
 4) You're done, enjoy your keyboard
 
### Flashing - Windows
 1) (First time only) Download [Zadig](http://zadig.akeo.ie/)
	1) Select "Options", and then select "List All Devices"
	2) From the dropdown, select "API Interface (Interface 5)
	3) Using the arrows below, select the driver libusbK (Version number will update) 
	4) Click the large button that says "Install Driver", "Reinstall Driver", or "Replace Driver" (this depends on the devices you have used with this computer)
	Note: The above steps are to guarantee that this process works on all Windows machines. 
	
	After you have installed libusbK on the API Interface, we are also going to install it onto the K-Type. This is also only to be done the first time you use a computer with an Input Club keyboard.
	1) Put the K-Type in flash mode by pressing the button on the back of the keyboard. 
    2) With your K-Type in Flash mode you should see an entry in the Zadig dropdown entitled simply "K-Type"
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
 
# Adding a static color map on the K-Type
 **Note: This process assumes you have followed the earlier steps and have Zadig properly set up as well as the client side configurator installed.**
 1) Select your active keyboard from the list of connected devices.
 2) Select the Visuals tab in the upper right side of the configurator window.
 3) Click on the art palette in the bottom left of the configurator window to access the "Static LED Maps" section.
 4) Name your LED Map something without spaces or punctuation and press enter.
 5) You can select the LEDs you want to program individually by clicking on the black bracket over each key, or in bulk by using the Backlighting, Underlighting or All buttons. 
 6) Choose the colors you want and then navigate to the gear section named "Manage Visualizations".
 7) You should see your new animation and you will need to set it to "Auto-Start". Make sure to turn off any other animations from "Auto-Start" or your desired animation may be overwritten. 
 8) Download your firmware and follow the instructions in the Flashing section to flash your new firmware to your keyboard. 
 
 # Adding an pre-built animation to the K-Type
  **Note: This process assumes you have followed the earlier steps and have Zadig properly set up as well as the client side configurator installed.**
 1) Select your active keyboard from the list of connected devices.
 2) Select the Visuals tab in the upper right side of the configurator window.
 3) Click on the wrench in the bottom left of the configurator window to access the "Customize Pre-built" section.
 4) Select a Pre-built animation from the list, we will use Miami Wave as an example for this manual. 
 5) Click the "Add Animation" button 
 6) You should see your new animation and you will need to set it to "Auto-Start". Make sure to turn off any other animations from "Auto-Start" or your desired animation may be overwritten. 
 7) Download your firmware and follow the instructions in the Flashing section to flash your new firmware to your keyboard.  

# Changing the switches on the K-Type
The keyboard has hotswap support via switch sockets allowing you to change out and replace the keyswitches with different ones as long as:
 1) The switches have the same pinspacing - for example Alps switches are not compatible
 2) The switches are plate mount. If you have PCB mount switches, you may, at your discretion, cut off the two small plastic pegs on the bottom of the switch to have them be the same as plate mount.
 3) The switches were not desoldered and/or were never soldered. This is due to desoldered pins having a coat of solder on them, which varies in thickness depending on desoldering quality. This additional thickness may damage the sockets or detach them from the circuit board due to the extra insertion force. If you are attempting to use desoldered switches, proceed at your own risk as this may void your warranty.

To replace the switches, use the following procedure:

 1) Remove the keycaps on the switches that you want to replace using the provided keycap puller.
 2) Using the provided switch removal tool, remove the switches from the switch plate by pushing the locking arms inward, then lifting the switch straight up.
 3) Ensure that the replacement switch has undamaged pins that are straight and not bent. Some switches have very soft pins that bend easily, so caution should be observed.
 4) Insert the new switch, positioning the switch pins to match the socket pins. Ensure that all sides of the switch are firmly on the plate.
 5) Put the keycaps back on the replaced switches.

Note that the sockets are rated for 100 insertion cycles. While the sockets themselves may be replaced as well, the procedure requires good soldering skills and may void your warranty.
