/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2018 Joe Watkins <krakjoe@php.net>                     |
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
#ifndef HAVE_BITS_CONTEXT
#define HAVE_BITS_CONTEXT

#include <php.h>

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <php_jitfu.h>
#include <zend_exceptions.h>

#include <bits/context.h>

zend_object_handlers php_jit_context_handlers;
zend_function_entry php_jit_context_methods[];

static inline void php_jit_context_free(zend_object *zobject) {
	php_jit_context_t *pcontext = 
		PHP_JIT_FETCH_CONTEXT_O(zobject);

	zend_object_std_dtor(&pcontext->std);

	jit_context_destroy(pcontext->ctx);
}

static inline zend_object* php_jit_context_create(zend_class_entry *ce) {
	php_jit_context_t *pcontext = 
		(php_jit_context_t*) ecalloc(1, sizeof(php_jit_context_t) + zend_object_properties_size(ce));
	
	zend_object_std_init(&pcontext->std, ce);
	object_properties_init(&pcontext->std, ce);
	
	pcontext->ctx = jit_context_create();
	
	pcontext->std.handlers = &php_jit_context_handlers;
	
	return &pcontext->std;
}

PHP_MINIT_FUNCTION(JITFU_Context) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Context", php_jit_context_methods);
	jit_context_ce = zend_register_internal_class(&ce);
	jit_context_ce->create_object = php_jit_context_create;
	
	memcpy(
		&php_jit_context_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_context_handlers));

	php_jit_context_handlers.offset = XtOffsetOf(php_jit_context_t, std);
	
	REGISTER_LONG_CONSTANT("JIT_OPTION_CACHE_LIMIT", JIT_OPTION_CACHE_LIMIT, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_CACHE_PAGE_SIZE", JIT_OPTION_CACHE_PAGE_SIZE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_PRE_COMPILE", JIT_OPTION_PRE_COMPILE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_DONT_FOLD", JIT_OPTION_DONT_FOLD, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_POSITION_INDEPENDENT", JIT_OPTION_POSITION_INDEPENDENT, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_OPTION_CACHE_MAX_PAGE_FACTOR", JIT_OPTION_CACHE_MAX_PAGE_FACTOR, CONST_CS|CONST_PERSISTENT);

	return SUCCESS;
}

PHP_METHOD(Context, start) {
	php_jit_context_t *pcontext;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pcontext = PHP_JIT_FETCH_CONTEXT(getThis());
	
	if (pcontext->st & PHP_JIT_CONTEXT_STARTED) {
		zend_throw_exception_ex(NULL, 0, 
			"this context was already started");
		return;
	}
	
	jit_context_build_start(pcontext->ctx);

	pcontext->st |= PHP_JIT_CONTEXT_STARTED;
}

PHP_METHOD(Context, finish) {
	php_jit_context_t *pcontext;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pcontext = PHP_JIT_FETCH_CONTEXT(getThis());
	
	if (pcontext->st & PHP_JIT_CONTEXT_FINISHED) {
		zend_throw_exception_ex(NULL, 0,
			"this context was already finished");
		return;
	}
	
	jit_context_build_end(pcontext->ctx);
	
	pcontext->st |= PHP_JIT_CONTEXT_FINISHED;
}

PHP_METHOD(Context, isStarted) {
	php_jit_context_t *pcontext;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pcontext = PHP_JIT_FETCH_CONTEXT(getThis());
	
	RETURN_BOOL(pcontext->st & PHP_JIT_CONTEXT_STARTED);
}

PHP_METHOD(Context, isFinished) {
	php_jit_context_t *pcontext;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pcontext = PHP_JIT_FETCH_CONTEXT(getThis());
	
	RETURN_BOOL(pcontext->st & PHP_JIT_CONTEXT_FINISHED);
}

zend_function_entry php_jit_context_methods[] = {
	PHP_ME(Context, start,      php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Context, finish,     php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	/* ... */
	PHP_ME(Context, isStarted,  php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Context, isFinished, php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
