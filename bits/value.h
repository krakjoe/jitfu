#ifndef HAVE_BITS_VALUE_H
#define HAVE_BITS_VALUE_H

#define PHP_JIT_VALUE_FUNCTIONS \
	JIT_FE(jit_value_get_param)

static const char *le_jit_value_name = "jit value";
static       int   le_jit_value;

ZEND_RSRC_DTOR_FUNC(php_jit_value_dtor);

static inline php_jit_minit_value(int module_number TSRMLS_DC) {
	le_jit_value = zend_register_list_destructors_ex
		(php_jit_value_dtor, NULL, le_jit_value_name, module_number);
}

ZEND_BEGIN_ARG_INFO_EX(jit_value_get_param_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, param)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_value_get_param);
#else
#ifndef HAVE_BITS_VALUE
#define HAVE_BITS_VALUE

/* {{{ php_jit_value_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_value_dtor) {
	
} /* }}} */

/* {{{ jit_value_t jit_value_get_param(jit_function_t function, int param) */
PHP_FUNCTION(jit_value_get_param) {
	zval *resource;
	long param;
	jit_function_t function;
	jit_value_t value;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &resource, &param) != SUCCESS) {
		return;
	}

	ZEND_FETCH_RESOURCE(function, jit_function_t, &resource, -1, le_jit_function_name, le_jit_function);

	value = jit_value_get_param(function, param);

	ZEND_REGISTER_RESOURCE(return_value, value, le_jit_value);
} /* }}} */
#endif
#endif
