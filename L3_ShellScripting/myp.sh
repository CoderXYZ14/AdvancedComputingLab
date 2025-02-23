#!/bin/bash
# This is a simple script for file operations

# If user types -1, this function will run
function add_date_to_files() {
    # $1 means first argument after -1
    folder=$1
    
    # Go through each file in the folder
    for file in $folder/*; do
        # Get just the file name
        name=$(basename "$file")
        # Get file extension
        ext="${name##*.}"
        # Get name without extension
        base="${name%.*}"
        # Get date (in format DDMMYYYY)
        date=$(date +%d%m%Y)
        
        # Create new name
        new_name="${base}${date}.${ext}"
        
        # Rename file
        mv "$file" "$folder/$new_name"
        echo "Changed $name to $new_name"
    done
}

# If user types -2, this function will run
function copy_common_files() {
    # Get the folders from arguments
    folder1=$1
    folder2=$2
    dest=$3
    
    # Make destination folder if it doesn't exist
    mkdir -p $dest
    
    # Look for common files
    for file in $folder1/*; do
        name=$(basename "$file")
        # Check if same file exists in folder2
        if [ -f "$folder2/$name" ]; then
            # Get name and extension
            ext="${name##*.}"
            base="${name%.*}"
            
            # Copy files with new names
            cp "$folder1/$name" "$dest/${base}1.${ext}"
            cp "$folder2/$name" "$dest/${base}2.${ext}"
            echo "Copied $name from both folders"
        fi
    done
}

# Main part of script
if [ "$1" = "-1" ]; then
    add_date_to_files $2
elif [ "$1" = "-2" ]; then
    copy_common_files $2 $3 $4
else
    echo "Please use -1 or -2"
    echo "-1: add date to files (use: ./myp -1 sourcefolder)"
    echo "-2: copy common files (use: ./myp -2 folder1 folder2 destfolder)"
fi