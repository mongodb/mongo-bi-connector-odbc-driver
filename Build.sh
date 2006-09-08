# #########################################################
# 
# \brief  Make the sources.
#
#
# #########################################################

echo "+-----------------------"
echo "| Force relink..."
echo "+-----------------------"
rm -rf lib
rm -rf bin

./CreateMakefiles.sh

echo "+-----------------------"
echo "| Making the libraries and executables..."
echo "+-----------------------"
make


