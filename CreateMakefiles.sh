# #########################################################
# 
# \brief  Create Makefiles from qmake project files.
#
# 
# #########################################################

echo "+-----------------------"
echo "| Recreating Makefiles..."
echo "+-----------------------"
qmake

echo "MYODBCDbg..."
cd MYODBCDbg/MYODBCDbgLib
rm -f Makefile
qmake
cd ..

echo "dltest..."
cd dltest
rm -f Makefile
qmake
cd ..

echo "myodbc3u..."
cd myodbc3u
rm -f Makefile
qmake
cd ..

echo "setup..."
cd setup
rm -f Makefile
qmake
cd ..

echo "installer..."
cd installer
rm -f Makefile
qmake
cd ..

echo "monitor..."
cd monitor
rm -f Makefile
qmake
cd ..

echo "myodbc3c..."
cd myodbc3c
rm -f Makefile
qmake
cd ..

echo "myodbc3..."
cd myodbc3
rm -f Makefile
qmake
cd ..


