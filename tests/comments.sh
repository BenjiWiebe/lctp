#!/bin/bash
result=$(../lctp --comments -q ${srcdir}/comments.dat)
if test "x${result}" = "xcomment:1001 comment:1002 6.00"
then
	exit 0
else
	exit 1
fi
