<?php
namespace JITFU {
	class Type {
		public function __construct ($type) {}
		public function getIdentifier () {}
		public function getIndirection () {}
		public function isPointer () {}
		public function dump ($output) {}
		public static function of ($type) {}
	}
}
