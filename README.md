JIT-Fu
======
*A lesson in the art of JoeJITFu!*

JIT-Fu is a PHP extension that exposes an OO API for the creation of native instructions to PHP userland, using ```libjit```.

![A JoeJITFu elephpant](http://i.imgur.com/GirIOWs.png)

```php
<?php
/* This is a fibonacci function, and is ~60 times faster than PHP :o */
use JITFu\Context;
use JITFu\Type;
use JITFu\Signature;
use JITFu\Func;
use JITFu\Value;
use JITFu\Builder;

$context = new Context();

$context->start();

$integer   = new Type(JIT_TYPE_LONG);
$signature = new Signature
	($integer, [$integer]);

$function = new Func($context, $signature);
$zero     = new Value($function, 0, $integer);
$one      = new Value($function, 1, $integer);
$two      = new Value($function, 2, $integer);
$three    = new Value($function, 3, $integer);

$arg      = $function->getParameter(0);

$builder  = new Builder($function);

/* if ($arg == 0) return 0; */
$builder->doIf(
	$builder->doEq($arg, $zero),
	function($builder) use ($zero) {
		$builder->doReturn($zero);
	}
);

/* if ($arg == 1) return 1; */
$builder->doIf(
	$builder->doEq($arg, $one),
	function($builder) use($one) {
		$builder->doReturn($one);
	}
);

/* return $function($arg-1) + $function($arg-2); */
$builder->doReturn(
	$builder->doAdd(
		$builder->doCall($function, [$builder->doSub($arg, $one)]),
		$builder->doCall($function, [$builder->doSub($arg, $two)])));

$context->finish();

$function->compile();

/* now the function is compiled, it can be passed around like a callable ... */

var_dump(
	$context, 
	$signature,
	$function,
	$function(40)); /* __invoke with magicalness */
?>
```

**This library is not useful for compiling Zend opcodes**
