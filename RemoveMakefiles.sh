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

echo "util..."
cd util
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

echo "monitor..."
cd monitor
rm -f Makefile.in
rm -f Makefile
cd ..

echo "dsn-editor..."
cd dsn-editor
rm -f Makefile.in
rm -f Makefile
cd ..

echo "driver..."
cd driver
rm -f Makefile.in
rm -f Makefile
cd ..


