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
	jit_type_t       type;
	zval             zreturns;
	zval             *zparams;
	zend_ulong        nparams;
	zend_object std;
} php_jit_signature_t;

#define PHP_JIT_FETCH_SIGNATURE_O(o) ((php_jit_signature_t*) ((char*) o - XtOffsetOf(php_jit_signature_t, std)))
#define PHP_JIT_FETCH_SIGNATURE(from) PHP_JIT_FETCH_SIGNATURE_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_SIGNATURE_I(from) (PHP_JIT_FETCH_SIGNATURE(from))->type

void php_jit_minit_signature(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_signature_methods[];
extern zend_object_handlers php_jit_signature_handlers;

#else
#ifndef HAVE_BITS_SIGNATURE
#define HAVE_BITS_SIGNATURE

zend_object_handlers php_jit_signature_handlers;

static inline void php_jit_signature_free(zend_object *zobject TSRMLS_DC) {
	php_jit_signature_t *psig = 
		(php_jit_signature_t *) PHP_JIT_FETCH_SIGNATURE_O(zobject);
	zend_ulong param = 0;
	
	while (param < psig->nparams) {
		zval_ptr_dtor(&psig->zparams[param]);
		param++;
	}
	
	zval_ptr_dtor(&psig->zreturns);	
	
	jit_type_free(psig->type);
	
	if (psig->zparams) {
		efree(psig->zparams);	
	}

	zend_object_std_dtor(&psig->std TSRMLS_CC);
}

static inline zend_object* php_jit_signature_create(zend_class_entry *ce TSRMLS_DC) {
	php_jit_signature_t *psig = 
		(php_jit_signature_t*) ecalloc(1, sizeof(php_jit_signature_t) + zend_object_properties_size(ce));
	
	zend_object_std_init(&psig->std, ce TSRMLS_CC);
	object_properties_init(&psig->std, ce);
    
    ZVAL_NULL(&psig->zreturns);

	psig->std.handlers = &php_jit_signature_handlers;
	
	return &psig->std;
}

void php_jit_minit_signature(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Signature", php_jit_signature_methods);
	jit_signature_ce = zend_register_internal_class_ex(&ce, jit_type_ce TSRMLS_CC);
	jit_signature_ce->create_object = php_jit_signature_create;
	
	memcpy(
		&php_jit_signature_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_signature_handlers));
	
	php_jit_signature_handlers.offset = XtOffsetOf(php_jit_signature_t, std);
	php_jit_signature_handlers.free_obj = php_jit_signature_free;
}

PHP_METHOD(Signature, __construct) {
	zval *ztype, *zztype;
	php_jit_signature_t *psig;
	php_jit_type_t      *preturns;
	HashTable *ztypes;
	HashPosition position;
	jit_type_t *params;
	zend_ulong   param = 0;
	
	if (php_jit_parameters("OH", &ztype, jit_type_ce, &ztypes) != SUCCESS) {
		php_jit_exception("invalid parameters, expected (Type returns, Type[] parameters)");
		return;
	}
	
	psig = PHP_JIT_FETCH_SIGNATURE(getThis());
	ZVAL_COPY(&psig->zreturns, ztype);
	preturns =
	    PHP_JIT_FETCH_TYPE(&psig->zreturns);
	
	psig->nparams = zend_hash_num_elements(ztypes);	
	psig->zparams = (zval*)
		ecalloc(psig->nparams, sizeof(zval));
	
	params = (jit_type_t*)
		ecalloc(psig->nparams, sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(ztypes, &position);
		(zztype = zend_hash_get_current_data_ex(ztypes, &position));
		zend_hash_move_forward_ex(ztypes, &position)) {
		
		if ((!zztype || Z_TYPE_P(zztype) != IS_OBJECT) ||
			!instanceof_function(Z_OBJCE_P(zztype), jit_type_ce TSRMLS_CC)) {
			php_jit_exception("unexpected type for parameter %d", param);
			efree(params);
			return;
		}
		ZVAL_COPY(&psig->zparams[param], zztype);

		params[param] = 
		    PHP_JIT_FETCH_TYPE_I(&psig->zparams[param]);
		param++;
	}
	
	psig->type = jit_type_create_signature(jit_abi_cdecl, preturns->type, params, param, 1);
	efree(params);
}

PHP_METHOD(Signature, getReturnType) {
	php_jit_signature_t *psig;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	psig = PHP_JIT_FETCH_SIGNATURE(getThis());
	
	ZVAL_COPY(return_value, &psig->zreturns);
}

PHP_METHOD(Signature, getParamType) {
	php_jit_signature_t *psig;
	long param = 0;
	
	if (php_jit_parameters("l", &param) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected (int parameter)");
		return;
	}
	
	psig = PHP_JIT_FETCH_SIGNATURE(getThis());

	ZVAL_COPY(return_value, &psig->zparams[param]);
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
