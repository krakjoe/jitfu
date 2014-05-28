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
#ifndef HAVE_BITS_BLOCK_H
#define HAVE_BITS_BLOCK_H

#define PHP_JIT_BLOCK_FUNCTIONS \
	JIT_FE(jit_block_get_function) \
	JIT_FE(jit_block_get_context) \
	JIT_FE(jit_block_get_label) \
	JIT_FE(jit_block_get_next_label) \
	JIT_FE(jit_block_next) \
	JIT_FE(jit_block_previous) \
	JIT_FE(jit_block_from_label) \
	JIT_FE(jit_block_is_reachable) \

static const char *le_jit_block_name = "jit block";
static       int   le_jit_block;

ZEND_RSRC_DTOR_FUNC(php_jit_block_dtor);

static inline php_jit_minit_block(int module_number TSRMLS_DC) {
	le_jit_block = zend_register_list_destructors_ex
		(php_jit_block_dtor, NULL, le_jit_block_name, module_number);
}

ZEND_BEGIN_ARG_INFO_EX(jit_block_get_function_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_get_context_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_get_label_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_get_next_label_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, block)
	ZEND_ARG_INFO(0, previous)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_next_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, previous)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_previous_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, previous)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_from_label_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, label)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_block_is_reachable_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_block_get_function);
PHP_FUNCTION(jit_block_get_context);
PHP_FUNCTION(jit_block_get_label);
PHP_FUNCTION(jit_block_get_next_label);
PHP_FUNCTION(jit_block_next);
PHP_FUNCTION(jit_block_previous);
PHP_FUNCTION(jit_block_from_label);
PHP_FUNCTION(jit_block_is_reachable);
#else
#ifndef HAVE_BITS_BLOCK
#define HAVE_BITS_BLOCK

/* {{{ php_jit_block_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_block_dtor) {
	
} /* }}} */

/* {{{ jit_function_t jit_block_get_function(jit_block_t block) */
PHP_FUNCTION(jit_block_get_function) {
	zval *zblock, **zfunction;
	jit_block_t block;
	jit_function_t function;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zblock) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(block,    jit_block_t, &zblock,   -1,  le_jit_block_name, le_jit_block);

	function = jit_block_get_function(block);
	
	if (zend_hash_index_find(
		&JG(func),
		(zend_ulong) function, (void**) &zfunction) == SUCCESS) {
		ZVAL_ZVAL(return_value, *zfunction, 1, 0);
	}
} /* }}} */

/* {{{ jit_context_t jit_block_get_context(jit_block_t block) */
PHP_FUNCTION(jit_block_get_context) {
	zval *zblock, **zcontext;
	jit_block_t block;
	jit_context_t context;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zblock) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(block,    jit_block_t, &zblock,   -1,  le_jit_block_name, le_jit_block);

	context = jit_block_get_context(block);
	
	if (zend_hash_index_find(
		&JG(ctx),
		(zend_ulong) context, (void**) &zcontext) == SUCCESS) {
		ZVAL_ZVAL(return_value, *zcontext, 1, 0);
	}
} /* }}} */

/* {{{ jit_label_t jit_block_get_label(jit_block_t block) */
PHP_FUNCTION(jit_block_get_label) {
	zval *zblock;
	jit_block_t block;
	jit_label_t label;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zblock) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(block,    jit_block_t, &zblock,   -1,  le_jit_block_name, le_jit_block);

	label = jit_block_get_label(block);
	
	RETURN_LONG(label);
} /* }}} */

/* {{{ jit_label_t jit_block_get_next_label(jit_block_t block, jit_label_t previous) */
PHP_FUNCTION(jit_block_get_next_label) {
	zval *zblock;
	long previous;
	jit_block_t block;
	jit_label_t label;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zblock, &previous) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(block,    jit_block_t, &zblock,   -1,  le_jit_block_name, le_jit_block);

	label = jit_block_get_next_label(block, previous);
	
	RETURN_LONG(label);
} /* }}} */

/* {{{ jit_block_t jit_block_next(jit_function_t function, jit_block_t previous) */
PHP_FUNCTION(jit_block_next) {
	zval *zfunction, *zprevious;
	jit_function_t function;
	jit_block_t previous, next;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zfunction, &zprevious) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction,   -1,  le_jit_function_name, le_jit_function);
	ZEND_FETCH_RESOURCE(previous, jit_block_t,    &zprevious,   -1,  le_jit_block_name,    le_jit_block);

	next = jit_block_next(function, previous);
	
	ZEND_REGISTER_RESOURCE(return_value, next, le_jit_block);
} /* }}} */

/* {{{ jit_block_t jit_block_previous(jit_function_t function, jit_block_t previous) */
PHP_FUNCTION(jit_block_previous) {
	zval *zfunction, *zprevious;
	jit_function_t function;
	jit_block_t previous, next;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zfunction, &zprevious) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction,   -1,  le_jit_function_name, le_jit_function);
	ZEND_FETCH_RESOURCE(previous, jit_block_t,    &zprevious,   -1,  le_jit_block_name,    le_jit_block);

	next = jit_block_previous(function, previous);
	
	ZEND_REGISTER_RESOURCE(return_value, next, le_jit_block);
} /* }}} */

/* {{{ jit_block_t jit_block_from_label(jit_function_t function, jit_label_t label) */
PHP_FUNCTION(jit_block_from_label) {
	zval *zfunction;
	jit_function_t function;
	jit_label_t label = jit_label_undefined;
	jit_block_t block;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zfunction, &label) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(function, jit_function_t, &zfunction,   -1,  le_jit_function_name, le_jit_function);

	block = jit_block_from_label(function, label);
	
	ZEND_REGISTER_RESOURCE(return_value, block, le_jit_block);
} /* }}} */

/* {{{ bool jit_block_is_reachable(jit_block_t block) */
PHP_FUNCTION(jit_block_is_reachable) {
	zval *zblock;
	jit_block_t block;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zblock) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(block,    jit_block_t, &zblock,   -1,  le_jit_block_name, le_jit_block);

	RETURN_BOOL(jit_block_is_reachable(block));
} /* }}} */
#endif
#endif
