<?php
/**
* This example is based loosely on tutorial from libjit source
*
* http://git.savannah.gnu.org/cgit/libjit.git/tree/tutorial/t2.c
*/
$context = jit_context_create();

jit_context_build_start($context);

$signature = jit_type_create_signature(
	JIT_ABI_CDECL, JIT_TYPE_INT, [
	JIT_TYPE_INT,
	JIT_TYPE_INT
], 1);

$function = jit_function_create($context, $signature);

var_dump($function, $context, $signature);

$x = jit_value_get_param($function, 0);
$y = jit_value_get_param($function, 1);

var_dump($x, $y);

$temp1 = jit_insn_eq($function, $x, $y);
$label1 = jit_insn_branch_if_not($function, $temp1);
jit_insn_return($function, $x);
jit_insn_label($function, $label1);
$temp2 = jit_insn_lt($function, $x, $y);
$label2 = jit_insn_branch_if_not($function, $temp2);

$temp3 = jit_insn_call($function, "gcd", $function, $signature, [
	$x, jit_insn_sub($function, $y, $x)
]);
jit_insn_return($function, $temp3);
jit_insn_label($function, $label2);
$temp4 = jit_insn_call($function, "gcd", $function, $signature, [
	jit_insn_sub($function, $x, $y), $y
]);
jit_insn_return($function, $temp4);

jit_function_compile($function);
jit_context_build_end($context);

$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	jit_function_apply($function, [40, 500], JIT_TYPE_INT);	
}
printf("jit: %.3f seconds\n", microtime(true)-$start);

function gcd($x, $y)
{
    if($x == $y)
    {
        return $x;
    }
    else if($x < $y)
    {
        return gcd($x, $y - $x);
    }
    else
    {
        return gcd($x - $y, $y);
    }
}

$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	gcd(40, 500);
}
printf("php: %.3f seconds\n", microtime(true)-$start);
?>
