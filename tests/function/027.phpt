--TEST--
Test doSize on strings
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

$function = new Func($context, new Signature(Type::of(Type::long), [Type::of(Type::string)]), function($args) {
	$this->doReturn(
		$this->doSize($args[0]));
});

var_dump(
	$function("hello world"));

$strings = new Type(Type::of(Type::string), true);
$function = new Func($context, new Signature(Type::of(Type::long), [$strings]), function($args) {
	$zero = new Value($this, 0, Type::of(Type::int));
	$first = $this
		->doLoadElem($args[0], $zero);
	$this->doReturn($this->doSize($first));
});

var_dump(
	$function(["hello world"]));
?>
--EXPECT--
int(11)
int(11)

