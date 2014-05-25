<?php
$context = jit_context_create();

jit_context_build_start($context);

$signature = jit_type_create_signature(
	JIT_ABI_CDECL, JIT_TYPE_INT, [
	JIT_TYPE_INT,
	JIT_TYPE_INT,
	JIT_TYPE_INT
], 1);

$function = jit_function_create($context, $signature);

var_dump($function, $context, $signature);

$params = [
	jit_value_get_param($function, 0),
	jit_value_get_param($function, 1),
	jit_value_get_param($function, 2)
];

$temp1 = jit_insn_mul($function, $params[0], $params[1]);
$temp2 = jit_insn_add($function, $temp1, $params[2]);

jit_insn_return($function, $temp2);

jit_function_compile($function);
jit_context_build_end($context);
?>
