--TEST--
Test builder eq
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
$function = new Func($context, new Signature($long, [$long, $long]), function(Value $x, Value $y) {
	/* return $x == $y; */
	$this->doReturn(
		$this->doEq($x, $y));
});

var_dump(
	(bool) $function(1, 1),
	(bool) $function(1, 2));
?>
--EXPECT--
bool(true)
bool(false)
