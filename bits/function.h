#ifndef HAVE_BITS_FUNCTION_H
#define HAVE_BITS_FUNCTION_H

#define PHP_JIT_FUNCTION_FUNCTIONS \
	JIT_FE(jit_function_create) \
	JIT_FE(jit_function_get_context) \
	JIT_FE(jit_function_abandon) \
	JIT_FE(jit_function_compile) \
	JIT_FE(jit_function_apply)

static const char *le_jit_function_name = "jit function";
static       int   le_jit_function;

ZEND_RSRC_DTOR_FUNC(php_jit_function_dtor);

static inline php_jit_minit_function(int module_number TSRMLS_DC) {
	le_jit_function = zend_register_list_destructors_ex
		(php_jit_function_dtor, NULL, le_jit_function_name, module_number);
}

ZEND_BEGIN_ARG_INFO_EX(jit_function_create_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, context)
	ZEND_ARG_INFO(0, signature)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_get_context_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_abandon_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_compile_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_apply_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, types)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_function_create);
PHP_FUNCTION(jit_function_get_context);
PHP_FUNCTION(jit_function_abandon);
PHP_FUNCTION(jit_function_compile);
PHP_FUNCTION(jit_function_apply);

#else
#ifndef HAVE_BITS_FUNCTION
#define HAVE_BITS_FUNCTION

/* {{{ php_jit_context_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_function_dtor) {
	
} /* }}} */

/* {{{ jit_function_t jit_function_create(jit_context_t context, jit_type_t signature) */
PHP_FUNCTION(jit_function_create) {
	zval *zcontext;
	zval *zsignature;
	jit_context_t  context;
	jit_function_t function;
	jit_type_t     signature;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zcontext, &zsignature) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(context,    jit_context_t, &zcontext,   -1,  le_jit_context_name, le_jit_context);
	ZEND_FETCH_RESOURCE(signature,  jit_type_t,    &zsignature, -1,  le_jit_type_name,    le_jit_type);
	
	function = jit_function_create(context, signature);
	
	ZEND_REGISTER_RESOURCE(return_value, function, le_jit_function);
} /* }}} */

/* {{{ jit_context_t jit_function_get_context(jit_function_t function) */
PHP_FUNCTION(jit_function_get_context) {
	zval *zfunction, **zcontext;
	jit_function_t function;
	jit_context_t context;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	context = jit_function_get_context(function);
	
	if (zend_hash_index_find(
		&JG(ctx),
		(zend_ulong) context, (void**) &zcontext) == SUCCESS) {
		ZVAL_ZVAL(return_value, *zcontext, 1, 0);
	}
} /* }}} */

/* {{{ void jit_function_abandon(jit_function_t function) */
PHP_FUNCTION(jit_function_abandon) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	jit_function_abandon(function);
} /* }}} */

/* {{{ void jit_function_compile(jit_function_t function) */
PHP_FUNCTION(jit_function_compile) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	jit_function_compile(function);
} /* }}} */

/* {{{ void jit_function_apply(jit_function_t function, array params, int returns) */
PHP_FUNCTION(jit_function_apply) {
	zval *zfunction, **zparam;
	HashTable *zparams;
	HashPosition position;
	jit_function_t function;
	void **args;
	void* result;
	long zreturns;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rHl", &zfunction, &zparams, &zreturns) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	args = ecalloc(zend_hash_num_elements(zparams), sizeof(void*));
	
	for (zend_hash_internal_pointer_reset_ex(zparams, &position);
		zend_hash_get_current_data_ex(zparams, (void**) &zparam, &position) == SUCCESS;
		zend_hash_move_forward_ex(zparams, &position)) {
		zend_ulong idx;
		
		zend_hash_get_current_key_ex(zparams, NULL, NULL, &idx, 0, &position);
		
		switch (Z_TYPE_PP(zparam)) {
			case IS_LONG:
				args[idx] = &Z_LVAL_PP(zparam);
			break;
			
			case IS_STRING:
				args[idx] = &Z_STRVAL_PP(zparam);
			break;
			
			case IS_DOUBLE:
				args[idx] = &Z_DVAL_PP(zparam);
			break;
		}
	}
	
	jit_function_apply(function, args, &result);
	
	if (zreturns == (zend_ulong) jit_type_int) {
		ZVAL_LONG(return_value, (long) result);
	} else if ((zreturns == (zend_ulong) jit_type_void_ptr)) {
		ZVAL_STRING(return_value, (char*)result, 1);
	} else {
		ZVAL_LONG(return_value, (long) result);
	}
	
	efree(args);
} /* }}} */
#endif
#endif
