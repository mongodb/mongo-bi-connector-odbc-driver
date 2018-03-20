# Copyright (c) 2018-Present MongoDB Inc., licensed under
# GNU GENERAL PUBLIC LICENSE Version 2.
if [ "$PLATFORM" = "" ]; then
    PLATFORM=win64
    echo "WARNING: no value provided for \$PLATFORM: using default of '$PLATFORM'"
fi

case "$PLATFORM" in
win64)
    PLATFORM_ARCH='64'
    PLATFORM_NAME='windows'
    ;;
win32)
    PLATFORM_ARCH='32'
    PLATFORM_NAME='windows'
    ;;
*)
    echo "ERROR: invalid value for \$PLATFORM: '$PLATFORM'"
    echo "Allowed values: 'win64', 'win32'"
    exit 1
    ;;
esac
