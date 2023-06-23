#!/bin/bash

VERSION=$1

if [[ ! $VERSION ]]
then
  echo "Usage:"
  echo "   $0 TAG"
  exit
fi

WRAPPER="== ${VERSION} =="

INFILE=release_notes
OUTFILE=release_body

STARTED=false

while IFS= read -r line
do
  if [[ $line == *"$WRAPPER"* && $STARTED == false ]]
  then
    rm -f $OUTFILE
    STARTED=true
    continue
  fi

  if [[ $line == *"$WRAPPER"* && $STARTED == true ]]
  then
    break
  fi

  echo $line >> $OUTFILE
done < $INFILE
