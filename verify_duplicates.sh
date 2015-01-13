#!/bin/bash

counter=0
file_count=$(ls x | wc -l)
#for file in $(ls x | head -10)
for file in $(echo block-sha1~sha1.c)
do
    counter=$((counter+1))
    actual_file=$(echo $file | tr '~' '/')
    printf "%03d/%03d %s" $counter $file_count  $actual_file

    echo
    while read escaped
    do
        echo "  " $escaped
        #sed -i "/$escaped/d" $actual_file || echo fix $actual_file $escaped
    done < x/$file

    #make clean &> /dev/null
    #make &> /tmp/make.out || printf " %s\n" FAILED
    #git co -- $actual_file 
    printf " %s\n" OK

done

exit
while read line
do
    file=$(echo $line | cut -f1 -d:)
    txt=$(echo $line | cut -f2 -d:)
    escaped=$(echo $txt | sed 's|/|\\/|g')
    escaped_file=$(echo $file | tr '/' '~')

    echo "$escaped" >> x/$escaped_file

    #if [ "$file" = "" ]
    #then
    #    make clean &> /dev/null
    #    make &> /tmp/make.out || echo "========>> $currentfile: FAILED"
    #    git co -- . 
    #    echo ""
    #else
    #    currentfile=$file
    #    echo $file $txt
    #    sed -i "/$escaped/d" $file || echo fix $file $line $escaped
    #fi

done <input
