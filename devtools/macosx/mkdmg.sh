#!/bin/sh

IMAGE_DIR=image
CONTENTS_DIR=$IMAGE_DIR/ngplant.app/Contents
CONTENTS_MACOS_DIR=$CONTENTS_DIR/MacOS
CONTENTS_RESOURCES_DIR=$CONTENTS_DIR/Resources

if [ $# != 1 ]; then
 echo "error: path to ngplant source directory required"
 exit 1
fi

NGPLANT_BASE="$1"

mkdir -p   $CONTENTS_DIR
mkdir -p   $CONTENTS_MACOS_DIR
mkdir -p   $CONTENTS_RESOURCES_DIR
cp "$NGPLANT_BASE"/ngplant/ngplant $CONTENTS_MACOS_DIR
cp "$NGPLANT_BASE"/devtools/macosx/Info.plist $CONTENTS_DIR
cp "$NGPLANT_BASE"/ngplant/images/icon.icns $CONTENTS_RESOURCES_DIR
cp -R "$NGPLANT_BASE"/plugins $CONTENTS_RESOURCES_DIR
pushd $IMAGE_DIR
ln -s /Applications Applications
popd

hdiutil create -srcfolder image ngPlant-0.9.11.dmg

