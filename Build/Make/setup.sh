#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

if [ ! -d "quickfix" ]; then
  wget http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.tar.gz
  gzip -d -c quickfix-1.14.3.tar.gz | tar -x
  cd quickfix
  ./configure
  make -j $cores
  make install
  cd ..
  rm quickfix-1.14.3.tar.gz
fi

if [ ! -d "sqlite" ]; then
  wget http://www.sqlite.org/2015/sqlite-autoconf-3080803.tar.gz
  gzip -d -c sqlite-autoconf-3080803.tar.gz | tar -x
  mv sqlite-autoconf-3080803 sqlite
  cd sqlite
  ./configure
  make -j $cores
  make install
  cd ..
  rm sqlite-autoconf-3080803.tar.gz
fi

if [ ! -d "qt-5.5.0" ]; then
  wget http://download.qt.io/official_releases/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.tar.xz
  tar xfv qt-everywhere-opensource-src-5.5.0.tar.xz
  rm qt-everywhere-opensource-src-5.5.0.tar.xz
  mv qt-everywhere-opensource-src-5.5.0 qt-5.5.0
  cd qt-5.5.0
  chmod +rwx configure
  ./configure -opensource -static -qt-xcb -make libs -make tools -opengl desktop -no-icu -qt-zlib -c++11 -confirm-license
  make -j $cores
  make install
fi

if [ ! -d "jquery-1.12.1" ]; then
  mkdir jquery-1.12.1
  cd jquery-1.12.1
  wget http://code.jquery.com/jquery-1.12.1.min.js
  cd ..
fi

if [ ! -d "react-0.14.7" ]; then
  mkdir react-0.14.7
  cd react-0.14.7
  wget http://fb.me/react-0.14.7.js -O react-0.14.7.js
  wget http://fb.me/react-dom-0.14.7.js -O react-dom-0.14.7.js
  cd ..
fi

if [ ! -d "requirejs-2.1.22" ]; then
  mkdir requirejs-2.1.22
  cd requirejs-2.1.22
  wget http://requirejs.org/docs/release/2.1.22/minified/require.js
  cd ..
fi

if [ ! -d "require-css" ]; then
  wget http://github.com/guybedford/require-css/archive/master.zip
  unzip master.zip
  mv require-css-master require-css
  rm master.zip
fi

pushd $directory/../../Applications/ClientWebPortal/Build/Make
./setup.sh
popd
