dnl $Id$
dnl config.m4 for extension jit

PHP_ARG_WITH(jitfu, for JIT-Fu support,
[  --with-jitfu             Include JIT-Fu support])

if test "$PHP_JITFU" != "no"; then
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/jit/jit.h"  # you most likely want to change this
  if test -r $PHP_JITFU/$SEARCH_FOR; then # path given as parameter
     JITFU_DIR=$PHP_JITFU
  else # search default path list
    AC_MSG_CHECKING([for jit files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        JITFU_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$JITFU_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the libjit distribution])
  fi

  PHP_ADD_INCLUDE($JITFU_DIR/include)

  LIBNAME=jit # you may want to change this
  LIBSYMBOL=jit_context_create # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $JITFU_DIR/lib, JITFU_SHARED_LIBADD)
    AC_DEFINE(HAVE_JITLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libjit version or libjit not found])
  ],[
    -L$JITFU_DIR/lib -lm
  ])
  
  PHP_SUBST(JITFU_SHARED_LIBADD)

  PHP_NEW_EXTENSION(jitfu, jitfu.c, $ext_shared)
fi

