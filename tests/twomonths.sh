#!/bin/bash
result=$(../lctp --no-warnings -q ${srcdir}/twomonths.dat)
if test "x${result}" = "x535.70"
then
	exit 0
else
	exit 1
fi
