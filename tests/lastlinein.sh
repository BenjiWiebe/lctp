#!/bin/bash
result=$(../lctp -q ${srcdir}/lastlinein.dat)
if test "x${result}" = "x5.00"
then
	exit 0
else
	exit 1
fi
