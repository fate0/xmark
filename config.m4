dnl $Id$


PHP_ARG_ENABLE(xmark, whether to enable xmark support,
[  --enable-xmark           Enable xmark support])


if test -z "$PHP_DEBUG"; then
    AC_ARG_ENABLE(debug,
    [ --enable-debug      compile with debugging symbols],
    [ PHP_DEBUG=$enableval ],
    [ PHP_DEBUG=no ])
fi

if test "$PHP_TAINT" != "no"; then
    PHP_NEW_EXTENSION(xmark, xmark.c, $ext_shared)
fi
