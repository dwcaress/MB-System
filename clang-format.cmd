# Script to run clang-format on the MB-System C source files
#   - The formatting is defined in the file _clang-format
#   - Using this tool and formatting was suggested by Joaquim Luis
#   - First applied on 3 June 2017 and the results were committed to the
#     SVN archive for revision 5.5.2307
#   David W. Caress
#
clang-format -i -style=file src/bsio/*.[ch]
clang-format -i -style=file src/gmt/*.[ch]
clang-format -i -style=file src/gsf/*.[ch]
clang-format -i -style=file src/mbaux/*.[ch]
clang-format -i -style=file src/mbedit/*.[ch]
clang-format -i -style=file src/mbeditviz/*.[ch]
clang-format -i -style=file src/mbgrdviz/*.[ch]
clang-format -i -style=file src/mbio/*.[ch]
clang-format -i -style=file src/mbnavadjust/*.[ch]
clang-format -i -style=file src/mbnavedit/*.[ch]
clang-format -i -style=file src/mbvelocitytool/*.[ch]
clang-format -i -style=file src/mbview/*.[ch]
clang-format -i -style=file src/otps/*.[ch]
clang-format -i -style=file src/proj/*.[ch]
clang-format -i -style=file src/utilities/*.[ch]
