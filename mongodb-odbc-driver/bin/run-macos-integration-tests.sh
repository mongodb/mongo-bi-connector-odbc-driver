#!/usr/bin/env bash

#shellcheck source=./prepare-shell.sh
. $(dirname "$0")/prepare-shell.sh

CASE="$1"
shift
BIC_SERVER="$1";
BIC_PORT="$2";
BIC_USER="$3";
BIC_PASSWORD="$4";

db=H1B-Visa-Applications
if [ "$CASE" = "local" ]; then
    db=information_schema
fi
dsn=TestODBC
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
cat <<EOS > "$ODBCINI"
[ODBC Data Sources]
$1 = evg

[$1]
Driver   = $DRIVERS_DIR/$2
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
    query="select agent_attorney_city\
              from year2015\
              where _id = '572cbdd9d2fc210e7ce696ec'"
    if [ "$CASE" = "local" ]; then
        query="select CATALOG_NAME from\
            information_schema.schemata\
            where schema_name = 'mysql' limit 1"
    fi
    echo "...running $CASE connection test '$testname'"
    for driver in libmdbodbcw.so libmdbodbca.so; do
        add_odbc_dsn "$dsn" "$driver" "Database=$db" "$@"
        out="$(echo "$query" | iodbctestw "DSN=$dsn")"
        # idodbctest has poorly formatted output, just check to make
        # sure that the attorney city of AUSTIN is present for this _id.
        if [ "$CASE" = "atlas" ] && [[ $out = *"MINNEAPOLIS"* ]]; then
            continue
        elif [ "$CASE" = "local" ] && [[ $out = *"def"* ]]; then
            continue
        else
            echo "......test '$testname' FAILED: connection was rejected"
            exit 1
        fi
    done
    echo "......test '$testname' SUCCEEDED"
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
    query="select _id\
              from year2015\
              where _id = '572cbdd9d2fc210e7ce696ec'"
    if [ "$CASE" = "local" ]; then
        query="select CATALOG_NAME from\
            information_schema.schemata\
            where schema_name = 'mysql' limit 1"
    fi
    echo "...running $CASE negative connection test '$testname'"
    for driver in libmdbodbca.so libmdbodbcw.so; do
        add_odbc_dsn "$dsn" "$driver" "Database=$db" "$@"
        out="$(echo "$query" | iodbctestw "DSN=$dsn" 2> /dev/null)"
    done
    if [[ $out = *"result set 1 returned 1 rows"* ]]; then
            echo "......test '$testname' FAILED: expected connection to be rejected, but it was accepted"
            exit 1
    fi
    echo "......test '$testname' SUCCEEDED"
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

