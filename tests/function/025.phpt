--TEST--
Test reserving labels
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

$test = new Func($context, new Signature($long, [$long]), function($args){
	$five = new Value($this, 5, Type::of(Type::long));
	
	$res = $this->reserveLabel();
	
	$this->doBranchIfNot($args[0], $res);
	
	$this->doReturn(
		$this->doMul($args[0], $args[0]));
	
	$this->doLabel($res);
		$this->doReturn($five);
});

var_dump(
	$test(10),
	$test(0));
?>
--EXPECT--
int(100)
int(5)

