--TEST--
Test context object
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Context;

$context = new Context();

var_dump($context->isStarted());
var_dump($context->isFinished());

$context->start();

var_dump($context->isStarted());
var_dump($context->isFinished());

$context->finish();

var_dump($context->isFinished());
?>
--EXPECT--
bool(false)
bool(false)
bool(true)
bool(false)
bool(true)

