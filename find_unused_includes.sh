#!/bin/bash

git grep -l -e '#include' -- '*.c' > /tmp/inputfiles
filecount=$(cat /tmp/inputfiles | wc -l)
counter=0
while read file
do
    counter=$((counter+1))
    printf "%03d/%03d %s\n" $counter $filecount $file
    grep "^#include" $file >/tmp/includes
    include_count=$(cat /tmp/includes | wc -l)
    include_counter=0
    while read line
    do
        include_counter=$((include_counter+1))
        echo   "  " $include_counter of $include_count $line
        escaped=$(echo $line | sed 's|/|\\/|g')
        sed -i "/$escaped/d" $file || echo fix $file $line $escaped
        object=$(echo $file | sed 's/\.c$/\.o/')
        rm -rf $object
        make &> /tmp/make.out && echo "========>> $file: $line NOT NEEDED"
        git co -- $file
    done </tmp/includes
done </tmp/inputfiles
