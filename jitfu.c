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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_closures.h"
#include "zend_exceptions.h"
#include "php_jitfu.h"

zend_class_entry *jit_exception_ce;

ZEND_DECLARE_MODULE_GLOBALS(jitfu);

#include "bits/context.h"
#include "bits/type.h"
#include "bits/struct.h"
#include "bits/signature.h"
#include "bits/function.h"
#include "bits/label.h"
#include "bits/value.h"
#include "bits/elf.h"

/* {{{ jit_module_entry
 */
zend_module_entry jitfu_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_JITFU_EXTNAME,
	NULL,
	PHP_MINIT(jitfu),
	NULL,
	PHP_RINIT(jitfu),
	PHP_RSHUTDOWN(jitfu),
	PHP_MINFO(jitfu),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_JITFU_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_JITFU
ZEND_GET_MODULE(jitfu)
#endif

static inline void php_jit_globals_ctor(zend_jitfu_globals *jg) {}

static inline const char* php_jit_exception_type(int type) {
	switch (type) {
		case JIT_RESULT_OK:               return "none";
		case JIT_RESULT_OVERFLOW:         return "overflow";
		case JIT_RESULT_ARITHMETIC:       return "arithmetic";
		case JIT_RESULT_DIVISION_BY_ZERO: return "division by zero";
		case JIT_RESULT_COMPILE_ERROR:    return "compile error";
		case JIT_RESULT_OUT_OF_MEMORY:    return "out of memory";
		case JIT_RESULT_NULL_REFERENCE:   return "null reference";
		case JIT_RESULT_NULL_FUNCTION:    return "null function";
		case JIT_RESULT_CALLED_NESTED:    return "called nested";
		case JIT_RESULT_OUT_OF_BOUNDS:    return "out of bounds";
		case JIT_RESULT_UNDEFINED_LABEL:  return "undefined label";
		case JIT_RESULT_MEMORY_FULL:      return "memory full";
		default:                          return "unknown error";
	}
}

static inline void* php_jit_exception_handler(int type) {
	TSRMLS_FETCH();
	
	zend_throw_exception_ex
		(jit_exception_ce, type TSRMLS_CC, php_jit_exception_type(type));
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(jitfu)
{
	zend_class_entry ce;
	
	ZEND_INIT_MODULE_GLOBALS(jitfu, php_jit_globals_ctor, NULL);
	
	jit_exception_set_handler(php_jit_exception_handler);
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Exception", NULL);
	
	php_jit_minit_context(module_number TSRMLS_CC);
	php_jit_minit_type(module_number TSRMLS_CC);
	php_jit_minit_struct(module_number TSRMLS_CC);
	php_jit_minit_signature(module_number TSRMLS_CC);
	php_jit_minit_function(module_number TSRMLS_CC);
	php_jit_minit_value(module_number TSRMLS_CC);
	php_jit_minit_label(module_number TSRMLS_CC);
	php_jit_minit_elf(module_number TSRMLS_CC);	

	jit_exception_ce = zend_register_internal_class_ex
		(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(jitfu)
{
	zend_hash_init(&JG(types), 8, NULL, ZVAL_PTR_DTOR, 0);
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(jitfu)
{
	zend_hash_destroy(&JG(types));
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(jitfu)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "JIT-Fu support", "enabled");
	php_info_print_table_end();
}
/* }}} */

#include "bits/context.h"
#include "bits/type.h"
#include "bits/struct.h"
#include "bits/signature.h"
#include "bits/function.h"
#include "bits/value.h"
#include "bits/label.h"
#include "bits/elf.h"

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
