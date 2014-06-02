--TEST--
Test function object
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Context;
use JITFu\Type;
use JITFu\Signature;
use JITFu\Func;

$context = new Context();
$context->start();

$int      = new Type(JIT_TYPE_INT);

/* int function(int); */
$sig      = new Signature($int, [$int]);

$function = new Func($context, $sig);

var_dump($function->isCompiled());

$function->compile();

var_dump(
	$function->isCompiled(), 
	is_callable($function));
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
