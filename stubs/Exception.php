<?php
namespace JITFU {
	class Exception {
		final private function __clone () {}
		public function __construct ($message, $code, $previous) {}
		final public function getMessage () {}
		final public function getCode () {}
		final public function getFile () {}
		final public function getLine () {}
		final public function getTrace () {}
		final public function getPrevious () {}
		final public function getTraceAsString () {}
		public function __toString () {}
	}
}
