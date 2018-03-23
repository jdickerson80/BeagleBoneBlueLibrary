#!/bin/bash

echo "This script runs doxygen and pushes the results to github. If no arguements are supplied to this script, the script does not print the results of doxygen to the terminal. However, if doxygen output is desired,
run the script with any number as an argument, and doxygen output will be printed to the terminal."
echo 
echo
echo Changing to docs folder.
cd docs/

echo Running Doxygen.

if [ "$#" -ne 1 ]; then
	doxygen Doxyfile &> /dev/null 
else
	doxygen Doxyfile
fi

echo Changing to html directory.
cd ../html/

echo Adding any untracked files.
git add --all

echo Commiting changes.
git commit --all -m "Updated doxygen files."

echo Pushing to git.
git push origin gh-pages
