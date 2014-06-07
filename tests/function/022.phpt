--TEST--
Test builder doBranchIfNot
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;
use JITFU\Value;

$context = new Context();

$long  = new Type(JIT_TYPE_LONG);

/*
long function (long n) {
	long zero = 0;
	long one = 1;
	
	if (!n) {
		goto false;
	}
	
	return one;
	
false:
	return zero;
}
*/

$function = new Func($context, new Signature($long, [$long]));

$function->implement(function($args) {
	$zero = new Value($this, 0, new Type(JIT_TYPE_LONG));
	$one  = new Value($this, 1, new Type(JIT_TYPE_LONG));

	$false = $this->doBranchIfNot($args[0]);
	
	$this->doReturn($one);
	
	$this->doLabel($false);
	
	$this->doReturn($zero);
});
var_dump(
	$function(1),
	$function(0));
?>
--EXPECT--
int(1)
int(0)

