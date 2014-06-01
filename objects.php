<?php
use JIT\Context;
use JIT\Type;
use JIT\Signature;
use JIT\Func;
use JIT\Value;
use JIT\Builder;

$context = new Context();

$context->start();

$integer   = new Type(JIT_TYPE_LONG);
$signature = new Signature
	($integer, [$integer]);

$function = new Func($context, $signature);
$zero     = new Value($function, 0, $integer);
$one      = new Value($function, 1, $integer);
$two      = new Value($function, 2, $integer);
$three    = new Value($function, 3, $integer);

$arg      = $function->getParameter(0);


$builder  = new Builder($function);

/* if ($arg == 0) return 0; */
$builder->doIf(
	$builder->doEq($arg, $zero),
	function($builder) use ($zero) {
		$builder->doReturn($zero);
	}
);

/* if ($arg == 1) return 1; */
$builder->doIf(
	$builder->doEq($arg, $one),
	function($builder) use($one) {
		$builder->doReturn($one);
	}
);

/* return $function($arg-1) + $function($arg-2); */
$builder->doReturn(
	$builder->doAdd(
		$builder->doCall($function, [$builder->doSub($arg, $one)], 0),
		$builder->doCall($function, [$builder->doSub($arg, $two)], 0)));

$context->finish();

$function->compile();

var_dump(
	$context, 
	$signature,
	$function,
	$function(40)); /* __invoke with magicalness */
?>
