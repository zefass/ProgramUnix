#!/bin/bash

dbus-monitor --system "sender=org.freedesktop.NetworkManager, path=/org/freedesktop/NetworkManager, member=StateChanged" | sed -u -n 's/   uint32 20/\/usr\/bin\/python3 \/home\/zefass\/Desktop\/practice6\/stop.py/p; s/   uint32 70/\/usr\/bin\/python3 \/home\/zefass\/Desktop\/practice6\/start.py/p' | sh
