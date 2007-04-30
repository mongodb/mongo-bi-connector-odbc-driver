# #########################################################
# 
# \brief  Create Makefiles from qmake project files.
# 
# #########################################################

echo "+-----------------------"
echo "| Recreating Makefiles..."
echo "+-----------------------"

dirs="\
.
MYODBCDbg/MYODBCDbgLib \
dltest \
util \
setup \
installer \
monitor \
dsn-editor \
driver \
test \
"

for d in $dirs
do
  echo "$d..."
  (cd $d; rm -f Makefile; qmake "$@")
done
