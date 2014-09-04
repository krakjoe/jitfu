<?php
namespace JITFU {
	class Signature {
		const void = 1;
		const int = 3;
		const uint = 2;
		const ulong = 4;
		const long = 5;
		const double = 6;
		const string = 7;
		const zval = 8;
		public function __construct ($returns, array $types) {}
		public function getReturnType () {}
		public function getParamType ($param) {}
		public function getIdentifier () {}
		public function getIndirection () {}
		public function isPointer () {}
		public function dump ($output) {}
		public static function of ($type) {}
	}
}
