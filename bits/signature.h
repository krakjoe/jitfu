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
#ifndef HAVE_BITS_SIGNATURE_H
#define HAVE_BITS_SIGNATURE_H

zend_class_entry *jit_signature_ce;

typedef struct _php_jit_signature_t {
	zend_object std;
	zend_object_handle h;
	jit_type_t       type;
	php_jit_type_t *returns;
	php_jit_type_t **params;
	zend_uint        nparams;
} php_jit_signature_t;

#define PHP_JIT_FETCH_SIGNATURE(from) \
	(php_jit_signature_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_SIGNATURE_I(from) \
	(PHP_JIT_FETCH_SIGNATURE(from))->type

void php_jit_minit_signature(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_signature_methods[];
extern zend_object_handlers php_jit_signature_handlers;

#else
#ifndef HAVE_BITS_SIGNATURE
#define HAVE_BITS_SIGNATURE

zend_object_handlers php_jit_signature_handlers;

static inline void php_jit_signature_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_signature_t *psig = 
		(php_jit_signature_t *) zobject;
	zend_uint param = 0;

	zend_object_std_dtor(&psig->std TSRMLS_CC);
	
	while (param < psig->nparams) {
		if (psig->params[param]) {
			zend_objects_store_del_ref_by_handle
				(psig->params[param]->h TSRMLS_CC);
			param++;
		}
	}
	
	if (psig->returns) {
		zend_objects_store_del_ref_by_handle(psig->returns->h TSRMLS_CC);
	}
}

static inline void php_jit_signature_free(void *zobject TSRMLS_DC) {
	php_jit_signature_t *psig = 
		(php_jit_signature_t *) zobject;
	
	jit_type_free(psig->type);
	
	efree(psig->params);
	efree(psig);
}

static inline zend_object_value php_jit_signature_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value intern;
	php_jit_signature_t *psig = 
		(php_jit_signature_t*) ecalloc(1, sizeof(php_jit_signature_t));
	
	zend_object_std_init(&psig->std, ce TSRMLS_CC);
	object_properties_init(&psig->std, ce);
	
	psig->h = zend_objects_store_put(
		psig,
		php_jit_signature_destroy, 
		php_jit_signature_free, NULL TSRMLS_CC);

	intern.handle   = psig->h;
	intern.handlers = &php_jit_signature_handlers;
	
	return intern;
}

void php_jit_minit_signature(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Signature", php_jit_signature_methods);
	jit_signature_ce = zend_register_internal_class_ex(&ce, jit_type_ce, NULL TSRMLS_CC);
	jit_signature_ce->create_object = php_jit_signature_create;
	
	memcpy(
		&php_jit_signature_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_signature_handlers));
}

PHP_METHOD(Signature, __construct) {
	zval *ztype, **zztype;
	php_jit_signature_t *psig;
	HashTable *ztypes;
	HashPosition position;
	jit_type_t *params;
	zend_uint   param = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OH", &ztype, jit_type_ce, &ztypes) != SUCCESS) {
		return;
	}
	
	psig = PHP_JIT_FETCH_SIGNATURE(getThis());
	psig->returns = PHP_JIT_FETCH_TYPE(ztype);
	zend_objects_store_add_ref_by_handle(psig->returns->h TSRMLS_CC);
	
	psig->nparams = zend_hash_num_elements(ztypes);
	psig->params = (php_jit_type_t**)
		ecalloc(psig->nparams, sizeof(php_jit_type_t));
	
	params = (jit_type_t*)
		ecalloc(zend_hash_num_elements(ztypes), sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(ztypes, &position);
		zend_hash_get_current_data_ex(ztypes, (void**)&zztype, &position) == SUCCESS;
		zend_hash_move_forward_ex(ztypes, &position)) {
		if (instanceof_function(Z_OBJCE_PP(zztype), jit_type_ce TSRMLS_CC)) {
			psig->params[param] = PHP_JIT_FETCH_TYPE(*zztype);
			zend_objects_store_add_ref_by_handle
				(psig->params[param]->h TSRMLS_CC);
			params[param] = PHP_JIT_FETCH_TYPE_I(*zztype);
		}
		param++;
	}
	
	psig->type = jit_type_create_signature(jit_abi_cdecl, psig->returns->type, params, param, 1);
	efree(params);
}

PHP_METHOD(Signature, getReturnType) {
	php_jit_signature_t *psig;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	psig = PHP_JIT_FETCH_SIGNATURE(getThis());
	
	if (psig) {
		zend_object_value value;
		
		value.handle = psig->returns->h;
		value.handlers = &php_jit_type_handlers;

		Z_TYPE_P(return_value) = IS_OBJECT;		
		Z_OBJVAL_P(return_value) = value;
		
		zend_objects_store_add_ref_by_handle(psig->returns->h TSRMLS_CC);
	}
}

PHP_METHOD(Signature, getParamType) {
	php_jit_signature_t *psig;
	long param = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &param) != SUCCESS) {
		return;
	}
	
	psig = PHP_JIT_FETCH_SIGNATURE(getThis());
	
	if (psig && param < psig->nparams) {
		zend_object_value value;
		
		value.handle = psig->params[param]->h;
		value.handlers = &php_jit_type_handlers;

		Z_TYPE_P(return_value) = IS_OBJECT;		
		Z_OBJVAL_P(return_value) = value;
		
		zend_objects_store_add_ref_by_handle
			(psig->params[param]->h TSRMLS_CC);
	}
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_signature_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, returns)
	ZEND_ARG_TYPE_INFO(0, types, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_signature_get_param_type_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, param)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_signature_methods[] = {
	PHP_ME(Signature, __construct,    php_jit_signature_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Signature, getReturnType,  php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Signature, getParamType,   php_jit_signature_get_param_type_arginfo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
