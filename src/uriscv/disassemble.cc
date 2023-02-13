/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/****************************************************************************
 *
 * This support module contains some functions used to convert MIPS
 * instructions in human-readable form. They are used by objdump utility
 * and Watch class.
 * It also contains some utility functions for instruction decoding and sign
 * extension of halfword quantities.
 *
 ****************************************************************************/

#include <cstring>
#include <stdio.h>

#include "uriscv/arch.h"
#include "uriscv/const.h"
#include "uriscv/cp0.h"
#include "uriscv/processor_defs.h"
#include "uriscv/types.h"

// register names
HIDDEN const char *const regName[CPUREGNUM] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1",
    "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

HIDDEN const char *const floatRegName[CPUREGNUM] = {
    "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "fs0", "fs1", "fa0", "fa1",
    "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7",
    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11", "fcsr"};

const char* RegName(unsigned int index)
{
	if (index < CPUREGNUM)
		return regName[index];
	else
		return EMPTYSTR;
}

// static string buffer size and definition
#define STRBUFSIZE 64
HIDDEN char strbuf[STRBUFSIZE];

HIDDEN const char *const RInstrName[][3] = {
	{"add", "mul", "sub"},
	{"sll", "mulh"},
	{"slt", "mulhsu"},
	{"sltu", "mulhu"},
	{"xor", "div"},
	{"srl", "divu", "sra"},
	{"or", "rem"},
	{"and", "remu"}
};

HIDDEN void StrRInstr(Word instr) {
	uint8_t func3 = FUNC3(instr);
	uint8_t func7 = FUNC7(instr);

	if (func7 == 0x20) func7 = 2;
	if (func7 > 2) {
		sprintf(strbuf, "unknown instruction");
		return;
	}

	sprintf(strbuf, "%s\t%s,%s,%s",
			RInstrName[func3][func7],
			regName[RD(instr)],
			regName[RS1(instr)],
			regName[RS2(instr)]
	);
}

HIDDEN const char *const loadInstrName[] = {
	"lb",
	"lh",
	"lw",
	"",
	"lbu",
	"lhu"
};

HIDDEN void StrLoadInstr(Word instr) {
	uint8_t func3 = FUNC3(instr);

	sprintf(strbuf, "%s\t%s,%d(%s)",
		loadInstrName[func3],
		regName[RD(instr)],
		SIGN_EXTENSION(I_IMM(instr), I_IMM_SIZE),
		regName[RS1(instr)]);
}

HIDDEN const char *const IInstrName[] = {
	"addi",
	"slli",
	"slti",
	"sltiu",
	"xori",
	"",
	"ori",
	"andi",
	"slli"
};

HIDDEN void StrNonLoadIInstr(Word instr) {
	uint8_t func3 = FUNC3(instr);

	switch (func3) {
		case OP_ADDI: {
			if(instr == INSTR_NOP) {
				sprintf(strbuf, "nop");
				return;
			}
			else if(SIGN_EXTENSION(I_IMM(instr), I_IMM_SIZE) == 0) {
				sprintf(strbuf, "mv\t%s,%s", regName[RD(instr)], regName[RS1(instr)]);
				return;
			}
		}
		case OP_SLTI:
		case OP_SLTIU:
		case OP_XORI:
		case OP_ORI:
		case OP_ANDI: {
            sprintf(strbuf, "%s\t%s,%s,%d",
				IInstrName[func3],
				regName[RD(instr)],
				regName[RS1(instr)],
				SIGN_EXTENSION(I_IMM(instr), I_IMM_SIZE)
			);
		}
		break;

		case OP_SLLI:
		case OP_SR: {
			sprintf(strbuf, "%s\t%s,%s,0x%x",
				(func3 == OP_SLLI ? 
					"slli" :
					(FUNC7(instr) == 0 ? "srli" : "srai")
				),
				regName[RD(instr)],
				regName[RS1(instr)],
				RS2(instr) // shamt
			);
		}
		break;

    	default: {
			sprintf(strbuf, "unknown instruction");
		}
		break;


	}
}

HIDDEN const char *const SInstrName[] = {
	"sb",
	"sh",
	"sw"
};

HIDDEN void StrSInstr(Word instr) {
	uint8_t func3 = FUNC3(instr);

	if(func3 > 2) {
		sprintf(strbuf, "unknown instruction");
		return;
	}

	sprintf(strbuf, "%s\t%s,%d(%s)",
		SInstrName[func3],
		regName[RS2(instr)],
		SIGN_EXTENSION(S_IMM(instr), S_IMM_SIZE),
		regName[RS1(instr)]);
}


HIDDEN const char *const BInstrName[] = {
	"beq",
	"bne",
	"",
	"",
	"blt",
	"bge",
	"bltu",
	"bgeu",
};

HIDDEN void StrBInstr(Word instr) {
	uint16_t func3 = FUNC3(instr);
	
	sprintf(strbuf, "%s\t%s,%s,%d",
			BInstrName[func3],
			regName[RS1(instr)],
			regName[RS2(instr)],
			SIGN_EXTENSION(B_IMM(instr), I_IMM_SIZE));
}

HIDDEN const char *const CSRInstrName[] = {
	"",
	"cssrw",
	"csrrs",
	"csrrc",
	"",
	"csrrwi",
	"csrrsi",
	"cssrci"
};

HIDDEN void StrCSRInstr(Word instr, uint8_t func3) {
	if(func3 < 4) {
		// Non immediate variants
		sprintf(strbuf, "%s\t%s,0x%x,%s",
			CSRInstrName[func3],
			regName[RD(instr)],
			I_IMM(instr),
			regName[RS1(instr)]
		);
	}
	else {
		// Immediate variants
		sprintf(strbuf, "%s\t%s,0x%x,%d",
			CSRInstrName[func3],
			regName[RD(instr)],
			I_IMM(instr),
			RS1(instr)
		);
	}
}

HIDDEN const char *const FenceOperandMnemonic[] = {
	"",
	"w",
	"r",
	"rw",
	"o",
	"ow",
	"or",
	"orw",
	"i",
	"iw",
	"ir",
	"irw",
	"io",
	"iow",
	"ior",
	"iorw",
};

HIDDEN uint8_t NO_ROUNDING_MODE = 8;
HIDDEN const char *const roundingModeNames[] = {
	"rne",
	"rtz",
	"rdn",
	"rup",
	"rmm",
	"illegal rounding mode",
	"illegal rounding mode",
	"dyn",
	""
};

HIDDEN const char *const fsgnjsInstrNames[] = {
	"fsgnj.s",
	"fsgnjn.s",
	"fsgnjx.s",
	"fsgnj.d",
	"fsgnjn.d",
	"fsgnjx.d"
};

HIDDEN const char *const fCompareInstrNames[] = {
	"fle.s",
	"flt.s",
	"feq.s",
	"fle.d",
	"flt.d",
	"feq.d"
};

HIDDEN void StrFloatArithmInstr(Word instr, char const *name, uint8_t func3) {
	sprintf(strbuf, "%s\t%s,%s,%s,%s",
		name,
		floatRegName[RD(instr)],
		floatRegName[RS1(instr)],
		floatRegName[RS2(instr)],
		roundingModeNames[func3]
	);
}

HIDDEN void StrFloatArithmMInstr(Word instr, const char *name) {
	sprintf(strbuf, "%s\t%s,%s,%s,%s,%s",
		name,
		floatRegName[RD(instr)],
		floatRegName[RS1(instr)],
		floatRegName[RS2(instr)],
		floatRegName[RS3(instr)],
		roundingModeNames[FUNC3(instr)]
	);
}

HIDDEN void StrFloatOpInstr(Word instr) {
	// func3 can represent Rounding Mode in some cases
	uint8_t func3 = FUNC3(instr);
	uint8_t func7 = FUNC7(instr);

	switch (func7) {
		case OP_FADDS_FUNC7:
		case OP_FADDD_FUNC7: {
			StrFloatArithmInstr(instr, func7 == OP_FADDS_FUNC7 ? "fadd.s" : "fadd.d", func3);
		}
		break;

		case OP_FSUBS_FUNC7:
		case OP_FSUBD_FUNC7: {
			StrFloatArithmInstr(instr, func7 == OP_FSUBS_FUNC7 ? "fsub.s" : "fsub.d", func3);
		}
		break;

		case OP_FMULS_FUNC7: 
		case OP_FMULD_FUNC7: {
			StrFloatArithmInstr(instr, func7 == OP_FMULS_FUNC7 ? "fmul.s" : "fmul.d", func3);
		}
		break;

		case OP_FDIVS_FUNC7: 
		case OP_FDIVD_FUNC7: {
			StrFloatArithmInstr(instr, func7 == OP_FDIVS_FUNC7 ? "fdiv.s" : "fdiv.d", func3);
		}
		break;

		case OP_FSQRTS_FUNC7:
		case OP_FSQRTD_FUNC7: {
			sprintf(strbuf, "%s\t%s,%s,%s",
				func7 == OP_FSQRTS_FUNC7 ? "fsqrt.s" : "fsqrt.d",
				floatRegName[RD(instr)],
				floatRegName[RS1(instr)],
				roundingModeNames[func3]
			);
		}
		break;

		case OP_FSGNJS_FUNC7: 
		case OP_FSGNJD_FUNC7: {
			if(func3 > 2) {
				sprintf(strbuf, "unknown instruction");
				return;
			}
			
			sprintf(strbuf, "%s\t%s,%s,%s",
				fsgnjsInstrNames[func3 + (func7 == OP_FSGNJS_FUNC7 ? 0 : 3)],
				floatRegName[RD(instr)],
				floatRegName[RS1(instr)],
				floatRegName[RS2(instr)]
			);
		}
		break;

		case OP_FMINMAXS_FUNC7: 
		case OP_FMINMAXD_FUNC7: {
			char const *precision = func7 == OP_FMINMAXS_FUNC7 ? "s" : "d";
			char const *instrName;
			if (func3 != OP_FMIN_FUNC3 && func3 != OP_FMAX_FUNC3) {
				instrName = "unknown instruction";
				precision = "";
			}
			else {
				instrName = func3 == OP_FMIN_FUNC3 ? "fmin." : "fmax.";
			}

			sprintf(strbuf, "%s%s\t%s,%s,%s",
				instrName,
				precision,
				floatRegName[RD(instr)],
				floatRegName[RS1(instr)],
				floatRegName[RS2(instr)]
			);
		}
		break;

		case OP_FCVTWS_FUNC7: 
		case OP_FCVTWD_FUNC7: {
			char const *instrName;

			switch (func7) {
				case OP_FCVTWS_FUNC7: {
					instrName = RS2(instr) == OP_FCVTWS_FUNCRS2 ? "fcvt.w.s" : "fcvt.wu.s";
				}
				break;

				case OP_FCVTWD_FUNC7: {
					instrName = RS2(instr) == OP_FCVTWD_FUNCRS2 ? "fcvt.w.d" : "fcvt.wu.d";
				}
				break;

				default: {
					instrName = "unknown instruction";
				}
				break;
			}

			sprintf(strbuf, "%s\t%s,%s,%s",
				instrName,
				regName[RD(instr)],
				floatRegName[RS1(instr)],
				roundingModeNames[func3]
			);
		}
		break;

		case OP_FMVCLASSS_FUNC7: 
		case OP_FCLASSD_FUNC7: {
			char const *instrName;
			if(func7 == OP_FCLASSD_FUNC7) {
				instrName = "fclass.d";
			}
			else {
				instrName = func3 == OP_FMVXW_FUNC3 || func3 == OP_FCLASSS_FUNC3 ?
								func3 == OP_FMVXW_FUNC3 ?  "fmv.x.w" : "fclass.s"
							: "unknown instruction";
			}
			sprintf(strbuf, "%s\t%s,%s",
				instrName,
				regName[RD(instr)],
				floatRegName[RS1(instr)]
			);
		}
		break;

		case OP_FCOMPARES_FUNC7: 
		case OP_FCOMPARED_FUNC7: {
			if(func3 > 2) {
				sprintf(strbuf, "unknown instruction");
				return;
			}

			sprintf(strbuf, "%s\t%s,%s,%s",
				fCompareInstrNames[func3 + (func7 == OP_FCOMPARES_FUNC7 ? 0 : 3)],
				regName[RD(instr)],
				floatRegName[RS1(instr)],
				floatRegName[RS2(instr)]
			);
		}
		break;

		case OP_FCVTSW_FUNC7:
		case OP_FCVTSD_FUNC7:
		case OP_FCVTDS_FUNC7:
		case OP_FCVTDW_FUNC7: {
			char const *instrName;
			int roundingModeIndex = func3;

			switch (func7) {
				case OP_FCVTSW_FUNC7: {
					instrName = RS2(instr) == OP_FCVTSW_FUNCRS2 ?  "fcvt.s.w" : "fcvt.s.wu";
				}
				break;

				case OP_FCVTDW_FUNC7: {
					instrName = RS2(instr) == OP_FCVTDW_FUNCRS2 ?  "fcvt.d.w" : "fcvt.d.wu";
					roundingModeIndex = NO_ROUNDING_MODE;
				}
				break;
				
				case OP_FCVTSD_FUNC7: {
					instrName = "fcvt.s.d";
				}
				break;
				case OP_FCVTDS_FUNC7: {
					instrName = "fcvt.d.s";
					roundingModeIndex = NO_ROUNDING_MODE;
				}
				break;

				default: {
					instrName = "unknown instruction";
					roundingModeIndex = NO_ROUNDING_MODE;
				}
				break;
			}

			sprintf(strbuf, "%s\t%s,%s%s%s",
				instrName,
				floatRegName[RD(instr)],
				regName[RS1(instr)],
				roundingModeIndex != 8 ? "," : "",
				roundingModeNames[roundingModeIndex]
			);
		}
		break;

		case OP_FMVWX_FUNC7: {
			sprintf(strbuf, "fmv.w.x\t%s,%s",
				floatRegName[RD(instr)],
				regName[RS1(instr)]
			);
		}
		break;

		default: {
			sprintf(strbuf, "unknown instruction");
		}
		break;
	}
}


// this function returns the pointer to a static buffer which contains
// the instruction translation into readable form
const char *StrInstr(Word instr) {
	uint8_t opcode = OPCODE(instr);

    switch (opcode) {
        case OP_L: {
            StrLoadInstr(instr);
        }
		break;

        case R_TYPE: {
			StrRInstr(instr);
        }
		break;

        case I_TYPE: {
			StrNonLoadIInstr(instr);
        }
		break;

        case I2_TYPE: {
			uint8_t func3 = FUNC3(instr);

			if(func3 == OP_ECALL_EBREAK)
				sprintf(strbuf, "%s",
					I_IMM(instr) == 0 ? "ecall" : "ebreak"
				);
			else
				StrCSRInstr(instr, func3);
        }
		break;

        case B_TYPE: {
			StrBInstr(instr);
        }
		break;

        case S_TYPE: {
			StrSInstr(instr);
        }
		break;

        case OP_AUIPC:
        case OP_LUI: {
			sprintf(strbuf, "%s\t%s,0x%x",
				opcode == OP_LUI ? "lui" : "auipc",
				regName[RD(instr)],
				U_IMM(instr)
			);
        }
		break;

        case OP_JAL: {
			uint8_t rd = RD(instr);
			int32_t offs = SIGN_EXTENSION(J_IMM(instr), J_IMM_SIZE);
			if(rd == 0)
				sprintf(strbuf, "j\t%s%d", offs > 0 ? "+" : "", offs);
			else if(rd == 1)
				sprintf(strbuf, "jal\t%s%d", offs > 0 ? "+" : "", offs);
			else
				sprintf(strbuf, "jal\t%s,%d", regName[rd], offs);
        }
		break;

        case OP_JALR: {
			if(instr == INSTR_RET) {
				sprintf(strbuf, "ret");
			}
			else {
				sprintf(strbuf, "jalr\t%s,%d(%s)",
					regName[RD(instr)],
					SIGN_EXTENSION(I_IMM(instr), I_IMM_SIZE),
					regName[RS1(instr)]
				);
			}
        }
		break;

		case OP_FENCE: {
			if(instr == 0x8330000f) sprintf(strbuf, "fence.tso");
			else if(instr == 0x0100000f) sprintf(strbuf, "pause");
			else {
				sprintf(strbuf, "fence\t%s,%s",
					FenceOperandMnemonic[FENCE_PRED(instr)],
					FenceOperandMnemonic[FENCE_SUCC(instr)]
				);
			}
		}
		break;

		case OP_FLOAT_OP: {
			StrFloatOpInstr(instr);
		}
		break;

		case OP_FLOAD: {
			sprintf(strbuf, "%s\t%s,%d(%s)",
				FUNC3(instr) == OP_FLW_FUNC3 ? "flw" : "fld",
				floatRegName[RD(instr)],
				SIGN_EXTENSION(I_IMM(instr), I_IMM_SIZE),
				regName[RS1(instr)]
			);
		}
		break;

		case OP_FSAVE: {
			sprintf(strbuf, "%s\t%s,%d(%s)",
				FUNC3(instr) == OP_FSW_FUNC3 ? "fsw" : "fsd",
				floatRegName[RS2(instr)],
				SIGN_EXTENSION(S_IMM(instr), S_IMM_SIZE),
				regName[RS1(instr)]
			);
		}
		break;

		case OP_FMADDS: {
			StrFloatArithmMInstr(instr, FUNC2(instr) == 0 ? "fmadd.s" : "fmadd.d");
		}
		break;

		case OP_FMSUBS: {
			StrFloatArithmMInstr(instr, FUNC2(instr) == 0 ? "fmsub.s" : "fmsub.d");
		}
		break;

		case OP_FNMSUBS: {
			StrFloatArithmMInstr(instr, FUNC2(instr) == 0 ? "fnmsub.s" : "fnmsub.d");
		}
		break;

		case OP_FNMADDS: {
			StrFloatArithmMInstr(instr, FUNC2(instr) == 0 ? "fnmadd.s" : "fnmadd.d");
		}
		break;

        default: {
			sprintf(strbuf, "unknown instruction");
        }
		break;
    }

  return strbuf;
}

const char* InstructionMnemonic(Word ins)
{
	// if (OpType(ins) == REGTYPE)
	// 	return regIName[FUNCT(ins)];
	// else
	// 	return iName[OPCODE(ins)];
	return NULL;
}

// This function returns CP0 register name indexed by position
const char* CP0RegName(unsigned int index)
{
	// if (index < CP0REGNUM)
	// 	return cp0RegName[index];
	// else
	// 	return "";
	return NULL;
}
