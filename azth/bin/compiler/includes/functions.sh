
function clean() {
  echo "Cleaning build files"

  CWD=$(pwd)

  cd $BUILDPATH

  make -f Makefile clean
  make clean
  find -iname '*cmake*' -not -name CMakeLists.txt -exec rm -rf {} \+

  cd $CWD
}

function configure() {
  CWD=$(pwd)

  cd $BUILDPATH

  echo "Build path: $BUILDPATH"
  echo "DEBUG info: $CDEBUG"
  echo "Compilation type: $CCTYPE"
  # -DCMAKE_BUILD_TYPE=$CCTYPE disable optimization "slow and huge amount of ram"
  # -DWITH_COREDEBUG=$CDEBUG compiled with debug information

  cmake $SRCPATH -DCMAKE_INSTALL_PREFIX=$BINPATH -DCONF_DIR=$CONFDIR -DSERVERS=$CSERVERS -DSCRIPTS=$CSCRIPTS \
  -DTOOLS=$CTOOLS -DUSE_SCRIPTPCH=$CSCRIPTPCH -DUSE_COREPCH=$CCOREPCH -DWITH_COREDEBUG=$CDEBUG  -DCMAKE_BUILD_TYPE=$CCTYPE -DWITH_WARNINGS=$CWARNINGS \
  -DAZTH_WITH_UNIT_TEST=$CAZTH_UNIT_TEST -DAZTH_WITH_CUSTOM_PLUGINS=$CAZTH_CUSTOM_PLG -DAZTH_WITH_PLUGINS=$CAZTH_PLG \
  -DCMAKE_C_COMPILER=$CCOMPILERC -DCMAKE_CXX_COMPILER=$CCOMPILERCXX

  cd $CWD

  runHooks "ON_AFTER_CONFIG"
}


function build() {
  [ $MTHREADS == 0 ] && MTHREADS=`grep -c ^processor /proc/cpuinfo` && MTHREADS=$(($MTHREADS + 2))

  echo "Using $MTHREADS threads"

  CWD=$(pwd)

  cd $BUILDPATH

  time make -j $MTHREADS
  make -j $MTHREADS install

  cd $CWD

  runHooks "ON_AFTER_BUILD"
}
