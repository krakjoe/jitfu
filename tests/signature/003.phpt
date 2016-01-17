--TEST--
Test signature error in return type
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Signature;
use JITFu\Type;

$int = Type::of(Type::int);

new Signature(null, [$int, $int]);
?>
--EXPECTF--
Fatal error: Uncaught JITFU\Exception: invalid parameters, expected (Type returns, Type[] parameters) in %s:%d
Stack trace:
#0 %s(%d): JITFU\Signature->__construct(NULL, Array)
#1 {main}
  thrown in %s on line %d

