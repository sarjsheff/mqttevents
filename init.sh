#!/bin/sh

echo "tele/switch/LWT" "Online"
if grep ON state.txt
then
  echo 'stat/switch/STATE {"POWER":"ON"}'
else
  echo 'stat/switch/STATE {"POWER":"OFF"}'
fi
