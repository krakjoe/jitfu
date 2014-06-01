<?php
use JIT\Context;
use JIT\Type;
use JIT\Signature;
use JIT\Func;
use JIT\Value;
use JIT\Builder;

$context = new Context();

$context->start();

$signature = new Signature
	(new Type(JIT_TYPE_INT), [new Type(JIT_TYPE_INT)]);

$function = new Func($context, $signature);
$builder  = new Builder($function);
$value    = new Value($function, 20, new Type(JIT_TYPE_INT));

$arg      = $function->getParameter(0);

$builder->doIfEqual($arg, $value, function($builder) use ($arg, $value) {
	$builder->doReturn($arg);
}, function($builder) use($arg, $value) {
	$builder->doReturn($value);
});

$context->finish();

$function->compile();

var_dump(
	$context, 
	$signature, 
	$function, 
	$function->getSignature(), $value, $value->getFunction(), $arg, $builder, $function(10));
?>
