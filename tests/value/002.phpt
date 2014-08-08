--TEST--
Test value conversion string -> long
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Func;
use JITFu\Value;
use JITFu\Type;
use JITFu\Context;
use JITFu\Signature;

$double = new Type(Type::double);
$func = new Func(new Context(), new Signature($double, []));
$value = new Value($func, '1.1', $double);

$value->dump();
?>
--EXPECT--
1.100000
