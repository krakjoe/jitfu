--TEST--
Test builder doBranchIf
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

/*
long function (long n) {
	long zero = 0;
	long one = 1;
	
	if (n) {
		return one;
	}
	
	return zero;
}
*/

$function = new Func($context, new Signature($long, [$long]));

new Builder($function, function(Value $n) {
	/* long zero = 0; */
	$zero = new Value($this, 0, new Type(JIT_TYPE_LONG));
	/* long one = 1; */
	$one  = new Value($this, 1, new Type(JIT_TYPE_LONG));

	/* long r = n[f][x]; */
	$not = $this->doBranchIfNot($n);
	
	$this->doReturn($one);
	
	$this->doLabel($not);
	
	/* return r; */
	$this->doReturn($zero);
});

var_dump(
	$function(1),
	$function(0));
?>
--EXPECT--
int(1)
int(0)

