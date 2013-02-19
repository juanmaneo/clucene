#!/bin/bash

APP_NAME="clucene"
APP_VERSION="2.3.3.4"
APP_DIR_NAME="${APP_NAME}-${APP_VERSION}"
APP_FILES="APACHE.license AUTHORS bin ChangeLog cmake CMakeLists.txt COPYING CTestTestfile.cmake doc INSTALL LGPL.license NEWS README README.PACKAGE REQUESTS src Testing "
TEMP_DIR="temp-build-deb"
SOURCE_ARCHIVE="${APP_NAME}-${APP_VERSION}.tar.gz"
SOURCE_ARCHIVE_ORIG="${APP_NAME}_${APP_VERSION}.orig.tar.gz"

echo '[*] Remove temp directory'
rm -rvf $TEMP_DIR >> /dev/null

echo '[*] Create temp directory'
mkdir $TEMP_DIR

#mkdir $TEMP_DIR/$APP_DIR_NAME

#cp -fr $APP_FILES debian $TEMP_DIR/ 

cd $TEMP_DIR

echo '[*] Copy source directory'
git clone .. $APP_DIR_NAME > /dev/null
#mv debian $APP_DIR_NAME

echo '[*] Create archive'
tar -zcf $SOURCE_ARCHIVE $APP_DIR_NAME/*

cp -v $SOURCE_ARCHIVE $SOURCE_ARCHIVE_ORIG

cd $APP_DIR_NAME

debuild

cd ..
echo '[*] Generated deb:' *.deb

echo '[*] Done'
