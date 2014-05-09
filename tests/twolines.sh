#!/bin/bash
result=$(../lctp -q ${srcdir}/twolines.dat)
if test "x${result}" = "x16.02"
then
	exit 0
else
	exit 1
fi
