--TEST--
Test doSize on array and reading from arrays
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;

$context = new Context();

$int = new Type(JIT_TYPE_INT);
$long = new Type(JIT_TYPE_LONG);
$longs = new Type($long, true);

$signature = new Signature($int, [$longs]);

$function = new Func($context, $signature, function($args) {
	$this->doReturn(
	    $this->doSize($args[0]));
});

var_dump($function([1, 2, 3, 4, 5])); # 5

$signature = new Signature($long, [$longs, $long]);
$function = new Func($context, $signature, function($args) {
	$this->doReturn(
	    $this->doLoadElem($args[0], $args[1]));
});

var_dump($function([1, 2, 3, 4, 5], 3)); # 4
?>
--EXPECT--
int(5)
int(4)
