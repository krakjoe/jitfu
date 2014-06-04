--TEST--
Test builder load element strings
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

$string  = new Type(JIT_TYPE_STRING);
$strings = new Type($string, true);

$signature = new Signature($string, [$strings, $string]);
$function = new Func($context, $signature);

/*
string function (string *n, long f) {
	return n[f];
}
*/

$n = $function->getParameter(0);
$f = $function->getParameter(1);

$builder  = new Builder($function);

$builder->doReturn(
	$builder->doLoadElem($n, $f, $strings));

$context->finish();

$function->compile();

$strings = [
	"Hello",
	"World",
	"PHP",
	"Rocks"
];

var_dump(
	$function($strings, 0),  /* should return Hello */
	$function($strings, 1),  /* should return World */ 
	$function($strings, 2),  /* should return PHP */
	$function($strings, 3)); /* should return Rocks */ 
?>
--EXPECT--
string(5) "Hello"
string(5) "World"
string(3) "PHP"
string(5) "Rocks"
