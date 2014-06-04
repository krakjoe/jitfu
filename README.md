JIT-Fu
======
*A lesson in the art of JoeJITFu!*

JIT-Fu is a PHP extension that exposes an OO API for the creation of native instructions to PHP userland, using ```libjit```.

![A JoeJITFu elephpant](http://i.imgur.com/GirIOWs.png)

*Disclaimer: the elephpant above is an artists rendition, the actual animal may or may not exist ...*

Fibonacci
=========
*Because apparently, that's super important ...*

```php
<?php
use JITFU\Context;
use JITFU\Type;
use JITFU\Struct;
use JITFU\Signature;
use JITFU\Func;
use JITFU\Value;
use JITFU\Builder;

$context = new Context();

$integer   = new Type(JIT_TYPE_INT);
$signature = new Signature
	($integer, [$integer]);

$function = new Func($context, $signature);
$zero     = new Value($function, 0, $integer);
$one      = new Value($function, 1, $integer);
$two      = new Value($function, 2, $integer);

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

$function->dump("Fibonacci");

var_dump($function(40)); /* __invoke with magicalness */
?>
```

The code above will yield something like the following output:

```
[joe@localhost jit]$ time php -dextension=jitfu.so objects.php 
function Fibonacci(i1 : int) : int
        incoming_reg(i1, rdi)
.L:
        i6 = i1 == 0
        if i1 != 0 then goto .L0
.L:
        return_int(0)
        ends_in_dead
.L0:
        i7 = i1 == 1
        if i1 != 1 then goto .L1
.L:
        return_int(1)
        ends_in_dead
.L1:
        i8 = i1 - 1
        outgoing_reg(i8, rdi)
        call 0x08215dfd0
.L:
        return_reg(i10, rax)
        i12 = i1 - 2
        outgoing_reg(i12, rdi)
        call 0x08215dfd0
.L:
        return_reg(i14, rax)
        i15 = i10 + i14
        return_int(i15)
        ends_in_dead
.L:
.L:
end

int(102334155)


real    0m1.001s
user    0m0.997s
sys     0m0.003s
```

On *my machine*, this is ~60 times faster than PHP.

**This library is not useful for compiling Zend opcodes**

TODO
====

  * integration tests for as much as possible
  * builder methods for all common control structures
  * expose ELF read/write api, somehow
  * bother with windows ... anyone ?
  * other cool things, probably
  * pecl & phpdoc
