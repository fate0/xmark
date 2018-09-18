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
  | Author:  fate0 <fate0@fatezero.org>                                  |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "zend_compile.h"
#include "zend_types.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_xmark.h"

ZEND_DECLARE_MODULE_GLOBALS(xmark)


/* These are most copied from zend_execute.c: zend_fetch_dimension_address */
static int php_xmark_make_real_object(zval *object) /* {{{ */ {
    if (UNEXPECTED(Z_TYPE_P(object) != IS_OBJECT)) {
        if (EXPECTED(Z_TYPE_P(object) <= IS_FALSE)) {
            /* nothing to destroy */
        } else if (EXPECTED((Z_TYPE_P(object) == IS_STRING && Z_STRLEN_P(object) == 0))) {
            zval_ptr_dtor_nogc(object);
        } else {
            return 0;
        }
        object_init(object);
        zend_error(E_WARNING, "Creating default object from empty value");
    }
    return 1;
}
/* }}} */

static zend_long php_xmark_check_string_offset(zval *dim, int type) /* {{{ */ {
    zend_long offset;

try_again:
    if (UNEXPECTED(Z_TYPE_P(dim) != IS_LONG)) {
        switch(Z_TYPE_P(dim)) {
            case IS_STRING:
                if (IS_LONG == is_numeric_string(Z_STRVAL_P(dim), Z_STRLEN_P(dim), NULL, NULL, -1)) {
                    break;
                }
                if (type != BP_VAR_UNSET) {
                    zend_error(E_WARNING, "Illegal string offset '%s'", Z_STRVAL_P(dim));
                }
                break;
            case IS_DOUBLE:
            case IS_NULL:
            case IS_FALSE:
            case IS_TRUE:
                zend_error(E_NOTICE, "String offset cast occurred");
                break;
            case IS_REFERENCE:
                dim = Z_REFVAL_P(dim);
                goto try_again;
            default:
                zend_error(E_WARNING, "Illegal offset type");
                break;
        }

        offset = zval_get_long(dim);
    } else {
        offset = Z_LVAL_P(dim);
    }

    return offset;
}
/* }}} */

static zval *php_xmark_fetch_dimension_address_inner(HashTable *ht, const zval *dim, int dim_type, int type) /* {{{ */ {
    zval *retval;
    zend_string *offset_key;
    zend_ulong hval;

    try_again:
    if (EXPECTED(Z_TYPE_P(dim) == IS_LONG)) {
        hval = Z_LVAL_P(dim);
        num_index:
        retval = zend_hash_index_find(ht, hval);
        if (retval == NULL) {
            switch (type) {
                case BP_VAR_R:
                    zend_error(E_NOTICE,"Undefined offset: " ZEND_LONG_FMT, hval);
                    /* break missing intentionally */
                case BP_VAR_UNSET:
                case BP_VAR_IS:
                    retval = &EG(uninitialized_zval);
                    break;
                case BP_VAR_RW:
                    zend_error(E_NOTICE,"Undefined offset: " ZEND_LONG_FMT, hval);
                    /* break missing intentionally */
                case BP_VAR_W:
                    retval = zend_hash_index_add_new(ht, hval, &EG(uninitialized_zval));
                    break;
            }
        }
    } else if (EXPECTED(Z_TYPE_P(dim) == IS_STRING)) {
        offset_key = Z_STR_P(dim);
        if (dim_type != IS_CONST) {
            if (ZEND_HANDLE_NUMERIC(offset_key, hval)) {
                goto num_index;
            }
        }
        str_index:
        retval = zend_hash_find(ht, offset_key);
        if (retval) {
            /* support for $GLOBALS[...] */
            if (UNEXPECTED(Z_TYPE_P(retval) == IS_INDIRECT)) {
                retval = Z_INDIRECT_P(retval);
                if (UNEXPECTED(Z_TYPE_P(retval) == IS_UNDEF)) {
                    switch (type) {
                        case BP_VAR_R:
                            zend_error(E_NOTICE, "Undefined index: %s", ZSTR_VAL(offset_key));
                            /* break missing intentionally */
                        case BP_VAR_UNSET:
                        case BP_VAR_IS:
                            retval = &EG(uninitialized_zval);
                            break;
                        case BP_VAR_RW:
                            zend_error(E_NOTICE,"Undefined index: %s", ZSTR_VAL(offset_key));
                            /* break missing intentionally */
                        case BP_VAR_W:
                            ZVAL_NULL(retval);
                            break;
                    }
                }
            }
        } else {
            switch (type) {
                case BP_VAR_R:
                    zend_error(E_NOTICE, "Undefined index: %s", ZSTR_VAL(offset_key));
                    /* break missing intentionally */
                case BP_VAR_UNSET:
                case BP_VAR_IS:
                    retval = &EG(uninitialized_zval);
                    break;
                case BP_VAR_RW:
                    zend_error(E_NOTICE,"Undefined index: %s", ZSTR_VAL(offset_key));
                    /* break missing intentionally */
                case BP_VAR_W:
                    retval = zend_hash_add_new(ht, offset_key, &EG(uninitialized_zval));
                    break;
            }
        }
    } else {
        switch (Z_TYPE_P(dim)) {
            case IS_NULL:
                offset_key = ZSTR_EMPTY_ALLOC();
                goto str_index;
            case IS_DOUBLE:
                hval = zend_dval_to_lval(Z_DVAL_P(dim));
                goto num_index;
            case IS_RESOURCE:
                zend_error(E_NOTICE, "Resource ID#%pd used as offset, casting to integer (%pd)", Z_RES_HANDLE_P(dim), Z_RES_HANDLE_P(dim));
                hval = Z_RES_HANDLE_P(dim);
                goto num_index;
            case IS_FALSE:
                hval = 0;
                goto num_index;
            case IS_TRUE:
                hval = 1;
                goto num_index;
            case IS_REFERENCE:
                dim = Z_REFVAL_P(dim);
                goto try_again;
            default:
                zend_error(E_WARNING, "Illegal offset type");
                retval = (type == BP_VAR_W || type == BP_VAR_RW) ?
#if PHP_VERSION_ID < 70100
                &EG(error_zval)
#else
                NULL
#endif
                : &EG(uninitialized_zval);
        }
    }
    return retval;
}
/* }}} */

static void php_xmark_fetch_dimension_address(zval *result, zval *container, zval *dim, int dim_type, int type) /* {{{ */ {
    zval *retval;

    if (EXPECTED(Z_TYPE_P(container) == IS_ARRAY)) {
try_array:
        SEPARATE_ARRAY(container);
fetch_from_array:
        if (dim == NULL) {
            retval = zend_hash_next_index_insert(Z_ARRVAL_P(container), &EG(uninitialized_zval));
            if (UNEXPECTED(retval == NULL)) {
                zend_error(E_WARNING, "Cannot add element to the array as the next element is already occupied");
#if PHP_VERSION_ID < 70100
                retval = &EG(error_zval);
#else
                ZVAL_ERROR(result);
                return;
#endif
            }
        } else {
            retval = php_xmark_fetch_dimension_address_inner(Z_ARRVAL_P(container), dim, dim_type, type);
        }
        ZVAL_INDIRECT(result, retval);
        return;
    } else if (EXPECTED(Z_TYPE_P(container) == IS_REFERENCE)) {
        container = Z_REFVAL_P(container);
        if (EXPECTED(Z_TYPE_P(container) == IS_ARRAY)) {
            goto try_array;
        }
    }
    if (EXPECTED(Z_TYPE_P(container) == IS_STRING)) {
        if (type != BP_VAR_UNSET && UNEXPECTED(Z_STRLEN_P(container) == 0)) {
            zval_ptr_dtor_nogc(container);
            convert_to_array:
            ZVAL_NEW_ARR(container);
            zend_hash_init(Z_ARRVAL_P(container), 8, NULL, ZVAL_PTR_DTOR, 0);
            goto fetch_from_array;
        }

        if (dim == NULL) {
            zend_throw_error(NULL, "[] operator not supported for strings");
#if PHP_VERSION_ID < 70100
            ZVAL_INDIRECT(result, &EG(error_zval));
#else
            ZVAL_ERROR(result);
#endif
        } else {
            php_xmark_check_string_offset(dim, type);
#if PHP_VERSION_ID < 70100
            ZVAL_INDIRECT(result, NULL); /* wrong string offset */
#else
            ZVAL_ERROR(result);
#endif
        }
    } else if (EXPECTED(Z_TYPE_P(container) == IS_OBJECT)) {
        if (!Z_OBJ_HT_P(container)->read_dimension) {
            zend_throw_error(NULL, "Cannot use object as array");
#if PHP_VERSION_ID < 70100
            retval = &EG(error_zval);
#else
            ZVAL_ERROR(result);
#endif
        } else {
            retval = Z_OBJ_HT_P(container)->read_dimension(container, dim, type, result);

            if (UNEXPECTED(retval == &EG(uninitialized_zval))) {
                zend_class_entry *ce = Z_OBJCE_P(container);

                ZVAL_NULL(result);
                zend_error(E_NOTICE, "Indirect modification of overloaded element of %s has no effect", ZSTR_VAL(ce->name));
            } else if (EXPECTED(retval && Z_TYPE_P(retval) != IS_UNDEF)) {
                if (!Z_ISREF_P(retval)) {
                    if (Z_REFCOUNTED_P(retval) &&
                        Z_REFCOUNT_P(retval) > 1) {
                        if (Z_TYPE_P(retval) != IS_OBJECT) {
                            Z_DELREF_P(retval);
                            ZVAL_DUP(result, retval);
                            retval = result;
                        } else {
                            ZVAL_COPY_VALUE(result, retval);
                            retval = result;
                        }
                    }
                    if (Z_TYPE_P(retval) != IS_OBJECT) {
                        zend_class_entry *ce = Z_OBJCE_P(container);
                        zend_error(E_NOTICE, "Indirect modification of overloaded element of %s has no effect", ZSTR_VAL(ce->name));
                    }
                } else if (UNEXPECTED(Z_REFCOUNT_P(retval) == 1)) {
                    ZVAL_UNREF(retval);
                }
                if (result != retval) {
                    ZVAL_INDIRECT(result, retval);
                }
            } else {
#if PHP_VERSION_ID < 70100
                ZVAL_INDIRECT(result, &EG(error_zval));
#else
                ZVAL_ERROR(result);
#endif
            }
        }
    } else if (EXPECTED(Z_TYPE_P(container) <= IS_FALSE)) {
        if (UNEXPECTED(XMARK_ISERR(container))) {
#if PHP_VERSION_ID < 70100
            ZVAL_INDIRECT(result, &EG(error_zval));
#else
            ZVAL_ERROR(result);
#endif
        } else if (type != BP_VAR_UNSET) {
            goto convert_to_array;
        } else {
            /* for read-mode only */
            ZVAL_NULL(result);
        }
    } else {
        if (type == BP_VAR_UNSET) {
            zend_error(E_WARNING, "Cannot unset offset in a non-array variable");
            ZVAL_NULL(result);
        } else {
            zend_error(E_WARNING, "Cannot use a scalar value as an array");
#if PHP_VERSION_ID < 70100
            ZVAL_INDIRECT(result, &EG(error_zval));
#else
            ZVAL_ERROR(result);
#endif
        }
    }
}
/* }}} */

static void php_xmark_assign_op_overloaded_property(zval *object, zval *property, void **cache_slot, zval *value, binary_op_type binary_op, zval *result) /* {{{ */ {
    zval *z;
    zval rv, obj;
    zval *zptr;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];

    ZVAL_OBJ(&obj, Z_OBJ_P(object));
    Z_ADDREF(obj);
    if (Z_OBJ_HT(obj)->read_property &&
        (z = Z_OBJ_HT(obj)->read_property(&obj, property, BP_VAR_R, cache_slot, &rv)) != NULL) {
        if (EG(exception)) {
            OBJ_RELEASE(Z_OBJ(obj));
            return;
        }
        if (Z_TYPE_P(z) == IS_OBJECT && Z_OBJ_HT_P(z)->get) {
            zval rv2;
            zval *value = Z_OBJ_HT_P(z)->get(z, &rv2);

            if (z == &rv) {
                zval_ptr_dtor(&rv);
            }
            ZVAL_COPY_VALUE(z, value);
        }
        zptr = z;
        ZVAL_DEREF(z);
        SEPARATE_ZVAL_NOREF(z);

        // only for ZEND_ASSIGN_CONCAT
        z_fname = zend_hash_index_find(&XMARK_G(callbacks), ZEND_ASSIGN_CONCAT);
        if (z_fname) {
            ZVAL_COPY_VALUE(&call_func_params[0], z);
            ZVAL_COPY_VALUE(&call_func_params[1], value);
            if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
                zend_error(E_WARNING, "call function error");
            }
            zval_ptr_dtor_nogc(z);
            ZVAL_COPY_VALUE(z, &call_func_ret);
        } else {
            binary_op(z, z, value);
        }

        Z_OBJ_HT(obj)->write_property(&obj, property, z, cache_slot);
        if (result) {
            ZVAL_COPY(result, z);
        }

        zval_ptr_dtor(zptr);
    } else {
        zend_error(E_WARNING, "Attempt to assign property of non-object");
        if (result) {
            ZVAL_NULL(result);
        }
    }
    OBJ_RELEASE(Z_OBJ(obj));
}
/* }}} */

static void php_xmark_binary_assign_op_obj_dim(zval *object, zval *property, zval *value, zval *retval, binary_op_type binary_op) /* {{{ */ {
    zval *z;
    zval rv, res;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];

    if (Z_OBJ_HT_P(object)->read_dimension &&
        (z = Z_OBJ_HT_P(object)->read_dimension(object, property, BP_VAR_R, &rv)) != NULL) {

        if (Z_TYPE_P(z) == IS_OBJECT && Z_OBJ_HT_P(z)->get) {
            zval rv2;
            zval *value = Z_OBJ_HT_P(z)->get(z, &rv2);

            if (z == &rv) {
                zval_ptr_dtor(&rv);
            }
            ZVAL_COPY_VALUE(z, value);
        }

        // only for ZEND_ASSIGN_CONCAT
        z_fname = zend_hash_index_find(&XMARK_G(callbacks), ZEND_ASSIGN_CONCAT);
        if (z_fname) {
            ZVAL_COPY_VALUE(&call_func_params[0], Z_ISREF_P(z) ? Z_REFVAL_P(z) : z);
            ZVAL_COPY_VALUE(&call_func_params[1], value);
            if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
                zend_error(E_WARNING, "call function error");
            }

            ZVAL_COPY_VALUE(&res, &call_func_ret);
        } else {
            binary_op(&res, Z_ISREF_P(z) ? Z_REFVAL_P(z) : z, value);
        }

        Z_OBJ_HT_P(object)->write_dimension(object, property, &res);
        if (z == &rv) {
            zval_ptr_dtor(&rv);
        }
        if (retval) {
            ZVAL_COPY(retval, &res);
        }

        zval_ptr_dtor(&res);
    } else {
        zend_error(E_WARNING, "Attempt to assign property of non-object");
        if (retval) {
            ZVAL_NULL(retval);
        }
    }
}
/* }}} */
/* Copied codes end */

/* These are most copied from taint.c: zend_fetch_dimension_address */

static zval *php_xmark_get_zval_ptr_tmpvar(zend_execute_data *execute_data, uint32_t var, zend_free_op *should_free) /* {{{ */ {
    zval *ret = EX_VAR(var);

    if (should_free) {
        *should_free = ret;
    }
    ZVAL_DEREF(ret);

    return ret;
}
/* }}} */

#ifndef CV_DEF_OF
#define CV_DEF_OF(i) (EX(func)->op_array.vars[i])
#endif

static zval *php_xmark_get_zval_ptr_cv(zend_execute_data *execute_data, uint32_t var, int type, int force_ret) /* {{{ */ {
    zval *ret = EX_VAR(var);

    if (UNEXPECTED(Z_TYPE_P(ret) == IS_UNDEF)) {
        if (force_ret) {
            switch (type) {
                case BP_VAR_R:
                case BP_VAR_UNSET:
                    zend_error(E_NOTICE, "Undefined variable: %s", ZSTR_VAL(CV_DEF_OF(EX_VAR_TO_NUM(var))));
                case BP_VAR_IS:
                    ret = &EG(uninitialized_zval);
                    break;
                case BP_VAR_RW:
                    zend_error(E_NOTICE, "Undefined variable: %s", ZSTR_VAL(CV_DEF_OF(EX_VAR_TO_NUM(var))));
                case BP_VAR_W:
                    ZVAL_NULL(ret);
                    break;
            }
        } else {
            return NULL;
        }
    } else {
        ZVAL_DEREF(ret);
    }
    return ret;
}
/* }}} */


static zval *php_xmark_get_zval_ptr(zend_execute_data *execute_data, int op_type, znode_op op, zend_free_op *should_free, int type, int force_ret) /* {{{ */ {
    if (op_type & (IS_TMP_VAR|IS_VAR)) {
        return php_xmark_get_zval_ptr_tmpvar(execute_data, op.var, should_free);
    } else {
        *should_free = NULL;
        if (op_type == IS_CONST) {
            return EX_CONSTANT(op);
        } else if (op_type == IS_CV) {
            return php_xmark_get_zval_ptr_cv(execute_data, op.var, type, force_ret);
        } else {
            return NULL;
        }
    }
}
/* }}} */


static zval *php_xmark_get_zval_ptr_ptr_var(zend_execute_data *execute_data, uint32_t var, zend_free_op *should_free) /* {{{ */ {
    zval *ret = EX_VAR(var);

    if (EXPECTED(Z_TYPE_P(ret) == IS_INDIRECT)) {
        *should_free = NULL;
        ret = Z_INDIRECT_P(ret);
    } else {
        *should_free = ret;
    }
    return ret;
}
/* }}} */


static zval *php_xmark_get_zval_ptr_ptr(zend_execute_data *execute_data, int op_type, znode_op op, zend_free_op *should_free, int type) /* {{{ */ {
    if (op_type == IS_CV) {
        *should_free = NULL;
        return php_xmark_get_zval_ptr_cv(execute_data, op.var, type, 1);
    } else if (op_type == IS_VAR) {
        ZEND_ASSERT(op_type == IS_VAR);
        return php_xmark_get_zval_ptr_ptr_var(execute_data, op.var, should_free);
    } else if (op_type == IS_UNUSED) {
        *should_free = NULL;
        return &EX(This);
    } else {
        ZEND_ASSERT(0);
    }
}
/* }}} */


static int php_xmark_binary_assign_op_helper(binary_op_type binary_op, zend_execute_data *execute_data) /* {{{ */ {
    const zend_op *opline = execute_data->opline;
    zval *var_ptr, *value;
    zend_free_op free_op1, free_op2;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];

    value = php_xmark_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 1);
    var_ptr = php_xmark_get_zval_ptr_ptr(execute_data, opline->op1_type, opline->op1, &free_op1, BP_VAR_RW);

    if (opline->op1_type == IS_VAR) {
        if (var_ptr == NULL || XMARK_ISERR(var_ptr)) {
            return ZEND_USER_OPCODE_DISPATCH;
        }
    }

    z_fname = zend_hash_index_find(&XMARK_G(callbacks), ZEND_ASSIGN_CONCAT);
    if (z_fname) {
        ZVAL_COPY_VALUE(&call_func_params[0], var_ptr);
        ZVAL_COPY_VALUE(&call_func_params[1], value);
        if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
            zend_error(E_WARNING, "call function error");
        }

        SEPARATE_ZVAL_NOREF(var_ptr);
        zval_ptr_dtor_nogc(var_ptr);
        ZVAL_COPY_VALUE(var_ptr, &call_func_ret);
    } else {
        SEPARATE_ZVAL_NOREF(var_ptr);
        binary_op(var_ptr, var_ptr, value);

    }

    if (XMARK_RET_USED(opline)) {
        ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
    }

    if ((XMARK_OP1_TYPE(opline) & (IS_VAR|IS_TMP_VAR)) && free_op1) {
        zval_ptr_dtor_nogc(free_op1);
    }

    if ((XMARK_OP2_TYPE(opline) & (IS_VAR|IS_TMP_VAR)) && free_op2) {
        zval_ptr_dtor_nogc(free_op2);
    }

    execute_data->opline++;

    return ZEND_USER_OPCODE_CONTINUE;
}
/* }}} */


static int php_xmark_binary_assign_op_obj_helper(binary_op_type binary_op, zend_execute_data *execute_data) /* {{{ */ {
    const zend_op *opline = execute_data->opline;
    zval *object, *property, *var_ptr, *value;
    zend_free_op free_op1, free_op2, free_op_data;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];

    object = php_xmark_get_zval_ptr_ptr(execute_data, opline->op1_type, opline->op1, &free_op1, BP_VAR_RW);
    if (opline->op1_type == IS_UNUSED && Z_OBJ_P(object) == NULL) {
        return ZEND_USER_OPCODE_DISPATCH;
    }
    if (opline->op1_type == IS_VAR && object == NULL) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    property = php_xmark_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 1);

    do {
        if (opline->op1_type == IS_UNUSED || Z_TYPE_P(object) != IS_OBJECT) {
            if (!php_xmark_make_real_object(object)) {
                zend_error(E_WARNING, "Attempt to assign property of non-object");
                if (XMARK_RET_USED(opline)) {
                    ZVAL_NULL(EX_VAR(opline->result.var));
                }
                break;
            }
        }

        value = php_xmark_get_zval_ptr(execute_data, (opline + 1)->op1_type, (opline + 1)->op1, &free_op_data, BP_VAR_R, 1);

        if (Z_OBJ_HT_P(object)->get_property_ptr_ptr
            && (var_ptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property, BP_VAR_RW, NULL)) != NULL) {
            ZVAL_DEREF(var_ptr);
            SEPARATE_ZVAL_NOREF(var_ptr);

            z_fname = zend_hash_index_find(&XMARK_G(callbacks), ZEND_ASSIGN_CONCAT);
            if (z_fname) {
                ZVAL_COPY_VALUE(&call_func_params[0], var_ptr);
                ZVAL_COPY_VALUE(&call_func_params[1], value);
                if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
                    zend_error(E_WARNING, "call function error");
                }

                zval_ptr_dtor_nogc(var_ptr);
                ZVAL_COPY_VALUE(var_ptr, &call_func_ret);
            } else {
                binary_op(var_ptr, var_ptr, value);
            }

            if (XMARK_RET_USED(opline)) {
                ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
            }
        } else {
            php_xmark_assign_op_overloaded_property(object, property, NULL, value, binary_op, EX_VAR(opline->result.var));
            if (!XMARK_RET_USED(opline)) {
                zval_ptr_dtor_nogc(EX_VAR(opline->result.var));
            }
        }
    } while (0);

    if ((opline->op2_type & (IS_VAR|IS_TMP_VAR)) && free_op2) {
        zval_ptr_dtor_nogc(free_op2);
    }
    if (((opline + 1)->op1_type & (IS_VAR|IS_TMP_VAR)) && free_op_data)   {
        zval_ptr_dtor_nogc(free_op_data);
    }
    if ((opline->op1_type & (IS_VAR|IS_TMP_VAR)) && free_op1) {
        zval_ptr_dtor_nogc(free_op1);
    }
    execute_data->opline += 2;

    return ZEND_USER_OPCODE_CONTINUE;
}
/* }}} */


static int php_xmark_binary_assign_op_dim_helper(binary_op_type binary_op, zend_execute_data *execute_data) /* {{{ */ {
    const zend_op *opline = execute_data->opline;
    zval *container, *dim, *var_ptr, *value, rv;
    zend_free_op free_op1, free_op2, free_op_data;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];

    container = php_xmark_get_zval_ptr_ptr(execute_data, opline->op1_type, opline->op1, &free_op1, BP_VAR_RW);
    if (opline->op1_type == IS_UNUSED && Z_OBJ_P(container) == NULL) {
        return ZEND_USER_OPCODE_DISPATCH;
    }
    if (opline->op1_type == IS_VAR && container == NULL) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    dim = php_xmark_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 1);

    do {
        if (opline->op1_type == IS_UNUSED || Z_TYPE_P(container) == IS_OBJECT) {
            value = php_xmark_get_zval_ptr(execute_data, (opline + 1)->op1_type, (opline + 1)->op1, &free_op_data, BP_VAR_R, 1);
            php_xmark_binary_assign_op_obj_dim(container, dim, value, EX_VAR(opline->result.var), binary_op);

            if (!XMARK_RET_USED(opline)) {
                zval_ptr_dtor_nogc(EX_VAR(opline->result.var));
            }
            break;
        }

        php_xmark_fetch_dimension_address(&rv, container, dim, opline->op2_type, BP_VAR_RW);
        value = php_xmark_get_zval_ptr(execute_data, (opline + 1)->op1_type, (opline + 1)->op1, &free_op_data, BP_VAR_R, 1);

        if (Z_TYPE(rv) != IS_INDIRECT) {
            var_ptr = NULL;
        } else {
            var_ptr = Z_INDIRECT(rv);
        }

        if (var_ptr == NULL) {
            zend_throw_error(NULL, "Cannot use assign-op operators with overloaded objects nor string offsets");
            if ((opline->op2_type & (IS_VAR|IS_TMP_VAR)) && free_op2) {
                zval_ptr_dtor_nogc(free_op2);
            }
            if (((opline + 1)->op1_type & (IS_VAR|IS_TMP_VAR)) && free_op_data)   {
                zval_ptr_dtor_nogc(free_op_data);
            }
            if ((opline->op1_type & (IS_VAR|IS_TMP_VAR)) && free_op1) {
                zval_ptr_dtor_nogc(free_op1);
            }
            execute_data->opline += 2;
            return ZEND_USER_OPCODE_CONTINUE;
        }

        if (XMARK_ISERR(var_ptr)) {
            if (XMARK_RET_USED(opline)) {
                ZVAL_NULL(EX_VAR(opline->result.var));
            }
        } else {
            ZVAL_DEREF(var_ptr);
            SEPARATE_ZVAL_NOREF(var_ptr);

            z_fname = zend_hash_index_find(&XMARK_G(callbacks), ZEND_ASSIGN_CONCAT);
            if (z_fname) {
                ZVAL_COPY_VALUE(&call_func_params[0], var_ptr);
                ZVAL_COPY_VALUE(&call_func_params[1], value);

                if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
                    zend_error(E_WARNING, "call function error");
                }
                zval_ptr_dtor_nogc(var_ptr);
                ZVAL_COPY_VALUE(var_ptr, &call_func_ret);
            } else {
                binary_op(var_ptr, var_ptr, value);
            }
            if (XMARK_RET_USED(opline)) {
                ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
            }
        }
    } while (0);

    if ((opline->op2_type & (IS_VAR|IS_TMP_VAR)) && free_op2) {
        zval_ptr_dtor_nogc(free_op2);
    }
    if (((opline + 1)->op1_type & (IS_VAR|IS_TMP_VAR)) && free_op_data)   {
        zval_ptr_dtor_nogc(free_op_data);
    }
    if ((opline->op1_type & (IS_VAR|IS_TMP_VAR)) && free_op1) {
        zval_ptr_dtor_nogc(free_op1);
    }
    execute_data->opline += 2;

    return ZEND_USER_OPCODE_CONTINUE;
}
/* }}} */
/* Copied codes end */


static int php_xmark_op1_handler(zend_execute_data *execute_data) {
    const zend_op *opline = execute_data->opline;
    zend_free_op free_op1;
    zval *op1;
    zval *z_fname;
    zval call_func_ret;

    if (XMARK_G(in_callback)) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    z_fname = zend_hash_index_find(&XMARK_G(callbacks), opline->opcode);
    if (!z_fname) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    XMARK_G(in_callback) = 1;

    op1 = php_xmark_get_zval_ptr(execute_data, opline->op1_type, opline->op1, &free_op1, BP_VAR_R, 0);

    if (op1) {
        if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 1, op1)) {
            zend_error(E_WARNING, "call function error");
        }

        zval_ptr_dtor_nogc(&call_func_ret);
    }

    XMARK_G(in_callback) = 0;
    return ZEND_USER_OPCODE_DISPATCH;
}


static int php_xmark_op2_handler(zend_execute_data *execute_data) {
    const zend_op *opline = execute_data->opline;
    zend_free_op free_op2;
    zval *op2;
    zval *z_fname;
    zval call_func_ret;

    if (XMARK_G(in_callback)) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    z_fname = zend_hash_index_find(&XMARK_G(callbacks), opline->opcode);
    if (!z_fname) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    XMARK_G(in_callback) = 1;
    op2 = php_xmark_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 0);

    if (op2) {
        if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 1, op2)) {
            zend_error(E_WARNING, "call function error");
        }
        zval_ptr_dtor_nogc(&call_func_ret);
    }

    XMARK_G(in_callback) = 0;
    return ZEND_USER_OPCODE_DISPATCH;
}


static int php_xmark_concat_handler(zend_execute_data *execute_data) {
    const zend_op *opline = execute_data->opline;
    zval *op1, *op2, *result;
    zend_free_op free_op1, free_op2;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];

    if (XMARK_G(in_callback)) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    z_fname = zend_hash_index_find(&XMARK_G(callbacks), opline->opcode);
    if (!z_fname) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    XMARK_G(in_callback) = 1;

    op1 = php_xmark_get_zval_ptr(execute_data, opline->op1_type, opline->op1, &free_op1, BP_VAR_R, 1);
    op2 = php_xmark_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 1);

    result = EX_VAR(opline->result.var);

    if (op1 && op2) {
        ZVAL_COPY_VALUE(&call_func_params[0], op1);
        ZVAL_COPY_VALUE(&call_func_params[1], op2);
        if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
            zend_error(E_WARNING, "call function error");
        }

        ZVAL_COPY_VALUE(result, &call_func_ret);
    }

    if ((XMARK_OP1_TYPE(opline) & (IS_VAR|IS_TMP_VAR)) && free_op1) {
        zval_ptr_dtor_nogc(free_op1);
    }

    if ((XMARK_OP2_TYPE(opline) & (IS_VAR|IS_TMP_VAR)) && free_op2) {
        zval_ptr_dtor_nogc(free_op2);
    }

    execute_data->opline++;
    XMARK_G(in_callback) = 0;

    return ZEND_USER_OPCODE_CONTINUE;
}


static int php_xmark_assign_concat_handler(zend_execute_data *execute_data) /* {{{ */ {
    const zend_op *opline = execute_data->opline;
    int result = 0;

    if (XMARK_G(in_callback)) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    XMARK_G(in_callback) = 1;

    if (EXPECTED(opline->extended_value == 0)) {
        result = php_xmark_binary_assign_op_helper(concat_function, execute_data);
    } else if (EXPECTED(opline->extended_value == ZEND_ASSIGN_DIM)) {
        result = php_xmark_binary_assign_op_dim_helper(concat_function, execute_data);
    } else {
        result = php_xmark_binary_assign_op_obj_helper(concat_function, execute_data);
    }

    XMARK_G(in_callback) = 0;
    return result;
} /* }}} */


/**
 * "$testval "
 * @param execute_data
 * @return
 */
static int php_xmark_rope_end_handler(zend_execute_data *execute_data) {
    const zend_op *opline = execute_data->opline;
    zval *op2, *result;
    zend_free_op free_op2;
    zend_string **rope;
    int i;
    zval *z_fname;
    zval call_func_ret, call_func_params[1];
    zval z_rope;

    if (XMARK_G(in_callback)) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    z_fname = zend_hash_index_find(&XMARK_G(callbacks), opline->opcode);
    if (!z_fname) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    XMARK_G(in_callback) = 1;

    rope = (zend_string **)EX_VAR(opline->op1.var);
    op2 = php_xmark_get_zval_ptr(execute_data, opline->op2_type, opline->op2, &free_op2, BP_VAR_R, 1);
    result = EX_VAR(opline->result.var);

    ZVAL_NEW_ARR(&z_rope);
    zend_hash_init(Z_ARRVAL(z_rope), opline->extended_value+1, NULL, ZVAL_PTR_DTOR, 0);

    if ((XMARK_OP2_TYPE(opline) & IS_CV) || Z_TYPE_P(op2) != IS_STRING)
        rope[opline->extended_value] = zval_get_string(op2);
    else
        rope[opline->extended_value] = Z_STR_P(op2);

    zval tmp;
    for (i=0; i<=opline->extended_value; i++) {
        ZVAL_STR(&tmp, rope[i]);
        zend_hash_next_index_insert(Z_ARRVAL(z_rope), &tmp);
    }

    ZVAL_COPY_VALUE(&call_func_params[0], &z_rope);

    if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 1, call_func_params)) {
        zend_error(E_WARNING, "call function error");
    }

    if (GC_REFCOUNT(Z_ARRVAL(z_rope)) <= 1)
        zend_array_destroy(Z_ARRVAL(z_rope));
    else
        GC_REFCOUNT(Z_ARRVAL(z_rope))--;

    ZVAL_COPY_VALUE(result, &call_func_ret);
    execute_data->opline++;
    XMARK_G(in_callback) = 0;

    return ZEND_USER_OPCODE_CONTINUE;
}


static int php_xmark_fcall_handler(zend_execute_data *execute_data) {
    const zend_op *opline = execute_data->opline;
    zend_execute_data *call = execute_data->call;
    zend_function *fbc = call->func;
    zval *z_fname;
    zval call_func_ret, call_func_params[2];
    zval z_call, z_params, tmp;
    zend_string *fname, *cname;
    uint32_t i;

    ZEND_CALL_ARG(call, 1);

    if (XMARK_G(in_callback)) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    z_fname = zend_hash_index_find(&XMARK_G(callbacks), opline->opcode);
    if (!z_fname) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    if (!fbc->common.function_name) {
        return ZEND_USER_OPCODE_DISPATCH;
    }

    XMARK_G(in_callback) = 1;

    uint32_t arg_count = ZEND_CALL_NUM_ARGS(call);

    ZVAL_NEW_ARR(&z_params);
    zend_hash_init(Z_ARRVAL(z_params), arg_count, NULL, ZVAL_PTR_DTOR, 0);

    for (i=0; i<arg_count; i++) {
        zval *p = ZEND_CALL_ARG(call, i + 1);
        if (Z_REFCOUNTED_P(p)) Z_ADDREF_P(p);
        zend_hash_next_index_insert(Z_ARRVAL(z_params), p);
    }

    ZVAL_COPY_VALUE(&call_func_params[1], &z_params);

    if (fbc->common.scope == NULL) {
        fname = fbc->common.function_name;
        ZVAL_STR_COPY(&z_call, fname);
        ZVAL_COPY_VALUE(&call_func_params[0], &z_call);
    } else {
        cname = fbc->common.scope->name;
        fname = fbc->common.function_name;

        ZVAL_NEW_ARR(&z_call);
        zend_hash_init(Z_ARRVAL(z_call), 2, NULL, ZVAL_PTR_DTOR, 0);

        ZVAL_STR_COPY(&tmp, cname);
        zend_hash_next_index_insert(Z_ARRVAL(z_call), &tmp);
        ZVAL_STR_COPY(&tmp, fname);
        zend_hash_next_index_insert(Z_ARRVAL(z_call), &tmp);

        ZVAL_COPY_VALUE(&call_func_params[0], &z_call);
    }

    if (SUCCESS != call_user_function(EG(function_table), NULL, z_fname, &call_func_ret, 2, call_func_params)) {
        zend_error(E_WARNING, "call function error");
    }

    if (IS_ARRAY == Z_TYPE_P(&z_call)) {
        if (GC_REFCOUNT(Z_COUNTED(z_call)) <= 1)
            zend_array_destroy(Z_ARRVAL(z_call));
        else
            Z_DELREF(z_call);
    } else {
        zend_string_release(Z_STR(z_call));
    }

    if (GC_REFCOUNT(Z_COUNTED(z_params)) <= 1)
        zend_array_destroy(Z_ARRVAL(z_params));
    else
        Z_DELREF(z_params);


    XMARK_G(in_callback) = 0;

    return ZEND_USER_OPCODE_DISPATCH;
}


static void php_xmark_register_opcode_handlers()
{
    zend_set_user_opcode_handler(ZEND_ECHO, php_xmark_op1_handler);
    zend_set_user_opcode_handler(ZEND_EXIT, php_xmark_op1_handler);
    zend_set_user_opcode_handler(ZEND_INIT_METHOD_CALL, php_xmark_op2_handler);
    zend_set_user_opcode_handler(ZEND_INIT_USER_CALL, php_xmark_op2_handler);
    zend_set_user_opcode_handler(ZEND_INIT_DYNAMIC_CALL, php_xmark_op2_handler);
    zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, php_xmark_op1_handler);
    zend_set_user_opcode_handler(ZEND_CONCAT, php_xmark_concat_handler);
    zend_set_user_opcode_handler(ZEND_FAST_CONCAT, php_xmark_concat_handler);
    zend_set_user_opcode_handler(ZEND_ASSIGN_CONCAT, php_xmark_assign_concat_handler);
    zend_set_user_opcode_handler(ZEND_ROPE_END, php_xmark_rope_end_handler);
    zend_set_user_opcode_handler(ZEND_DO_FCALL, php_xmark_fcall_handler);
    zend_set_user_opcode_handler(ZEND_DO_ICALL, php_xmark_fcall_handler);
    zend_set_user_opcode_handler(ZEND_DO_UCALL, php_xmark_fcall_handler);
    zend_set_user_opcode_handler(ZEND_DO_FCALL_BY_NAME, php_xmark_fcall_handler);
}


static void rename_from_ini_value(HashTable *ht, const char *ini_value)
{
    char *e, *orig_name = NULL, *new_name = NULL;

    if (!ini_value) {
        return;
    }

    e = strdup(ini_value);
    if (e == NULL) {
        return;
    }

    while (*e) {
        switch (*e) {
            case ' ':
            case '\r':
            case '\n':
            case '\t':
            case ',':
                if (orig_name && new_name) {
                    *e = '\0';
                    rename_hash_str_key(ht, orig_name, new_name);
                }
                orig_name = NULL;
                new_name = NULL;
                break;
            case ':':
                if (orig_name) {
                    *e = '\0';
                }
                if (!new_name) {
                    new_name = e + 1;
                }
                break;
            default:
                if (!orig_name) {
                    orig_name = e;
                }
                break;
        }
        e++;
    }
    if (orig_name && new_name) {
        rename_hash_str_key(ht, orig_name, new_name);
    }
}

static zend_always_inline int xmark_zstr(zval *z_str)
{
    if (!XCHECK_FLAG(Z_STR_P(z_str))) {
        zend_string *str = zend_string_init(Z_STRVAL_P(z_str), Z_STRLEN_P(z_str), 0);
        ZSTR_LEN(str) = Z_STRLEN_P(z_str);
        zend_string_release(Z_STR_P(z_str));
        XMARK_FLAG(str);
        ZVAL_STR(z_str, str);
    }

    return SUCCESS;
}


static zend_always_inline Bucket *rename_hash_key(HashTable *ht, zend_string *orig_name, zend_string *new_name)
{
    zend_ulong h;
    uint32_t nIndex;
    uint32_t idx;
    Bucket *p = NULL, *arData, *prev = NULL;
    zend_bool found = 0;

    orig_name = zend_string_tolower(orig_name);
    new_name = zend_string_tolower(new_name);

    if (zend_hash_exists(ht, new_name)) {
        zend_string_release(orig_name);
        zend_string_release(new_name);
        zend_error(E_ERROR, "function/class '%s' already exists", ZSTR_VAL(new_name));
        return NULL;
    }

    h = zend_string_hash_val(orig_name);
    arData = ht->arData;
    nIndex = h | ht->nTableMask;
    idx = HT_HASH_EX(arData, nIndex);
    while (EXPECTED(idx != HT_INVALID_IDX)) {
        prev = p;
        p = HT_HASH_TO_BUCKET_EX(arData, idx);
        if (EXPECTED(p->key == orig_name)) { /* check for the same interned string */
            found = 1;
            break;
        } else if (EXPECTED(p->h == h) &&
                   EXPECTED(p->key) &&
                   EXPECTED(ZSTR_LEN(p->key) == ZSTR_LEN(orig_name)) &&
                   EXPECTED(memcmp(ZSTR_VAL(p->key), ZSTR_VAL(orig_name), ZSTR_LEN(orig_name)) == 0)) {
            found = 1;
            break;
        }
        idx = Z_NEXT(p->val);
    }

    if (!found) {
        zend_string_release(orig_name);
        zend_string_release(new_name);
        zend_error(E_ERROR, "function/class '%s' does not exists", ZSTR_VAL(orig_name));
        return NULL;
    }

    // rehash
    if (!prev && Z_NEXT(p->val) == HT_INVALID_IDX) {  // only p
        HT_HASH(ht, nIndex) = HT_INVALID_IDX;
    } else if (prev && Z_NEXT(p->val) != HT_INVALID_IDX) {  // p in middle
        Z_NEXT(prev->val) = Z_NEXT(p->val);
    } else if (prev && Z_NEXT(p->val) == HT_INVALID_IDX) {  // p in tail
        Z_NEXT(prev->val) = HT_INVALID_IDX;
    } else if (!prev && Z_NEXT(p->val) != HT_INVALID_IDX) {  // p in head
        HT_HASH(ht, nIndex) = Z_NEXT(p->val);
    }

    zend_string_release(p->key);
    p->key = zend_string_init_interned(ZSTR_VAL(new_name), ZSTR_LEN(new_name), 1);
    p->h = h = zend_string_hash_val(p->key);
    nIndex = h | ht->nTableMask;

    if (HT_HASH(ht, nIndex) != HT_INVALID_IDX)
        Z_NEXT(p->val) = HT_HASH(ht, nIndex);

    // 这里没必要再继续使用 HT_IDX_TO_HASH, 因为我们直接从 ht 中拿到的 idx, 不然 x86 下会报错
    HT_HASH(ht, nIndex) = idx;

    zend_string_release(orig_name);
    zend_string_release(new_name);

    return p;
}


static zend_always_inline Bucket *rename_hash_str_key(HashTable *ht, const char *orig_name, const char *new_name)
{
    zend_string *str_orig_name, *str_new_name;
    Bucket *p;

    str_orig_name = zend_string_init(orig_name, strlen(orig_name), 0);
    str_new_name = zend_string_init(new_name, strlen(new_name), 0);

    p = rename_hash_key(ht, str_orig_name, str_new_name);

    zend_string_release(str_orig_name);
    zend_string_release(str_new_name);

    return p;
}


static void clear_function_run_time_cache(zend_function *fbc)
{
    if (fbc->type != ZEND_USER_FUNCTION ||
            fbc->op_array.cache_size == 0 || fbc->op_array.run_time_cache == NULL) return;

    memset(fbc->op_array.run_time_cache, 0, fbc->op_array.cache_size);
}

static void clear_run_time_cache()
{
    zend_function *fbc;
    zend_class_entry *ce;

    ZEND_HASH_FOREACH_PTR(EG(function_table), fbc) {
        clear_function_run_time_cache(fbc);
    } ZEND_HASH_FOREACH_END();


    ZEND_HASH_FOREACH_PTR(EG(class_table), ce) {
        ZEND_HASH_FOREACH_PTR(&(ce->function_table), fbc) {
            clear_function_run_time_cache(fbc);
        } ZEND_HASH_FOREACH_END();
    } ZEND_HASH_FOREACH_END();
}


/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
                STD_PHP_INI_BOOLEAN("xmark.enable",      "0", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_xmark_globals, xmark_globals)
                STD_PHP_INI_ENTRY("xmark.rename_functions", "", PHP_INI_SYSTEM, OnUpdateString, rename_functions, zend_xmark_globals, xmark_globals)
                STD_PHP_INI_ENTRY("xmark.rename_classes", "", PHP_INI_SYSTEM, OnUpdateString, rename_classes, zend_xmark_globals, xmark_globals)
PHP_INI_END()
/* }}} */


PHP_FUNCTION(xid)
{
    zval *zv;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &zv) == FAILURE) {
        return;
    }

    RETURN_LONG((zend_long)zv->value.ptr);
}


PHP_FUNCTION(xmark)
{
    zval *z_str;

    if (!XMARK_G(enable)) {
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &z_str) == FAILURE) {
        return;
    }

    ZVAL_DEREF(z_str);
    if (IS_STRING != Z_TYPE_P(z_str) || Z_STRLEN_P(z_str) == 0) {
        RETURN_FALSE;
    }

    if (xmark_zstr(z_str) == FAILURE) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(xclear)
{
    zval *z_str;

    if (!XMARK_G(enable)) {
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &z_str) == FAILURE) {
        return;
    }

    ZVAL_DEREF(z_str);
    if (IS_STRING != Z_TYPE_P(z_str) || Z_STRLEN_P(z_str) == 0 || !XCHECK_FLAG(Z_STR_P(z_str))) {
        RETURN_FALSE;
    }

    XCLEAR_FLAG(Z_STR_P(z_str));
    RETURN_TRUE;
}


PHP_FUNCTION(xcheck)
{
    zval *z_str;

    if (!XMARK_G(enable)) {
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &z_str) == FAILURE) {
        return;
    }

    ZVAL_DEREF(z_str);
    if (IS_STRING != Z_TYPE_P(z_str) || Z_STRLEN_P(z_str) == 0 || !XCHECK_FLAG(Z_STR_P(z_str))) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


/*
 * xrename_function('phpinfo', 'myphpinfo')
 */
PHP_FUNCTION(xrename_function)
{
    zend_string *orig_fname, *new_fname, *lc_orig_fname;
    zval *z_func;

    if (!XMARK_G(enable)) {
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &orig_fname, &new_fname) == FAILURE) {
        return;
    }

    lc_orig_fname = zend_string_tolower(orig_fname);
    z_func = zend_hash_find(EG(function_table), lc_orig_fname);
    zend_string_release(lc_orig_fname);
    if (!z_func) {
        zend_error(E_ERROR, "function '%s' does not exists", ZSTR_VAL(orig_fname));
        return;
    }

    if (Z_FUNC_P(z_func)->type != ZEND_USER_FUNCTION) {
        zend_error(E_ERROR, "xrename_function can only rename user function");
        return;
    }

    Bucket *p = rename_hash_key(EG(function_table), orig_fname, new_fname);
    if (!p) {
        zend_error(E_ERROR, "rename function '%s' to '%s' failed", ZSTR_VAL(orig_fname), ZSTR_VAL(new_fname));
        RETURN_FALSE;
    }

    zend_string_release(Z_FUNC(p->val)->common.function_name);
    Z_FUNC(p->val)->common.function_name = zend_string_init_interned(ZSTR_VAL(new_fname), ZSTR_LEN(new_fname), 1);

    clear_run_time_cache();

    RETURN_TRUE;
}


PHP_FUNCTION(xrename_class)
{
    zend_string *orig_cname, *new_cname, *lc_orig_cname;
    zval *z_class;

    if (!XMARK_G(enable)) {
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &orig_cname, &new_cname) == FAILURE) {
        return;
    }

    lc_orig_cname = zend_string_tolower(orig_cname);
    z_class = zend_hash_find(EG(class_table), lc_orig_cname);
    zend_string_release(lc_orig_cname);
    if (!z_class) {
        zend_error(E_ERROR, "class '%s' does not exists", ZSTR_VAL(orig_cname));
        return;
    }

    if (Z_CE_P(z_class)->type != ZEND_USER_CLASS) {
        zend_error(E_ERROR, "xrename_class can only rename user class");
        return;
    }

    Bucket *p = rename_hash_key(EG(class_table), orig_cname, new_cname);
    if (!p) {
        zend_error(E_ERROR, "rename class '%s' to '%s' failed", ZSTR_VAL(orig_cname), ZSTR_VAL(new_cname));
        RETURN_FALSE;
    }

    zend_string_release(Z_CE(p->val)->name);
    Z_CE(p->val)->name = zend_string_init_interned(ZSTR_VAL(new_cname), ZSTR_LEN(new_cname), 1);

    RETURN_TRUE;
}


PHP_FUNCTION(xregister_opcode_callback)
{
    zend_fcall_info callable;
    zend_fcall_info_cache call_cache;
    zend_ulong opcode;

    if (!XMARK_G(enable)) {
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "lf", &opcode, &callable, &call_cache) == FAILURE) {
        RETURN_FALSE;
    }

    zend_string_addref(Z_STR(callable.function_name));
    if (!zend_hash_index_update(&XMARK_G(callbacks), opcode, &callable.function_name)) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}



/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xmark)
{
    REGISTER_INI_ENTRIES();

    if (!XMARK_G(enable)) {
        return SUCCESS;
    }

    REGISTER_LONG_CONSTANT("XMARK_ECHO", ZEND_ECHO, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_EXIT", ZEND_EXIT, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_INIT_METHOD_CALL", ZEND_INIT_METHOD_CALL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_INIT_USER_CALL", ZEND_INIT_USER_CALL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_INIT_DYNAMIC_CALL", ZEND_INIT_DYNAMIC_CALL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_INCLUDE_OR_EVAL", ZEND_INCLUDE_OR_EVAL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_CONCAT", ZEND_CONCAT, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_FAST_CONCAT", ZEND_FAST_CONCAT, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_ASSIGN_CONCAT", ZEND_ASSIGN_CONCAT, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_ROPE_END", ZEND_ROPE_END, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_DO_FCALL", ZEND_DO_FCALL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_DO_ICALL", ZEND_DO_ICALL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_DO_UCALL", ZEND_DO_UCALL, CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("XMARK_DO_FCALL_BY_NAME", ZEND_DO_FCALL_BY_NAME, CONST_CS|CONST_PERSISTENT);

    php_xmark_register_opcode_handlers();
    rename_from_ini_value(CG(function_table), XMARK_G(rename_functions));
    rename_from_ini_value(CG(class_table), XMARK_G(rename_classes));

    return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(xmark)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(xmark)
{
    if (XMARK_G(enable)) {
        XMARK_G(in_callback) = 0;
        zend_hash_init(&XMARK_G(callbacks), 1, NULL, ZVAL_PTR_DTOR, 0);
    }

#if defined(COMPILE_DL_XMARK) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(xmark)
{
    if (XMARK_G(enable)) {
        zend_hash_destroy(&XMARK_G(callbacks));
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xmark)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "xmark support", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(xid_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(1, z)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xmark_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(1, string)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xclear_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(1, string)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xcheck_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, string)
ZEND_END_ARG_INFO()

/* {{{ xmark_functions[]
 *
 * Every user visible function must have an entry in xmark_functions[].
 */
const zend_function_entry xmark_functions[] = {
    PHP_FE(xid, xid_arginfo)
    PHP_FE(xmark, xmark_arginfo)
    PHP_FE(xclear, xclear_arginfo)
    PHP_FE(xcheck, xcheck_arginfo)
    PHP_FE(xrename_function, NULL)
    PHP_FE(xrename_class, NULL)
    PHP_FE(xregister_opcode_callback, NULL)
    PHP_FE_END    /* Must be the last line in xmark_functions[] */
};
/* }}} */

/** {{{ module depends
*/
zend_module_dep xmark_deps[] = {
        ZEND_MOD_CONFLICTS("xdebug")
        ZEND_MOD_CONFLICTS("taint")
        {NULL, NULL, NULL}
};
/* }}} */

/* {{{ xmark_module_entry
 */
zend_module_entry xmark_module_entry = {
    STANDARD_MODULE_HEADER_EX, NULL,
    xmark_deps,
    "xmark",
    xmark_functions,
    PHP_MINIT(xmark),
    PHP_MSHUTDOWN(xmark),
    PHP_RINIT(xmark),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(xmark),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(xmark),
    PHP_XMARK_VERSION,
    PHP_MODULE_GLOBALS(xmark),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_XMARK
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(xmark)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
