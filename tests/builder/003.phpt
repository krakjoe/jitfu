--TEST--
Test builder mul
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

$int      = new Type(JIT_TYPE_INT);

/* int function(int x, int y); */
$sig      = new Signature($int, [$int, $int]);

$function = new Func($context, $sig);

$x       = $function->getParameter(0);
$y       = $function->getParameter(1);

$builder = new Builder($function);

/* return x * y; */
$builder->doReturn(
	$builder->doMul($x, $y));

$function->compile();

var_dump(
	$function(1, 2), 
	$function(2, 3), 
	$function(3, 4));
?>
--EXPECT--
int(2)
int(6)
int(12)
