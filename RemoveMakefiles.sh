# #########################################################
# 
# \brief  Remove makefiles so gnu auto-tools will create new ones.
#
# 
# #########################################################

echo "+-----------------------"
echo "| Removing Makefiles..."
echo "+-----------------------"

echo "dltest..."
cd dltest
rm -f Makefile.in
rm -f Makefile
cd ..

echo "myodbc3u..."
cd myodbc3u
rm -f Makefile.in
rm -f Makefile
cd ..

echo "setup..."
cd setup
rm -f Makefile.in
rm -f Makefile
cd ..

echo "installer..."
cd installer
rm -f Makefile.in
rm -f Makefile
cd ..

echo "myodbc3m..."
cd myodbc3m
rm -f Makefile.in
rm -f Makefile
cd ..

echo "myodbc3c..."
cd myodbc3c
rm -f Makefile.in
rm -f Makefile
cd ..

echo "myodbc3..."
cd myodbc3
rm -f Makefile.in
rm -f Makefile
cd ..


