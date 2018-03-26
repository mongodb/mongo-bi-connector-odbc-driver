#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc., licensed under
# GNU GENERAL PUBLIC LICENSE Version 2.

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

# clear MSI_BUILD_DIR
rm -rf "$MSI_BUILD_DIR"
mkdir -p "$MSI_BUILD_DIR"

cd "$MSI_BUILD_DIR"

# copy msi sources to current directory
cp -R "$PROJECT_ROOT/mongo-odbc-driver/installer/msi/"* ./

# copy driver libraries to appropriate location
cp "$DRIVERS_DIR"/* ./

if [ "$PLATFORM_ARCH" = "64" ]; then
       arch="x64"
else
       arch="x86"
fi

powershell.exe \
	    -NoProfile \
	    -NoLogo \
	    -NonInteractive \
	    -File ./build-msi.ps1 \
	    -Arch "$arch"

# clear PKG_DIR
rm -rf "$PKG_DIR"
mkdir -p "$PKG_DIR"

# copy the msi to PKG_DIR
cp release.msi "$PKG_DIR"/mongo-odbc.msi
