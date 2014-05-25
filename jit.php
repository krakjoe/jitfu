<?php
$context = jit_context_create();

jit_context_build_start($context);

$union = jit_type_create_union([
	JIT_TYPE_VOID_PTR,
	JIT_TYPE_INT
], 0);

$struct = jit_type_create_struct([
	JIT_TYPE_INT,
	JIT_TYPE_INT,
	$union
], 0);

$ptr = jit_type_create_pointer($struct);

$sig = jit_type_create_signature(JIT_ABI_CDECL, JIT_TYPE_INT, [$union, $struct, $ptr], 0);

$function = jit_function_create($context, $sig);

var_dump($context, $struct, $union, $sig, $function);

jit_context_build_end($context);

jit_context_destroy($context);
?>
