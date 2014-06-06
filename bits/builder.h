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

typedef int         (*jit_builder_mem_func)     (jit_function_t, jit_value_t, jit_value_t, jit_value_t);
typedef jit_value_t (*jit_builder_binary_func)  (jit_function_t, jit_value_t, jit_value_t);
typedef jit_value_t (*jit_builder_unary_func)   (jit_function_t, jit_value_t);

/* {{{ */
static inline void php_jit_do_mem_op(jit_builder_mem_func func, php_jit_function_t *pfunc, zval *zin[3], zval *return_value TSRMLS_DC) {
	jit_value_t in[3];
	int result;
	
	in[0]    = PHP_JIT_FETCH_VALUE_I(zin[0]);
	in[1]    = PHP_JIT_FETCH_VALUE_I(zin[1]);
	in[2]    = PHP_JIT_FETCH_VALUE_I(zin[2]);
	
	result = func
		(pfunc->func, in[0], in[1], in[2]);
	
	ZVAL_LONG(return_value, result);
} /* }}} */

/* {{{ */
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

	if (pbuild->func) {
		zend_objects_store_del_ref_by_handle(pbuild->func->h TSRMLS_CC);	
	}
	
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
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Builder", php_jit_builder_methods);
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
	zval *zbuilder;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|O", &zfunction, jit_function_ce, &zbuilder, zend_ce_closure) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	pbuild->func = PHP_JIT_FETCH_FUNCTION(zfunction);
	zend_objects_store_add_ref_by_handle(pbuild->func->h TSRMLS_CC);
		
	if (ZEND_NUM_ARGS() > 1) {
		zval *retval_ptr = NULL, 
			 *tmp_ptr = NULL, 
			 params;
		zval closure;
		zend_fcall_info       fci;
		zend_fcall_info_cache fcc;
		zend_uint             nparam = 0;
		const zend_function   *function = zend_get_closure_method_def(zbuilder TSRMLS_CC);
		int result = FAILURE;
		
		if (function->common.num_args != pbuild->func->sig->nparams) {
			/* throw incorrect number of argument accepted by builder function */
			return;
		}
		
		/* bind builder function to current scope and object */
		zend_create_closure(&closure, (zend_function*) function, EG(scope), EG(This) TSRMLS_CC);

		if (zend_fcall_info_init(&closure, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, NULL, NULL TSRMLS_CC) != SUCCESS) {
			/* throw failed to initialize closure method */
			zval_dtor(&closure);
			return;
		}
		
		/* build params for builder function */
		array_init(&params);
		
		fci.retval_ptr_ptr = &retval_ptr;
		
		while (nparam < pbuild->func->sig->nparams) {
			zval *o;
			php_jit_value_t *pval;
			
			MAKE_STD_ZVAL(o);

			object_init_ex(o, jit_value_ce);

			pval = PHP_JIT_FETCH_VALUE(o);

			pval->builder = pbuild;
			zend_objects_store_add_ref_by_handle(pval->builder->h TSRMLS_CC);

			pval->type = pbuild->func->sig->params[nparam];
			zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);

			pval->value = jit_value_get_param(pbuild->func->func, nparam);

			add_next_index_zval(&params, o);

			nparam++;
		}

		/* call builder function */
		zend_fcall_info_args(&fci, &params TSRMLS_CC);

		result = zend_call_function(&fci, &fcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}
		
		/* cleanup */
		zend_fcall_info_args_clear(&fci, 1);
		
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}

		zval_dtor(&params);
		zval_dtor(&closure);
	}
}

PHP_METHOD(Builder, doWhile) {
	zval *op = NULL;
	php_jit_builder_t *pbuild;
	jit_value_t condition, stop, compare;
	jit_label_t label[2] = 
		{jit_label_undefined, jit_label_undefined};
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	int result = FAILURE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of", &op, jit_value_ce, &fci, &fcc) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	condition = PHP_JIT_FETCH_VALUE_I(op);

	stop      = jit_value_create_nint_constant(pbuild->func->func, jit_type_sys_long, 0);
	
	jit_insn_label(pbuild->func->func, &label[0]);
	
	compare = jit_insn_gt(pbuild->func->func, condition, stop);
	
	jit_insn_branch_if_not(pbuild->func->func, compare, &label[1]);
	{
		zval *retval_ptr = NULL;
		
		fci.retval_ptr_ptr = &retval_ptr;
		fci.params = NULL;
		fci.param_count = 0;
		
		result = zend_call_function(&fci, &fcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}
		
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		
		jit_insn_branch(pbuild->func->func, &label[0]);
	}
	
	jit_insn_label(pbuild->func->func, &label[1]);
}

PHP_METHOD(Builder, doIf) {
	zval *op = NULL;
	php_jit_builder_t *pbuild;
	jit_value_t temp;
	jit_label_t label = jit_label_undefined;
	zend_fcall_info zpfci, znfci;
	zend_fcall_info_cache zpfcc, znfcc;
	zval *zretnull = NULL;
	int result = FAILURE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|f", &op, jit_value_ce, &zpfci, &zpfcc, &znfci, &znfcc) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	jit_insn_branch_if_not(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(op), &label);
	
	zpfci.retval_ptr_ptr = &zretnull;
	zpfci.params = NULL;
	zpfci.param_count = 0;
	
	result = zend_call_function(&zpfci, &zpfcc TSRMLS_CC);
	
	if (result == FAILURE) {
		/* throw failed to call builder function */
		return;
	}
	
	if (zretnull) {
		zval_ptr_dtor(&zretnull);
	}
	
	jit_insn_label(pbuild->func->func, &label);
	
	if (ZEND_NUM_ARGS() > 3) {
		znfci.retval_ptr_ptr = &zretnull;
		znfci.params = NULL;
		znfci.param_count = 0;

		result = zend_call_function(&znfci, &znfcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder function */
			return;
		}

		if (zretnull) {
			zval_ptr_dtor(&zretnull);
		}
	}
}

PHP_METHOD(Builder, doLabel) {
	php_jit_builder_t *pbuild;
	jit_label_t label = jit_label_undefined;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	jit_insn_label(pbuild->func->func, &label);
	
	RETURN_LONG(label);
}

PHP_METHOD(Builder, doBranch) {
	php_jit_builder_t *pbuild;
	jit_label_t label = jit_label_undefined;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &label) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	jit_insn_branch(pbuild->func->func, &label);
	
	RETURN_LONG(label);
}

PHP_METHOD(Builder, doBranchIf) {
	php_jit_builder_t *pbuild;
	jit_label_t label = jit_label_undefined;
	zval *zin = NULL;
	php_jit_value_t *pval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &zin, jit_value_ce, &label) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	pval = PHP_JIT_FETCH_VALUE(zin);
	
	jit_insn_branch_if(pbuild->func->func, pval->value, &label);
	
	RETURN_LONG(label);
}

PHP_METHOD(Builder, doBranchIfNot) {
	php_jit_builder_t *pbuild;
	jit_label_t label = jit_label_undefined;
	zval *zin = NULL;
	php_jit_value_t *pval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &zin, jit_value_ce, &label) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	pval = PHP_JIT_FETCH_VALUE(zin);
	
	jit_insn_branch_if_not(pbuild->func->func, pval->value, &label);
	
	RETURN_LONG(label);
}

PHP_METHOD(Builder, doIfNot) {
	zval *op = NULL;
	php_jit_builder_t *pbuild;
	jit_value_t temp;
	jit_label_t label = jit_label_undefined;
	zend_fcall_info zpfci, znfci;
	zend_fcall_info_cache zpfcc, znfcc;
	zval *zretnull = NULL;
	int result = FAILURE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|f", &op, jit_value_ce, &zpfci, &zpfcc, &znfci, &znfcc) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	jit_insn_branch_if(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(op), &label);
	
	zpfci.retval_ptr_ptr = &zretnull;
	zpfci.params = NULL;
	zpfci.param_count = 0;
	
	result = zend_call_function(&zpfci, &zpfcc TSRMLS_CC);
	
	if (result == FAILURE) {
		/* throw failed to call builder */
		return;
	}

	if (zretnull) {
		zval_ptr_dtor(&zretnull);
	}
	
	jit_insn_label(pbuild->func->func, &label);
	
	if (ZEND_NUM_ARGS() > 3) {
		znfci.retval_ptr_ptr = &zretnull;
		znfci.params = NULL;
		znfci.param_count = 0;

		result = zend_call_function(&znfci, &znfcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}

		if (zretnull) {
			zval_ptr_dtor(&zretnull);
		}
	}
}

PHP_METHOD(Builder, doJumpTable) {
	zval *zvalue, **zmember;
	HashTable *table;
	HashPosition position;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	jit_label_t *labels = NULL, after = jit_label_undefined;
	zend_uint nlabels = 0;
	zend_uint nlabel = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OH", &zvalue, jit_value_ce, &table) != SUCCESS) {
		return;
	}
	
	nlabels = zend_hash_num_elements(table);
	
	if (!nlabels) {
		return;
	}
	
	labels = (jit_label_t*) ecalloc(nlabels, sizeof(jit_label_t));

	for (nlabel = 0; nlabel < nlabels; nlabel++) {
		labels[nlabel] = jit_label_undefined;
	}
	
	jit_insn_jump_table
		(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zvalue), labels, nlabels);
	jit_insn_branch
		(pbuild->func->func, &after);
	
	nlabel = 0;
	
	for (zend_hash_internal_pointer_reset_ex(table, &position);
		zend_hash_get_current_data_ex(table, (void**)&zmember, &position) == SUCCESS;
		zend_hash_move_forward_ex(table, &position)) {
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;
		zval *retval_ptr = NULL;
		int result = FAILURE;
		
		jit_insn_label(pbuild->func->func, &labels[nlabel]);
		
		if (zend_fcall_info_init(*zmember, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, NULL, NULL TSRMLS_CC) != SUCCESS) {
			/* throw, not callable ? */
			nlabel++;
			continue;
		}
		
		fci.retval_ptr_ptr = &retval_ptr;
		fci.params = NULL;
		fci.param_count = 0;
		
		result = zend_call_function(&fci, &fcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}
		
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		
		nlabel++;
	}
	
	jit_insn_label(pbuild->func->func, &after);
	
	efree(labels);
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_to_bool, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doToNotBool) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_to_not_bool, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAcos) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_acos, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAsin) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_asin, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAtan) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_ceil, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCos) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_cos, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCosh) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_cosh, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doExp) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_exp, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doFloor) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_floor, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLog) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_log, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLog10) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_log10, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doRint) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_rint, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doRound) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_round, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSin) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sin, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSinh) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sinh, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSqrt) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	php_jit_do_unary_op(jit_insn_sqrt, pbuild->func, zin, return_value TSRMLS_CC);
	
}

PHP_METHOD(Builder, doTan) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_tan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doTanh) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_tan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAbs) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_abs, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doSign) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sign, pbuild->func, zin, return_value TSRMLS_CC);
}


PHP_METHOD(Builder, doIsNAN) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_nan, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doIsFinite) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_finite, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doIsInf) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_inf, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doAlloca) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_alloca, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLoad) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_load, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLoadSmall) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_load_small, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doDup) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_dup, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doStore) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_store(
		pbuild->func->func,
		PHP_JIT_FETCH_VALUE_I(zin[0]),
		PHP_JIT_FETCH_VALUE_I(zin[1])));
}

PHP_METHOD(Builder, doAddressof) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_address_of, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doCheckNull) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_check_null(
		pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zin)));
}

PHP_METHOD(Builder, doMemcpy) {
	zval *zin[3] = {NULL, NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_mem_op(jit_insn_memcpy, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doMemmove) {
	zval *zin[3] = {NULL, NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_mem_op(jit_insn_memmove, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doMemset) {
	zval *zin[3] = {NULL, NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_mem_op(jit_insn_memset, pbuild->func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Builder, doLoadElem) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	php_jit_value_t *pval, *lval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	lval = PHP_JIT_FETCH_VALUE(zin[0]);
	
	if (!lval->type->pt) {
		/* throw not a pointer type */
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_load_elem(
		pbuild->func->func,
		lval->value, 
		PHP_JIT_FETCH_VALUE_I(zin[1]),
		lval->type->type);
	pval->type = lval->type;
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
}

PHP_METHOD(Builder, doLoadElemAddress) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	php_jit_value_t *pval, *lval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}

	lval = PHP_JIT_FETCH_VALUE(zin[0]);
	
	if (!lval->type->pt) {
		/* throw not a pointer type */
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_load_elem_address(
		pbuild->func->func,
		lval->value, 
		PHP_JIT_FETCH_VALUE_I(zin[1]), 
		lval->type->type);
	pval->type = lval->type;
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
}

PHP_METHOD(Builder, doLoadRelative) {
	zval *zin = NULL;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	long index = 0;
	php_jit_value_t *pval, *lval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &zin, jit_value_ce, &index) != SUCCESS) {
		return;
	}
	
	lval = PHP_JIT_FETCH_VALUE(zin);
	
	if (!lval->type->pt) {
		/* throw not a pointer type */
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_load_relative(
		pbuild->func->func,
		lval->value,
		index,
		lval->type->type);
	pval->type = lval->type;
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
}

PHP_METHOD(Builder, doStoreRelative) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	long index = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &index, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_store_relative(
		pbuild->func->func,
		PHP_JIT_FETCH_VALUE_I(zin[0]), 
		index, 
		PHP_JIT_FETCH_VALUE_I(zin[1])));
}

PHP_METHOD(Builder, doConvert) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	php_jit_value_t *pval;
	zend_bool overflow = 0;
	long index = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO|b", &zin[0], jit_value_ce, &zin[1], jit_type_ce, &overflow) != SUCCESS) {
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_convert(
		pbuild->func->func,
		PHP_JIT_FETCH_VALUE_I(zin[0]),
		PHP_JIT_FETCH_TYPE_I(zin[1]), overflow);
	pval->type = PHP_JIT_FETCH_TYPE(zin[1]);
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
}

PHP_METHOD(Builder, doStoreElem) {
	zval *zin[3] = {NULL, NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_store_elem(
		pbuild->func->func,
		PHP_JIT_FETCH_VALUE_I(zin[0]),
		PHP_JIT_FETCH_VALUE_I(zin[1]), 
		PHP_JIT_FETCH_VALUE_I(zin[2])));
}

PHP_METHOD(Builder, doDefaultReturn) {
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_BOOL(jit_insn_default_return(pbuild->func->func));
}

PHP_METHOD(Builder, doGetCallStack) {
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	php_jit_value_t   *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_get_call_stack(pbuild->func->func);
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

PHP_METHOD(Builder, doPush) {
	zval *zin;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_BOOL(jit_insn_push(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zin)));
}

PHP_METHOD(Builder, doPop) {
	zval *zin;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	long nitems = 1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &nitems) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_LONG(jit_insn_pop_stack(pbuild->func->func, (jit_nint) nitems));
}

PHP_METHOD(Builder, doDeferPop) {
	zval *zin;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	long nitems = 1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &nitems) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_LONG(jit_insn_defer_pop_stack(pbuild->func->func, (jit_nint) nitems));
}

PHP_METHOD(Builder, doFlushDeferPop) {
	zval *zin;
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	long nitems = 1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &nitems) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_LONG(jit_insn_flush_defer_pop(pbuild->func->func, (jit_nint) nitems));
}

PHP_METHOD(Builder, doReturnPtr) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_type_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_BOOL(jit_insn_return_ptr
		(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zin[0]), PHP_JIT_FETCH_TYPE_I(zin[2])));
}

PHP_METHOD(Builder, doPushPtr) {
	zval *zin[2] = {NULL, NULL};
	php_jit_builder_t *pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_type_ce) != SUCCESS) {
		return;
	}
	
	pbuild = PHP_JIT_FETCH_BUILDER(getThis());
	
	RETURN_BOOL(jit_insn_push_ptr
		(pbuild->func->func, PHP_JIT_FETCH_VALUE_I(zin[0]), PHP_JIT_FETCH_TYPE_I(zin[2])));
}

PHP_METHOD(Builder, doCall) {
	zval *zfunction = NULL, *zsignature = NULL;
	HashTable *zparams;
	HashPosition position;
	zval **zparam;
	jit_value_t *args;
	zend_uint arg = 0;
	long flags = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OH|l", &zfunction, jit_function_ce, &zparams, &flags) != SUCCESS) {
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
		php_jit_builder_t *pbuild = 
			PHP_JIT_FETCH_BUILDER(getThis());
		php_jit_function_t *pfunc = 
			PHP_JIT_FETCH_FUNCTION(zfunction);
		php_jit_value_t *pval =
			PHP_JIT_FETCH_VALUE(return_value);
		
		pval->value = jit_insn_call(
			pbuild->func->func,
			NULL,
			pfunc->func,
			pfunc->sig->type,
			args, arg, flags);
	}

	efree(args);
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_ternary_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, op1) 
	ZEND_ARG_INFO(0, op2) 
	ZEND_ARG_INFO(0, op3)
ZEND_END_ARG_INFO()

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

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doWhile_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doIncrement_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, op)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doDecrement_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, op)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doLabel_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doBranch_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, label) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doBranchIf_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, label) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doBranchIfNot_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, label) 
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

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doMemcpy_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, src)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doMemmove_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, src)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doMemset_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()
	
ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doLoadElem_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doLoadRelative_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doStoreRelative_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doStoreElem_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doJumpTable_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doConvert_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, overflow)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doReturnPtr_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doPushPtr_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_builder_doPop_arginfo, 0, 0, 0) 
	ZEND_ARG_INFO(0, items)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_builder_methods[] = {
	PHP_ME(Builder, __construct,  php_jit_builder_construct_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLabel,      php_jit_builder_doLabel_arginfo,    ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doBranch,     php_jit_builder_doBranch_arginfo,   ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doBranchIf,   php_jit_builder_doBranchIf_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doBranchIfNot,php_jit_builder_doBranchIfNot_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIf,         php_jit_builder_doIf_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIfNot,      php_jit_builder_doIf_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doWhile,      php_jit_builder_doWhile_arginfo,    ZEND_ACC_PUBLIC)
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
	PHP_ME(Builder, doFloor,           php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLog,             php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLog10,           php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doRint,            php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doRound,           php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSin,             php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSinh,            php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSqrt,            php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doTan,             php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doTanh,            php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAbs,             php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doSign,            php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIsNAN,           php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIsFinite,        php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doIsInf,           php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCall,            php_jit_builder_doCall_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAlloca,          php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLoad,            php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLoadSmall,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doDup,             php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doStore,           php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doAddressof,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doCheckNull,       php_jit_builder_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMemcpy,          php_jit_builder_doMemcpy_arginfo,   ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMemmove,         php_jit_builder_doMemmove_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doMemset,          php_jit_builder_doMemset_arginfo,   ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLoadElem,        php_jit_builder_doLoadElem_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLoadRelative,    php_jit_builder_doLoadRelative_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doLoadElemAddress, php_jit_builder_doLoadElem_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doStoreElem,       php_jit_builder_doStoreElem_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doStoreRelative,   php_jit_builder_doStoreRelative_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doJumpTable,       php_jit_builder_doJumpTable_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doPush,            php_jit_builder_unary_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doPushPtr,         php_jit_builder_doPushPtr_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doPop,             php_jit_builder_doPop_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doDeferPop,        php_jit_builder_doPop_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doFlushDeferPop,   php_jit_builder_doPop_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doReturn,          php_jit_builder_unary_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doReturnPtr,       php_jit_builder_doReturnPtr_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doDefaultReturn,   php_jit_no_arginfo,                      ZEND_ACC_PUBLIC)
	PHP_ME(Builder, doGetCallStack,    php_jit_no_arginfo,                      ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
