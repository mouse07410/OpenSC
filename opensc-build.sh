#!/usr/bin/env bash +ex

sudo date
#make clean && 
rm -rf target/* && ./bootstrap && MacOSX/build && sudo /usr/local/bin/opensc-uninstall && sudo installer -pkg OpenSC.pkg -target /
date
