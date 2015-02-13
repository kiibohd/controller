#!/bin/bash

# Just in case there was an extra ledTest.bash (don't care if error)
rm /dev/ttyACM0

./swdLoad.bash kiibohd_manufacturing_2014-11-16.bin 0

echo "Press CTRL+C to Continue"

while true; do
	sleep 1
	if [ -e /dev/ttyACM0 ]; then
		./ledTest.bash
	fi
done

exit 0

