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

$x = jit_value_get_param($function, 0);
$y = jit_value_get_param($function, 1);
$z = jit_value_get_param($function, 2);

var_dump($x, $y, $z);

$temp1 = jit_insn_mul($function, $x, $y);
$temp2 = jit_insn_add($function, $temp1, $z);

jit_insn_return($function, $temp2);

jit_function_compile($function);
jit_context_build_end($context);

$start = microtime(true);
for ($i = 0; $i<100000; $i++) {
	jit_function_apply($function, [2, 8, 8]);
}
printf("jit: %.3f seconds\n", microtime(true) - $start);

function mul_add($x, $y, $z) {
	return $x * $y + $z;
}

$start = microtime(true);
for ($i = 0; $i<100000; $i++) {
	mul_add(2, 8, 8);
}
printf("php: %.3f seconds\n", microtime(true) - $start);
?>
