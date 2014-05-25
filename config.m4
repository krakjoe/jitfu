dnl $Id$
dnl config.m4 for extension jit

PHP_ARG_WITH(jit, for libjit support,
[  --with-jit             Include jit support])

if test "$PHP_JIT" != "no"; then
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/jit/jit.h"  # you most likely want to change this
  if test -r $PHP_JIT/$SEARCH_FOR; then # path given as parameter
     JIT_DIR=$PHP_JIT
  else # search default path list
    AC_MSG_CHECKING([for jit files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        JIT_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$JIT_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the libjit distribution])
  fi

  PHP_ADD_INCLUDE($JIT_DIR/include)

  LIBNAME=jit # you may want to change this
  LIBSYMBOL=jit_context_create # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $JIT_DIR/lib, JIT_SHARED_LIBADD)
    AC_DEFINE(HAVE_JITLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libjit version or libjit not found])
  ],[
    -L$JIT_DIR/lib -lm
  ])
  
  PHP_SUBST(JIT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(jit, jit.c, $ext_shared)
fi
