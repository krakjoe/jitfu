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
use JITFu\Value;

$context = new Context();

$int      = new Type(JIT_TYPE_LONG);

/* int function(int x, int y); */
$function = new Func($context, new Signature($int, [$int, $int]), function($args) {
	/* return x * y; */
	$this->doReturn(
		$this->doMul($args[0], $args[1]));
});

var_dump(
	$function(1, 2), 
	$function(2, 3), 
	$function(3, 4));
?>
--EXPECT--
int(2)
int(6)
int(12)
