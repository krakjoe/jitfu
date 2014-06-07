--TEST--
Test type object
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Type;

$int = new Type(JIT_TYPE_LONG);

/* check ids are set */
var_dump($int->getIdentifier() == JIT_TYPE_LONG);

$ptr = new Type($int, true);

/* check indirection is set and id is inherited */
var_dump($ptr->getIdentifier() == JIT_TYPE_LONG,
		 $ptr->getIndirection() == 1);

$ptrptr = new Type($ptr, true);

/* check indirection increases and id is still set */
var_dump($ptrptr->getIdentifier() == JIT_TYPE_LONG,
		 $ptrptr->getIndirection() == 2);

/* check class constants match global constants */		 
var_dump(
	JIT_TYPE_LONG == Type::long,
	JIT_TYPE_ULONG == Type::ulong,
	JIT_TYPE_INT == Type::int,
	JIT_TYPE_UINT == Type::uint,
	JIT_TYPE_STRING == Type::string,
	JIT_TYPE_VOID_PTR == Type::pvoid,
	JIT_TYPE_VOID == Type::void,
	JIT_TYPE_DOUBLE == Type::double);

/* check type cache is working */
$cache = [Type::of(Type::long), Type::of(Type::long)];

var_dump($cache[0] === $cache[1]);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
