--TEST--
Test creating contexts
--SKIPIF--
<?php include ("../skip-if.inc"); ?>
--FILE--
<?php
$context = jit_context_create();

var_dump($context);

jit_context_destroy($context);

var_dump($context);
?>
--EXPECTF--
resource(4) of type (jit context)
resource(4) of type (Unknown)
