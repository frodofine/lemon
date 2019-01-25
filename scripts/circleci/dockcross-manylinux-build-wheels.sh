#!/bin/bash
set -e

# Pull dockcross manylinux images
docker pull dockcross/manylinux-x64

# Generate dockcross scripts
docker run dockcross/manylinux-x64 > /tmp/dockcross-manylinux-x64
chmod u+x /tmp/dockcross-manylinux-x64

pushd .
mkdir deps
cd deps
wget --quiet https://cmake.org/files/v3.6/cmake-3.6.2-Linux-x86_64.tar.gz && tar -xvf cmake-3.6.2-Linux-x86_64.tar.gz
curl -L http://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.gz -O
gunzip boost_1_58_0.tar.gz
tar xf boost_1_58_0.tar
cd boost_1_58_0/
sh bootstrap.sh
./bjam cxxflags=-fPIC cflags=-fPIC -a --with-filesystem --with-program_options
cd ..
git clone https://github.com/frodofine/chemfiles.git -b read_from_memory_2
cd chemfiles
mkdir build
cd build
../../cmake-3.6.2-Linux-x86_64/bin/cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/deps/chfl -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON -DCMAKE_BUILD_TYPE:STRING=Release
make
make install
popd

# Build wheels
mkdir -p dist
cd lemon
DOCKER_ARGS="-v $HOME/dist:/work/dist/ -v $HOME/deps:/deps"
/tmp/dockcross-manylinux-x64 \
  -a "$DOCKER_ARGS" \
"/work/scripts/circleci/manylinux-build-wheels.sh" "$@"
