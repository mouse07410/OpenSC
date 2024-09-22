#!/usr/bin/env bash +ex

sudo date
make clean || true
rm -rf target/* && ./bootstrap && MacOSX/build && sudo /usr/local/bin/opensc-uninstall && sudo installer -pkg OpenSC.pkg -target /
date
