--TEST--
Test builder neg
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

$long = new Type(JIT_TYPE_LONG);

/* long function(long x); */
$sig      = new Signature($long, [$long]);

$function = new Func($context, $sig);

$x       = $function->getParameter(0);
$y       = $function->getParameter(1);

$builder = new Builder($function);

/* return -x; */
$builder->doReturn(
	$builder->doNeg($x));

$function->compile();

var_dump(
	$function(20),
	$function(30),
	$function(40));
?>
--EXPECT--
int(-20)
int(-30)
int(-40)
