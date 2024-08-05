#pragma once

static int
instrsize(unsigned int oparg) {
	return oparg <= 0xff ? 1 :
		oparg <= 0xffff ? 2 :
		oparg <= 0xffffff ? 3 : 4;
}

#define PACKOPARG(opcode, oparg) ((_Py_CODEUNIT)(((oparg) << 8) | (opcode)))

static void
write_op_arg(_Py_CODEUNIT *codestr, unsigned char opcode,
		unsigned int oparg, int ilen) {
	switch (ilen) {
	case 1:
		*codestr++ = PACKOPARG(opcode, oparg & 0xff);
		break;
  default:
		assert(false);
	}
}
