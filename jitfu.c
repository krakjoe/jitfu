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
#include "php_jitfu.h"
#ifdef HAVE_SPL
#include "ext/spl/spl_exceptions.h"
#endif

#include "bits/context.h"
#include "bits/type.h"
#include "bits/signature.h"
#include "bits/function.h"
#include "bits/value.h"
#include "bits/builder.h"

/* {{{ jit_module_entry
 */
zend_module_entry jitfu_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_JITFU_EXTNAME,
	NULL,
	PHP_MINIT(jitfu),
	PHP_MSHUTDOWN(jitfu),
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

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(jitfu)
{
	php_jit_minit_context(module_number TSRMLS_CC);
	php_jit_minit_type(module_number TSRMLS_CC);
	php_jit_minit_signature(module_number TSRMLS_CC);
	php_jit_minit_function(module_number TSRMLS_CC);
	php_jit_minit_value(module_number TSRMLS_CC);
	php_jit_minit_builder(module_number TSRMLS_CC);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(jitfu)
{	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(jitfu)
{	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(jitfu)
{
	return SUCCESS;
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
#include "bits/signature.h"
#include "bits/function.h"
#include "bits/value.h"
#include "bits/builder.h"

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
