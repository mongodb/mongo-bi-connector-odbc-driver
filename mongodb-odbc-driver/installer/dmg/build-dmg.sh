#!/bin/sh

rm ./*.dmg
rm dmg-contents/*.pkg || true

ROOT=/Library/MongoDB/ODBC/$MDBODBC_VER
mkdir -p components/"$ROOT"
mv ./*.so components/"$ROOT"/
mv ./*.dylib components/"$ROOT"/
cp ./resources/*.rtf components/"$ROOT"/

# build component pkg
pkgbuild --root=components/ --scripts=scripts/ --identifier='MongoDB ODBC' 'mongodb-odbc-component.pkg'

# set the version based on VERSION.txt (stored in $MDBODBC_VER)
sed -i '.bak' "s|__VERSION__|$MDBODBC_VER|g" distribution.xml

PRODUCT="$PREFIX".pkg
# build product pkg (which can install multiple component pkgs, but we only have one)
productbuild --distribution distribution.xml \
	--resources ./resources \
	--package-path . \
	"$PRODUCT"

mkdir -p dmg-contents

mv "$PRODUCT" dmg-contents/

hdiutil create -fs HFS+ -srcfolder dmg-contents -volname mongodb-odbc mongodb-odbc.dmg
