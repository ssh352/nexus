#!/bin/bash
if [ $# -eq 0 ] || [ $1 != 'Debug' ]; then export PROD_ENV=1; fi
directory=$(pwd)
pushd $directory/../../../library/build/posix
./build.sh
popd
pushd $directory/../..
if [ -d application ]; then rm -rf application; fi
mkdir -p application
cp -r ../resources application
cp -r source/index.html application
pushd node_modules
if [ -d beam ]; then rm -rf beam; fi
cp -r ../../../../../../Beam/web_api/library/* .
if [ -d @types/beam ]; then rm -rf @types/beam; fi
mkdir @types/beam
cp -r ../../../../../../Beam/web_api/library/beam/library/beam/* @types/beam
if [ -d nexus ]; then rm -rf nexus; fi
cp -r ../../../../../web_api/library/* .
if [ -d @types/nexus ]; then rm -rf @types/nexus; fi
mkdir @types/nexus
cp -r ../../../../../web_api/library/nexus/library/nexus/* @types/nexus
popd
node ./node_modules/webpack/bin/webpack.js
popd
