#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
	DMG_SIGN_DIR="$PROJECT_ROOT/mongodb-odbc-driver/dmg-sign"
	rm -Rf "$DMG_SIGN_DIR"
	mkdir -p "$DMG_SIGN_DIR"

	cd "$DMG_SIGN_DIR"
	cp -R "$PROJECT_ROOT/mongodb-odbc-driver/installer/dmg/dmg-contents" ./

	for mount in /Volumes/mongodb-odbc*; do
		hdiutil detach "$mount" || true
	done
	hdiutil attach "$PKG_DIR"/mongodb-odbc.dmg

	PKG="mongodb-connector-odbc-$MDBODBC_VER-macos-x86-64.pkg"
	cp /Volumes/mongodb-odbc/"$PKG" ./

	hdiutil detach /Volumes/mongodb-odbc

	# The developer ID will be passed as an argument to this script.
	productsign --sign "$1" "$PKG" ./dmg-contents/"$PKG"

	hdiutil create -fs HFS+ -srcfolder dmg-contents -volname mongodb-odbc mongodb-odbc-signed.dmg

	cp ./*.dmg "$PKG_DIR"/
) > $LOG_FILE 2>&1

print_exit_msg


) > $LOG_FILE 2>&1

print_exit_msg
