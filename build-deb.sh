#!/bin/bash

APP_NAME="clucene"
APP_VERSION="2.3.3.4"
APP_DIR_NAME="${APP_NAME}-${APP_VERSION}"
APP_FILES="APACHE.license AUTHORS bin ChangeLog cmake CMakeLists.txt COPYING CTestTestfile.cmake doc INSTALL LGPL.license NEWS README README.PACKAGE REQUESTS src Testing "
TEMP_DIR="temp-build-deb"
SOURCE_ARCHIVE="${APP_NAME}-${APP_VERSION}.tar.gz"
SOURCE_ARCHIVE_ORIG="${APP_NAME}_${APP_VERSION}.orig.tar.gz"

rm -rvf $TEMP_DIR >> /dev/null

mkdir $TEMP_DIR

#mkdir $TEMP_DIR/$APP_DIR_NAME

cp -fvr $APP_FILES debian $TEMP_DIR/ #>> /dev/null

cd $TEMP_DIR

git clone .. $APP_DIR_NAME
mv debian $APP_DIR_NAME


tar -zcvf $SOURCE_ARCHIVE $APP_DIR_NAME/* #>> /dev/null

cp -v $SOURCE_ARCHIVE $SOURCE_ARCHIVE_ORIG  #>> /dev/null

cd $APP_DIR_NAME

debuild

cd ..
echo "[*] Generated deb:" *.deb

echo "[*] Done"
