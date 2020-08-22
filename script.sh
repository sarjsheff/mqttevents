#!/bin/sh

echo "tele/switch/LWT" "Online"
if grep ON state.txt
then
  echo 'stat/switch/STATE {"POWER":"OFF"}'
  echo OFF > state.txt
else
  echo 'stat/switch/STATE {"POWER":"ON"}'
  echo ON > state.txt
fi
