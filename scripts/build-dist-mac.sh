#!/bin/bash

echo 'Building librealsense C++ API...'
pushd .  > /dev/null
pwd

cd librealsense
mkdir -p build
cd build

# Step 1
cmake .. -DBUILD_EXAMPLES=false -DBUILD_UNIT_TESTS=false -DBUILD_WITH_OPENMP=false -DHWM_OVER_XU=false -G Xcode
if [ "$?" -gt 0 ]
then
	echo 'Failed to configure librealsense'
	exit 1
fi

#step 2
xcodebuild -configuration Release
if [ "$?" -gt 0 ]
then
	echo 'Failed to build librealsense'
	exit 2
fi

cd ../..

popd > /dev/null
