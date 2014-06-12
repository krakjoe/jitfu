--TEST--
Test doEcho generator
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;

$context = new Context();

$signature = new Signature(Type::of(Type::void), [Type::of(Type::string)]);

$function = new Func($context, $signature, function($args) {
	$this->doEcho($args[0]);
});

$function("hello world");
?>
--EXPECT--
hello world


