--TEST--
Test calling other jit'd functions
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

$mul = new Func($context, new Signature($long, [$long]), function($args){
	$this->doReturn(
		$this->doMul($args[0], $args[0]));
});

$function = new Func($context, new Signature($long, [$long]), function($args) use ($mul) {
	$this->doReturn(
		$this->doCall($mul, $args));
});

var_dump(
	$function(10));
?>
--EXPECT--
int(100)
