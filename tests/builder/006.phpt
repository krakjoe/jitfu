--TEST--
Test builder pow
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

$long   = new Type(JIT_TYPE_LONG);

/* long function(long x, long y); */
$sig      = new Signature($long, [$long, $long]);

$function = new Func($context, $sig);

$x       = $function->getParameter(0);
$y       = $function->getParameter(1);

$builder = new Builder($function);

/* return x ** y; */
$builder->doReturn(
	$builder->doPow($x, $y));

var_dump(
	$function(20, 2),
	$function(30, 2),
	$function(40, 3));
?>
--EXPECT--
int(400)
int(900)
int(64000)
