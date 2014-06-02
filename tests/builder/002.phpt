--TEST--
Test builder add
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

/* return x + y; */
$builder->doReturn(
	$builder->doAdd($x, $y));

$function->compile();

var_dump(
	$function(10, 20), 
	$function(20, 30), 
	$function(30, 40));
?>
--EXPECT--
int(30)
int(50)
int(70)
