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
use JITFU\Value;
use JITFU\Builder;

$context = new Context();

$context->start();

$long  = new type(JIT_TYPE_LONG);
$longs = new Type($long, true);

$signature = new Signature($long, [$longs, $long]);
$function = new Func($context, $signature);

/*
long function (long *n, long f) {
	return n[f] * f;
}
*/

$n = $function->getParameter(0);
$f = $function->getParameter(1);

$builder  = new Builder($function);

$builder->doReturn(
	$builder->doMul(
		$builder->doLoadElem($n, $f, $longs), $f));

$context->finish();

$function->compile();

$numbers = [0, 1, 2, 3, 4, 5];

var_dump(
	$function($numbers, 3),  /* should return 3 */
	$function($numbers, 2),  /* should return 2 */ 
	$function($numbers, 1)); /* should return 1 */ 
?>
--EXPECT--
int(9)
int(4)
int(1)
