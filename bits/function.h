#ifndef HAVE_BITS_FUNCTION_H
#define HAVE_BITS_FUNCTION_H

#define PHP_JIT_FUNCTION_FUNCTIONS \
	JIT_FE(jit_function_create) \
	JIT_FE(jit_function_get_context) \
	JIT_FE(jit_function_abandon)

static const char *le_jit_function_name = "jit function";
static       int   le_jit_function;

ZEND_RSRC_DTOR_FUNC(php_jit_function_dtor);

static inline php_jit_minit_function(int module_number TSRMLS_DC) {
	le_jit_function = zend_register_list_destructors_ex
		(php_jit_function_dtor, NULL, le_jit_function_name, module_number);
}

ZEND_BEGIN_ARG_INFO_EX(jit_function_create_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_get_context_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_abandon_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_function_create);
PHP_FUNCTION(jit_function_get_context);
PHP_FUNCTION(jit_function_abandon);

#else
#ifndef HAVE_BITS_FUNCTION
#define HAVE_BITS_FUNCTION

/* {{{ php_jit_context_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_function_dtor) {
	
} /* }}} */

/* {{{ jit_function_t jit_function_create(jit_context_t context, jit_type_t signature) */
PHP_FUNCTION(jit_function_create) {
	zval *resource;
	zval *signature;
	jit_context_t context;
	jit_function_t function;
	jit_type_t     sig;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &resource, &signature) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(context, jit_context_t, &resource, -1,  le_jit_context_name, le_jit_context);
	ZEND_FETCH_RESOURCE(sig,     jit_type_t,    &signature, -1, le_jit_type_name,    le_jit_type);
	
	function = jit_function_create(context, sig);
	
	ZEND_REGISTER_RESOURCE(return_value, function, le_jit_function);
} /* }}} */

/* {{{ jit_context_t jit_function_get_context(jit_function_t function) */
PHP_FUNCTION(jit_function_get_context) {
	zval *resource, **found;
	jit_function_t function;
	jit_context_t context;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &resource, -1, le_jit_function_name, le_jit_function);
	
	context = jit_function_get_context(function);
	
	if (zend_hash_index_find(
		&JG(ctx), 
		(zend_ulong) context, (void**) &found) == SUCCESS) {
		ZVAL_ZVAL(return_value, *found, 1, 0);
	}
} /* }}} */

/* {{{ void jit_function_abandon(jit_function_t function) */
PHP_FUNCTION(jit_function_abandon) {
	zval *resource;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &resource, -1, le_jit_function_name, le_jit_function);
	
	jit_function_abandon(function);
} /* }}} */
#endif
#endif
