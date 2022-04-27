#!/bin/bash

#
#  Environment setup file for nscldaq
#


# Assume that if DAQROOT is defined, we're already set up.

function set_env() {
    export DAQROOT=/usr/opt/daq/11.4-028
    export DAQBIN=$DAQROOT/bin
    export DAQLIB=$DAQROOT/lib
    export DAQINC=$DAQROOT/include
    export PYTHONPATH=$DAQROOT/pythonLibs:$PYTHONPATH
}

if test "$1" = "-h"
then
  echo "Usage: . daqsetup.bash [-f]"
  echo ""
  echo "Description:"
  echo "  Set the DAQROOT, DAQLIB, DAQBIN, and PYTHONPATH environment variables"
  echo "  to refer to "/usr/opt/daq/11.4-028
  echo ""
  echo "Options:"
  echo "  -h  Display this help message"
  echo "  -f  Override environment variables even if DAQROOT is already set"

  return 0
fi


if test "$1" = "-f"
then
  set_env
else   
  if test x${DAQROOT} = x
  then
    set_env
  else 
    echo "Cannot override existing value of DAQROOT. To override, call this "
    echo "with the -f parameter."
  fi
fi
