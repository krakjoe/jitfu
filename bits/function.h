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
#ifndef HAVE_BITS_FUNCTION_H
#define HAVE_BITS_FUNCTION_H

#define PHP_JIT_FUNCTION_FUNCTIONS \
	JIT_FE(jit_function_create) \
	JIT_FE(jit_function_create_nested) \
	JIT_FE(jit_function_get_nested_parent) \
	JIT_FE(jit_function_get_context) \
	JIT_FE(jit_function_abandon) \
	JIT_FE(jit_function_set_recompilable) \
	JIT_FE(jit_function_clear_recompilable) \
	JIT_FE(jit_function_compile) \
	JIT_FE(jit_function_is_compiled) \
	JIT_FE(jit_function_set_optimization_level) \
	JIT_FE(jit_function_get_optimization_level) \
	JIT_FE(jit_function_get_max_optimization_level) \
	JIT_FE(jit_function_apply) \
	JIT_FE(jit_function_reserve_label) \
	JIT_FE(jit_function_labels_equal)

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

ZEND_BEGIN_ARG_INFO_EX(jit_function_create_nested_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, context)
	ZEND_ARG_INFO(0, signature)
	ZEND_ARG_INFO(0, parent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_get_nested_parent_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_get_context_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_abandon_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_set_recompilable_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_clear_recompilable_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_is_recompilable_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_compile_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_is_compiled_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_set_optimization_level_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_get_optimization_level_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_get_max_optimization_level_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_apply_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, returns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_reserve_label_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_function_labels_equal_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, label1)
	ZEND_ARG_INFO(0, label2)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_function_create);
PHP_FUNCTION(jit_function_create_nested);
PHP_FUNCTION(jit_function_get_nested_parent);
PHP_FUNCTION(jit_function_get_context);
PHP_FUNCTION(jit_function_abandon);
PHP_FUNCTION(jit_function_set_recompilable);
PHP_FUNCTION(jit_function_clear_recompilable);
PHP_FUNCTION(jit_function_is_recompilable);
PHP_FUNCTION(jit_function_compile);
PHP_FUNCTION(jit_function_is_compiled);
PHP_FUNCTION(jit_function_set_optimization_level);
PHP_FUNCTION(jit_function_get_optimization_level);
PHP_FUNCTION(jit_function_get_max_optimization_level);
PHP_FUNCTION(jit_function_apply);
PHP_FUNCTION(jit_function_reserve_label);
PHP_FUNCTION(jit_function_labels_equal);
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
	
	if (Z_TYPE_P(return_value) == IS_RESOURCE) {
		zend_hash_index_update(
			&JG(func), 
			(zend_ulong) function, 
			&return_value, sizeof(zval*), NULL);
		Z_ADDREF_P(return_value);
	}
} /* }}} */

/* {{{ jit_function_t jit_function_create_nested(jit_context_t context, jit_type_t signature, jit_function_t parent) */
PHP_FUNCTION(jit_function_create_nested) {
	zval *zcontext;
	zval *zsignature;
	zval *zparent;
	jit_context_t  context;
	jit_function_t function, parent;
	jit_type_t     signature;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrr", &zcontext, &zsignature, &zparent) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(context,    jit_context_t,     &zcontext,   -1,  le_jit_context_name,  le_jit_context);
	ZEND_FETCH_RESOURCE(signature,  jit_type_t,        &zsignature, -1,  le_jit_type_name,     le_jit_type);
	ZEND_FETCH_RESOURCE(parent,     jit_function_t,    &zparent,    -1,  le_jit_function_name, le_jit_function);
	
	function = jit_function_create_nested(context, signature, parent);
	
	ZEND_REGISTER_RESOURCE(return_value, function, le_jit_function);
	
	if (Z_TYPE_P(return_value) == IS_RESOURCE) {
		zend_hash_index_update(
			&JG(func), 
			(zend_ulong) function, 
			&return_value, sizeof(zval*), NULL);
		Z_ADDREF_P(return_value);
	}
} /* }}} */

/* {{{ jit_function_t jit_function_get_nested_parent(jit_function_t function) */
PHP_FUNCTION(jit_function_get_nested_parent) {
	zval *zfunction, **zparent;
	jit_function_t function, parent;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function,     jit_function_t,    &zfunction,    -1,  le_jit_function_name, le_jit_function);
	
	parent = jit_function_get_nested_parent(function);
	
	if (zend_hash_index_find(
		&JG(func),
		(zend_ulong) parent, (void**) &zparent) == SUCCESS) {
		ZVAL_ZVAL(return_value, *zparent, 1, 0);
	}
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

/* {{{ void jit_function_set_recompilable(jit_function_t function) */
PHP_FUNCTION(jit_function_set_recompilable) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	jit_function_set_recompilable(function);
} /* }}} */

/* {{{ void jit_function_clear_recompilable(jit_function_t function) */
PHP_FUNCTION(jit_function_clear_recompilable) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	jit_function_clear_recompilable(function);
} /* }}} */

/* {{{ bool jit_function_is_recompilable(jit_function_t function) */
PHP_FUNCTION(jit_function_is_recompilable) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	RETURN_BOOL(jit_function_is_recompilable(function));
} /* }}} */

/* {{{ int jit_function_compile(jit_function_t function) */
PHP_FUNCTION(jit_function_compile) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	RETURN_LONG(jit_function_compile(function));
} /* }}} */

/* {{{ bool jit_function_is_compiled(jit_function_t function) */
PHP_FUNCTION(jit_function_is_compiled) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	RETURN_BOOL(jit_function_is_compiled(function));
} /* }}} */

/* {{{ void jit_function_set_optimization_level(jit_function_t function, int level) */
PHP_FUNCTION(jit_function_set_optimization_level) {
	zval *zfunction;
	jit_function_t function;
	long level = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zfunction, &level) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	jit_function_set_optimization_level(function, level);
} /* }}} */

/* {{{ int jit_function_get_optimization_level(jit_function_t function) */
PHP_FUNCTION(jit_function_get_optimization_level) {
	zval *zfunction;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	RETURN_LONG(jit_function_get_optimization_level(function));
} /* }}} */

/* {{{ int jit_function_get_max_optimization_level(void) */
PHP_FUNCTION(jit_function_get_max_optimization_level) {
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_function_get_max_optimization_level());
} /* }}} */

/* {{{ void jit_function_apply(jit_function_t function, array params, jit_type_t returns) */
PHP_FUNCTION(jit_function_apply) {
	zval *zfunction, **zparam;
	HashTable *zparams;
	HashPosition position;
	jit_function_t function;
	void **args;
	void *result;
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
	
	switch (zreturns) {
		case PHP_JIT_TYPE_CHAR: ZVAL_STRING(return_value, (char*) result, 1); break;
		
		case PHP_JIT_TYPE_ULONG:
		case PHP_JIT_TYPE_LONG:
		case PHP_JIT_TYPE_UINT:
		case PHP_JIT_TYPE_INT: ZVAL_LONG(return_value,      (long) result); break;
		case PHP_JIT_TYPE_DOUBLE: {
			double doubled = 
				*(double *) &result;

			ZVAL_DOUBLE(return_value, doubled);
		} break;
		
		case PHP_JIT_TYPE_VOID_PTR: ZVAL_LONG(return_value, (long) result); break;
	}
	
	efree(args);
} /* }}} */

/* {{{ jit_label_t jit_function_reserve_label(jit_function_t function) */
PHP_FUNCTION(jit_function_reserve_label) {
	zval *zfunction;
	jit_function_t function;
	jit_label_t label;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zfunction) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	RETURN_LONG(jit_function_reserve_label(function));
} /* }}} */

/* {{{ bool jit_function_labels_equal(jit_function_t function, jit_label_t label1, jit_label_t label2) */
PHP_FUNCTION(jit_function_labels_equal) {
	zval *zfunction, *zlabel1, *zlabel2;
	jit_function_t function;
	jit_label_t label1, label2;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll", &zfunction, &zlabel1, &zlabel2) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction, -1, le_jit_function_name, le_jit_function);
	
	RETURN_BOOL(jit_function_labels_equal(function, label1, label2));
} /* }}} */
#endif
#endif
