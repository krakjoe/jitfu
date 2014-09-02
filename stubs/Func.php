<?php
namespace JITFU {
	class Func {
		public function __construct ($context, $signature, $builder, $parent) {}
		public function implement ($builder) {}
		public function setAsync ($async) {}
		public function isImplemented () {}
		public function compile () {}
		public function isCompiled () {}
		public function isNested () {}
		public function getParent () {}
		public function getContext () {}
		public function getSignature () {}
		public function dump () {}
		public function reserveLabel () {}
		public function __invoke () {}
		public function doLabel ($label) {}
		public function doBranch ($label) {}
		public function doBranchIf ($op, $label) {}
		public function doBranchIfNot ($op, $label) {}
		public function doIf ($op, $positive, $negative) {}
		public function doIfNot ($op, $positive, $negative) {}
		public function doWhile ($condition, $block) {}
		public function doMul ($op1, $op2) {}
		public function doMulOvf ($op1, $op2) {}
		public function doAdd ($op1, $op2) {}
		public function doAddOvf ($op1, $op2) {}
		public function doSub ($op1, $op2) {}
		public function doSubOvf ($op1, $op2) {}
		public function doDiv ($op1, $op2) {}
		public function doPow ($op1, $op2) {}
		public function doRem ($op1, $op2) {}
		public function doRemIEEE ($op1, $op2) {}
		public function doNeg ($op) {}
		public function doAnd ($op) {}
		public function doOr ($op) {}
		public function doXor ($op) {}
		public function doShl ($op) {}
		public function doShr ($op) {}
		public function doUshr ($op) {}
		public function doSshr ($op) {}
		public function doEq ($op1, $op2) {}
		public function doNe ($op1, $op2) {}
		public function doLt ($op1, $op2) {}
		public function doLe ($op1, $op2) {}
		public function doGt ($op1, $op2) {}
		public function doGe ($op1, $op2) {}
		public function doCmpl ($op1, $op2) {}
		public function doCmpg ($op1, $op2) {}
		public function doToBool ($op) {}
		public function doToNotBool ($op) {}
		public function doAcos ($op) {}
		public function doAsin ($op) {}
		public function doAtan ($op) {}
		public function doAtan2 ($op1, $op2) {}
		public function doMin ($op1, $op2) {}
		public function doMax ($op1, $op2) {}
		public function doCeil ($op) {}
		public function doCos ($op) {}
		public function doCosh ($op) {}
		public function doExp ($op) {}
		public function doFloor ($op) {}
		public function doLog ($op) {}
		public function doLog10 ($op) {}
		public function doRint ($op) {}
		public function doRound ($op) {}
		public function doSin ($op) {}
		public function doSinh ($op) {}
		public function doSqrt ($op) {}
		public function doTan ($op) {}
		public function doTanh ($op) {}
		public function doAbs ($op) {}
		public function doSign ($op) {}
		public function doIsNAN ($op) {}
		public function doIsFinite ($op) {}
		public function doIsInf ($op) {}
		public function doCall ($function, $params, $flags) {}
		public function doAlloca ($op) {}
		public function doLoad ($op) {}
		public function doLoadSmall ($op) {}
		public function doDup ($op) {}
		public function doStore ($op) {}
		public function doAddressof ($op) {}
		public function doCheckNull ($op) {}
		public function doMemcpy ($dest, $src, $size) {}
		public function doMemmove ($dest, $src, $size) {}
		public function doMemset ($dest, $value, $size) {}
		public function doLoadElem ($base, $index) {}
		public function doLoadRelative ($base, $offset) {}
		public function doLoadElemAddress ($base, $index) {}
		public function doStoreElem ($base, $index, $value) {}
		public function doStoreRelative ($base, $offset, $value) {}
		public function doJumpTable ($value, $table) {}
		public function doPush ($op) {}
		public function doPushPtr ($value, $type) {}
		public function doPop ($items) {}
		public function doDeferPop ($items) {}
		public function doFlushDeferPop ($items) {}
		public function doConvert ($value, $type, $overflow) {}
		public function doSize ($value) {}
		public function doReturn ($op) {}
		public function doReturnPtr ($value, $type) {}
		public function doDefaultReturn () {}
		public function doGetCallStack () {}
		public function doEcho ($string) {}
	}
}
