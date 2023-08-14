#!/bin/bash

. "$(dirname $0)/platforms.sh"
. "$(dirname $0)/prepare-shell.sh"

(
    set -o errexit
    echo "starting mongodb..."
    cd mongodb-odbc-driver/bin

    DIR=$(dirname $0)
    echo $PWD
    echo $DIR

    MONGODB_VERSION=${MONGODB_VERSION:-latest}

    DIR=$(dirname $0)
    # Functions to fetch MongoDB binaries
    . $DIR/download-mongodb.sh

    get_distro
    get_mongodb_download_url_for "$DISTRO" "$MONGODB_VERSION"
    echo "downloading mongodb $MONGODB_VERSION for $DISTRO..."
    set_mongodb_binaries "$MONGODB_DOWNLOAD_URL" "$EXTRACT" "$MONGODB_VERSION"

    echo "done downloading mongodb"

    PYTHON_EXEC=python
    if command -v python3 &> /dev/null; then
        echo "Has python3 ?: $(python3 --version 2>&1)"

        PYTHON_VERSION=$($PYTHON_EXEC --version 2>&1)
        echo "Current python version: $PYTHON_VERSION"
        PYTHON_MAJOR_VERSION="$(echo $PYTHON_VERSION | cut -d ' ' -f 2 | cut -d '.' -f 1)"
        echo "Current python major version: $PYTHON_MAJOR_VERSION"
        PYTHON_MINOR_VERSION="$(echo $PYTHON_VERSION | cut -d ' ' -f 2 | cut -d '.' -f 2)"
        echo "Current python minor version: $PYTHON_MINOR_VERSION"
        # We need to use Python2 or Python 3.7+, the earlier version of mtools with Python 3 are buggy
        if [[ "$PYTHON_MAJOR_VERSION" -lt "3" ]] || [[ "$PYTHON_MAJOR_VERSION" -eq "3" && "$PYTHON_MINOR_VERSION" -lt "7" ]] ; then
            # The last Python2 compatible version of mlaunch
            CHECKOUT_VERSION="e544bbced1a070d7024931e7c1736ced7d9bcdd6"
            # Make sure that we use Python2 if `python` points to Python 3.7+
            if [ -x "$(command -v python2)" ]; then
                PYTHON_EXEC="python2"
            fi
        fi
    fi

    echo "Used python version: $($PYTHON_EXEC --version 2>&1)"

    mkdir -p "$ARTIFACTS_DIR/mlaunch"
    cd "$ARTIFACTS_DIR/mlaunch"

    if [ "$VARIANT" = '' ]; then
      mlaunch_cache_dir="./mlaunch"
      mkdir -p "$mlaunch_cache_dir"
      cd "$mlaunch_cache_dir"
    fi

    echo 'cloning mtools...'
    rm -rf mtools
    git clone https://github.com/rueckstiess/mtools.git
    cd mtools

    # We should avoid checking out the master branch because it is a dev branch
    # that has occasionally had bugs committed.
    if [[ ! -z "$CHECKOUT_VERSION" ]] ; then
      echo "Checking out : $CHECKOUT_VERSION"
      git checkout $CHECKOUT_VERSION
    else
      # Check-out the lastest tag
      git fetch --tags
      latestTag=$(git describe --tags `git rev-list --tags --max-count=1`)
      echo "Checking out : $latestTag"
      git checkout $latestTag
    fi

    venv='venv'
    if [ "$VARIANT" = 'centos6-perf' ]; then
      venv="$PROJECT_DIR/../../../../venv"
    fi

    # Setup or use the existing virtualenv for mtools
    if [ -f "$venv"/bin/activate ]; then
      echo 'using existing virtualenv'
      . "$venv"/bin/activate
    elif [ -f "$venv"/Scripts/activate ]; then
      echo 'using existing virtualenv'
      . "$venv"/Scripts/activate
    elif virtualenv --python=$PYTHON_EXEC "$venv" || $PYTHON_EXEC -m virtualenv "$venv"; then
      echo 'creating new virtualenv'
      if [ -f "$venv"/bin/activate ]; then
        . "$venv"/bin/activate
      elif [ -f "$venv"/Scripts/activate ]; then
        . "$venv"/Scripts/activate
      fi

      PYMONGO_VERSION=$(pip show pymongo | grep "Version:" | cut -d ' ' -f 2)
      echo "Pymongo version: $PYMONGO_VERSION"
      # Make sure to install the correct requirement for mtools
      PYMONGO_REQ=$(less requirements.txt | grep "pymongo")
      PSUTIL_REQ=$(less requirements.txt | grep "psutil")
      echo "Executing: pip install $PYMONGO_REQ"
      output=$(pip install $PYMONGO_REQ 2>&1)
      code=$?
      if [ "$code" != "0" ]; then
        echo "$output"
        # The required pymongo version is not available for this platform
        exit $code
      fi

      echo "Executing: pip install $PSUTIL_REQ"
      output=$(pip install $PSUTIL_REQ)
      code=$?
      if [ "$code" != "0" ]; then
        echo "$output"
        # The required psutil version is not available for this platform
        exit $code
      fi

      echo 'installing mtools...'
      pip install .[mlaunch]
      pip freeze
      echo 'done installing mtools'
    fi

    cd "$ARTIFACTS_DIR/mlaunch"

    mlaunch_args="--binarypath $ARTIFACTS_DIR/mongodb/bin"
    mlaunch_init_args="$mlaunch_args"

    echo "mlaunch init args: $mlaunch_init_args"

    mlaunch init $mlaunch_init_args
    mlaunch list $mlaunch_args

    echo "done starting mongodb"
    sleep 30

) > $LOG_FILE 2>&1

print_exit_msg
