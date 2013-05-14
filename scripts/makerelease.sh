#! /bin/bash
if [[ "$1" == "" ]] ; then
    echo Usage: ${0} \<version\>
else
    VERSION="$1"
    echo Check if these values inside src/version.h are right:
    cat ../src/version.h
    curdir=`pwd`
    cd ${curdir%/scripts}
# To use current work tree attributes: "--worktree-attributes"
    git archive --format=tar --prefix=aoflagger-${VERSION}/ master | bzip2 -9 > ${curdir}/aoflagger-${VERSION}.tar.bz2 &&
    echo Wrote ${curdir}/aoflagger-${VERSION}.tar.bz2
fi
