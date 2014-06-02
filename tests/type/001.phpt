--TEST--
Test type object
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Type;

$int = new Type(JIT_TYPE_INT);

/* check ids are set */
var_dump($int->getIdentifier() == JIT_TYPE_INT);

$ptr = new Type($int, true);

/* check indirection is set and id is inherited */
var_dump($ptr->getIdentifier() == JIT_TYPE_INT,
		 $ptr->getIndirection() == 1);
		 
$ptrptr = new Type($ptr, true);

/* check indirection increases and id is still set */
var_dump($ptrptr->getIdentifier() == JIT_TYPE_INT,
		 $ptrptr->getIndirection() == 2);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

