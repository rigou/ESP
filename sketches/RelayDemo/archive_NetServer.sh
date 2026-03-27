#!/bin/bash
declare FARC='netserver_v1-2-x'

declare APPDIR="$HOME/ARDUINO/Espressif/RelayDemo"
declare ARCDIR=/tmp

cd $HOME/ARDUINO/libraries/
tar cf $ARCDIR/$FARC.lib.tar rgNetworkCredentials rgWiFi

cd $APPDIR
tar cf $ARCDIR/$FARC.src.tar client $(ls *ino *cpp *h)

mv $ARCDIR/$FARC* $APPDIR/archives

echo "archives created"
ls -l $APPDIR/archives/$FARC*
