--TEST--
Test builder load element longs
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;
use JITFu\Value;
use JITFU\Builder;

$context = new Context();

$long  = new type(JIT_TYPE_LONG);
$longs = new Type($long, true);

$function = new Func($context, new Signature($long, [$longs, $long]));

/*
long function (long *n, long f) {
	return n[f] * f;
}
*/

new Builder($function, function(Value $n, Value $f) {
	$this->doReturn(
		$this->doMul(
			$this->doLoadElem($n, $f), $f));	
});

$numbers = [0, 1, 2, 3, 4, 5];

var_dump(
	$function($numbers, 3),
	$function($numbers, 2),
	$function($numbers, 1));
?>
--EXPECT--
int(9)
int(4)
int(1)
