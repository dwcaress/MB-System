#!/bin/bash
# Add an id attribute to each behavior in a Dorado MAUV mission script.
# Assumes there currently are no id attributes assigned.
#
# Does NOT run correctly in QNX shell. Use Linux or macOS bash shell.
#
# Usage:
# add_id.sh orig-mission-file.cfg > new-mission-file.cfg
#

if [ "$#" -ne 1 ]; then
  echo "Usage:"
  echo "  mauv_add_ids.sh  orig-mission-file.cfg"
  echo "  mauv_add_ids.sh  orig-mission-file.cfg > new-mission-file.cfg"
  exit
fi

echo "# $1 with unique ids added to each behavior"
echo "# "

# id counter
id=0

# Read each line in the file
#
while IFS= read -r line; do
    # Pass each line through to stdout
    #
    echo $line

    # A '{' at the beginning of a line is the start of a new behavior,
    # so on the next line, at the id attribute
    #
    if [[ $line == {* ]]; then
      id=$((id + 1))
      echo "id = $id;"
    fi

done < "$1"
