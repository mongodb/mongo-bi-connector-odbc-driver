#!/usr/bin/env bash

#shellcheck source=./prepare-shell.sh
. $(dirname "$0")/prepare-shell.sh

# if we are on macos, make sure we copy the openssl libraries to the same directory as the driver
# files.
if [ "$PLATFORM_NAME" = "macos" ]; then
    curl -O https://mongo-bic-odbc-driver-resources.s3.amazonaws.com/macos/openssl-1.0.2n.zip
    unzip openssl-1.0.2n.zip
    OPENSSL_PATH="./1.0.2n/lib"
    chmod 777 "$OPENSSL_PATH/libssl.1.0.0.dylib"
    chmod 777 "$OPENSSL_PATH/libcrypto.1.0.0.dylib"
    install_name_tool -change "/usr/local/Cellar/openssl/1.0.2n/lib/libcrypto.1.0.0.dylib" "@loader_path/libcrypto.1.0.0.dylib" "$OPENSSL_PATH/libssl.1.0.0.dylib"
    cp "$OPENSSL_PATH/libssl.1.0.0.dylib"  "$DRIVERS_DIR"/ || exit "could not find libssl"
    cp "$OPENSSL_PATH/libcrypto.1.0.0.dylib"  "$DRIVERS_DIR"/ || exit "could not find libcrypto"
fi

TEST_BIN="$1"
# iodbctestw expects DSN= before the DSN name, iusql expects nothing
BIN_ARG_PREFIX="$2"
CASE="$3"
shift 3
BIC_SERVER="$1";
BIC_PORT="$2";
BIC_USER="$3";
BIC_PASSWORD="$4";

db=test
if [ "$CASE" = "local" ]; then
db=information_schema
fi

CA_PATH="$SCRIPT_DIR/../resources"
digicertCA="$CA_PATH/digicert.pem"
invalidCA="$CA_PATH/invalid.pem"
localCA="$CA_PATH/local_ca.pem"

default_args=("Server=$BIC_SERVER" "Port=$BIC_PORT" "User=$BIC_USER" "Password=$BIC_PASSWORD")

# Info on odbc.ini can be found here:
# https://bit.ly/2qRxuov
export ODBCINI="$ARTIFACTS_DIR"/tempODBC.ini

#libmdbodbca.so  libmdbodbcw.so
function add_odbc_dsn {

DRIVER_PATH="$DRIVERS_DIR/$2"
if [ ! -f $DRIVER_PATH ]; then
  echo "!!!! No driver found at path : $DRIVER_PATH"
  exit 1
else
  echo "Driver $DRIVER_PATH found"
fi

cat <<EOS > "$ODBCINI"
[ODBC Data Sources]
$1 = evg

[$1]
Driver   = $DRIVER_PATH
EOS
    shift 2
    for arg in "$@"; do
        # failure test configurations include an error message; we do not want
        # to write the error message to the odbc.ini, thus we only echo args
        # with '=' in them.
        if [[ "$arg" = *"="* ]]; then
            echo "$arg" >> "$ODBCINI"
        fi
    done

    for default in "${default_args[@]}"; do
        echo "$default" >> "$ODBCINI"
    done
}

function test_connect_success {
    testname=$1; shift
    dsn=$testname
    query="select message from greeting where _id = '5c64a48d1c9d44000046008d'"
    if [ "$CASE" = "local" ]; then
        query="select CATALOG_NAME from\
            information_schema.schemata\
            where schema_name = 'mysql' limit 1"
    fi
    echo "...running $CASE connection test '$testname'"
    FAILED_WITH=""
    for driver in libmdbodbcw.so libmdbodbca.so; do
        add_odbc_dsn "$dsn" "$driver" "Database=$db" "$@"
        echo "----- Is a driver in the expected directory? -----"
        ls -lrt $DRIVERS_DIR
        echo "--------------------------------------------------"
        echo "----- Is ODBCINI properly set? -----"
        echo "$ODBCINI"
        echo "--------------------------------------------------"
        echo "---------------- ODBCINI content -----------------"
        less $ODBCINI
        echo "--------------------------------------------------"
	set +o errexit
        out="$(echo "$query" | "$TEST_BIN" "$BIN_ARG_PREFIX""$dsn")"
	set -o errexit
        if [ "$CASE" = "atlas" ] && [[ $out = *"Hello, world!"* ]]; then
            continue
        elif [ "$CASE" = "local" ] && [[ $out = *"def"* ]]; then
            continue
        else
            echo "......test '$testname' FAILED: connection was rejected"
            FAILED_WITH+=" $driver"
            #exit 1
        fi
    done
    if [[ -z "$FAILED_WITH" ]]; then
      echo "......test '$testname' SUCCEEDED"
    else
      echo "......test '$testname' FAILED"
      #exit 1
    fi
}

function test_connect_failure {
    testname=$1; shift
    # unfortunately, we only ever get one error message if we fail to connect
    # on macos:
    #
    # "Prompting is not supported on this platform. Please provide all required
    # connect information. (0) SQLSTATE=HY000"
    #
    # Thus, checking for specific errors
    # like on Windows is not possible. This is only a problem with iodbctest,
    # but we do not have another means to test this.
    query="select message from greeting where _id = '5c64a48d1c9d44000046008d'"
    if [ "$CASE" = "local" ]; then
        query="select CATALOG_NAME from information_schema.schemata where schema_name = 'mysql' limit 1"
    fi
    echo "...running $CASE negative connection test '$testname'"
    FAILED_WITH=""
    for driver in libmdbodbca.so libmdbodbcw.so; do
        add_odbc_dsn "$dsn" "$driver" "Database=$db" "$@"
	set +o errexit
        out="$(echo "$query" | "$TEST_BIN" "$BIN_ARG_PREFIX""$dsn" 2> /dev/null)"
	set -o errexit
      if [[ $out = *"result set 1 returned 1 rows"* ]]; then
          echo "......test '$testname' FAILED: expected connection to be rejected, but it was accepted"
          FAILED_WITH+=" $driver"
          #exit 1
      fi
    done
    if [[ -z "$FAILED_WITH" ]]; then
      echo "......test '$testname' SUCCEEDED"
    else
       echo "......test '$testname' FAILED"
       #exit 1
    fi
}

if [ "$CASE" = "atlas" ]; then
    echo 'Tests that should successfully connect...'
    while read -r line
    do
        eval "test_connect_success $line"
    done < "$SCRIPT_DIR/atlas-integration-test-success-cases".tsv

    echo 'Tests that should fail to connect...'
    while read -r line
    do
        eval "test_connect_failure $line"
    done < "$SCRIPT_DIR/atlas-integration-test-fail-cases".tsv
fi

if [ "$CASE" = "local" ]; then
    if [ "$TEST_SET" = 'SSL' ]; then
        echo 'Tests that should successfully connect...'
        while read -r line
        do
            eval "test_connect_success $line"
        done < "$SCRIPT_DIR/local_ssl-integration-test-success-cases".tsv

        echo 'Tests that should fail to connect...'
        while read -r line
        do
            eval "test_connect_failure $line"
        done < "$SCRIPT_DIR/local_ssl-integration-test-fail-cases".tsv
    else
        echo 'Tests that should successfully connect...'
        while read -r line
        do
            eval "test_connect_success $line"
        done < "$SCRIPT_DIR/local-integration-test-success-cases".tsv
    fi
fi
