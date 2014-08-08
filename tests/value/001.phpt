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

$long = new Type(Type::long);
$func = new Func(new Context(), new Signature($long, []));
$value = new Value($func, '0', $long);

$value->dump();
?>
--EXPECT--
0
