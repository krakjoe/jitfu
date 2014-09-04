<?php
namespace JITFU {
	class Value {
		public function __construct ($function, $value, $type = null) {}
		public function isTemporary () {}
		public function isLocal () {}
		public function isConstant () {}
		public function isParameter () {}
		public function isVolatile () {}
		public function isAddressable () {}
		public function isTrue () {}
		public function setVolatile () {}
		public function setAddressable () {}
		public function getType () {}
		public function getFunction () {}
		public function dump ($output = null, $prefix = null) {}
	}
}
