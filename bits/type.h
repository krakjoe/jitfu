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

#define PHP_JIT_TYPE_VOID  		1
#define PHP_JIT_TYPE_CHAR		2
#define PHP_JIT_TYPE_INT		3
#define PHP_JIT_TYPE_UINT		4
#define PHP_JIT_TYPE_LONG		5
#define PHP_JIT_TYPE_ULONG		6
#define PHP_JIT_TYPE_DOUBLE		7
#define PHP_JIT_TYPE_VOID_PTR	8

static inline jit_type_t php_jit_type(short type) {
	
	switch (type) {
		case PHP_JIT_TYPE_VOID:		return jit_type_void;
		case PHP_JIT_TYPE_CHAR:		return jit_type_sys_char;
		case PHP_JIT_TYPE_INT:		return jit_type_sys_int;
		case PHP_JIT_TYPE_UINT:		return jit_type_sys_uint;
		case PHP_JIT_TYPE_LONG:		return jit_type_sys_long;
		case PHP_JIT_TYPE_ULONG:	return jit_type_sys_ulong;
		case PHP_JIT_TYPE_DOUBLE:	return jit_type_sys_double;
		case PHP_JIT_TYPE_VOID_PTR:	return jit_type_void_ptr;
	}
	
	return jit_type_void;
}

static inline php_jit_minit_type(int module_number TSRMLS_DC) {
	le_jit_type = zend_register_list_destructors_ex
		(php_jit_type_dtor, NULL, le_jit_type_name, module_number);

	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID",      PHP_JIT_TYPE_VOID,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_CHAR",      PHP_JIT_TYPE_CHAR,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_INT",       PHP_JIT_TYPE_INT,         CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UINT",      PHP_JIT_TYPE_UINT,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_LONG",      PHP_JIT_TYPE_LONG,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ULONG",     PHP_JIT_TYPE_ULONG,       CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_DOUBLE",    PHP_JIT_TYPE_DOUBLE,      CONST_CS|CONST_PERSISTENT);
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

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_signature_arginfo, 0, 0, 2)
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
				jfields[arg] = php_jit_type(Z_LVAL_PP(field)); 
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
				jfields[arg] = php_jit_type(Z_LVAL_PP(field)); 
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

/* {{{ jit_type_t jit_type_create_signature(mixed returns, array params [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_signature) 
{
	zval       **zparam, *zreturns;
	HashTable   *zparams;
	HashPosition position;
	jit_type_t   returns;
	jit_type_t  *params;
	zend_uint    param = 0;
	jit_type_t   signature;
	long         incref = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zH|l", &zreturns, &zparams, &incref) != SUCCESS) {
		return;
	}
	
	params = (jit_type_t*) ecalloc(zend_hash_num_elements(zparams), sizeof(jit_type_t));
	
	if (Z_TYPE_P(zreturns) == IS_LONG) {
		returns = php_jit_type(Z_LVAL_P(zreturns));
	} else if (Z_TYPE_P(zreturns) == IS_RESOURCE) {
		ZEND_FETCH_RESOURCE(returns, jit_type_t, &zreturns, -1, le_jit_type_name, le_jit_type);
	}
	
	for (zend_hash_internal_pointer_reset_ex(zparams, &position);	
		zend_hash_get_current_data_ex(zparams, (void**) &zparam, &position) == SUCCESS;
		zend_hash_move_forward_ex(zparams, &position)) {
		
		switch (Z_TYPE_PP(zparam)) {
			case IS_LONG:
				params[param] = php_jit_type(Z_LVAL_PP(zparam)); 
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
		jit_abi_cdecl, returns, params, param, incref);

	ZEND_REGISTER_RESOURCE(return_value, signature, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_pointer(jit_type_t type [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_pointer) {
	zval *ztype;
	jit_type_t type, pointer;
	long incref = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|l", &ztype, &incref) != SUCCESS) {
		return;
	}
	
	if (ztype) {
		switch (Z_TYPE_P(ztype)) {
			case IS_RESOURCE: {
				ZEND_FETCH_RESOURCE(type, jit_type_t, &ztype, -1, le_jit_type_name, le_jit_type);
			} break;
			
			case IS_LONG:
				type = php_jit_type(Z_LVAL_P(ztype));
			break;
		}
	}
	
	pointer = jit_type_create_pointer(type, incref);
	
	ZEND_REGISTER_RESOURCE(return_value, pointer, le_jit_type);
} /* }}} */
#endif
#endif
