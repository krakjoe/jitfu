#ifndef HAVE_BITS_INSN_H
#define HAVE_BITS_INSN_H

#define PHP_JIT_INSN_FUNCTIONS \
	JIT_FE(jit_insn_mul) \
	JIT_FE(jit_insn_add) \
	JIT_FE(jit_insn_return)

ZEND_BEGIN_ARG_INFO_EX(jit_insn_mul_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, op1)
	ZEND_ARG_INFO(0, op2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_insn_add_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, op1)
	ZEND_ARG_INFO(0, op2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_insn_return_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()


PHP_FUNCTION(jit_insn_mul);
PHP_FUNCTION(jit_insn_add);
PHP_FUNCTION(jit_insn_return);
#else
#ifndef HAVE_BITS_INSN
#define HAVE_BITS_INSN

/* {{{ jit_value_t jit_insn_mul(jit_function_t function, jit_value_t op1, jit_value_t op2) */
PHP_FUNCTION(jit_insn_mul) {
	zval *zfunction, *zin[2];
	jit_function_t function;
	jit_value_t in[2], result;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrr", &zfunction, &zin[0], &zin[1]) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	ZEND_FETCH_RESOURCE(in[0], jit_value_t, &zin[0], -1, le_jit_value_name, le_jit_value);
	ZEND_FETCH_RESOURCE(in[1], jit_value_t, &zin[1], -1, le_jit_value_name, le_jit_value);

	result = jit_insn_mul(function, in[0], in[1]);
	
	ZEND_REGISTER_RESOURCE(return_value, result, le_jit_value);
} /* }}} */

/* {{{ jit_value_t jit_insn_add(jit_function_t function, jit_value_t op1, jit_value_t op2) */
PHP_FUNCTION(jit_insn_add) {
	zval *zfunction, *zin[2];
	jit_function_t function;
	jit_value_t in[2], result;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrr", &zfunction, &zin[0], &zin[1]) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	ZEND_FETCH_RESOURCE(in[0], jit_value_t, &zin[0], -1, le_jit_value_name, le_jit_value);
	ZEND_FETCH_RESOURCE(in[1], jit_value_t, &zin[1], -1, le_jit_value_name, le_jit_value);

	result = jit_insn_add(function, in[0], in[1]);
	
	ZEND_REGISTER_RESOURCE(return_value, result, le_jit_value);
} /* }}} */

/* {{{ void jit_insn_return(jit_function_t function, jit_value_t result) */
PHP_FUNCTION(jit_insn_return) {
	zval *zfunction, *zresult;
	jit_function_t function;
	jit_value_t result;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zfunction, &zresult) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	ZEND_FETCH_RESOURCE(result, jit_value_t, &zresult, -1, le_jit_value_name, le_jit_value);

	jit_insn_return(function, result);
} /* }}} */
#endif
#endif
