<?php
/**
* This example is based loosely on ...
*/
$context = jit_context_create();

jit_context_build_start($context);

$signature = jit_type_create_signature(
	JIT_ABI_CDECL, JIT_TYPE_INT, [
	JIT_TYPE_INT
], 1);

$function = jit_function_create($context, $signature);

var_dump($function, $context, $signature);

$n = jit_value_get_param($function, 0);

$zero  = jit_value_create_long_constant($function, JIT_TYPE_INT, 0);
$one   = jit_value_create_long_constant($function, JIT_TYPE_INT, 1);
$two   = jit_value_create_long_constant($function, JIT_TYPE_INT, 2);

$temp1 = jit_insn_eq($function, $n, $zero);
$label1 = jit_insn_branch_if_not($function, $temp1);
jit_insn_return($function, $zero);
jit_insn_label($function, $label1);

$temp2 = jit_insn_eq($function, $n, $one);
$label2 = jit_insn_branch_if_not($function, $temp2);
jit_insn_return($function, $one);
jit_insn_label($function, $label2);

jit_insn_return($function, jit_insn_add($function, jit_insn_call($function, "Fibonacci", $function, $signature, [
	jit_insn_sub($function, $n, $one)	
]), jit_insn_call($function, "Fibonacci", $function, $signature, [
	jit_insn_sub($function, $n, $two)
])));

jit_function_compile($function);
jit_context_build_end($context);

$start = microtime(true);
var_dump(jit_function_apply($function, [40], JIT_TYPE_INT));
printf("jit: %.3f seconds\n", microtime(true)-$start);

function Fibonacci($n)
{
   if ( $n == 0 )
      return 0;
   else if ( $n == 1 )
      return 1;
   else
      return ( Fibonacci($n-1) + Fibonacci($n-2) );
}

$start = microtime(true);
var_dump(Fibonacci(40));
printf("php: %.3f seconds\n", microtime(true)-$start);
?>
