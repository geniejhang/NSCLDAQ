#!/bin/bash
#
#   Do login like stuff.
#
RUNREADOUTdir=$1
RUNREADOUTexec=$2
if [ -e .profile ]; then
  . .profile
fi
if [ -e .bashrc ]; then
  . .bashrc
fi

cd $RUNREADOUTdir 

./$RUNREADOUTexec 2>&1
