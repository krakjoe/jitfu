--TEST--
Test builder rem
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Context;
use JITFu\Type;
use JITFu\Signature;
use JITFu\Func;
use JITFu\Builder;

$context = new Context();

$long = new Type(JIT_TYPE_LONG);

/* long function(long x, long y); */
$sig      = new Signature($long, [$long, $long]);

$function = new Func($context, $sig);

$x       = $function->getParameter(0);
$y       = $function->getParameter(1);

$builder = new Builder($function);

/* return x % y; */
$builder->doReturn(
	$builder->doRem($x, $y));

var_dump(
	$function(20, 3),
	$function(30, 4),
	$function(40, 3));
?>
--EXPECT--
int(2)
int(2)
int(1)
