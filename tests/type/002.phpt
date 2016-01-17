--TEST--
Test type error parameters
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Type;

new Type(null);

?>
--EXPECTF--
Fatal error: Uncaught JITFU\Exception: unexpected parameters, expected (int|Type of [, bool pointer = false]) in %s:%d
Stack trace:
#0 %s(%d): JITFU\Type->__construct(NULL)
#1 {main}
  thrown in %s on line %d

