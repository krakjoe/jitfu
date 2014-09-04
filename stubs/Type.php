<?php
namespace JITFU {
	class Type {
		const void = 1;
		const int = 3;
		const uint = 2;
		const ulong = 4;
		const long = 5;
		const double = 6;
		const string = 7;
		const zval = 8;
		public function __construct ($type) {}
		public function getIdentifier () {}
		public function getIndirection () {}
		public function isPointer () {}
		public function dump ($output) {}
		public static function of ($type) {}
	}
}
