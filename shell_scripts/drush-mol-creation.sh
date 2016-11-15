#!/bin/bash
# Creates an individual MOL file.
drush -u 1 -v iccm --filepath=$1 | tee -a "$2-drush-output.log"
INGEST_EXIT_CODE=$?
exit $INGEST_EXIT_CODE