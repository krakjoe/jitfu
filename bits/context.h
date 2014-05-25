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
#ifndef HAVE_BITS_CONTEXT_H
#define HAVE_BITS_CONTEXT_H

#define PHP_JIT_CONTEXT_FUNCTIONS \
	JIT_FE(jit_context_create) \
	JIT_FE(jit_context_destroy) \
	JIT_FE(jit_context_build_start) \
	JIT_FE(jit_context_build_end)

static const char *le_jit_context_name = "jit context";
static        int  le_jit_context;

ZEND_RSRC_DTOR_FUNC(php_jit_context_dtor);

static inline php_jit_minit_context(int module_number TSRMLS_DC) {
	le_jit_context = zend_register_list_destructors_ex
		(php_jit_context_dtor, NULL, le_jit_context_name, module_number);

	REGISTER_LONG_CONSTANT("JIT_OPTION_CACHE_LIMIT", JIT_OPTION_CACHE_LIMIT, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_CACHE_PAGE_SIZE", JIT_OPTION_CACHE_PAGE_SIZE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_PRE_COMPILE", JIT_OPTION_PRE_COMPILE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_DONT_FOLD", JIT_OPTION_DONT_FOLD, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_POSITION_INDEPENDENT", JIT_OPTION_POSITION_INDEPENDENT, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_CACHE_MAX_PAGE_FACTOR", JIT_OPTION_CACHE_MAX_PAGE_FACTOR, CONST_CS|CONST_PERSISTENT);
}

ZEND_BEGIN_ARG_INFO_EX(jit_context_create_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(jit_context_destroy_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_context_build_start_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(jit_context_build_end_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_context_create);
PHP_FUNCTION(jit_context_destroy);

PHP_FUNCTION(jit_context_build_start);
PHP_FUNCTION(jit_context_build_end);

#else
#ifndef HAVE_BITS_CONTEXT
#define HAVE_BITS_CONTEXT

/* {{{ php_jit_context_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_context_dtor) {
	jit_context_destroy((jit_context_t) rsrc->ptr);
} /* }}} */

/* {{{ proto resource jit_context_create(void)
   Create a jit context */
PHP_FUNCTION(jit_context_create)
{
	jit_context_t context;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	context = jit_context_create();
		
	ZEND_REGISTER_RESOURCE(return_value, context, le_jit_context);
	
	if (Z_TYPE_P(return_value) == IS_RESOURCE) {
		zend_hash_index_update(
			&JG(ctx), 
			(zend_ulong) context, 
			&return_value, sizeof(zval*), NULL);
		Z_ADDREF_P(return_value);
	}
}
/* }}} */	

/* {{{ proto void jit_context_destroy(resource context)
	Destroy a previously created jit context */
PHP_FUNCTION(jit_context_destroy) 
{
	zval *zcontext;
	jit_context_t context;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zcontext) != SUCCESS) {
		return;
	}
	
	zend_list_delete(Z_RESVAL_P(zcontext));
}

/* {{{ proto void jit_context_build_start(resource context) */
PHP_FUNCTION(jit_context_build_start) 
{
	zval *zcontext;
	jit_context_t context;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zcontext) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(context, jit_context_t, &zcontext, -1, le_jit_context_name, le_jit_context);
	
	jit_context_build_start(context);
} /* }}} */

/* {{{ proto void jit_context_build_end(resource context) */
PHP_FUNCTION(jit_context_build_end) 
{
	zval *zcontext;
	jit_context_t context;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zcontext) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(context, jit_context_t, &zcontext, -1, le_jit_context_name, le_jit_context);
	
	jit_context_build_end(context);
} /* }}} */
#endif
#endif
