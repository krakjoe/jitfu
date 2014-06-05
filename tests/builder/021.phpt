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

$function = new Func($context, new Signature($long, [$llongs, $long, $long]));

new Builder($function, function(Value $n, Value $f, Value $x) {
	/* long zero = 0; */
	$zero = new Value($this, 0, new Type(JIT_TYPE_LONG));
	/* long one = 1; */
	$one  = new Value($this, 1, new Type(JIT_TYPE_LONG));

	/* long r = n[f][x]; */
	$r = $this->doLoadElem
			($this->doLoadElem
				($n, $f), $x);

	/* do { */
	$this->doWhile(function() use($r, $zero, $one) {	
		/* if (r > zero) { */
		$this->doIf($this->doGt($r, $zero), function() use($r, $zero, $one){
			/* r = (r - one); */
			$this->doStore(
				$r, $this->doSub($r, $one));
		});
		/* } */
	}, $r); /* while (r); */

	/* return r; */
	$this->doReturn($r);		
});

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

