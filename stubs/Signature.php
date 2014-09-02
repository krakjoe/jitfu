<?php
namespace JITFU {
	class Signature {
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
