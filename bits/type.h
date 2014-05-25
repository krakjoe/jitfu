/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe@php.net                                              |
  +----------------------------------------------------------------------+
*/
#ifndef HAVE_BITS_TYPE_H
#define HAVE_BITS_TYPE_H

#define PHP_JIT_TYPE_FUNCTIONS \
	JIT_FE(jit_type_copy) \
	JIT_FE(jit_type_free) \
	JIT_FE(jit_type_create_struct) \
	JIT_FE(jit_type_create_union) \
	JIT_FE(jit_type_create_signature) \
	JIT_FE(jit_type_create_pointer)

static const char *le_jit_type_name = "jit type";
static       int   le_jit_type;

ZEND_RSRC_DTOR_FUNC(php_jit_type_dtor);

static inline php_jit_minit_type(int module_number TSRMLS_DC) {
	le_jit_type = zend_register_list_destructors_ex
		(php_jit_type_dtor, NULL, le_jit_type_name, module_number);

	REGISTER_LONG_CONSTANT("JIT_ABI_CDECL",      (zend_ulong) jit_abi_cdecl,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ABI_VARARG",     (zend_ulong) jit_abi_vararg,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ABI_STDCALL",    (zend_ulong) jit_abi_stdcall,    CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ABI_FASTCALL",   (zend_ulong) jit_abi_fastcall,   CONST_CS|CONST_PERSISTENT);	

	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID",      (zend_ulong) jit_type_void,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_SBYTE",     (zend_ulong) jit_type_sbyte,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UBYTE",     (zend_ulong) jit_type_ubyte,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_SHORT",     (zend_ulong) jit_type_short,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_USHORT",    (zend_ulong) jit_type_ushort,    CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_INT",       (zend_ulong) jit_type_int,       CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UINT",      (zend_ulong) jit_type_uint,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_NINT",      (zend_ulong) jit_type_nint,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_NUINT",     (zend_ulong) jit_type_nuint,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_LONG",      (zend_ulong) jit_type_long,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ULONG",     (zend_ulong) jit_type_ulong,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_FLOAT32",   (zend_ulong) jit_type_float32,   CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_FLOAT64",   (zend_ulong) jit_type_float64,   CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_NFLOAT",    (zend_ulong) jit_type_nfloat,    CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID_PTR",  (zend_ulong) jit_type_void_ptr,  CONST_CS|CONST_PERSISTENT);
}

ZEND_BEGIN_ARG_INFO_EX(jit_type_copy_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_free_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_struct_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_union_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_signature_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, abi)
	ZEND_ARG_INFO(0, returns)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_pointer_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_type_copy);
PHP_FUNCTION(jit_type_free);
PHP_FUNCTION(jit_type_create_struct);
PHP_FUNCTION(jit_type_create_union);
PHP_FUNCTION(jit_type_create_signature);
PHP_FUNCTION(jit_type_create_pointer);
#else
#ifndef HAVE_BITS_TYPE
#define HAVE_BITS_TYPE

/* {{{ php_jit_type_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_type_dtor) {
	//jit_type_free((jit_type_t) rsrc->ptr);
} /* }}} */

/* {{{ jit_type_t jit_type_copy(jit_type_t type) */
PHP_FUNCTION(jit_type_copy) {
	zval *resource;
	jit_type_t type, copy;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(type, jit_type_t, &resource, -1, le_jit_type_name, le_jit_type);
	
	copy = jit_type_copy(type);
	
	ZEND_REGISTER_RESOURCE(return_value, copy, le_jit_type);
} /* }}} */

/* {{{ void jit_type_free(jit_type_t type) */
PHP_FUNCTION(jit_type_free) {
	zval *resource;
	jit_type_t type;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(type, jit_type_t, &resource, -1, le_jit_type_name, le_jit_type);
	
	zend_list_delete(Z_RESVAL_P(resource));
} /* }}} */

/* {{{ jit_type_t jit_type_create_struct(array fields [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_struct) {
	zval       **field;
	HashTable   *fields;
	long         incref = 0;
	HashPosition position;
	jit_type_t  *jfields;
	jit_type_t   structure;
	zend_uint    arg = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "H|l", &fields, &incref) != SUCCESS) {
		return;
	}
	
	jfields = (jit_type_t*) ecalloc(zend_hash_num_elements(fields), sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(fields, &position);	
		zend_hash_get_current_data_ex(fields, (void**) &field, &position) == SUCCESS;
		zend_hash_move_forward_ex(fields, &position)) {
		
		switch (Z_TYPE_PP(field)) {
			case IS_LONG: 
				jfields[arg] = (jit_type_t) Z_LVAL_PP(field); 
			break;
			
			case IS_RESOURCE: {
				jit_type_t res;
				ZEND_FETCH_RESOURCE(res, jit_type_t, field, -1, le_jit_type_name, le_jit_type);
				jfields[arg] = jit_type_copy(res);
			} break;
		}
		arg++;
	}
	
	structure = jit_type_create_struct(jfields, zend_hash_num_elements(fields), incref);
	
	ZEND_REGISTER_RESOURCE(return_value, structure, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_union(array fields [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_union) {
	zval       **field;
	HashTable   *fields;
	HashPosition position;
	jit_type_t  *jfields;
	jit_type_t   structure;
	zend_uint    arg = 0;
	long         incref = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Hl", &fields, &incref) != SUCCESS) {
		return;
	}
	
	jfields = (jit_type_t*) ecalloc(zend_hash_num_elements(fields), sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(fields, &position);	
		zend_hash_get_current_data_ex(fields, (void**) &field, &position) == SUCCESS;
		zend_hash_move_forward_ex(fields, &position)) {
		
		switch (Z_TYPE_PP(field)) {
			case IS_LONG: 
				jfields[arg] = (jit_type_t) Z_LVAL_PP(field); 
			break;
			
			case IS_RESOURCE: {
				jit_type_t res;
				ZEND_FETCH_RESOURCE(res, jit_type_t, field, -1, le_jit_type_name, le_jit_type);
				jfields[arg] = jit_type_copy(res);
			} break;
		}
		arg++;
	}
	
	structure = jit_type_create_union(jfields, zend_hash_num_elements(fields), incref);
	
	ZEND_REGISTER_RESOURCE(return_value, structure, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_signature(int abi, mixed returns, array params [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_signature) 
{
	long         abi;
	zval       **zparam, *zreturns;
	HashTable   *zparams;
	HashPosition position;
	jit_type_t   returns;
	jit_type_t  *params;
	zend_uint    param = 0;
	jit_type_t   signature;
	long         incref = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lzH|l", &abi, &zreturns, &zparams, &incref) != SUCCESS) {
		return;
	}
	
	params = (jit_type_t*) ecalloc(zend_hash_num_elements(zparams), sizeof(jit_type_t));
	
	if (Z_TYPE_P(zreturns) == IS_LONG) {
		returns = (jit_type_t) Z_LVAL_P(zreturns);
	} else if (Z_TYPE_P(zreturns) == IS_RESOURCE) {
		ZEND_FETCH_RESOURCE(returns, jit_type_t, &zreturns, -1, le_jit_type_name, le_jit_type);
	}
	
	for (zend_hash_internal_pointer_reset_ex(zparams, &position);	
		zend_hash_get_current_data_ex(zparams, (void**) &zparam, &position) == SUCCESS;
		zend_hash_move_forward_ex(zparams, &position)) {
		
		switch (Z_TYPE_PP(zparam)) {
			case IS_LONG:
				params[param] = (jit_type_t) Z_LVAL_PP(zparam); 
			break;
			
			case IS_RESOURCE: {
				jit_type_t res;
				ZEND_FETCH_RESOURCE(res, jit_type_t, zparam, -1, le_jit_type_name, le_jit_type);
				params[param] = jit_type_copy(res);
			} break;
		}
		
		param++;
	}
	
	signature = jit_type_create_signature(
		abi, returns, params, param, incref);

	ZEND_REGISTER_RESOURCE(return_value, signature, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_pointer(jit_type_t type [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_pointer) {
	zval *ztype;
	jit_type_t type, pointer;
	long incref = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &ztype, &incref) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(type, jit_type_t, &ztype, -1, le_jit_type_name, le_jit_type);
	
	pointer = jit_type_create_pointer(type, incref);
	
	ZEND_REGISTER_RESOURCE(return_value, pointer, le_jit_type);
} /* }}} */
#endif
#endif
