#!/bin/bash
#           "$iusql_bin" \
#            "" \
#            "local" \
#            "127.0.0.1" \
#            "3307" \
#            "user_not_used" \
#            "password_not_used"

. "$(dirname $0)/prepare-shell.sh"

TEST_BIN="$1"
# iodbctestw expects DSN= before the DSN name, iusql expects nothing
BIN_ARG_PREFIX="$2"
CASE="$3"
shift 3
BIC_SERVER="$1";
BIC_PORT="$2";

db=kerberos

default_args=("Server=$BIC_SERVER" "Port=$BIC_PORT")

# Info on odbc.ini can be found here:
# https://bit.ly/2qRxuov
echo "export ODBCINI="$ARTIFACTS_DIR"/tempODBC.ini"
export ODBCINI="$ARTIFACTS_DIR"/tempODBC.ini

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

test_connect() {
    query="select authenticated from kerberos.test"
    FAILED_WITH=""
    for driver in libmdbodbcw.so; do
    #    for driver in libmdbodbca.so libmdbodbcw.so; do
        add_odbc_dsn "$dsn" "$driver" "Database=$db" "$@"
        echo "ODBCINI = $ODBCINI"
        echo "--------------------------------------------------"
        echo "---------------- ODBCINI content -----------------"
        less $ODBCINI
        echo "--------------------------------------------------"

        set +o errexit
        set -x
              out="$(echo "$query" | "$TEST_BIN" "$BIN_ARG_PREFIX""$dsn" 2> /dev/null)"
        set +x
        set -o errexit

        echo "------------ Query result ------------------"
        echo "$out"
        echo "-------------------------------------------"

        if [[ $out = *"yeah"* ]]; then
            continue
        else
            echo "......test '$testname' FAILED: connection was rejected"
            FAILED_WITH+=" $driver"
        fi
    done



    if [[ -z "$FAILED_WITH" ]]; then
      echo "......test '$testname' SUCCEEDED"
    else
       echo "......test '$testname' FAILED"
       #exit 1
    fi
}

(
    set -o errexit

    if [ "$GSSAPI_USER" = '' ]; then
        echo 'GSSAPI_USER environment variable not set'
        GSSAPI_USER="drivers"
        #exit 1
    fi
    if [ "$GSSAPI_PASSWD" = '' ]; then
        echo 'GSSAPI_PASSWD environment variable not set'
        GSSAPI_PASSWD="powerbook17"
        #exit 1
    fi


    GSSAPI_KTNAME="$PROJECT_DIR"/mongodb-odbc-driver/resources/gssapi/drivers.keytab
    if [ ! -f "$GSSAPI_KTNAME" ]; then
        echo "could not find file specified in GSSAPI_KTNAME $GSSAPI_KTNAME"
        exit 1
    fi

    #if [ "KRB5_KTNAME" = '' ]; then
    #    echo 'KRB5_KTNAME environment variable not set'
    #    echo "PROJECT_DIR = $PROJECT_DIR"
    #   export KRB5_KTNAME="$PROJECT_DIR"/mongodb-odbc-driver/resources/gssapi/mongosqlsvc.keytab
    #    echo "$KRB5_KTNAME"
        #exit 1
    #fi
    #if [ ! -f "$KRB5_KTNAME" ]; then
    #    echo "could not find file specified in KRB5_KTNAME $KRB5_KTNAME"
    #    exit 1
    #fi

    export KRB5_TRACE="${ARTIFACTS_DIR}/log/krb5.log"
    echo "KRB5_TRACE = $KRB5_TRACE"

    #export KRB5_CONFIG="${PROJECT_ROOT}/mongodb-odbc-driver/resources/gssapi/krb5.conf"
    #echo "KRB5_CONFIG = $KRB5_CONFIG"

    principal="$GSSAPI_USER@LDAPTEST.10GEN.CC"
    password=""

    # Test auth after kinit
    # Cache a delegated credential from the default keytab
    echo "Running test with credentials cache"
    kinit -k -t $GSSAPI_KTNAME -f $principal
    dsn="GSSAPI_PrincipalWithCredCache"
    test_connect "User=$principal?mechanism=GSSAPI&serviceName=$GSSAPI_SERVICENAME&source=\$external"

    # Test providing password
    kdestroy
    echo "Running test with username-password"
    password="$GSSAPI_PASSWD"
    dsn="GSSAPI_UsernamePassword"
    test_connect "User=$principal?mechanism=GSSAPI&serviceName=$GSSAPI_SERVICENAME&source=\$external" "Password=$password"

    # Test providing a client keytab with an environment variable.
    # This test does not work on MacOS Heimdal Kerberos.
    # case $VARIANT in
    # ubuntu*|rhel*)
        # kdestroy
        # export KRB5_CLIENT_KTNAME=$GSSAPI_KTNAME
        # password=""
        # echo "running test with keytab environment variable"
        # test_connect
        # ;;
    # *)
        # echo "Skipping keytab test because variant is '$VARIANT'"
        # ;;
    # esac
) 2>&1 | tee $LOG_FILE

print_exit_msg
