--TEST--
Test load element strings
--SKIPIF--
<?php if (!extension_loaded("jitfu")) die("skip JITFu not loaded"); ?>
--FILE--
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Signature;
use JITFU\Func;
use JITFu\Value;

$context = new Context();

$string  = Type::of(Type::string);
$strings = new Type($string, true);
$long    = Type::of(Type::int);

/*
string function (string *n, long f) {
	return n[f];
}
*/

$function = new Func($context, new Signature($string, [$strings, $long]), function($args) {
	$this->doReturn(
		$this->doLoadElem($args[0], $args[1]));
});

$data = [
	"Hello",
	"World",
	"PHP",
	"Rocks"
];

var_dump(
	$function($data, 0),
	$function($data, 1),
	$function($data, 2),
	$function($data, 3));
?>
--EXPECT--
string(5) "Hello"
string(5) "World"
string(3) "PHP"
string(5) "Rocks"
