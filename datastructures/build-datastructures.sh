#!/bin/sh
#FOLDERS="hashmap linkedlist redblacktree"
FOLDERS="hashmap hashmap-static redblacktree" # linkedlist"

if [ $# -eq 0 ] ; then
    echo " === ERROR At the very least, we need the backend name in the first parameter. === "
    exit 1
fi

backend=$1  # e.g.: herwl


htm_retries=5
rot_retries=2

if [ $# -eq 3 ] ; then
    htm_retries=$2 # e.g.: 5
    rot_retries=$3 # e.g.: 2, this can also be retry policy for tle
fi

rm common/Defines.common.mk
rm common/Makefile.common
rm common/Makefile.htm_ibm
rm -rf lib/

cp ../common/Defines.common.mk common
cp ../common/Makefile.common common
cp ../common/Makefile.htm_ibm common
cp -r ../lib/ lib

for F in $FOLDERS
do
    cd $F
echo "executing make"
	if [[ $backend == htm-sgl || $backend == tle ]] ; then
		make_command="make -f Makefile.htm_ibm HTM_RETRIES=-DHTM_RETRIES=$htm_retries RETRY_POLICY=-DRETRY_POLICY=$rot_retries"
	else
        	make_clean="make -f Makefile.htm_ibm clean"
                make_default="make -f Makefile.htm_ibm default"
	fi
        $make_clean
        $make_default

    rc=$?
    if [[ $rc != 0 ]] ; then
        echo ""
        echo "=================================== ERROR BUILDING $F - $name ===================================="
        echo ""
        exit 1
    fi
    cd ..
done

