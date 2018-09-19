#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

beam_commit="0e9346b4deb301373226727290f76196c91c6b84"
if [ ! -d "Beam" ]; then
  sudo -u $(logname) git clone https://www.github.com/eidolonsystems/beam.git Beam
  pushd Beam
  sudo -u $(logname) git checkout "$beam_commit"
  popd
  ./Beam/Build/Make/setup.sh
  pushd ./Beam/Build/Make
  sudo -u $(logname) ./run_cmake.sh
  sudo -u $(logname) ./build.sh
  popd
fi
if [ -d "Beam" ]; then
  pushd Beam
  commit="`git log -1 | head -1 | awk '{ print $2 }'`"
  if [ "$commit" != "$beam_commit" ]; then
    sudo -u $(logname) git checkout master
    sudo -u $(logname) git pull
    sudo -u $(logname) git checkout "$beam_commit"
    popd
    ./Beam/Build/Make/setup.sh
    pushd ./Beam/Build/Make
    sudo -u $(logname) ./run_cmake.sh
    sudo -u $(logname) ./build.sh
  fi
  popd
fi

if [ ! -d "quickfix-v.1.14.4" ]; then
  sudo -u $(logname) wget https://github.com/quickfix/quickfix/archive/v.1.14.4.zip --no-check-certificate -O v.1.14.4.zip
  if [ -f v.1.14.4.zip ]; then
    sudo -u $(logname) unzip v.1.14.4.zip
    pushd quickfix-v.1.14.4
    sudo -u $(logname) ./bootstrap
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    sudo -u $(logname) make check
    make install
    popd
    rm v.1.14.4.zip
  fi
fi

sudo -u $(logname) pip3 install argparse
sudo -u $(logname) pip3 install HTMLParser
sudo -u $(logname) pip3 install GitPython
