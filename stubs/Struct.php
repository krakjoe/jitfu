<?php
namespace JITFU {
	class Struct {
		public function __construct ($fields) {}
		public function getIdentifier () {}
		public function getIndirection () {}
		public function isPointer () {}
		public function getOffset ($field) {}
		public function getFieldType ($field) {}
		public function dump ($output) {}
		public static function of ($type) {}
	}
}
