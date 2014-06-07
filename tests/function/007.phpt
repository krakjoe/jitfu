--TEST--
Test builder rem
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
	/* return x % y; */
	$this->doReturn(
		$this->doRem($args[0], $args[1]));
});

var_dump(
	$function(20, 3),
	$function(30, 4),
	$function(40, 3));
?>
--EXPECT--
int(2)
int(2)
int(1)
