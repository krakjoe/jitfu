<?php
/**
* This example compiles a fibonacci function, and is extremely fast !
**/
$context = jit_context_create();

jit_context_build_start($context);

$signature = jit_type_create_signature(
	JIT_TYPE_INT, [
	JIT_TYPE_INT
]);

$function = jit_function_create($context, $signature);

$n = jit_value_get_param($function, 0);

$zero  = jit_value_create_constant($function,  0, JIT_TYPE_INT);
$one   = jit_value_create_constant($function,  1, JIT_TYPE_INT);
$two   = jit_value_create_constant($function,  2, JIT_TYPE_INT);

$temp1 = jit_insn_eq($function, $n, $zero);
$label1 = jit_insn_branch_if_not($function, $temp1);
jit_insn_return($function, $zero);
jit_insn_label($function, $label1);

$temp2 = jit_insn_eq($function, $n, $one);
$label2 = jit_insn_branch_if_not($function, $temp2);
jit_insn_return($function, $one);
jit_insn_label($function, $label2);

jit_insn_return($function, jit_insn_add($function, jit_insn_call($function, "jit-fibonacci", $function, $signature, [
	jit_insn_sub($function, $n, $one)	
]), jit_insn_call($function, "jit-fibonacci", $function, $signature, [
	jit_insn_sub($function, $n, $two)
])));

jit_function_compile($function);
jit_context_build_end($context);

var_dump(
	jit_function_apply($function, [40], JIT_TYPE_INT));
?>
