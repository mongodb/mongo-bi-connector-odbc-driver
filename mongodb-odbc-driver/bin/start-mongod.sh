#!/bin/bash

. "$(dirname $0)/platforms.sh"
. "$(dirname $0)/prepare-shell.sh"

(
    set -o errexit

    cd mongodb-odbc-driver/bin
    echo "starting mongodb..."

    MONGODB_VERSION=${MONGODB_VERSION:-latest}

    DIR=$(dirname $0)
    echo $PWD
    echo $DIR
    # Functions to fetch MongoDB binaries
    . $DIR/download-mongodb.sh

    get_distro
    get_mongodb_download_url_for "$DISTRO" "$MONGODB_VERSION"
    echo "downloading mongodb $MONGODB_VERSION for $DISTRO..."
    set_mongodb_binaries "$MONGODB_DOWNLOAD_URL" "$EXTRACT" "$MONGODB_VERSION"
    echo "done downloading mongodb"

    mkdir -p "$ARTIFACTS_DIR/mlaunch"
    cd "$ARTIFACTS_DIR/mlaunch"

    if [ "$VARIANT" = '' ]; then
      mlaunch_cache_dir="./mlaunch"
      mkdir -p "$mlaunch_cache_dir"
      cd "$mlaunch_cache_dir"
    fi

    venv='venv'
    if [ "$VARIANT" = 'centos6-perf' ]; then
      venv="$PROJECT_DIR/../../../../venv"
    fi

    PYTHON_EXEC=python
    if command -v python3; then
        PYTHON_EXEC=python2
    fi

    # Setup or use the existing virtualenv for mtools
    if [ -f "$venv"/bin/activate ]; then
      echo 'using existing virtualenv'
      . "$venv"/bin/activate
    elif [ -f "$venv"/Scripts/activate ]; then
      echo 'using existing virtualenv'
      . "$venv"/Scripts/activate
    elif virtualenv "$venv" || "$PYTHON_EXEC" -m virtualenv "$venv"; then
      echo 'creating new virtualenv'
      if [ -f "$venv"/bin/activate ]; then
        . "$venv"/bin/activate
      elif [ -f "$venv"/Scripts/activate ]; then
        . "$venv"/Scripts/activate
      fi

      echo 'cloning mtools...'
      rm -rf mtools
      git clone git@github.com:rueckstiess/mtools
      cd mtools
      git checkout bd116d10bc01e4394269ea9be08d548a674711da
      echo 'installing mtools...'
      pip install .[mlaunch]
      pip freeze
      echo 'done installing mtools'
    fi

    cd "$ARTIFACTS_DIR/mlaunch"

    mlaunch_args=''
    mlaunch_args="$mlaunch_args --verbose"
    mlaunch_args="$mlaunch_args --binarypath $ARTIFACTS_DIR/mongodb/bin"

    mlaunch_init_args="$mlaunch_args --single"
    echo "mlaunch init args: $mlaunch_init_args"

    mlaunch init $mlaunch_init_args
    echo "sleeping for a bit"
    sleep 30
    echo "done sleeping"
    mlaunch list $mlaunch_args

    echo "done starting mongodb"

) > $LOG_FILE 2>&1

print_exit_msg
