#///////////////////////////////////////////////////////////////////////////////
#//
#//  dsnap - configure.sh
#//
#//  Checks to ensure all system requirements are met in order to use dsnap.
#//
#///////////////////////////////////////////////////////////////////////////////

#!/bin/bash

fail=false

printf "\nChecking requirements...\n\n"

# Check for debugfs mount point
echo -n "debugfs mounted..."

if [ "$(mount | grep -c debugfs)" != 0 ]; then
	echo "yes"
else
	fail=true
	echo "no"
fi

# Check for python
echo -n "python installed..."

if [ "$(printf '%s\n' ${PATH//:/\/* } | grep -c python)" != 0 ]; then
	echo "yes"
else
	fail=true
	echo "no"
fi

# Check for pahole
echo -n "pahole installed..."

if [ "$(printf '%s\n' ${PATH//:/\/* } | grep -c pahole)" != 0 ]; then
	echo "yes"
else
	fail=true
	echo "no"
fi

# Report results
echo ""

if [ "$fail" == true ]; then
	echo "Your system does not meet the requirements to run dsnap."
else
	echo "System check complete. All requirements met."
fi

echo ""
