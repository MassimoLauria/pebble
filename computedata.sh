#!/bin/sh
#
# Copyright (C) 2013 by Massimo Lauria
#
# Created   : "2013-09-09, Monday 11:44 (CEST) Massimo Lauria"
# Time-stamp: "2013-09-09, 15:07 (CEST) Massimo Lauria"
#
# Description::
#
# Compute the pebbling numbers for some standard graphs
#

# Code::

ulimit -m 100000

# Black white pebbling for pyramid
for h in 1 2 3 4 5 6 7 8; do

    ./bwpebble -p $h -b 8 2> data/pyr${h}-bwpebbling.txt >data/pyr${h}-bwpebbling.dot

    echo "BW pebbling for a pyramid of height $h" >> data/final_report.txt

    ./bwpebble -p $h -b 8 -Z 2> data/pyr${h}-bwpebblingZ.txt >data/pyr${h}-bwpebblingZ.dot

    echo "Persistent BW pebbling for a pyramid of height $h" >> data/final_report.txt

done


# Reversible pebbling for pyramid
for h in 1 2 3 4 5 6 7; do

    ./revpebble -p $h -b 10 2> data/pyr${h}-revpebbling.txt >data/pyr${h}-revpebbling.dot
    
    echo "Reversible pebbling for a pyramid of height $h" >> data/final_report.txt
    
done

# Reversible pebbling for trees
for h in 1 2 3 4 5; do

    ./revpebble -2 $h -b 8 2> data/tree${h}-revpebbling.txt >data/tree${h}-revpebbling.dot

    echo "Reversible pebbling for a tree of height $h" >> data/final_report.txt
        


done


# Local Variables:
# fill-column: 80
# End:
