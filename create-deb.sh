#!/bin/sh
#
# This script creates Q Light Controller DEB packages.

dpkg-buildpackage -rfakeroot -I.svn -uc -us -sa -D
