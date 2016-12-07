#!/bin/bash
ENV=$1
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../..
webpack --env.environment $ENV
rm -rf ../../Application/webapp
cp -a dist/. ../../Application/webapp/
popd

