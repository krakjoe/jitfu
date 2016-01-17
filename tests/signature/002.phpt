--TEST--
Test signature error in type list
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Signature;
use JITFu\Type;

$int = Type::of(Type::int);

new Signature($int, [$int, null]);
?>
--EXPECTF--
Fatal error: Uncaught JITFU\Exception: unexpected type for parameter 1 in %s:%d
Stack trace:
#0 %s(%d): JITFU\Signature->__construct(Object(JITFU\Type), Array)
#1 {main}
  thrown in %s on line %d
