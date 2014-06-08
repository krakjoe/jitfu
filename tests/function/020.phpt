--TEST--
Test load element from ptrptr
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

$long  = Type::of(Type::long);
$longs = new Type($long, true);
$llongs = new Type($longs, true);

/*
long function (long **n, long f, long x) {
	return n[f][x];
}
*/

$function = new Func($context, new Signature($long, [$llongs, $long, $long]), function($args) {
	$this->doReturn
		($this->doLoadElem
			($this->doLoadElem
				($args[0], $args[1]), $args[2]));	
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
int(2)
int(1)
int(0)

