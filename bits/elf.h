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
#ifndef HAVE_BITS_ELF_H
#define HAVE_BITS_ELF_H

typedef struct _php_jit_elf_t {
	zend_object std;
	zend_object_handle h;
	jit_writeelf_t wr;
	jit_readelf_t  rd;
	zend_ulong     mode;
} php_jit_elf_t;

zend_class_entry *jit_elf_ce;

void php_jit_minit_elf(int module_number TSRMLS_DC);

#define PHP_JIT_ELF_READ  (1<<1)
#define PHP_JIT_ELF_WRITE (1<<2)

#define PHP_JIT_FETCH_ELF(from) \
	(php_jit_elf_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_ELFR_I(from) \
	(PHP_JIT_FETCH_ELF(from))->rd
#define PHP_JIT_FETCH_ELFW_I(from) \
	(PHP_JIT_FETCH_ELF(from))->wr

extern zend_function_entry php_jit_elf_methods[];
extern zend_object_handlers php_jit_elf_handlers;

#else
#ifndef HAVE_BITS_ELF
#define HAVE_BITS_ELF
zend_object_handlers php_jit_elf_handlers;

static inline void php_jit_elf_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	zend_objects_destroy_object(zobject, handle TSRMLS_CC);
}

static inline void php_jit_elf_free(void *zobject TSRMLS_DC) {
	php_jit_elf_t *pelf = 
		(php_jit_elf_t *) zobject;

	zend_object_std_dtor(&pelf->std TSRMLS_CC);

	switch (pelf->mode) {
		case PHP_JIT_ELF_READ:
		
		break;
		
		case PHP_JIT_ELF_WRITE:
			jit_writeelf_destroy(pelf->wr);
		break;
	}

	efree(pelf);
}

static inline zend_object_value php_jit_elf_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;
	php_jit_elf_t *pelf = 
		(php_jit_elf_t*) ecalloc(1, sizeof(php_jit_elf_t));
	
	zend_object_std_init(&pelf->std, ce TSRMLS_CC);
	object_properties_init(&pelf->std, ce);
	
	pelf->h = zend_objects_store_put(
		pelf, 
		php_jit_elf_destroy, 
		php_jit_elf_free, NULL TSRMLS_CC);

	value.handle   = pelf->h;
	value.handlers = &php_jit_elf_handlers;
	
	return value;
}

void php_jit_minit_elf(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "ELF", php_jit_elf_methods);
	jit_elf_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_elf_ce->create_object = php_jit_elf_create;
	
	memcpy(
		&php_jit_elf_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_elf_handlers));
	
	REGISTER_LONG_CONSTANT("JIT_ELF_READ",  PHP_JIT_ELF_READ,  CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ELF_WRITE", PHP_JIT_ELF_WRITE, CONST_CS|CONST_PERSISTENT);
}

PHP_METHOD(ELF, __construct) {
	zval *zlibrary = NULL;
	long zmode = 0;
	php_jit_elf_t *pelf;
	
	if (php_jit_parameters("zl", &zlibrary, &zmode) != SUCCESS ||
		(!zlibrary || Z_TYPE_P(zlibrary) != IS_STRING)) {
		php_jit_exception("unexpected parameters, expected (string lib, int mode)");
		return;
	}
	
	if (zmode != PHP_JIT_ELF_READ && zmode != PHP_JIT_ELF_WRITE) {
		php_jit_exception("unexpected mode, expected JIT_ELF_READ or JIT_ELF_WRITE");
		return;
	}
	
	pelf = PHP_JIT_FETCH_ELF(getThis());
	pelf->mode = zmode;
	
	switch (pelf->mode) {
		case PHP_JIT_ELF_READ: {
		
		} break;
		
		case PHP_JIT_ELF_WRITE: {
			pelf->wr = jit_writeelf_create(Z_STRVAL_P(zlibrary));
		} break;
	}
}

PHP_METHOD(ELF, addFunction) {
	zval *zfunction = NULL, 
		 *zname = NULL;
	php_jit_elf_t *pelf;
	php_jit_function_t *pfunc;
	
	if (php_jit_parameters("zO", &zname, &zfunction, jit_function_ce) != SUCCESS ||
		(!zname || Z_TYPE_P(zname) != IS_STRING)) {
		php_jit_exception("unexpected parameters, expected (string name, Func function)");
		return;
	}
	
	pelf = PHP_JIT_FETCH_ELF(getThis());
	
	if (pelf->mode != PHP_JIT_ELF_WRITE) {
		php_jit_exception("incorrect mode for method");
		return;
	}
	
	RETURN_BOOL(jit_writeelf_add_function(pelf->wr, PHP_JIT_FETCH_FUNCTION_I(zfunction), Z_STRVAL_P(zname)));
}

PHP_METHOD(ELF, addNeeded) {
	zval *zlibrary = NULL;
	php_jit_elf_t *pelf;
	
	if (php_jit_parameters("z", &zlibrary) != SUCCESS ||
		(!zlibrary || Z_TYPE_P(zlibrary) != IS_STRING)) {
		php_jit_exception("unexpected parameters, expected (string library)");
		return;
	}
	
	pelf = PHP_JIT_FETCH_ELF(getThis());
	
	if (pelf->mode != PHP_JIT_ELF_WRITE) {
		php_jit_exception("incorrect mode for method");
		return;
	}
	
	RETURN_BOOL(jit_writeelf_add_needed(pelf->wr, Z_STRVAL_P(zlibrary)));
}

PHP_METHOD(ELF, write) {
	zval *zfilename = NULL;
	php_jit_elf_t *pelf;
	
	if (php_jit_parameters("z", &zfilename) != SUCCESS ||
		(!zfilename || Z_TYPE_P(zfilename) != IS_STRING)) {
		php_jit_exception("unexpected parameters, expected (string filename)");
		return;
	}
	
	pelf = PHP_JIT_FETCH_ELF(getThis());
	
	if (pelf->mode != PHP_JIT_ELF_WRITE) {
		php_jit_exception("incorrect mode for method");
		return;
	}
	
	RETURN_BOOL(jit_writeelf_write(pelf->wr, Z_STRVAL_P(zfilename)));
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_elf_construct_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, lib)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_elf_addFunction_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_elf_addNeeded_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, library)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_elf_write_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_elf_methods[] = {
	PHP_ME(ELF, __construct,    php_jit_elf_construct_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(ELF, addFunction,    php_jit_elf_addFunction_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(ELF, addNeeded,      php_jit_elf_addNeeded_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(ELF, write,          php_jit_elf_write_arginfo,           ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
