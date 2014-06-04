--TEST--
Test builder load element from ptrptr
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;
use JITFU\Value;
use JITFU\Builder;

$context = new Context();

$context->start();

$long  = new Type(JIT_TYPE_LONG);
$longs = new Type($long, true);
$llongs = new Type($longs, true);

$signature = new Signature($long, [$llongs, $long, $long]);
$function = new Func($context, $signature);

/*
long function (long **n, long f, long x) {
	return n[f][x];
}
*/

$n = $function->getParameter(0);
$f = $function->getParameter(1);
$x = $function->getParameter(2);

$builder  = new Builder($function);

$base = 
	$builder
		->doLoadElem($n, $f, $llongs);
$y = $builder->doLoadElem($base, $x, $longs);

$builder
	->doReturn($y);

$longs = [
	[0, 1, 2],
	[0, 1, 2], 
	[0, 1, 2]];

var_dump(
	$function($longs, 0, 2),
	$function($longs, 1, 1),
	$function($longs, 2, 0));
?>
--EXPECT--
int(2)
int(1)
int(0)

