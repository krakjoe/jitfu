--TEST--
Test builder doWhile
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

$long  = new Type(JIT_TYPE_LONG);
$longs = new Type($long, true);
$llongs = new Type($longs, true);

$signature = new Signature($long, [$llongs, $long, $long]);
$function = new Func($context, $signature);
$builder = new Builder($function);

/*
long function (long **n, long f, long x) {
	long zero = 0;
	long one = 1;
	long r = n[f][x];

	do {
		if (r > zero) {
			r = (r - 1);
		}
	} while (r);
	
	return r;
}
*/

/* body starts here */
$n = $function->getParameter(0);
$f = $function->getParameter(1);
$x = $function->getParameter(2);

/* long zero = 0; */
$zero = new Value($function, 0, new Type(JIT_TYPE_LONG));
/* long one = 1; */
$one  = new Value($function, 1, new Type(JIT_TYPE_LONG));
/* long r = n[f][x]; */
$r = $builder->doLoadElem
		($builder->doLoadElem
			($n, $f, $llongs), $x, $longs);

/* do { */
$builder->doWhile(function($builder) use($r, $zero, $one) {	
	/* if (r > zero) { */
	$builder->doIf($builder->doGt($r, $zero), function($builder) use($r, $zero, $one){
		/* r = (r - one); */
		$builder->doStore(
			$r, $builder->doSub($r, $one));
	});
	/* } */
}, $r); /* while (r); */

/* return r; */
$builder->doReturn($r);

$longs = [
	[0, 1, 2],
	[0, 1, 2], 
	[0, 1, 2]];

var_dump(
	$function($longs, 0, 2),
	$function($longs, 1, 1),
	$function($longs, 2, 0));
?>
--EXPECT--
int(0)
int(0)
int(0)

