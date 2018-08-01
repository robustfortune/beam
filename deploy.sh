#!/bin/bash

cd $HOME

REPO=https://${GITHUB_API_KEY}@github.com/beam-mw/beam-builds

git config --global user.email "vadim@beam-mw.com"
git config --global user.name "Vadim Grigoruk"
git clone $REPO
cd beam-builds

# create folders for the build
APP=beam
DATE_FOLDER=nightly/$(date +%Y.%m.%d)
RELEASE_FOLDER=$DATE_FOLDER/release

if [[ "$OSTYPE" == "linux"* ]]; then
    OS_FOLDER=$RELEASE_FOLDER/linux
    LATEST_OS_FOLDER=latest/release/linux
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS_FOLDER=$RELEASE_FOLDER/mac
    LATEST_OS_FOLDER=latest/release/mac
else
    echo "Error, OS:$OSTYPE not supported"
    exit 1
fi

mkdir -p $OS_FOLDER
mkdir -p $LATEST_OS_FOLDER

tar -cvf $OS_FOLDER/$APP.tar --directory=$HOME/build/beam-mw/beam/beam beam beam.cfg
tar -rvf $OS_FOLDER/$APP.tar --directory=$HOME/build/beam-mw/beam/ui beam-ui.app beam-ui.cfg
tar -czvf $OS_FOLDER/$APP.tar.gz --directory=$OS_FOLDER $APP.tar

cp -f $OS_FOLDER/$APP.tar.gz $LATEST_OS_FOLDER/$APP.tar.gz

git add $OS_FOLDER/$APP.tar.gz $LATEST_OS_FOLDER/$APP.tar.gz

if [[ "$OSTYPE" == "darwin"* ]]; then
	cp -f $HOME/build/beam-mw/beam/Beam-0.0.1-Darwin.dmg $OS_FOLDER/$APP-setup.dmg
	cp -f $OS_FOLDER/$APP-setup.dmg $LATEST_OS_FOLDER/$APP-setup.dmg
	git add $OS_FOLDER/$APP-setup.dmg $LATEST_OS_FOLDER/$APP-setup.dmg
fi

git commit -m "Travis build $TRAVIS_BUILD_NUMBER on $OSTYPE"
git push $REPO master

echo "Done"
