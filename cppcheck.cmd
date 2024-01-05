echo "Running cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive on MB-System source code"

echo src/bsio > includes.txt
echo src/deprecated >> includes.txt
echo src/gmt >> includes.txt
echo src/gsf >> includes.txt
echo src/mbaux >> includes.txt
echo src/mbedit >> includes.txt
echo src/mbeditviz >> includes.txt
echo src/mbgrd2gltf >> includes.txt
echo src/mbgrdviz >> includes.txt
echo src/mbio >> includes.txt
echo src/mbnavadjust >> includes.txt
echo src/mbnavedit >> includes.txt
echo src/mbtrn >> includes.txt
echo src/mbtrnav >> includes.txt
echo src/mbtrnframe >> includes.txt
echo src/mbtrnutils >> includes.txt
echo src/mbvelocitytool >> includes.txt
echo src/mbview >> includes.txt
echo src/otps >> includes.txt
echo src/photo >> includes.txt
echo src/surf >> includes.txt
echo src/utilities >> includes.txt
echo /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include >> includes.txt
echo /opt/local/include >> includes.txt

echo " "
echo "#-----------bsio"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/bsio/

echo " "
echo "#-----------gmt"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/gmt/

echo " "
echo "#-----------gsf"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/gsf/

echo " "
echo "#-----------mbaux"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbaux/

echo " "
echo "#-----------mbedit"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbedit/

echo " "
echo "#-----------mbeditviz"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbeditviz/

echo " "
echo "#-----------mbgrdviz"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbgrdviz/

echo " "
echo "#-----------mbio"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbio/

echo " "
echo "#-----------mbnavadjust"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbnavadjust/

echo " "
echo "#-----------mbnavedit"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbnavedit/

echo " "
echo "#-----------mbvelocitytool"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbvelocitytool/

echo " "
echo "#-----------mbview"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/mbview/

echo " "
echo "#-----------otps"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/otps/

echo " "
echo "#-----------proj"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/proj/

echo " "
echo "#-----------surf"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/surf/

echo " "
echo "#-----------utilities"
cppcheck --enable=all --force --std=c11 --std=c++11 --includes-file=includes.txt --inconclusive src/utilities/

rm includes.txt


