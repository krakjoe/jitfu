--TEST--
Test builder shr
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

$long = new Type(JIT_TYPE_LONG);

/* long function(long x, long y); */
$function = new Func($context, new Signature($long, [$long, $long]));
$function->implement(function($args) {
	/* return $x >> $y; */
	$this->doReturn(
		$this->doShr($args[0], $args[1]));
});
var_dump(
	$function(100, 1),
	$function(100, 2),
	$function(100, 3));
?>
--EXPECT--
int(50)
int(25)
int(12)
