#!/bin/sh

IMAGE_DIR=image
CONTENTS_DIR=$IMAGE_DIR/ngplant.app/Contents
CONTENTS_MACOS_DIR=$CONTENTS_DIR/MacOS

mkdir -p   $CONTENTS_DIR
mkdir -p   $CONTENTS_MACOS_DIR
cp ngplant $CONTENTS_MACOS_DIR
cp Info.plist $CONTENTS_DIR
pushd $IMAGE_DIR
ln -s /Applications Applications
popd

hdiutil create -srcfolder image ngPlant-0.9.8.dmg

