#!/bin/bash
result=$(../lctp -q ${srcdir}/onehour.dat)
if test "x${result}" = "x1.00"
then
	exit 0
else
	exit 1
fi
