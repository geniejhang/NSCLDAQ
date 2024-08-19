#!/bin/bash

##  Incorporate the unfied formatting library:

# Most recently used version 2.0

REPOSITORY="https://github.com/FRIBDAQ/UnifiedFormat.git"
TAG=$1
TARGET="unifiedformat"

if [[ ! $TAG ]]
then
  TAG="2.0"
fi

rm -rf $TARGET
git clone $REPOSITORY $TARGET
(cd $TARGET; git checkout $TAG)
