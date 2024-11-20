#!/bin/bash
set -o xtrace
set -o errexit
set -o verbose

for mount in /Volumes/mongodb-odbc*; do
	hdiutil detach "$mount" || true
done

dmg_file="mongo-odbc-driver/mongodb-odbc-driver/artifacts/pkg/mongodb-odbc.dmg"

hdiutil attach $dmg_file

curl -LO https://macos-notary-1628249594.s3.amazonaws.com/releases/client/v3.9.0/darwin_amd64.zip
unzip darwin_amd64.zip
chmod 0755 ./darwin_amd64/macnotary

./darwin_amd64/macnotary -v

MDBODBC_VER=$(cat "mongo-odbc-driver/mongodb-odbc-driver/bin/VERSION.txt")

pkg_name="mongodb-connector-odbc-$MDBODBC_VER-macos-x86-64.pkg"
pkg_path="/Volumes/mongodb-odbc/$pkg_name"
zip_file="mongodb-odbc.zip"

zip -j $zip_file $pkg_path

mkdir ./dmg-contents

./darwin_amd64/macnotary \
	--file "$zip_file" \
  --task-comment "Signing the ODBC Driver DMG" \
  --mode sign \
  --url https://dev.macos-notary.build.10gen.cc/api \
  --key-id $MACOS_NOTARY_KEY \
  --secret $MACOS_NOTARY_SECRET \
  --bundleId com.mongodb.odbc \
  --artifact-type "pkg" \
  --out-path "./dmg-contents/$pkg_name"

hdiutil detach /Volumes/mongodb-odbc

hdiutil create -fs HFS+ -srcfolder dmg-contents -volname mongodb-odbc mongo-odbc-driver/mongodb-odbc-driver/artifacts/pkg/mongodb-odbc-signed.dmg
