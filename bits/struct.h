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
#ifndef HAVE_BITS_STRUCT_H
#define HAVE_BITS_STRUCT_H

typedef struct _php_jit_struct_t {
	jit_type_t          type;
	zval                *zfields;
	zend_string         **names;
	zend_ulong          nfields;
	zend_object         std;
} php_jit_struct_t;

zend_class_entry *jit_struct_ce;

#define PHP_JIT_FETCH_STRUCT_O(o) ((php_jit_struct_t*) ((char*) o - XtOffsetOf(php_jit_struct_t, std)))
#define PHP_JIT_FETCH_STRUCT(from) PHP_JIT_FETCH_STRUCT_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_STRUCT_I(from) (PHP_JIT_FETCH_STRUCT(from))->struct

extern PHP_MINIT_FUNCTION(JITFU_Struct);
#endif
