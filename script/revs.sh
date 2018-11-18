#!/bin/sh
gcc --std=c99 -Wall -S -masm=intel -g -m32 -O0 -fno-asynchronous-unwind-tables -fno-dwarf2-cfi-asm $1 -o $1.asm.tmp

cat $1.asm.tmp |\
  awk '/file/,/section.*debug/' |\
  egrep '^L' -v |\
  while read line ;do
    if echo $line |\
      egrep 'loc 1 [0-9 ]*$' >/dev/null ;then
        echo -n '; -------------------- '
        l=$(echo $line |sed -e 's/.* 1 //' -e 's/ 0$//')
        cat $1 |awk "NR==$l"
    else
      echo $line
    fi
  done |\
  while read line ;do
    if echo $line |egrep '^.def' >/dev/null ;then
      echo ''
      echo '; ----------------------------------------'
    fi
    echo $line
  done |\
  while read line ;do
    if echo $line |egrep '^_' >/dev/null ;then
      echo ''
    fi
    echo $line
  done |\
  egrep -v '^.section.*debug_frame' \
  > $1.asm

cat $1.asm && rm $1.asm.tmp
