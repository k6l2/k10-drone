#!/bin/bash
SCRIPT=`realpath -s $0`
SCRIPTPATH=`dirname $SCRIPT`
GLSL_COMPILER=`cygpath $VULKAN_SDK/Bin32/glslc.exe`
echo "SCRIPTPATH=$SCRIPTPATH"
mkdir -p "$SCRIPTPATH/shader-bins"
for file in $SCRIPTPATH/shaders/*; do
	BASEFILENAME=`basename -- $file`
	FILEEXT="${BASEFILENAME##*.}"
	FILENAME="${BASEFILENAME%.*}"
	SPV_FILENAME=`cygpath -w "$SCRIPTPATH/shader-bins/$FILENAME-$FILEEXT.spv"`
	WINDOWS_FILE=`cygpath -w $file`
	$GLSL_COMPILER $WINDOWS_FILE -o $SPV_FILENAME --target-env=opengl
done
