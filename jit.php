<?php
/**
* This example is based loosely on tutorial from libjit documentation
*
* http://www.gnu.org/software/libjit/doc/libjit_3.html#Tutorials
*/
$context = jit_context_create();

jit_context_build_start($context);

$signature = jit_type_create_signature(
	JIT_ABI_CDECL, JIT_TYPE_INT, [
	JIT_TYPE_INT,
	JIT_TYPE_INT,
	JIT_TYPE_INT, 
	JIT_TYPE_INT,
	JIT_TYPE_INT
], 1);

$function = jit_function_create($context, $signature);

var_dump($function, $context, $signature);

$x = jit_value_get_param($function, 0);
$y = jit_value_get_param($function, 1);
$z = jit_value_get_param($function, 2);
$r = jit_value_get_param($function, 3);
$t = jit_value_get_param($function, 4);

var_dump($x, $y, $z, $r, $t);

$temp1 = jit_insn_mul($function, $x, $y);
$temp2 = jit_insn_add($function, $temp1, $z);
$temp3 = jit_insn_sub($function, $temp2, $r);
$temp4 = jit_insn_div($function, $temp3, $t);

jit_insn_return($function, $temp4);

jit_function_compile($function);
jit_context_build_end($context);

$start = microtime(true);
for ($i = 0; $i<100000; $i++) {
	jit_function_apply($function, [200, 800, 800, 500, 200], JIT_TYPE_INT);
}
printf("jit: %.3f seconds\n", microtime(true) - $start);

function mul_add_sub($x, $y, $z, $r, $t) {
	return $x * $y + $z - $r / $t;
}

$start = microtime(true);
for ($i = 0; $i<100000; $i++) {
	mul_add_sub(200, 800, 800, 500, 200);
}
printf("php: %.3f seconds\n", microtime(true) - $start);
?>
