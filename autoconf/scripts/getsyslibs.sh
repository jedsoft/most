#!/bin/sh

# This script parses /etc/ld.so.conf and returns lib and include directories 
# in a form that JD_CHECK_FOR_LIBRARY can grok.

SH_TRUE=0
SH_FALSE=1

# Usage: sh_substr string a b  ==> string[a:b]  (1s-based)
sh_substr()
{
   echo "$1" | cut "-c" "$2-$3"
}

# if sh_is_comment line; then ...
sh_is_comment()
{
  ch=`sh_substr "$1" 1 1`
  if test "X$ch" = "X#" -o "X$ch" = "X"
  then
     return $SH_TRUE;
  fi
  return $SH_FALSE;
}

sh_read_ldsoconf ()
{
   file="$1"
   dirlist=""
   if test ! -f "$file"
   then
     return $SH_FALSE;
   fi

   while read line
   do
     if sh_is_comment "$line"; then continue; fi
     read p1 p2 pn << EOF
       ${line}
EOF
     if test "$p1" = "include"
     then
       for file in $p2
       do
         dirs=`sh_read_ldsoconf "$file"`
         dirlist="$dirlist $dirs"
       done
     else
       dirlist="$dirlist $p1"
     fi
   done < "$file"
   echo "$dirlist"
}

dirs=`sh_read_ldsoconf "/etc/ld.so.conf"`
XY=""
for Y in $dirs
do
   if test "/usr/lib" = `sh_substr $Y 1 8`
   then
     X="/usr/include"
   else
     if test "/lib" = `sh_substr $Y 1 4`
     then
       X="/usr/include"
     else
       X=`dirname $Y`"/include"
     fi
   fi
   if test -d "$Y"
   then
      XY="$XY $X,$Y"
   fi
done

echo $XY

