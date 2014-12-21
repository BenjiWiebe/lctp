#!/bin/bash
TZ='America/Chicago'
export TZ
result=$(../lctp -q ${srcdir}/dst.dat)
if test "x${result}" = "x2.00"
then
	exit 0
else
	exit 1
fi
