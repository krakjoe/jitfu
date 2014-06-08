--TEST--
Test bug found in parameter, constant and return handling
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

$double    = Type::of(Type::double);

$function = new Func($context, new Signature($double, [$double]), function($args) use($double) {
	$two = new Value($this, 29, $double);
	$this->doReturn(
		$this->doMul($args[0], $two));
});

var_dump(
	$function(2), 
	$function(2.2));
?>
--EXPECT--
float(58)
float(63.8)


