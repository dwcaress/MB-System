#!/usr/bin/env bash
set -ex

echo ------------------- >> test/utilities/test-suite.log
echo Test logs: >> test/utilities/test-suite.log
echo ------------------- >> test/utilities/test-suite.log
for i in test/utilities/*_test.log
do
  echo ------------------- >> test/utilities/test-suite.log
  cat $i >> test/utilities/test-suite.log
done
echo ------------------- >> test/utilities/test-suite.log


if [ -e test/utilities/test-suite.log ]; then
    cat test/utilities/test-suite.log
fi
