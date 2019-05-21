dnl $Id$
dnl config.m4 for extension faiss

PHP_ARG_ENABLE(faiss, whether to enable faiss support,
dnl Make sure that the comment is aligned:
[  --enable-faiss           Enable faiss support])

if test "$PHP_FAISS" != "no"; then
  PHP_REQUIRE_CXX()

  # --with-faiss -> check with-path
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/faiss/Index.h"
  if test -r $PHP_FAISS/$SEARCH_FOR; then # path given as parameter
    FAISS_DIR=$PHP_FAISS
  else # search default path list
    AC_MSG_CHECKING([for faiss files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        FAISS_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$FAISS_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the faiss distribution, "$PHP_FAISS"])
  fi

  # --with-faiss -> add include path
  PHP_ADD_INCLUDE($FAISS_DIR/include/faiss)

  # --with-faiss -> check for lib and symbol presence
  LIBNAME="faiss"
  LIBSYMBOL="FAISS"

  PHP_SUBST(FAISS_SHARED_LIBADD)

  PHP_ADD_LIBRARY(stdc++, 1, FAISS_SHARED_LIBADD)
  PHP_ADD_LIBRARY(faiss, 1, FAISS_SHARED_LIBADD)
  PHP_ADD_LIBRARY(faiss_c, 1, FAISS_SHARED_LIBADD)
  CFLAGS="-O3 -funroll-loops"
  CXXFLAGS="-pthread -std=c++14 -O3 -funroll-loops"

  PHP_NEW_EXTENSION(faiss, php_faiss.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
