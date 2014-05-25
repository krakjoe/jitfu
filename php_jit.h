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

/* $Id$ */

#ifndef PHP_JIT_H
#define PHP_JIT_H

extern zend_module_entry jit_module_entry;
#define phpext_jit_ptr &jit_module_entry

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

PHP_MINIT_FUNCTION(jit);
PHP_MSHUTDOWN_FUNCTION(jit);
PHP_RINIT_FUNCTION(jit);
PHP_RSHUTDOWN_FUNCTION(jit);
PHP_MINFO_FUNCTION(jit);

ZEND_BEGIN_MODULE_GLOBALS(jit)
	HashTable ctx;
ZEND_END_MODULE_GLOBALS(jit)

#define PHP_JIT_EXTNAME "jit"
#define PHP_JIT_VERSION "0.1"

#define JIT_FE(f) PHP_FE(f, f##_arginfo)

#ifdef ZTS
#define JG(v) TSRMG(jit_globals_id, zend_jit_globals *, v)
#else
#define JG(v) (jit_globals.v)
#endif

#endif	/* PHP_JIT_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
