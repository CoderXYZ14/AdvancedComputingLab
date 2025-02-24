#!/bin/zsh

function add_date_to_files() {
    folder=$1
    
    for file in $folder/*; do
        name=$(basename "$file")
        ext="${name##*.}"
        base="${name%.*}"
        date=$(date +%d%m%Y)
        new_name="${base}${date}.${ext}"
        mv "$file" "$folder/$new_name"
        echo "Changed $name to $new_name"
    done
}

function copy_common_files() {
    folder1=$1
    folder2=$2
    dest=$3
    
    
    for file in $folder1/*; do
        name=$(basename "$file")
        if [ -f "$folder2/$name" ]; then
            ext="${name##*.}"
            base="${name%.*}"
            cp "$folder1/$name" "$dest/${base}1.${ext}"
            cp "$folder2/$name" "$dest/${base}2.${ext}"
            echo "Copied $name from both folders"
        fi
    done
}

if [ "$1" = "-1" ]; then
    add_date_to_files $2
elif [ "$1" = "-2" ]; then
    copy_common_files $2 $3 $4
else
    echo "Please use -1 or -2"
    echo "-1: add date to files (use: ./myp -1 sourcefolder)"
    echo "-2: copy common files (use: ./myp -2 folder1 folder2 destfolder)"
fi
