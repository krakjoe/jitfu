--TEST--
Test builder gt
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

/* long function(long x, long y); */
$function = new Func($context, new Signature($long, [$long, $long]), function($args) {
	/* return $x > $y; */
	$this->doReturn(
		$this->doGt($args[0], $args[1]));
});

var_dump(
	(bool) $function(1, 1),
	(bool) $function(1, 2),
	(bool) $function(3, 1));
?>
--EXPECT--
bool(false)
bool(false)
bool(true)
