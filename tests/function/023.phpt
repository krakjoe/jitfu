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

$context = new Context();

$long  = new Type(JIT_TYPE_LONG);

/*
long function (long n) {
	long zero = 0;
	long one = 1;
	
	if (n) {
		goto label;
	}
	
	return one;
	
label:
	return zero;
}
*/

$function = new Func($context, new Signature($long, [$long]), function(Value $n) {
	$zero = new Value($this, 0, new Type(JIT_TYPE_LONG));
	$one  = new Value($this, 1, new Type(JIT_TYPE_LONG));

	$label = $this->doBranchIf($n);
	
	$this->doReturn($one);
	
	$this->doLabel($label);
	
	$this->doReturn($zero);
});

var_dump(
	$function(1),
	$function(0));
?>
--EXPECT--
int(0)
int(1)

