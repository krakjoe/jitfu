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

/* write disassembly to stdout */

$function->dump("Fibonacci");

/* now the function is compiled, it can be passed around like a callable ... */

var_dump($function(40)); /* __invoke with magicalness */
?>
```

The code above will yield the following output:

```
[joe@localhost jit]$ time php -dextension=jitfu.so objects.php 
function Fibonacci(long) : long

/tmp/libjit-dump.o:     file format elf64-x86-64


Disassembly of section .text:

00007ff108046144 <.text>:
    7ff108046144:       55                      push   %rbp
    7ff108046145:       48 8b ec                mov    %rsp,%rbp
    7ff108046148:       48 83 ec 20             sub    $0x20,%rsp
    7ff10804614c:       4c 89 3c 24             mov    %r15,(%rsp)
    7ff108046150:       4c 8b ff                mov    %rdi,%r15
    7ff108046153:       33 c0                   xor    %eax,%eax
    7ff108046155:       4c 3b f8                cmp    %rax,%r15
    7ff108046158:       0f 85 07 00 00 00       jne    0x7ff108046165
    7ff10804615e:       33 c0                   xor    %eax,%eax
    7ff108046160:       e9 5b 00 00 00          jmpq   0x7ff1080461c0
    7ff108046165:       b8 01 00 00 00          mov    $0x1,%eax
    7ff10804616a:       4c 3b f8                cmp    %rax,%r15
    7ff10804616d:       0f 85 0a 00 00 00       jne    0x7ff10804617d
    7ff108046173:       b8 01 00 00 00          mov    $0x1,%eax
    7ff108046178:       e9 43 00 00 00          jmpq   0x7ff1080461c0
    7ff10804617d:       49 8b c7                mov    %r15,%rax
    7ff108046180:       b9 01 00 00 00          mov    $0x1,%ecx
    7ff108046185:       48 2b c1                sub    %rcx,%rax
    7ff108046188:       48 8b f8                mov    %rax,%rdi
    7ff10804618b:       48 89 45 f8             mov    %rax,-0x8(%rbp)
    7ff10804618f:       b8 08 00 00 00          mov    $0x8,%eax
    7ff108046194:       e8 67 ff ff ff          callq  0x7ff108046100
    7ff108046199:       48 89 45 f0             mov    %rax,-0x10(%rbp)
    7ff10804619d:       49 8b c7                mov    %r15,%rax
    7ff1080461a0:       b9 02 00 00 00          mov    $0x2,%ecx
    7ff1080461a5:       48 2b c1                sub    %rcx,%rax
    7ff1080461a8:       48 8b f8                mov    %rax,%rdi
    7ff1080461ab:       48 89 45 e8             mov    %rax,-0x18(%rbp)
    7ff1080461af:       b8 08 00 00 00          mov    $0x8,%eax
    7ff1080461b4:       e8 47 ff ff ff          callq  0x7ff108046100
    7ff1080461b9:       48 8b 4d f0             mov    -0x10(%rbp),%rcx
    7ff1080461bd:       48 03 c1                add    %rcx,%rax
    7ff1080461c0:       4c 8b 3c 24             mov    (%rsp),%r15
    7ff1080461c4:       48 8b e5                mov    %rbp,%rsp
    7ff1080461c7:       5d                      pop    %rbp
    7ff1080461c8:       c3                      retq   

end

int(102334155)


real    0m1.092s
user    0m1.078s
sys     0m0.011s
```

**This library is not useful for compiling Zend opcodes**

TODO
====

  * integration tests for as much as possible
  * builder methods for all common control structures
  * develop struct/union API
  * expose ELF read/write api, somehow
  * bother with windows ... anyone ?
  * other cool things, probably
  * pecl & phpdoc
