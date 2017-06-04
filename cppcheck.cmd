echo "Running cppcheck --enable=all --force --std=c99 --inconclusive on MB-System source code"

echo " "
echo "#-----------bsio"
cppcheck --enable=all --force --std=c99 --inconclusive src/bsio/

echo " "
echo "#-----------gmt"
cppcheck --enable=all --force --std=c99 --inconclusive src/gmt/

echo " "
echo "#-----------gsf"
cppcheck --enable=all --force --std=c99 --inconclusive src/gsf/

echo " "
echo "#-----------mbaux"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbaux/

echo " "
echo "#-----------mbedit"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbedit/

echo " "
echo "#-----------mbeditviz"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbeditviz/

echo " "
echo "#-----------mbgrdviz"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbgrdviz/

echo " "
echo "#-----------mbio"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbio/

echo " "
echo "#-----------mbnavadjust"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbnavadjust/

echo " "
echo "#-----------mbnavedit"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbnavedit/

echo " "
echo "#-----------mbvelocitytool"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbvelocitytool/

echo " "
echo "#-----------mbview"
cppcheck --enable=all --force --std=c99 --inconclusive src/mbview/

echo " "
echo "#-----------otps"
cppcheck --enable=all --force --std=c99 --inconclusive src/otps/

echo " "
echo "#-----------proj"
cppcheck --enable=all --force --std=c99 --inconclusive src/proj/

echo " "
echo "#-----------surf"
cppcheck --enable=all --force --std=c99 --inconclusive src/surf/

echo " "
echo "#-----------utilities"
cppcheck --enable=all --force --std=c99 --inconclusive src/utilities/



