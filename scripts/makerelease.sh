#! /bin/bash
if [[ "$1" == "" ]] ; then
    echo Usage: ${0} \<version\>
else
    VERSION="$1"
    curdir=`pwd`
    cd ${curdir%/scripts}
# To use current work tree attributes: "--worktree-attributes"
    git archive --format=tar --prefix=aoflagger-${VERSION}/ master | bzip2 -9 > ${curdir}/aoflagger-${VERSION}.tar.bz2 &&
    echo Wrote ${curdir}/aoflagger-${VERSION}.tar.bz2
fi
