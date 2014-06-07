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

/* $Id$ */

#ifndef PHP_JITFU_H
#define PHP_JITFU_H

extern zend_module_entry jitfu_module_entry;
#define phpext_jitfu_ptr &jitfu_module_entry

#ifdef PHP_WIN32
#	define PHP_JIT_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_JIT_API __attribute__ ((visibility("default")))
#else
#	define PHP_JIT_API
#endif

#include <jit/jit.h>

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(jitfu);
PHP_MINFO_FUNCTION(jitfu);
PHP_RINIT_FUNCTION(jitfu);
PHP_RSHUTDOWN_FUNCTION(jitfu);

#define PHP_JITFU_EXTNAME "JITFU"
#define PHP_JITFU_VERSION "0.4"

ZEND_BEGIN_ARG_INFO_EX(php_jit_no_arginfo, 0, 0, 0) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_MODULE_GLOBALS(jitfu)
	HashTable types;
ZEND_END_MODULE_GLOBALS(jitfu)

#ifdef ZTS
#define JG(v) TSRMG(jitfu_globals_id, zend_jitfu_globals *, v)
#else
#define JG(v) (jitfu_globals.v)
#endif

extern zend_class_entry *jit_exception_ce;

#define php_jit_exception(s, ...) zend_throw_exception_ex(jit_exception_ce, 0 TSRMLS_CC, s, ##__VA_ARGS__)
#define php_jit_parameters(e, s, ...) do { \
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, s, ##__VA_ARGS__) != SUCCESS) { \
		php_jit_exception("expected " e); \
		return; \
	} \
} while(0)
#endif	/* PHP_JITFU_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
