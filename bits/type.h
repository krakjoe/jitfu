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
#ifndef HAVE_BITS_TYPE_H
#define HAVE_BITS_TYPE_H

typedef struct _php_jit_type_t {
	jit_type_t          type;
	/* must be first three members */
	zend_ulong          id;
	zend_ulong          pt;
	zend_bool           copied;
	zend_object         std;
} php_jit_type_t;

zend_class_entry *jit_type_ce;

typedef struct _php_jit_sized_t {
	void       **data;
	ulong        length;
} php_jit_sized_t;

typedef struct _php_jit_string_t {
	ulong len;
	char  *val;
} php_jit_string_t;

jit_type_t jit_type_sizable;
jit_type_t jit_type_sized;
jit_type_t jit_type_zobject;
jit_type_t jit_type_zvalue;
jit_type_t jit_type_zval;
jit_type_t jit_type_pzval;

#define PHP_JIT_FETCH_TYPE_O(o) ((php_jit_type_t*) ((char*) o - XtOffsetOf(php_jit_type_t, std)))
#define PHP_JIT_FETCH_TYPE(from) PHP_JIT_FETCH_TYPE_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_TYPE_I(from) (PHP_JIT_FETCH_TYPE(from))->type

#define PHP_JIT_TYPE_VOID  		1
#define PHP_JIT_TYPE_UINT       2
#define PHP_JIT_TYPE_INT        3
#define PHP_JIT_TYPE_ULONG      4
#define PHP_JIT_TYPE_LONG		5
#define PHP_JIT_TYPE_DOUBLE		6
#define PHP_JIT_TYPE_STRING		7
#define PHP_JIT_TYPE_ZVAL	    8

jit_type_t php_jit_type(short type);

extern PHP_MINIT_FUNCTION(JITFU_Type);

jit_type_t jit_type_string;
#endif
