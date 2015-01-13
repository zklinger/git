#!/bin/bash

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
