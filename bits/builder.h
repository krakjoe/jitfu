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
#ifndef HAVE_BITS_BUILDER_H
#define HAVE_BITS_BUILDER_H

typedef struct _php_jit_builder_t {
	zend_object std;
	zend_object_handle h;
	php_jit_function_t *func;
} php_jit_builder_t;

zend_class_entry *jit_builder_ce;

void php_jit_minit_builder(int module_number TSRMLS_DC);

#define PHP_JIT_FETCH_BUILDER(from) \
	(php_jit_builder_t*) zend_object_store_get_object((from) TSRMLS_CC)

extern zend_function_entry php_jit_builder_methods[];
extern zend_object_handlers php_jit_builder_handlers;

#else
#ifndef HAVE_BITS_BUILDER
#define HAVE_BITS_BUILDER
zend_object_handlers php_jit_builder_handlers;

typedef jit_value_t (*jit_builder_binary_func) (jit_function_t, jit_value_t, jit_value_t);
typedef jit_value_t (*jit_builder_unary_func)   (jit_function_t, jit_value_t);

static inline void php_jit_do_binary_op(jit_builder_binary_func func, php_jit_function_t *pfunc, zval *zin[2], zval *return_value TSRMLS_DC) {
	jit_value_t in[2];
	php_jit_value_t *pval;
	
	in[0]    = PHP_JIT_FETCH_VALUE_I(zin[0]);
	in[1]    = PHP_JIT_FETCH_VALUE_I(zin[1]);
	
	object_init_ex(return_value, jit_value_ce);

	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = func(pfunc->func, in[0], in[1]);
} /* }}} */

/* {{{ */
static inline void php_jit_do_unary_op(jit_builder_unary_func func, php_jit_function_t *pfunc, zval *zin, zval *return_value TSRMLS_DC) {
	jit_value_t in;
	php_jit_value_t *pval;

	in       = PHP_JIT_FETCH_VALUE_I(zin);
	
	object_init_ex(return_value, jit_value_ce);

	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = func(pfunc->func, in);
} /* }}} */

static inline void php_jit_builder_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_builder_t *pbuild = 
		(php_jit_builder_t *) zobject;

	zend_object_std_dtor(&pbuild->std TSRMLS_CC);

	zend_objects_store_del_ref_by_handle(pbuild->func->h TSRMLS_CC);
	
	efree(pbuild);
}

static inline zend_object_value php_jit_builder_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;
	php_jit_builder_t *pbuild = 
		(php_jit_builder_t*) ecalloc(1, sizeof(php_jit_builder_t));
	
	zend_object_std_init(&pbuild->std, ce TSRMLS_CC);
	object_properties_init(&pbuild->std, ce);
	
	pbuild->h = zend_objects_store_put(
		pbuild, 
		php_jit_builder_destroy, NULL, NULL TSRMLS_CC);

	value.handle   = pbuild->h;
	value.handlers = &php_jit_builder_handlers;
	
	return value;
}

void php_jit_minit_builder(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JIT", "Builder", php_jit_builder_methods);
	jit_builder_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_builder_ce->create_object = php_jit_builder_create;
	
	memcpy(
		&php_jit_builder_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_builder_handlers));
}

PHP_METHOD(Builder, __construct) {
	zval *zfunction;
	php_jit_builder_t *pbuild;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zfunction, jit_function_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	pbuild->func = PHP_JIT_FETCH_FUNCTION(zfunction);
	zend_objects_store_add_ref_by_handle
		(pbuild->func->h TSRMLS_CC);
}

PHP_METHOD(Builder, doIf) {
	zval *op = NULL;
	php_jit_builder_t *pbuild;
	jit_value_t temp;
	jit_label_t label = jit_label_undefined;
	zend_fcall_info zpfci, znfci;
	zend_fcall_info_cache zpfcc, znfcc;
	zval *zretnull = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|f", &op, jit_value_ce, &zpfci, &zpfcc, &znfci, &znfcc) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	jit_insn_branch_if_not(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(op), &label);
	
	zpfci.retval_ptr_ptr = &zretnull;
	
	zend_fcall_info_argn(&zpfci TSRMLS_CC, 1, &getThis());
	
	zend_try {
		zend_call_function(&zpfci, &zpfcc TSRMLS_CC);
	} zend_end_try();
	
	zend_fcall_info_args_clear(&zpfci, 1);
	
	if (zretnull) {
		zval_ptr_dtor(&zretnull);
	}
	
	jit_insn_label(pbuild->func->func, &label);
	
	if (ZEND_NUM_ARGS() > 3) {
		znfci.retval_ptr_ptr = &zretnull;
	
		zend_fcall_info_argn(&znfci TSRMLS_CC, 1, &getThis());
	
		zend_try {
			zend_call_function(&znfci, &znfcc TSRMLS_CC);
		} zend_end_try();
	
		zend_fcall_info_args_clear(&znfci, 1);

		if (zretnull) {
			zval_ptr_dtor(&zretnull);
		}
	}
}

PHP_METHOD(Builder, doEq) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_eq, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doNe) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_ne, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLt) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_lt, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLe) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_le, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doGt) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_gt, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doGe) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_ge, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCmpl) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_cmpl, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCmpg) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_cmpg, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doMul) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_mul, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doMulOvf) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_mul_ovf, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAdd) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_add, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAddOvf) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_add_ovf, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSub) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_sub, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSubOvf) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_sub_ovf, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doDiv) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_div, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doPow) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_pow, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doRem) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_rem, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doRemIEEE) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_rem_ieee, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doNeg) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_neg, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAnd) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_and, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doOr) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_or, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doXor) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_xor, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doShl) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_shl, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doShr) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_shr, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doUshr) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_ushr, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSshr) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_sshr, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doToBool) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_to_bool, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doToNotBool) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_to_not_bool, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAcos) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_acos, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAsin) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_asin, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAtan) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_atan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAtan2) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_atan2, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doMin) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_min, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doMax) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_max, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCeil) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_ceil, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCos) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_cos, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCosh) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_cosh, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doExp) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_exp, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doFloor) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_floor, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLog) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_log, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLog10) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_log10, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doRint) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_rint, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doRound) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_round, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSin) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sin, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSinh) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sinh, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSqrt) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sqrt, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doTan) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_tan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doTanh) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_tan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAbs) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_abs, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSign) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sign, pbuild->func, zin, return_value TSRMLS_CC);
}


PHP_METHOD(Builder, doIsNAN) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_nan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doIsFinite) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_finite, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doIsInf) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_inf, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doReturn) {
	zval *zin;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_BOOL(jit_insn_return(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zin)));
}

PHP_METHOD(Builder, doCall) {
	zval *zfunction = NULL, *zsignature = NULL;
	HashTable *zparams;
	HashPosition position;
	zval **zparam;
	jit_value_t *args;
	int arg = 0;
	long flags = 0;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOH|l", &zfunction, jit_function_ce, &zsignature, jit_signature_ce, &zparams, &flags) != SUCCESS) {
		return;
	}

	args = (jit_value_t*) ecalloc(zend_hash_num_elements(zparams), sizeof(jit_value_t));

	for (zend_hash_internal_pointer_reset_ex(zparams, &position);
		zend_hash_get_current_data_ex(zparams, (void**)&zparam, &position) == SUCCESS;
		zend_hash_move_forward_ex(zparams, &position)) {
		args[arg] = PHP_JIT_FETCH_VALUE_I(*zparam);
		arg++;
	}

	object_init_ex(return_value, jit_value_ce);

	{
		php_jit_value_t *pval =
			PHP_JIT_FETCH_VALUE(return_value);

		pval->value = jit_insn_call(
			pbuild->func->func,
			"function",
			PHP_JIT_FETCH_FUNCTION_I(zfunction),
			NULL, /* this should be signature */
			args, arg, flags);
	}

	efree(args);
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_binary_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, op1) 
	ZEND_ARG_INFO(0, op2) 
ZEND_END_ARG_INFO()
	
ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_unary_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, op) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doIf_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, positive)
	ZEND_ARG_INFO(0, negative)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doCall_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, signature)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_builder_methods[] = {
	PHP_ME(Builder, __construct,  php_jit_builder_construct_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIf,         php_jit_builder_doIf_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMul,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMulOvf,     php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAdd,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAddOvf,     php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSub,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSubOvf,     php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doDiv,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doPow,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doRem,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doRemIEEE,    php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doNeg,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAnd,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doOr,         php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doXor,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doShl,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doShr,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doUshr,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSshr,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doEq,         php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doNe,         php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLt,         php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLe,         php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doGt,         php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doGe,         php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCmpl,       php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCmpg,       php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doToBool,     php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doToNotBool,  php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAcos,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAsin,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAtan,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAtan2,      php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMin,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMax,        php_jit_builder_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCeil,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCos,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCosh,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doExp,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doFloor,      php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLog,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLog10,      php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doRint,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doRound,      php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSin,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSinh,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSqrt,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doTan,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doTanh,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAbs,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSign,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIsNAN,      php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIsFinite,   php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIsInf,      php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCall,       php_jit_builder_doCall_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doReturn,     php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
