--TEST--
Test builder div
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
$context->start();

$double   = new Type(JIT_TYPE_DOUBLE);

/* double function(double x, double y); */
$sig      = new Signature($double, [$double, $double]);

$function = new Func($context, $sig);

$x       = $function->getParameter(0);
$y       = $function->getParameter(1);

$builder = new Builder($function);

/* return x - y; */
$builder->doReturn(
	$builder->doDiv($x, $y));

$function->compile();

var_dump(
	$function(20, 2),
	$function(30, 2),
	$function(40, 3));
?>
--EXPECT--
float(10)
float(15)
float(13.333333333333)
