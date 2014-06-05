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
use JITFu\Value;
use JITFu\Builder;

$context = new Context();

$long = new Type(JIT_TYPE_LONG);

/* long function(long x); */
$function = new Func($context, new Signature($long, [$long]));

new Builder($function, function(Value $x) {
	/* return -x; */
	$this->doReturn(
		$this->doNeg($x));
});

var_dump(
	$function(20),
	$function(30),
	$function(40));
?>
--EXPECT--
int(-20)
int(-30)
int(-40)
