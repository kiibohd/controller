#!/usr/bin/env python3
'''
Basic animation test case for Host-side KLL
'''

# Copyright (C) 2016 by Jacob Alexander
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http://www.gnu.org/licenses/>.

### Imports ###

import interface as i

from common import (ERROR, WARNING, check, result)



### Test ###

# Reference to callback datastructure
data = i.control.data

# Drop to cli, type exit in the displayed terminal to continue
#i.control.cli()

print("-Pixel Test-")

# Read status of animation display buffers
#print( i.control.cmd('animationDisplayBuffers')() )

# Add Animation, index 0, to Stack
i.control.cmd('addAnimation')()

# Read animation stack info
print( "Expecting Stack Size: 1 Got:", i.control.cmd('animationStackInfo')().size )
check( i.control.cmd('animationStackInfo')().size == 1 )

# Loop once
i.control.loop(1)

# Check Pixel 0
expecting = ((0, 33, 49), (30, 70, 120))
print( "Expecting:", expecting, "Got:", i.control.cmd('readPixel')(0) )
check( i.control.cmd('readPixel')(0) == expecting )
i.control.cmd('rectDisp')()

# Update FrameState and Loop again
i.control.cmd('setFrameState')(2)
i.control.loop(1)

# Check Pixel 0
expecting = ((0, 33, 49), (0, 0, 0))
print( "Expecting:", expecting, "Got:", i.control.cmd('readPixel')(0) )
check( i.control.cmd('readPixel')(0) == expecting )
i.control.cmd('rectDisp')()

# Update FrameState and Loop again
i.control.cmd('setFrameState')(2)
i.control.loop(1)

# Check Pixel 0
expecting = ((0, 33, 49), (60, 90, 140))
print( "Expecting:", expecting, "Got:", i.control.cmd('readPixel')(0) )
check( i.control.cmd('readPixel')(0) == expecting )
i.control.cmd('rectDisp')()

# Update FrameState and Loop again to clear the stack
i.control.cmd('setFrameState')(2)
i.control.loop(1)

# Read status of animation display buffers
#print( i.control.cmd('animationDisplayBuffers')() )

# Read animation stack info
print( "Expecting Stack Size: 0 Got:", i.control.cmd('animationStackInfo')().size )
check( i.control.cmd('animationStackInfo')().size == 0 )


##### Next Test #####


print("-Rainbow Test-")
# Add Animation, index 0, to Stack
i.control.cmd('addAnimation')(index=1, pfunc=1)

# Read animation stack info
print( "Expecting Stack Size: 1 Got:", i.control.cmd('animationStackInfo')().size )
check( i.control.cmd('animationStackInfo')().size == 1 )

# Loop once
i.control.cmd('setFrameState')(2)
i.control.loop(1)

# Show output
i.control.cmd('rectDisp')()

# Update FrameState and Loop again to clear the stack
i.control.cmd('setFrameState')(2)
i.control.loop(1)

# Read animation stack info
print( "Expecting Stack Size: 0 Got:", i.control.cmd('animationStackInfo')().size )
check( i.control.cmd('animationStackInfo')().size == 0 )


##### Tests Complete #####

result()

