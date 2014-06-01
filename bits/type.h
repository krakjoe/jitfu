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

typedef struct _php_jit_type_t {
	zend_object         std;
	zend_object_handle  h;
	jit_type_t          type;
	zend_ulong          id;
	zend_bool           copied;
} php_jit_type_t;

zend_class_entry *jit_type_ce;

#define PHP_JIT_FETCH_TYPE(from) \
	(php_jit_type_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_TYPE_I(from) \
	(PHP_JIT_FETCH_TYPE(from))->type

#define PHP_JIT_TYPE_VOID  		1
#define PHP_JIT_TYPE_CHAR		2
#define PHP_JIT_TYPE_INT		3
#define PHP_JIT_TYPE_UINT		4
#define PHP_JIT_TYPE_LONG		5
#define PHP_JIT_TYPE_ULONG		6
#define PHP_JIT_TYPE_DOUBLE		7
#define PHP_JIT_TYPE_VOID_PTR	8

jit_type_t php_jit_type(short type);
void php_jit_minit_type(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_type_methods[];
extern zend_object_handlers php_jit_type_handlers;
#else
#ifndef HAVE_BITS_TYPE
#define HAVE_BITS_TYPE
zend_object_handlers php_jit_type_handlers;

jit_type_t php_jit_type(short type) {
	
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

static inline void php_jit_type_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_type_t *ptype = 
		(php_jit_type_t *) zobject;

	zend_object_std_dtor(&ptype->std TSRMLS_CC);

	if (ptype->copied) {
		jit_type_free(ptype->type);
	}
	
	efree(ptype);
}

static inline zend_object_value php_jit_type_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value intern;
	php_jit_type_t *ptype = 
		(php_jit_type_t*) emalloc(sizeof(php_jit_type_t));
	
	zend_object_std_init(&ptype->std, ce TSRMLS_CC);
	object_properties_init(&ptype->std, ce);
	
	ptype->h = zend_objects_store_put(
		ptype, 
		php_jit_type_destroy, NULL, NULL TSRMLS_CC);
	
	intern.handle   = ptype->h;
	intern.handlers = &php_jit_type_handlers;
	
	return intern;
}

void php_jit_minit_type(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JIT", "Type", php_jit_type_methods);
	jit_type_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_type_ce->create_object = php_jit_type_create;
	
	memcpy(
		&php_jit_type_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_type_handlers));
	
	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID",      PHP_JIT_TYPE_VOID,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_CHAR",      PHP_JIT_TYPE_CHAR,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_INT",       PHP_JIT_TYPE_INT,         CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UINT",      PHP_JIT_TYPE_UINT,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_LONG",      PHP_JIT_TYPE_LONG,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ULONG",     PHP_JIT_TYPE_ULONG,       CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_DOUBLE",    PHP_JIT_TYPE_DOUBLE,      CONST_CS|CONST_PERSISTENT);
}

PHP_METHOD(Type, __construct) {
	zval *ztype, *ztypes = NULL;
	php_jit_type_t *intern;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &ztype) != SUCCESS) {
		return;
	}
	
	intern = PHP_JIT_FETCH_TYPE(getThis());
	
	switch (Z_TYPE_P(ztype)) {
		case IS_LONG:
			intern->id   = Z_LVAL_P(ztype);
			intern->type = php_jit_type(Z_LVAL_P(ztype));
		break;
		
		case IS_OBJECT:
			intern->type = jit_type_copy(
				PHP_JIT_FETCH_TYPE_I(ztype));
			intern->copied = 1;
		break;
	}
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_type_methods[] = {
	PHP_ME(Type, __construct,  php_jit_type_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
