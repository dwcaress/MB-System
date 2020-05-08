#!/usr/bin/env bash
set -ex

if [ -e test/utilities/test-suite.log ]; then
    cat test/utilities/test-suite.log
fi
echo -------------------
echo Test logs:
echo -------------------
for i in test/utilities/*_test.log
do
  echo -------------------
  cat $i
done
echo -------------------
