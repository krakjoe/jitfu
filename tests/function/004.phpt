--TEST--
Test sub
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
$function = new Func($context, new Signature($int, [$int, $int]), function($args){
	/* return x - y; */
	$this->doReturn(
		$this->doSub($args[0], $args[1]));
});

var_dump(
	$function(2, 1), 
	$function(3, 2), 
	$function(4, 3));
?>
--EXPECT--
int(1)
int(1)
int(1)
