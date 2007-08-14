#!/bin/bash
#
#   Initiates a build of the device driver for the 2.6 kernel.
#
#   First need to locate the kernel sources.  For now we support two styles
#   of kernel source directory locations:
#
#    Debian:  /usr/src/kernel-source-<version>
#    Redhat:  /usr/src/linux-<version>
#

#  Note that you may need to edit
#  the code below up to the ---- line so that the
#  symbol Linuxsrc points to the directory
#  that has to kernel sources.. configured as per the running kernel
#

Linuxsrc=$1

version="$(uname -r)"
Debiansrc1="/usr/src/kernel-source-${version}"
Debiansrc2="/usr/src/linux-source-${version}"
RedHatsrc="/usr/src/linux-${version}"

#  Figure out which it is:

if [ "$Linuxsrc" == "" ]
then

    echo checking $Debiansrc1
    if [ -d $Debiansrc1 ] 
    then
	echo found
	Linuxsrc="$Debiansrc1"
    fi
    echo checking $Debiansrc2
    if [ -d $Debiansrc2 ]
    then
	echo found
       Linuxsrc="$Debiansrc2"
    fi
    echo checking $RedHatsrc
    if [ -d $RedHatsrc ]
    then
	echo found
	Linuxsrc="RedHatsrc"
    fi
else
    echo linux source provided: $Linuxsrc
fi

#-------------------------------------------------
#
#  At this point, Linuxsrc must point to the
#  directory that contains the kernel sources configured
#  for the running kernel.
#  As a last resort, uncomment the line below and fill in 
# the correct definition:
#Linuxsrc=replace-this-with-the-correct-directory

#
#  Everything below should be relatively system independent.
#
#------------------------------------------------------------
BTDRIVER=$(pwd)

pushd dd

# Always make from scratch:

rm -f *.o
make -C $Linuxsrc SUBDIRS=$(pwd) modules BTDRIVER=${BTDRIVER} 



popd
