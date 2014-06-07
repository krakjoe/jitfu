--TEST--
Test store relative/load relative
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;
use JITFU\Value;

$context = new Context();

$long  = Type::of(Type::long);
$longs = new Type($long, true);

$function = new Func($context, new Signature($long, [$longs, $longs]), function($args) {
	$first = new Value($this, 1, Type::of(Type::int));
	$v = $this->doLoadElem($args[0], $first);
	$this
		->doStoreElem($args[1], $first, $v);
	$v = $this->doLoadElem($args[1], $first);
	$this->doReturn($v);
});

$longs = [
	[1, 1020, 1], /* #1 will be copied */
	[2, 200, 2]   /* #1 will be set */
];

var_dump(
	$function($longs[0], $longs[1]));
?>
--EXPECT--
int(1020)

