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
#ifndef HAVE_BITS_FUNCTION_H
#define HAVE_BITS_FUNCTION_H

typedef struct _php_jit_function_t php_jit_function_t;
struct _php_jit_function_t {
	zval                zctx;
	zval                zsig;
	zval                zparent;
	jit_function_t      func;
	zend_ulong          st;
	zend_object         std;
};

zend_class_entry *jit_function_ce;

extern PHP_MINIT_FUNCTION(JITFU_Func);

#define PHP_JIT_FETCH_FUNCTION_O(o) ((php_jit_function_t*) ((char*) o - XtOffsetOf(php_jit_function_t, std)))
#define PHP_JIT_FETCH_FUNCTION(from) PHP_JIT_FETCH_FUNCTION_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_FUNCTION_I(from) (PHP_JIT_FETCH_FUNCTION(from))->func
	
#define PHP_JIT_FUNCTION_CREATED     (1<<1)
#define PHP_JIT_FUNCTION_COMPILED    (1<<2)
#define PHP_JIT_FUNCTION_IMPLEMENTED (1<<3)
#endif
