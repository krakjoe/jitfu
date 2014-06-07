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
#ifndef HAVE_BITS_LABEL_H
#define HAVE_BITS_LABEL_H

typedef struct _php_jit_label_t {
	zend_object         std;
	zend_object_handle  h;
	php_jit_function_t  *func;
	jit_label_t         label;
} php_jit_label_t;

zend_class_entry *jit_label_ce;

#define PHP_JIT_FETCH_LABEL(from) \
	(php_jit_label_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_LABEL_I(from) \
	(PHP_JIT_FETCH_LABEL(from))->type

void php_jit_minit_label(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_label_methods[];
extern zend_object_handlers php_jit_label_handlers;

#else
#ifndef HAVE_BITS_LABEL
#define HAVE_BITS_LABEL
zend_object_handlers php_jit_label_handlers;

static inline void php_jit_label_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_label_t *plabel = 
		(php_jit_label_t *) zobject;

	zend_object_std_dtor(&plabel->std TSRMLS_CC);
	
	if (plabel->func) {
		zend_objects_store_del_ref_by_handle(plabel->func->h TSRMLS_CC);
	}
	
	efree(plabel);
}

static inline zend_object_value php_jit_label_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value intern;
	php_jit_label_t *plabel = 
		(php_jit_label_t*) ecalloc(1, sizeof(php_jit_label_t));
	
	zend_object_std_init(&plabel->std, ce TSRMLS_CC);
	object_properties_init(&plabel->std, ce);
	
	plabel->h = zend_objects_store_put(
		plabel, 
		php_jit_label_destroy, NULL, NULL TSRMLS_CC);
	plabel->label = jit_label_undefined;
		
	intern.handle   = plabel->h;
	intern.handlers = &php_jit_label_handlers;
	
	return intern;
}

void php_jit_minit_label(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Label", php_jit_label_methods);
	jit_label_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_label_ce->create_object = php_jit_label_create;
	
	memcpy(
		&php_jit_label_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_label_handlers));
}

PHP_METHOD(Label, __construct) {
	zval *zfunction = NULL;
	php_jit_function_t *pfunc = NULL;
	php_jit_label_t *plabel = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zfunction, jit_function_ce) != SUCCESS) {
		return;
	}
	
	plabel = PHP_JIT_FETCH_LABEL(getThis());
	pfunc = PHP_JIT_FETCH_FUNCTION(zfunction);
	
	jit_insn_label(pfunc->func, &plabel->label);
}

PHP_METHOD(Label, equal) {
	php_jit_label_t *plabels[2];
	zval *zlabel;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zlabel, jit_label_ce) != SUCCESS) {
		return;
	}
	
	plabels[0] = PHP_JIT_FETCH_LABEL(getThis());
	plabels[1] = PHP_JIT_FETCH_LABEL(zlabel);
	
	if (plabels[0]->func->func != plabels[1]->func->func) {
		RETURN_FALSE;
	}
	
	RETURN_BOOL(jit_function_labels_equal(plabels[0]->func->func, plabels[0]->label, plabels[1]->label));
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_label_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_label_equal_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, other)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_label_methods[] = {
	PHP_ME(Label, __construct,     php_jit_label_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Label, equal,           php_jit_label_equal_arginfo,     ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
