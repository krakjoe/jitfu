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
#ifndef HAVE_BITS_CONTEXT_H
#define HAVE_BITS_CONTEXT_H

typedef struct _php_jit_context_t {
	jit_context_t       ctx;
	zend_ulong          st;
	zend_object         std;
} php_jit_context_t;

zend_class_entry *jit_context_ce;

extern PHP_MINIT_FUNCTION(JITFU_Context);

#define PHP_JIT_FETCH_CONTEXT_O(o) ((php_jit_context_t*) ((char*) o - XtOffsetOf(php_jit_context_t, std)))
#define PHP_JIT_FETCH_CONTEXT(from) PHP_JIT_FETCH_CONTEXT_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_CONTEXT_I(from) (PHP_JIT_FETCH_CONTEXT(from))->ctx
	
#define PHP_JIT_CONTEXT_STARTED  (1<<1)
#define PHP_JIT_CONTEXT_FINISHED (1<<2)
#endif
