#!/usr/bin/env bash
#shellcheck source=./prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

	mkdir -p "$BUILD_DIR"
	cd "$BUILD_DIR"
	echo "downloading iODBC"
	curl -O "http://noexpire.s3.amazonaws.com/sqlproxy/binary/linux/$IODBC_VERSION.tar.gz" \
       --silent \
       --fail \
       --max-time 60 \
       --retry 5 \
       --retry-delay 0
	tar xf "$IODBC_VERSION.tar.gz"
	cd "$IODBC_BUILD_DIR"
	# we need to modify the Makefile
	mv GNUMakefile old_gnumakefile
	echo 'PROJBUILD := /usr/bin/xcodebuild -configuration Deployment' > GNUMakefile
	# shellcheck disable=SC2016
    echo 'YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY'
	echo 'IODBC_TARGET := $(shell sw_vers -productVersion | cut -d . -f 1-2)'
    echo 'YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY'
	echo 'IODBC_TARGET := $(shell sw_vers -productVersion | cut -d . -f 1-2)' >> GNUMakefile
	cat old_gnumakefile >> GNUMakefile
	rm old_gnumakefile
	echo 'rewrite framework locations'
	export LC_CTYPE=C
	export LANG=C
	find . -type f -print0 | xargs -0 -n1 sed -i '.bak' "s|/Library/Frameworks|$BUILD_DIR/Library/Frameworks|g"
	echo "building iODBC"
	make -f GNUMakefile -j 4 all install || true
    cat GNUMakefile
    find ../.. | grep -i iodbc

print_exit_msg
