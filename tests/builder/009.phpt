--TEST--
Test builder shl
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

/* return $x << $y; */
$builder->doReturn(
	$builder->doShl($x, $y));

var_dump(
	$function(1, 1),
	$function(1, 2),
	$function(1, 3));
?>
--EXPECT--
int(2)
int(4)
int(8)
