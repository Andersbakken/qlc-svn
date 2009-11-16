#!/bin/sh
#
# This script creates Q Light Controller RPM packages.

VERSION=3.0.4
SOURCES=qlc
RPMBUILD=~/rpmbuild

# Prepare RPM build directory hierarchy
if [ ! -f $RPMBUILD ]; then
	mkdir -p $RPMBUILD/SOURCES $RPMBUILD/SPECS $RPMBUILD/BUILD $RPMBUILD/SRPMS
	mkdir -p $RPMBUILD/RPMS/i386 $RPMBUILD/RPMS/athlon $RPMBUILD/RPMS/i486
	mkdir -p $RPMBUILD/RPMS/i586 $RPMBUILD/RPMS/i686 $RPMBUILD/RPMS/noarch
fi

# Copy the RPM spec file so that rpmbuild finds it
cp -f rpm/qlc.spec $RPMBUILD/SPECS

# Prepare a source tarball and move it under $RPMBUILD/SOURCES
cd ..
cp -R $SOURCES qlc-$VERSION
tar -czf qlc-$VERSION.tar.gz qlc-$VERSION
mv qlc-$VERSION.tar.gz $RPMBUILD/SOURCES

cd $RPMBUILD/SPECS
rpmbuild -bb qlc.spec
