--TEST--
Test builder sub
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

$int      = new Type(JIT_TYPE_LONG);

/* int function(int x, int y); */
$sig      = new Signature($int, [$int, $int]);

$function = new Func($context, $sig);

$x       = $function->getParameter(0);
$y       = $function->getParameter(1);

$builder = new Builder($function);

/* return x - y; */
$builder->doReturn(
	$builder->doSub($x, $y));

$function->compile();

var_dump(
	$function(2, 1), 
	$function(3, 2), 
	$function(4, 3));
?>
--EXPECT--
int(1)
int(1)
int(1)
