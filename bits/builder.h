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
#ifndef HAVE_BITS_BUILDER_H
#define HAVE_BITS_BUILDER_H

typedef struct _php_jit_builder_t {
	zend_object std;
	zend_object_handle h;
	php_jit_function_t *func;
} php_jit_builder_t;

zend_class_entry *jit_builder_ce;

void php_jit_minit_builder(int module_number TSRMLS_DC);

#define PHP_JIT_FETCH_BUILDER(from) \
	(php_jit_builder_t*) zend_object_store_get_object((from) TSRMLS_CC)

extern zend_function_entry php_jit_builder_methods[];
extern zend_object_handlers php_jit_builder_handlers;

#else
#ifndef HAVE_BITS_BUILDER
#define HAVE_BITS_BUILDER
zend_object_handlers php_jit_builder_handlers;

static inline void php_jit_builder_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_builder_t *pbuild = 
		(php_jit_builder_t *) zobject;

	zend_object_std_dtor(&pbuild->std TSRMLS_CC);

	zend_objects_store_del_ref_by_handle(pbuild->func->h TSRMLS_CC);
	
	efree(pbuild);
}

static inline zend_object_value php_jit_builder_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;
	php_jit_builder_t *pbuild = 
		(php_jit_builder_t*) ecalloc(1, sizeof(php_jit_builder_t));
	
	zend_object_std_init(&pbuild->std, ce TSRMLS_CC);
	object_properties_init(&pbuild->std, ce);
	
	pbuild->h = zend_objects_store_put(
		pbuild, 
		php_jit_builder_destroy, NULL, NULL TSRMLS_CC);

	value.handle   = pbuild->h;
	value.handlers = &php_jit_builder_handlers;
	
	return value;
}

void php_jit_minit_builder(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JIT", "Builder", php_jit_builder_methods);
	jit_builder_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_builder_ce->create_object = php_jit_builder_create;
	
	memcpy(
		&php_jit_builder_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_builder_handlers));
}

PHP_METHOD(Builder, __construct) {
	zval *zfunction;
	php_jit_builder_t *pbuild;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zfunction, jit_function_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	pbuild->func = PHP_JIT_FETCH_FUNCTION(zfunction);
	zend_objects_store_add_ref_by_handle
		(pbuild->func->h TSRMLS_CC);
}

PHP_METHOD(Builder, doIfEqual) {
	zval *op1, *op2 = NULL;
	php_jit_builder_t *pbuild;
	jit_value_t temp;
	jit_label_t label = jit_label_undefined;
	zend_fcall_info zpfci, znfci;
	zend_fcall_info_cache zpfcc, znfcc;
	zval *zretnull = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOf|f", &op1, jit_value_ce, &op2, jit_value_ce, &zpfci, &zpfcc, &znfci, &znfcc) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	temp = jit_insn_eq
		(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(op1), PHP_JIT_FETCH_VALUE_I(op2));
	
	jit_insn_branch_if_not(pbuild->func->func, temp, &label);
	
	zpfci.retval_ptr_ptr = &zretnull;
	
	zend_fcall_info_argn(&zpfci TSRMLS_CC, 1, &getThis());
	
	zend_try {
		zend_call_function(&zpfci, &zpfcc TSRMLS_CC);
	} zend_end_try();
	
	zend_fcall_info_args_clear(&zpfci, 1);
	
	if (zretnull) {
		zval_ptr_dtor(&zretnull);
	}
	
	jit_insn_label(pbuild->func->func, &label);
	
	if (ZEND_NUM_ARGS() > 3) {
		znfci.retval_ptr_ptr = &zretnull;
	
		zend_fcall_info_argn(&znfci TSRMLS_CC, 1, &getThis());
	
		zend_try {
			zend_call_function(&znfci, &znfcc TSRMLS_CC);
		} zend_end_try();
	
		zend_fcall_info_args_clear(&znfci, 1);
	
		if (zretnull) {
			zval_ptr_dtor(&zretnull);
		}
	}
}

PHP_METHOD(Builder, doReturn) {
	zval *zvalue;
	php_jit_builder_t *pbuild;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zvalue, jit_value_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_BOOL(jit_insn_return(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zvalue)));
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doReturn_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doIfEqual_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, op1)
	ZEND_ARG_INFO(0, op2)
	ZEND_ARG_INFO(0, positive)
	ZEND_ARG_INFO(0, negative)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_builder_methods[] = {
	PHP_ME(Builder, __construct,  php_jit_builder_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIfEqual,    php_jit_builder_doIfEqual_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doReturn,     php_jit_builder_doReturn_arginfo,  ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
