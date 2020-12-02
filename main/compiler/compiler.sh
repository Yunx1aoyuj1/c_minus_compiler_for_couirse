#!/bin/bash

if [[ ! -z $1 ]] ; then
    PROG_PATH=$(realpath -q $1)
fi

flex cminus.l
bison -d cminus.y

gcc -c $C_FLAGS *.c
gcc -o cminus *.o -ly -lfl

./cminus ${PROG_PATH} $2

#rm -rf *.tab.*
#rm -rf *.yy.*
rm -rf *.gch
rm -rf *.o
rm cminus

