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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_closures.h"
#include "php_jit.h"
#ifdef HAVE_SPL
#include "ext/spl/spl_exceptions.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(jit)

#include "bits/context.h"
#include "bits/type.h"
#include "bits/function.h"
#include "bits/value.h"
#include "bits/insn.h"

/* {{{ jit_functions[]
 */
const zend_function_entry jit_functions[] = {
	PHP_JIT_CONTEXT_FUNCTIONS
	PHP_JIT_TYPE_FUNCTIONS
	PHP_JIT_FUNCTION_FUNCTIONS
	PHP_JIT_VALUE_FUNCTIONS
	PHP_JIT_INSN_FUNCTIONS
	PHP_FE_END
};
/* }}} */

/* {{{ jit_module_entry
 */
zend_module_entry jit_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_JIT_EXTNAME,
	jit_functions,
	PHP_MINIT(jit),
	PHP_MSHUTDOWN(jit),
	PHP_RINIT(jit),
	PHP_RSHUTDOWN(jit),
	PHP_MINFO(jit),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_JIT_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_JIT
ZEND_GET_MODULE(jit)
#endif

/* {{{ php_jit_init_globals
 */
static void php_jit_globals_ctor(zend_jit_globals *jit_globals){}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(jit)
{
	ZEND_INIT_MODULE_GLOBALS(jit, php_jit_globals_ctor, NULL);
	
	php_jit_minit_context(module_number TSRMLS_CC);
	php_jit_minit_type(module_number TSRMLS_CC);
	php_jit_minit_function(module_number TSRMLS_CC);
	php_jit_minit_value(module_number TSRMLS_CC);
	php_jit_minit_insn(module_number TSRMLS_CC);	

	zend_hash_init(&JG(ctx), 8, NULL, NULL, 1);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(jit)
{	
	zend_hash_destroy(&JG(ctx));
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(jit)
{	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(jit)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(jit)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "jit support", "enabled");
	php_info_print_table_end();
}
/* }}} */

#include "bits/context.h"
#include "bits/type.h"
#include "bits/function.h"
#include "bits/value.h"
#include "bits/insn.h"

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
