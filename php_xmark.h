/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_XMARK_H
#define PHP_XMARK_H

extern zend_module_entry xmark_module_entry;
#define phpext_xmark_ptr &xmark_module_entry

#define PHP_XMARK_VERSION "1.0.0-dev"

#ifdef PHP_WIN32
#   define PHP_XMARK_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_XMARK_API __attribute__ ((visibility("default")))
#else
#   define PHP_XMARK_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if PHP_VERSION_ID < 70300
#   define IS_XMARK_FLAG            (1<<6)
#   define XMARK_FLAG(str)          (GC_FLAGS((str)) |= IS_XMARK_FLAG)
#   define XCLEAR_FLAG(str)         (GC_FLAGS((str)) &= ~IS_XMARK_FLAG)
#   define XCHECK_FLAG(str)         (GC_FLAGS((str)) & IS_XMARK_FLAG)
#else
#   define EX_CONSTANT(op)          RT_CONSTANT(EX(opline), op)
#   define IS_XMARK_FLAG            (1<<5)
#   define XMARK_FLAG(str)          GC_ADD_FLAGS(str, IS_XMARK_FLAG)
#   define XCLEAR_FLAG(str)         GC_DEL_FLAGS(str, IS_XMARK_FLAG)
#   define XCHECK_FLAG(str)         (GC_FLAGS((str)) & IS_XMARK_FLAG)
#endif


#define XMARK_OP1_TYPE(opline)  ((opline)->op1_type)
#define XMARK_OP2_TYPE(opline)  ((opline)->op2_type)


#if PHP_VERSION_ID < 70100
#define XMARK_RET_USED(opline) (!((opline)->result_type & EXT_TYPE_UNUSED))
#define XMARK_ISERR(var)       ((var) == &EG(error_zval))
#define XMARK_ERR_ZVAL(var)    ((var) = &EG(error_zval))
#else
#define XMARK_RET_USED(opline) ((opline)->result_type != IS_UNUSED)
#define XMARK_ISERR(var)       (Z_ISERROR_P(var))
#define XMARK_ERR_ZVAL(var)    (ZVAL_ERROR(var))
#endif


ZEND_BEGIN_MODULE_GLOBALS(xmark)
    zend_bool   enable;
    zend_bool   enable_rename;
    zend_bool   in_callback;
    char *rename_functions;
    char *rename_classes;
    HashTable   callbacks;
ZEND_END_MODULE_GLOBALS(xmark)

/* Always refer to the globals in your function as XMARK_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define XMARK_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(xmark, v)

#if defined(ZTS) && defined(COMPILE_DL_XMARK)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#if PHP_VERSION_ID < 70200
static zend_always_inline zend_string *zend_string_init_interned(const char *str, size_t len, int persistent)
{
    zend_string *ret = zend_string_init(str, len, persistent);

    return zend_new_interned_string(ret);
}
#endif

PHP_FUNCTION(xmark);
PHP_FUNCTION(xcheck);
PHP_FUNCTION(xclear);
PHP_FUNCTION(xrename_function);
PHP_FUNCTION(xrename_class);
PHP_FUNCTION(xregister_opcode_callback);

static void rename_from_ini_value(HashTable *ht, const char *ini_name);
static zend_always_inline int xmark_zstr(zval *z_str);
static zend_always_inline Bucket *rename_hash_key(HashTable *ht, zend_string *orig_name, zend_string *new_name);
static zend_always_inline Bucket *rename_hash_str_key(HashTable *ht, const char *orig_name, const char *new_name);

#endif /* PHP_XMARK_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
