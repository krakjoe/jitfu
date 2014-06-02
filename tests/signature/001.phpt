--TEST--
Test type object
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Signature;
use JITFu\Type;

$int = new Type(JIT_TYPE_INT);

$sig = new Signature($int, [$int]);

var_dump(
	$sig->getReturnType()->getIdentifier() == JIT_TYPE_INT);
var_dump(
	$sig->getParamType(0)->getIdentifier() == JIT_TYPE_INT);
?>
--EXPECT--
bool(true)
bool(true)
