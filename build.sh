#!/bin/bash

# get current directory-path and the path of the parent-directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PARENT_DIR="$(dirname "$DIR")"

# create build-directory
BUILD_DIR="$PARENT_DIR/build"
mkdir -p $BUILD_DIR

# create directory for the final result
RESULT_DIR="$PARENT_DIR/result"
mkdir -p $RESULT_DIR

#-----------------------------------------------------------------------------------------------------------------

function build_kitsune_lib_repo () {
    REPO_NAME=$1
    NUMBER_OF_THREADS=$2

    # create build directory for repo and go into this directory
    REPO_DIR="$BUILD_DIR/$REPO_NAME"
    mkdir -p $REPO_DIR
    cd $REPO_DIR

    # build repo library with qmake
    /usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/$REPO_NAME/$REPO_NAME.pro" -spec linux-g++ "CONFIG += optimize_full staticlib"
    /usr/bin/make -j$NUMBER_OF_THREADS

    # copy build-result and include-files into the result-directory
    echo "----------------------------------------------------------------------"
    echo $RESULT_DIR
    cp $REPO_DIR/src/$REPO_NAME.a $RESULT_DIR/
    cp -r $PARENT_DIR/$REPO_NAME/include $RESULT_DIR/
    ls -l $RESULT_DIR/include/
    ls -l $RESULT_DIR
}

function get_required_kitsune_lib_repo () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2
    NUMBER_OF_THREADS=$3

    # clone repo
    git clone https://github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH

    build_kitsune_lib_repo $REPO_NAME $NUMBER_OF_THREADS
}


function download_repo_github () {
    REPO_NAME=$1
    TAG_OR_BRANCH=$2

    # clone repo
    git clone https://github.com/kitsudaiki/$REPO_NAME.git "$BUILD_DIR/$REPO_NAME"
    git clone https://github.com/kitsudaiki/$REPO_NAME.git "$PARENT_DIR/$REPO_NAME"
    cd "$BUILD_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH
    cd "$PARENT_DIR/$REPO_NAME"
    git checkout $TAG_OR_BRANCH
}

#-----------------------------------------------------------------------------------------------------------------

echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiCommon" "v0.27.x" 8
get_required_kitsune_lib_repo "libKitsunemimiJson" "v0.12.x" 1
get_required_kitsune_lib_repo "libKitsunemimiJinja2" "v0.10.x" 1
get_required_kitsune_lib_repo "libKitsunemimiIni" "v0.6.x" 1
get_required_kitsune_lib_repo "libKitsunemimiNetwork" "v0.9.x" 8
get_required_kitsune_lib_repo "libKitsunemimiArgs" "v0.5.x" 8
get_required_kitsune_lib_repo "libKitsunemimiConfig" "v0.5.x" 8
get_required_kitsune_lib_repo "libKitsunemimiCrypto" "v0.3.x" 8
get_required_kitsune_lib_repo "libKitsunemimiJwt" "v0.5.x" 8
get_required_kitsune_lib_repo "libKitsunemimiSqlite" "v0.4.x" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiSakuraNetwork" "v0.9.x" 8
get_required_kitsune_lib_repo "libKitsunemimiSakuraLang" "v0.13.x" 1
get_required_kitsune_lib_repo "libKitsunemimiSakuraDatabase" "v0.6.x" 8
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libKitsunemimiHanamiCommon" "v0.3.x" 8
get_required_kitsune_lib_repo "libKitsunemimiHanamiEndpoints" "v0.2.x" 1
get_required_kitsune_lib_repo "libKitsunemimiHanamiNetwork" "v0.5.x" 8
get_required_kitsune_lib_repo "libKitsunemimiHanamiDatabase" "v0.4.x" 8
download_repo_github "libKitsunemimiHanamiMessages" "v0.1.x"
echo ""
echo "###########################################################################################################"
echo ""
get_required_kitsune_lib_repo "libAzukiHeart" "v0.3.x" 8
get_required_kitsune_lib_repo "libMisakiGuard" "v0.2.x" 8
get_required_kitsune_lib_repo "libShioriArchive" "v0.3.x" 8
echo ""
echo "###########################################################################################################"

#-----------------------------------------------------------------------------------------------------------------

# create build directory for ShioriArchive and go into this directory
LIB_KITSUNE_SAKURA_TREE_DIR="$BUILD_DIR/ShioriArchive"
mkdir -p $LIB_KITSUNE_SAKURA_TREE_DIR
cd $LIB_KITSUNE_SAKURA_TREE_DIR

# build ShioriArchive with qmake
/usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/ShioriArchive/ShioriArchive.pro" -spec linux-g++ "CONFIG += optimize_full"
/usr/bin/make -j8

# copy build-result and include-files into the result-directory
cp "$LIB_KITSUNE_SAKURA_TREE_DIR/ShioriArchive" "$RESULT_DIR/"

#-----------------------------------------------------------------------------------------------------------------

