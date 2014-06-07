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
use JITFu\Value;

$context = new Context();

$int      = new Type(JIT_TYPE_LONG);

/* int function(int x, int y); */
$function = new Func($context, new Signature($int, [$int, $int]), function($args) {
	/* return x + y; */
	$this->doReturn(
		$this->doAdd($args[0], $args[1]));
});

var_dump(
	$function(10, 20), 
	$function(20, 30), 
	$function(30, 40));
?>
--EXPECT--
int(30)
int(50)
int(70)
