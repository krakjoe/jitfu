--TEST--
Test builder div
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFu\Context;
use JITFu\Type;
use JITFu\Signature;
use JITFu\Func;
use JITFu\Value;
use JITFu\Builder;

$context = new Context();

$double   = new Type(JIT_TYPE_DOUBLE);

/* double function(double x, double y); */
$function = new Func($context, new Signature($double, [$double, $double]));

new Builder($function, function(Value $x, Value $y) {
	/* return x / y; */
	$this->doReturn(
		$this->doDiv($x, $y));
});

var_dump(
	$function(20, 2),
	$function(30, 2),
	$function(40, 3));
?>
--EXPECT--
float(10)
float(15)
float(13.333333333333)
