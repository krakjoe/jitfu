/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2014 Joe Watkins <krakjoe@php.net>                     |
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
	zend_ulong          pt;
	zend_bool           copied;
} php_jit_type_t;

zend_class_entry *jit_type_ce;

#define PHP_JIT_FETCH_TYPE(from) \
	(php_jit_type_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_TYPE_I(from) \
	(PHP_JIT_FETCH_TYPE(from))->type

#define PHP_JIT_TYPE_VOID  		1
#define PHP_JIT_TYPE_UINT       2
#define PHP_JIT_TYPE_INT        3
#define PHP_JIT_TYPE_ULONG      4
#define PHP_JIT_TYPE_LONG		5
#define PHP_JIT_TYPE_DOUBLE		6
#define PHP_JIT_TYPE_STRING		7
#define PHP_JIT_TYPE_VOID_PTR	8

jit_type_t php_jit_type(short type);
void php_jit_minit_type(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_type_methods[];
extern zend_object_handlers php_jit_type_handlers;

jit_type_t jit_type_string;

#else
#ifndef HAVE_BITS_TYPE
#define HAVE_BITS_TYPE
zend_object_handlers php_jit_type_handlers;

jit_type_t php_jit_type(short type) {

	switch (type) {
		case PHP_JIT_TYPE_VOID:		return jit_type_void;
		case PHP_JIT_TYPE_UINT:     return jit_type_sys_uint;
		case PHP_JIT_TYPE_INT:      return jit_type_sys_int;
		case PHP_JIT_TYPE_ULONG:    return jit_type_sys_ulong;
		case PHP_JIT_TYPE_LONG:		return jit_type_sys_long;
		case PHP_JIT_TYPE_DOUBLE:	return jit_type_sys_double;
		case PHP_JIT_TYPE_STRING:   return jit_type_string;
		case PHP_JIT_TYPE_VOID_PTR:	return jit_type_void_ptr;
	}

	return jit_type_void;
}

static inline void php_jit_type_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_type_t *ptype = 
		(php_jit_type_t *) zobject;

	zend_object_std_dtor(&ptype->std TSRMLS_CC);
}

static inline void php_jit_type_free(void *zobject TSRMLS_DC) {
	php_jit_type_t *ptype = 
		(php_jit_type_t *) zobject;
		
	if (ptype->copied) {
		jit_type_free(ptype->type);
	}
	
	efree(ptype);
}

static inline zend_object_value php_jit_type_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value intern;
	php_jit_type_t *ptype = 
		(php_jit_type_t*) ecalloc(1, sizeof(php_jit_type_t));
	
	zend_object_std_init(&ptype->std, ce TSRMLS_CC);
	object_properties_init(&ptype->std, ce);
	
	ptype->h = zend_objects_store_put(
		ptype, 
		php_jit_type_destroy, 
		php_jit_type_free, NULL TSRMLS_CC);
	
	intern.handle   = ptype->h;
	intern.handlers = &php_jit_type_handlers;
	
	return intern;
}

void php_jit_minit_type(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Type", php_jit_type_methods);
	jit_type_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_type_ce->create_object = php_jit_type_create;
	
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("void"),    PHP_JIT_TYPE_VOID TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("int"),     PHP_JIT_TYPE_INT TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("uint"),    PHP_JIT_TYPE_UINT TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("ulong"),   PHP_JIT_TYPE_ULONG TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("long"),    PHP_JIT_TYPE_LONG TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("double"),  PHP_JIT_TYPE_DOUBLE TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("string"),  PHP_JIT_TYPE_STRING TSRMLS_CC);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("pvoid"),   PHP_JIT_TYPE_VOID_PTR TSRMLS_CC);
	
	memcpy(
		&php_jit_type_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_type_handlers));
	
	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID",      PHP_JIT_TYPE_VOID,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_INT",       PHP_JIT_TYPE_INT,         CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UINT",      PHP_JIT_TYPE_UINT,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_LONG",      PHP_JIT_TYPE_LONG,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ULONG",     PHP_JIT_TYPE_ULONG,       CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_DOUBLE",    PHP_JIT_TYPE_DOUBLE,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_STRING",    PHP_JIT_TYPE_STRING,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID_PTR",  PHP_JIT_TYPE_VOID_PTR,    CONST_CS|CONST_PERSISTENT);

	jit_type_string = jit_type_create_pointer(jit_type_sys_char, 0);
}

PHP_METHOD(Type, __construct) {
	zval *ztype;
	zend_bool zpointer = 0;
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &ztype, &zpointer) != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	switch (Z_TYPE_P(ztype)) {
		case IS_LONG:
			if (zpointer) {
				ptype->type = jit_type_create_pointer(
					php_jit_type(Z_LVAL_P(ztype)), 1);
				ptype->pt   = 1;
			} else ptype->type = php_jit_type(Z_LVAL_P(ztype));
			ptype->id   = Z_LVAL_P(ztype);
		break;
		
		case IS_OBJECT: {
			php_jit_type_t *patype = PHP_JIT_FETCH_TYPE(ztype);
			if (zpointer) {
				ptype->type = jit_type_create_pointer(patype->type, 1);
				ptype->pt   = (patype->pt + 1);
			} else ptype->type = jit_type_copy(patype->type);
			ptype->id = patype->id;
			ptype->copied = 1;
		} break;
	}
}

PHP_METHOD(Type, of) {
	zval *ztype = NULL,
		 **zcache = NULL;
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &ztype) != SUCCESS) {
		return;
	}
	
	if (!ztype) {
		php_jit_exception("null type in constructor");
		return;
	}
	
	switch (Z_TYPE_P(ztype)) {
		case IS_LONG: {
			 if (zend_hash_index_find(&JG(types), Z_LVAL_P(ztype), (void**)&zcache) != SUCCESS) {
			 	object_init_ex(return_value, jit_type_ce);
			 	ptype = PHP_JIT_FETCH_TYPE(return_value);
			 	ptype->id = Z_LVAL_P(ztype);
			 	ptype->type = php_jit_type(Z_LVAL_P(ztype));
			 	zend_hash_index_update(
			 		&JG(types), Z_LVAL_P(ztype), (void**) &return_value, sizeof(zval*), NULL);
			 	Z_ADDREF_P(return_value);
			 } else ZVAL_ZVAL(return_value, *zcache, 1, 0);
		} break;
		
		case IS_STRING: {
			
		} break;
		
		default: {
			php_jit_exception("unexpected type in constructor, %s",
				zend_get_type_by_const(Z_TYPE_P(ztype)));
		}
	}
}

PHP_METHOD(Type, getIdentifier) {
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	RETURN_LONG(ptype->id);
}

PHP_METHOD(Type, getIndirection) {
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	RETURN_LONG(ptype->pt);
}

PHP_METHOD(Type, isPointer) {
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	RETURN_BOOL(ptype->pt > 0);
}

PHP_METHOD(Type, dump) {
	zval *zoutput = NULL;
	php_jit_type_t *ptype;
	php_stream *pstream = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &zoutput) != SUCCESS) {
		return;
	}

	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	if (!zoutput) {
		jit_dump_type(stdout, ptype->type);
		return;
	}
	
	php_stream_from_zval(pstream, &zoutput);
	
	if (php_stream_can_cast(pstream, PHP_STREAM_AS_STDIO|PHP_STREAM_CAST_TRY_HARD) == SUCCESS) {
		FILE *stdio;
		if (php_stream_cast(pstream, PHP_STREAM_AS_STDIO, (void**)&stdio, 0) == SUCCESS) {
			jit_dump_type(stdio, ptype->type);
		}
	}
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_of_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_dump_arginfo, 0, 0, 0) 
	ZEND_ARG_INFO(0, output)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_type_methods[] = {
	PHP_ME(Type, __construct,     php_jit_type_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Type, getIdentifier,   php_jit_no_arginfo,             ZEND_ACC_PUBLIC)
	PHP_ME(Type, getIndirection,  php_jit_no_arginfo,             ZEND_ACC_PUBLIC)
	PHP_ME(Type, isPointer,       php_jit_no_arginfo,             ZEND_ACC_PUBLIC)
	PHP_ME(Type, dump,            php_jit_type_dump_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Type, of,              php_jit_type_of_arginfo,        ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};
#endif
#endif
