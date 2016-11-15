#!/bin/bash
# This script finds all files to be have MOLs created for.
TIMESTAMP=`date +"%Y-%m-%d_%H-%M-%S"`
FILE_PATH=${1:?No path specified.}
LOG_NAME="${2:-jobs}-$TIMESTAMP"
find $1 -mindepth 1 -maxdepth 1 -regex '^.*-MOL$' | parallel --joblog $LOG_NAME.log ./drush-fragment-creation.sh {} $LOG_NAME