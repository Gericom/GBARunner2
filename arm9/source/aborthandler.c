#include <nds.h>

extern char thumb_string[];
extern char unk_string[];
extern char ok_string[];

extern uint8_t nibble_to_char[];

extern uint8_t NIBBLE_LOOKUP[];

extern uint32_t DISPCNT_copy;
extern uint32_t WAITCNT_copy;
extern uint32_t counter;

typedef struct
{
	uint32_t eeprom_address;
	uint8_t eeprom_bitstream_data_in[4 * 1024];
	uint8_t eeprom_bitstream_data_out[4 * 1024];
	uint8_t eeprom_data[16 * 1024];
} eeprom_vram;

#define EEPROM_VRAM_BLOCK	((eeprom_vram*)(0x02400000 - 1536 - (32 * 1024)))//0x06898000)



ITCM_CODE static inline uint8_t count_ones_8(uint16_t byte)
{
	return NIBBLE_LOOKUP[byte & 0x0F] + NIBBLE_LOOKUP[(byte >> 4) & 0x0F];
}

ITCM_CODE static inline uint8_t count_ones_16(uint16_t byte)
{
	return NIBBLE_LOOKUP[byte & 0x0F] + NIBBLE_LOOKUP[(byte >> 4) & 0x0F] + NIBBLE_LOOKUP[(byte >> 8) & 0x0F] + NIBBLE_LOOKUP[(byte >> 12) & 0x0F];
}

ITCM_CODE void PrintDebug(char* text, uint16_t* dst)
{
	uint16_t cur = 0;
	int first = 1;
	char curchar;
	while((curchar = *text++) != 0)
	{
		if(first)
		{
			cur = curchar;
		}
		else
		{
			cur |= curchar << 8;
			*dst++ = cur;
		}
		first = !first;
	}
	if(!first)
		*dst++ = cur;
}

ITCM_CODE void PrintHexDebug(uint32_t value, uint16_t* dst)
{
	uint16_t chars = nibble_to_char[value >> 28];
	value <<= 4;
	chars |= nibble_to_char[value >> 28] << 8;
	value <<= 4;
	*dst++ = chars;

	chars = nibble_to_char[value >> 28];
	value <<= 4;
	chars |= nibble_to_char[value >> 28] << 8;
	value <<= 4;
	*dst++ = chars;

	chars = nibble_to_char[value >> 28];
	value <<= 4;
	chars |= nibble_to_char[value >> 28] << 8;
	value <<= 4;
	*dst++ = chars;

	chars = nibble_to_char[value >> 28];
	value <<= 4;
	chars |= nibble_to_char[value >> 28] << 8;
	value <<= 4;
	*dst++ = chars;
}

static ITCM_CODE uint32_t Shift(uint32_t ShiftType, uint32_t Value, uint32_t NrBits, uint32* Carry)
{
	switch (ShiftType)
	{
	case 0:
		if (NrBits > 0) *Carry = ((Value >> (32 - (int)NrBits)) & 1) == 1;
		return Value << (int)NrBits;
	case 1:
		if (NrBits > 0) *Carry = ((Value >> ((int)NrBits - 1)) & 1) == 1;
		else *Carry = ((Value >> 31) & 1) == 1;
		return Value >> (int)NrBits;
	case 2:
		if (NrBits > 0)
		{
			*Carry = ((Value >> ((int)NrBits - 1)) & 1) == 1;
			return (uint32_t)(((int)Value) >> (int)NrBits);
		}
		else
		{
			*Carry = ((Value >> 31) & 1) == 1;
			return ((Value >> 31) & 1) * 0xFFFFFFFF;
		}
	case 3:
		if (NrBits > 0)
		{
			*Carry = ((Value >> ((int)NrBits - 1)) & 1) == 1;
			return (Value >> (int)NrBits) | (Value << (32 - (int)NrBits));
		}
		else
		{
			uint32_t tmp = ((*Carry ? 1u : 0u) << 31) | (Value >> 1);
			*Carry = (Value & 1) == 1;
			return tmp;
		}
	}
	return 0xFFFFFFFF;
}

static ITCM_CODE __attribute__ ((hot)) uint32_t ReadIOAddress(uint32_t address, uint32_t size)
{
	if(address >= 0x04000060 && address <= 0x040000A8)
	{
		return 0;
	}

	if((address >> 24) == 0xD)
		return 1;

	if(address == 0x4000000 && (size == 2 || size == 4))
		return DISPCNT_copy;

	if(address == 0x4000000 && size == 1)
		return DISPCNT_copy & 0xFF;

	if(address == 0x4000001)
		return DISPCNT_copy >> 8;

	if(address == 0x04000204 || address == 0x04000205 || address == 0x04000206 || address == 0x04000207)	
	{
		address = (uint32_t)&WAITCNT_copy + (address - 0x04000204);
	}

	if((address == 0x04000100 || address == 0x04000104 || address == 0x04000108 || address == 0x0400010C) && size == 2)
	{
		return *((uint16_t*)address) / 2;
	}

	if(address == 0x4000200 && size == 4)
	{
		return *((uint16_t*)0x4000210) | (*((uint16_t*)0x4000214) << 16);
	}

	if(address == 0x4000200 && size == 2)
	{
		return *((uint16_t*)0x4000210);
	}

	if(address == 0x4000200 && size == 1)
	{
		return *((uint8_t*)0x4000210);
	}

	if(address == 0x4000201 && size == 1)
	{
		return *((uint8_t*)0x4000211);
	}

	if(address == 0x4000202 && size == 2)
	{
		return *((uint16_t*)0x4000214);
	}

	if(address == 0x4000202 && size == 1)
	{
		return *((uint8_t*)0x4000214);
	}

	if(address == 0x4000203 && size == 1)
	{
		return *((uint8_t*)0x4000215);
	}

	if(address >= 0x06010000)
		address += 0x3F0000;

	if(size == 1)
		return *((uint8_t*)address);
	else if(size == 2)
		return *((uint16_t*)address);
	else
		return *((uint32_t*)address);
}

static ITCM_CODE __attribute__ ((hot)) void WriteIOAddress(uint32_t address, uint32_t value, uint32_t size)
{
	//sound
	if(address >= 0x04000060 && address <= 0x040000A8)
		return;

	if(address >= 0x04000180 && address <= 0x040001BC)
		return;

	if(address == 0x04000204 || address == 0x04000205 || address == 0x04000206 || address == 0x04000207)	
	{
		address = (uint32_t)&WAITCNT_copy + (address - 0x04000204);
	}

	if(address >= 0x04000210 && address <= 0x0400024A)
		return;

	if(address >= 0x04000280 && address <= 0x040002C0)
		return;

	if(address >= 0x04000320 && address <= 0x040006A3)
		return;

	if(address == 0x04000304)
		return;

	if(address >= 0x06010000)
		address += 0x3F0000;

	if((address == 0x04000100 || address == 0x04000104 || address == 0x04000108 || address == 0x0400010C) && size == 2)
	{
		value *= 2;
		value &= 0xFFFF;
	}

	if((address == 0x04000100 || address == 0x04000104 || address == 0x04000108 || address == 0x0400010C) && size == 4)
	{
		value = (value & 0xFFFF0000) | (((value & 0xFFFF) * 2) & 0xFFFF);
	}

	//dma src
	if((address == 0x40000B0 || address == 0x40000BC || address == 0x40000C8 || address == 0x40000D4) && size == 4)
	{
		if(value >= 0x0D000000 && value <= 0x0DFFFFFF)
		{
			//eeprom shit
			//parse eeprom_bitstream_data_in
			if(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[0] == 1 && ((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[1] == 1)
			{
				//read request
				EEPROM_VRAM_BLOCK->eeprom_address = 
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[2] << 13) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[3] << 12) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[4] << 11) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[5] << 10) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[6] << 9) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[7] << 8) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[8] << 7) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[9] << 6) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[10] << 5) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[11] << 4) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[12] << 3) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[13] << 2) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[14] << 1) |
					(((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in)[15] << 0);
				EEPROM_VRAM_BLOCK->eeprom_address <<= 3;//64bits=8bytes each address

				((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_out)[0] = 0;
				((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_out)[1] = 0;
				((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_out)[2] = 0;
				((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_out)[3] = 0;
				int q = 4;
				int i;
				for(i = 0; i < 64; i++)
				{
					((uint16_t*)EEPROM_VRAM_BLOCK->eeprom_bitstream_data_out)[q++] = (EEPROM_VRAM_BLOCK->eeprom_data[EEPROM_VRAM_BLOCK->eeprom_address + i / 8] >> (7 - (i & 7))) & 1;
				}
				value = (uint32_t)&EEPROM_VRAM_BLOCK->eeprom_bitstream_data_out[0];

				//PrintHexDebug(0xEEEEEEE2, (uint16_t*)(0x06202000 + 32 * 29));
			}
		}
		else if((value >> 24) >= 8 && (value >> 24) <= 0xD)
			value -= 0x5FC0000;
		else if(value >= 0x06010000 && value <= 0x06017FFF)
			value += 0x3F0000;
	}

	if((address == 0x40000B2 || address == 0x40000BE || address == 0x40000CA || address == 0x40000D6) && size == 2)
	{
		if((value >> 8) >= 8 && (value >> 8) <= 0xD)
			value -= 0x5FC;
		else if(value == 0x0601)
			value += 0x3F;
	}

	//dma dst
	if((address == 0x40000B4 || address == 0x40000C0 || address == 0x40000CC || address == 0x40000D8) && size == 4)
	{
		if(value >= 0x06010000 && value <= 0x06017FFF)
			value += 0x3F0000;
		else if(value >= 0x0D000000 && value <= 0x0DFFFFFF)
		{
			//eeprom shit
			value = (uint32_t)&EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in[0];

			//PrintHexDebug(0xEEEEEEEE, (uint16_t*)(0x06202000 + 32 * 30));
		}
	}

	if((address == 0x40000B6 || address == 0x40000C2 || address == 0x40000CE || address == 0x40000DA) && size == 2)
	{
		if(value == 0x0601)
			value += 0x3F;
	}

	if((address == 0x040000B8 || address == 0x40000C4 || address == 0x040000D0 || address == 0x040000DC) && size == 2)
	{
		if(value == 0 && address != 0x040000DC)
			value = 0x4000;
	}

	if((address == 0x040000BA || address == 0x40000C6 || address == 0x040000D2 || address == 0x040000DE) && size == 2)
	{
		value = (value & ~0x1F);
		if(address == 0x040000DE && *((uint16_t*)(address - 2)) == 0)
			value |= 1;
		if(((value >> 12) & 0x3) < 3)
			value = (value & ~0x3800) | (((value >> 12) & 0x3) << 11);
		else
		{
			if(address == 0x40000C6)//sound
			{
				//dst
				*((uint32_t*)0x40000C0) = 0x02400000 - 1536;//0x023FFFF0;
				value = (value & ~0x3800) | (3 << 5);
				value = (value & ~0x1F);
				*((uint16_t*)(address - 2)) = 384;
			}
			else
				value &= ~0x8000;
		}

		if(address == 0x040000DE && *((uint32_t*)0x40000D8) == (uint32_t)&EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in[0])
		{
			uint16_t* src = (uint16_t*)(*((uint32_t*)0x40000D4));
			if(src[0] == 1 && src[1] == 0)
			{
				EEPROM_VRAM_BLOCK->eeprom_address = 
					(src[2] << 13) |
					(src[3] << 12) |
					(src[4] << 11) |
					(src[5] << 10) |
					(src[6] << 9) |
					(src[7] << 8) |
					(src[8] << 7) |
					(src[9] << 6) |
					(src[10] << 5) |
					(src[11] << 4) |
					(src[12] << 3) |
					(src[13] << 2) |
					(src[14] << 1) |
					(src[15] << 0);
				EEPROM_VRAM_BLOCK->eeprom_address <<= 3;//64bits=8bytes each address
				int q = 16;
				int i;
				for(i = 0; i < 64; i++)
				{
					EEPROM_VRAM_BLOCK->eeprom_data[EEPROM_VRAM_BLOCK->eeprom_address + i / 8] &= ~(1 << (7 - (i & 7)));
					EEPROM_VRAM_BLOCK->eeprom_data[EEPROM_VRAM_BLOCK->eeprom_address + i / 8] |= (src[q++] & 1) << (7 - (i & 7));
				}
				//uint32_t test = *((uint32_t*)&EEPROM_VRAM_BLOCK->eeprom_data[0x80]);
				//PrintHexDebug(test, (uint16_t*)(0x06202000 + 32 * 31));
			}
		}
	}

	if((address == 0x040000B8 || address == 0x40000C4 || address == 0x040000D0 || address == 0x040000DC) && size == 4)
	{
		int realsize = value & 0xFFFF;
		if(realsize == 0 && address != 0x040000DC)
			realsize = 0x4000;
		else if(realsize == 0 && address == 0x040000DC)
			realsize = 0x10000;

		value = (value & ~0x1FFFFF) | realsize;
		if(((value >> 28) & 0x3) < 3)
			value = (value & ~0x38000000) | (((value >> 28) & 0x3) << 27);
		else
		{
			if(address == 0x40000C4)//sound
			{
				//dst
				*((uint32_t*)0x40000C0) = 0x02400000 - 1536;
				value = (value & ~0x38000000) | (3 << (5 + 16));
				value = (value & ~0x1FFFFF);
				value |= 384;
			}
			else
				value &= ~0x80000000;
		}

		if(address == 0x040000DC && *((uint32_t*)0x40000D8) == (uint32_t)&EEPROM_VRAM_BLOCK->eeprom_bitstream_data_in[0])
		{
			uint16_t* src = (uint16_t*)(*((uint32_t*)0x40000D4));
			if(src[0] == 1 && src[1] == 0)
			{
				EEPROM_VRAM_BLOCK->eeprom_address = 
					(src[2] << 13) |
					(src[3] << 12) |
					(src[4] << 11) |
					(src[5] << 10) |
					(src[6] << 9) |
					(src[7] << 8) |
					(src[8] << 7) |
					(src[9] << 6) |
					(src[10] << 5) |
					(src[11] << 4) |
					(src[12] << 3) |
					(src[13] << 2) |
					(src[14] << 1) |
					(src[15] << 0);
				EEPROM_VRAM_BLOCK->eeprom_address <<= 3;//64bits=8bytes each address
				int q = 16;
				int i;
				for(i = 0; i < 64; i++)
				{
					EEPROM_VRAM_BLOCK->eeprom_data[EEPROM_VRAM_BLOCK->eeprom_address + i / 8] &= ~(1 << (7 - (i & 7)));
					EEPROM_VRAM_BLOCK->eeprom_data[EEPROM_VRAM_BLOCK->eeprom_address + i / 8] |= (src[q++] & 1) << (7 - (i & 7));
				}
				//uint32_t test = *((uint32_t*)&EEPROM_VRAM_BLOCK->eeprom_data[0x80]);
				//PrintHexDebug(test, (uint16_t*)(0x06202000 + 32 * 31));
			}
		}
	}

	//display control
	if(address == 0x4000000)
	{
		if(size == 2 || size == 4)
		{
			DISPCNT_copy = value & 0xFFFF;
			size = 4;
			uint32_t newval = value & 0xFF80;
			newval |= ((value >> 5) & 1) << 23;//hblank free bit is moved on the ds
			newval |= ((value >> 6) & 1) << 4;//obj mode bit is moved on the ds aswell
			newval |= 1 << 16;//display mode, which did not exist on gba
			int mode = value & 7;
			if(mode == 1)
			{
				mode = 2;
				newval &= ~(1 << 11);//disable bg3
			}
			else if(mode >= 3)
			{
				mode = 5;
			}
			newval |= mode;

			value = newval;
		}
		else if(size == 1)
		{
			DISPCNT_copy = (DISPCNT_copy & 0xFF00) | value;
			size = 4;
			uint32_t newval = (value | ((*((uint8_t*)0x4000001)) << 8)) & 0xFF80;
			newval |= ((value >> 5) & 1) << 23;//hblank free bit is moved on the ds
			newval |= ((value >> 6) & 1) << 4;//obj mode bit is moved on the ds aswell
			newval |= 1 << 16;//display mode, which did not exist on gba
			int mode = value & 7;
			if(mode == 1)
			{
				mode = 2;
				newval &= ~(1 << 11);//disable bg3
			}
			else if(mode >= 3)
			{
				mode = 5;
			}
			newval |= mode;

			value = newval;
		}
	}

	if(address == 0x4000001)
	{
		DISPCNT_copy = (DISPCNT_copy & 0xFF) | (value << 8);
	}

	if(address == 0x4000200 && size == 4)
	{
		*((uint16_t*)0x4000210) = value & 0xFFFF;
		*((uint16_t*)0x4000214) = value >> 16;
		return;
	}

	if(address == 0x4000200 && size == 2)
	{
		*((uint16_t*)0x4000210) = value;
		return;
	}

	if(address == 0x4000200 && size == 1)
	{
		*((uint8_t*)0x4000210) = value;
		return;
	}

	if(address == 0x4000201 && size == 1)
	{
		*((uint8_t*)0x4000211) = value;
		return;
	}

	if(address == 0x4000202 && size == 2)
	{
		*((uint16_t*)0x4000214) = value;
		return;
	}

	if(address == 0x4000202 && size == 1)
	{
		*((uint8_t*)0x4000214) = value;
		return;
	}

	if(address == 0x4000203 && size == 1)
	{
		*((uint8_t*)0x4000215) = value;
		return;
	}

	if(size == 1)
		*((uint8_t*)address) = value;
	else if(size == 2)
		*((uint16_t*)address) = value;
	else
		*((uint32_t*)address) = value;
}

//#define DEBUG_ADDRESSES

ITCM_CODE __attribute__ ((hot)) int DataAbortHandler(void* instructionAddress, uint32_t* registerTable, uint32_t carry)//, uint32_t thumb)
{
	uint32_t instruction = *((uint32_t*)instructionAddress);
	instruction &= 0x0FFFFFFF;//remove condition code
	uint32_t Rn = (instruction >> 16) & 0xF;
	if((instruction >> 25) == 0)
	{
		//PrintDebug(unk_string, (uint16_t*)0x06202000);
		//PrintHexDebug(instruction, (uint16_t*)(0x06202000 + 32));
		//while(1);
		uint32_t Rd_nr = (instruction >> 12) & 0xF;

		uint32_t Offset;
		if ((instruction >> 22) & 1)
		{
			Offset = (((instruction >> 8) & 0xF) << 4) | (instruction & 0xF);
		}
		else
		{
			Offset = registerTable[instruction & 0xF];
		}

		uint32_t MemoryOffset = registerTable[Rn];
		//if (Rn == &mGlobalState.registers[15]) MemoryOffset = mExecutionState.instructionPC + 8;
		if ((instruction >> 24) & 1)
		{
			if ((instruction >> 23) & 1) MemoryOffset += Offset;
			else MemoryOffset -= Offset;
			//if (W) *Rn = MemoryOffset;
		}

#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(MemoryOffset, (uint16_t*)(0x06202000 + 64));
#endif

		if((MemoryOffset >> 24) == 4 || (MemoryOffset >> 24) == 0xD || (MemoryOffset >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			if(((instruction >> 24) & 1) && ((instruction >> 21) & 1))//P and W
			{
				registerTable[Rn] = MemoryOffset;
			}
			if((instruction >> 20) & 1)//Load
			{
				int op = (instruction >> 5) & 3;
				if (op == 1) //byte
					registerTable[Rd_nr] = ReadIOAddress(MemoryOffset, 2);
				else if(op == 2)
					registerTable[Rd_nr] = (uint32_t)(((int)(ReadIOAddress(MemoryOffset, 1) << 24)) >> 24);
				else if(op == 3)
					registerTable[Rd_nr] = (uint32_t)(((int)(ReadIOAddress(MemoryOffset, 2) << 16)) >> 16);
				else
				{
					PrintDebug(unk_string, (uint16_t*)0x06202000);
					PrintHexDebug(instruction, (uint16_t*)(0x06202000 + 32));
					while(1);
				}
			}
			else
			{
				int op = (instruction >> 5) & 3;
				if(op == 1)
					WriteIOAddress(MemoryOffset, registerTable[Rd_nr] & 0xFFFF, 2);
				else
				{
					PrintDebug(unk_string, (uint16_t*)0x06202000);
					PrintHexDebug(instruction, (uint16_t*)(0x06202000 + 32));
					while(1);
				}
			}
			if (!((instruction >> 24) & 1))
			{
				if ((instruction >> 23) & 1) MemoryOffset += Offset;
				else MemoryOffset -= Offset;
				registerTable[Rn] = MemoryOffset;
			}
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((MemoryOffset >> 24) >= 8 && (MemoryOffset >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			if(!((instruction >> 22) & 1))
			{
				if((registerTable[Rn] >> 24) >= 8 && (registerTable[Rn] >> 24) <= 0xD)
				{
					registerTable[Rn] &= ~0x070000000;
					registerTable[Rn] -= 0x5FC0000;
				}
				else
				{
					registerTable[instruction & 0xF] &= ~0x070000000;
					registerTable[instruction & 0xF] -= 0x5FC0000;
				}
			}
			else
			{
				registerTable[Rn] &= ~0x070000000;
				registerTable[Rn] -= 0x5FC0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((MemoryOffset >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(MemoryOffset >= 0x06010000)
			{
				if(((instruction >> 22) & 1) || (registerTable[Rn] >> 24) == 6)
					registerTable[Rn] += 0x3F0000;
				else
					registerTable[instruction & 0xF] += 0x3F0000;
			}
			else
			{
				if(((instruction >> 22) & 1) || (registerTable[Rn] >> 24) == 6)
					registerTable[Rn] -= 0x3F0000;
				else
					registerTable[instruction & 0xF] -= 0x3F0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((MemoryOffset >> 24) == 0xE)
		{
			//save
			if(((instruction >> 22) & 1) || (registerTable[Rn] >> 24) == 0xE)
				registerTable[Rn] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			else
				registerTable[instruction & 0xF] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}
		while(1);
	}
	/*else if((instruction >> 25) == 1)
	{
		PrintDebug(unk_string, (uint16_t*)0x06202000);
		PrintHexDebug(instruction, (uint16_t*)(0x06202000 + 32));
		while(1);
	}*/
	else if((instruction >> 25) == 2 || (instruction >> 25) == 3)
	{
		uint32_t Rd_nr = (instruction >> 12) & 0xF;

		uint32_t Offset;
		if ((instruction >> 25) & 1)//I
		{
			uint32_t Is = (instruction >> 7) & 0x1F;
			uint32_t ShiftType = (instruction >> 5) & 0x3;
			uint32_t Rm_nr = instruction & 0xF;
			uint32_t Shift_C = carry;
			Offset = Shift(ShiftType, registerTable[Rm_nr], Is, &Shift_C);
		}
		else
		{
			Offset = instruction & 0xFFF;
		}

		uint32_t MemoryOffset = registerTable[Rn];
		//if (Rn == 15) MemoryOffset = mExecutionState.instructionPC + 8;
		if ((instruction >> 24) & 1)
		{
			if ((instruction >> 23) & 1) MemoryOffset += Offset;
			else MemoryOffset -= Offset;
			//if (W) *Rn = MemoryOffset;
		}
#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(MemoryOffset, (uint16_t*)(0x06202000 + 64));
#endif

		if((MemoryOffset >> 24) == 4 || (MemoryOffset >> 24) == 0xD || (MemoryOffset >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			if(((instruction >> 24) & 1) && ((instruction >> 21) & 1))//P and W
			{
				registerTable[Rn] = MemoryOffset;
			}
			if((instruction >> 20) & 1)//Load
			{
				if ((instruction >> 22) & 1) //byte
					registerTable[Rd_nr] = ReadIOAddress(MemoryOffset, 1);
				else
					registerTable[Rd_nr] = ReadIOAddress(MemoryOffset, 4);
			}
			else
			{
				if ((instruction >> 22) & 1) //byte
					WriteIOAddress(MemoryOffset, registerTable[Rd_nr] & 0xFF, 1);
				else
					WriteIOAddress(MemoryOffset, registerTable[Rd_nr], 4);
			}
			if (!((instruction >> 24) & 1))
			{
				if ((instruction >> 23) & 1) MemoryOffset += Offset;
				else MemoryOffset -= Offset;
				registerTable[Rn] = MemoryOffset;
			}
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((MemoryOffset >> 24) >= 8 && (MemoryOffset >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			registerTable[Rn] &= ~0x070000000;
			registerTable[Rn] -= 0x5FC0000;
			if(((instruction >> 25) & 1) && (registerTable[instruction & 0xF] >> 24) >= 8)
			{
				PrintDebug(thumb_string, (uint16_t*)0x06202000);
				while(1);
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((MemoryOffset >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(MemoryOffset >= 0x06010000)
			{
				registerTable[Rn] += 0x3F0000;
			}
			else
			{
				registerTable[Rn] -= 0x3F0000;
			}
			if(((instruction >> 25) & 1) && (registerTable[instruction & 0xF] >> 24) == 0x6)
			{
				PrintDebug(thumb_string, (uint16_t*)0x06202000);
				while(1);
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((MemoryOffset >> 24) == 0xE)
		{
			//save
			registerTable[Rn] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			if(((instruction >> 25) & 1) && (registerTable[instruction & 0xF] >> 24) == 0xE)
			{
				PrintDebug(thumb_string, (uint16_t*)0x06202000);
				while(1);
			}
			return 1;
		}
		while(1);
	}
	else if((instruction >> 25) == 4)
	{
		uint32_t RList = instruction & 0xFFFF;
		uint32_t base_address = registerTable[Rn];
		int nrregisters = count_ones_16(RList);
		uint32_t start_address;
		if (((instruction >> 24) & 1) && ((instruction >> 23) & 1)) start_address = base_address + 4;
		else if (!((instruction >> 24) & 1) && ((instruction >> 23) & 1)) start_address = base_address;
		else if (((instruction >> 24) & 1) && !((instruction >> 23) & 1)) start_address = base_address - 4 * nrregisters;
		else start_address = base_address - 4 * (nrregisters - 1);
		//if (W) *GetRegisterById(Rn) = base_address + (U ? 4 * nrregisters : -4 * nrregisters);

#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(start_address, (uint16_t*)(0x06202000 + 64));
#endif

		if((start_address >> 24) == 4 || (start_address >> 24) == 0xD || (start_address >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			if((instruction >> 21) & 1)
				registerTable[Rn] = base_address + (((instruction >> 23) & 1) ? 4 * nrregisters : -4 * nrregisters);

			uint32_t cur = start_address;
			//uint32_t end = base_address + (((instruction >> 23) & 1) ? 4 * nrregisters : 0) + ((((instruction >> 23) & 1) && ((instruction >> 24) & 1)) ? 4 : 0);

			int i;
			if((instruction >> 20) & 1)//load
			{
				for(i = 0; i < 16; i++)
				{
					if(!((RList >> i) & 1))
						continue;
					registerTable[i] = ReadIOAddress(cur, 4);
					cur += 4;
				}
			}
			else
			{
				for(i = 0; i < 16; i++)
				{
					if(!((RList >> i) & 1))
						continue;
					WriteIOAddress(cur, registerTable[i], 4);
					cur += 4;
				}
			}
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((start_address >> 24) >= 8 && (start_address >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			registerTable[Rn] &= ~0x070000000;
			registerTable[Rn] -= 0x5FC0000;
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((start_address >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(start_address >= 0x06010000)
			{
				registerTable[Rn] += 0x3F0000;
			}
			else
			{
				registerTable[Rn] -= 0x3F0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((start_address >> 24) == 0xE)
		{
			//save
			registerTable[Rn] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}

		while(1);
	}
	else
	{
		//?
		PrintDebug(unk_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(instruction, (uint16_t*)(0x06202000 + 64));
		while(1);
	}
}

ITCM_CODE __attribute__ ((hot)) int DataAbortHandlerThumb(void* instructionAddress, uint32_t* registerTable)
{
	uint16_t instruction = *((uint16_t*)((uint32_t)instructionAddress & ~1));//just to be safe, it seems like bit 0 is not set

	if ((instruction >> 12) == 5 && ((instruction >> 9) & 1) == 0)
	{
		//mExecutionState.instructionProc = &ARM7TDMI::Instruction_Thumb_7;
		uint32_t Ro = (instruction >> 6) & 7;
		uint32_t Rb = (instruction >> 3) & 7;
		uint32_t address = registerTable[Rb] + registerTable[Ro];
#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(address, (uint16_t*)(0x06202000 + 64));
#endif

		if((address >> 24) == 4 || (address >> 24) == 0xD || (address >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			uint32_t Opcode = (instruction >> 10) & 3;
			uint32_t Rd = instruction & 7;
			if(Opcode == 0)
				WriteIOAddress(address, registerTable[Rd], 4);
			else if(Opcode == 1)
				WriteIOAddress(address, registerTable[Rd] & 0xFF, 1);
			else if(Opcode == 2)
				registerTable[Rd] = ReadIOAddress(address, 4);
			else
				registerTable[Rd] = ReadIOAddress(address, 1);
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((address >> 24) >= 8 && (address >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			if((registerTable[Rb] >> 24) >= 8 && (registerTable[Rb] >> 24) <= 0xD)
			{
				registerTable[Rb] &= ~0x070000000;
				registerTable[Rb] -= 0x5FC0000;
			}
			else
			{
				registerTable[Ro] &= ~0x070000000;
				registerTable[Ro] -= 0x5FC0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(address >= 0x06010000)
			{
				if((registerTable[Rb] >> 24) == 6)
					registerTable[Rb] += 0x3F0000;
				else
					registerTable[Ro] += 0x3F0000;
			}
			else
			{
				if((registerTable[Rb] >> 24) == 6)
					registerTable[Rb] -= 0x3F0000;
				else
					registerTable[Ro] -= 0x3F0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 0xE)
		{
			//save
			if((registerTable[Rb] >> 24) == 0xE)
				registerTable[Rb] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			else
				registerTable[Ro] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}
		while(1);
	}
	else if ((instruction >> 12) == 5 && ((instruction >> 9) & 1) == 1)
	{
		//mExecutionState.instructionProc = &ARM7TDMI::Instruction_Thumb_8;
		uint32_t Ro = (instruction >> 6) & 7;
		uint32_t Rb = (instruction >> 3) & 7;
		uint32_t address = registerTable[Rb] + registerTable[Ro];
#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(address, (uint16_t*)(0x06202000 + 64));
#endif
		if((address >> 24) == 4 || (address >> 24) == 0xD || (address >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			uint32_t Opcode = (instruction >> 10) & 3;
			uint32_t Rd = instruction & 7;
			if(Opcode == 0)
				WriteIOAddress(address, registerTable[Rd] & 0xFFFF, 2);
			else if(Opcode == 1)
				registerTable[Rd] = (uint32_t)(((int)(ReadIOAddress(address, 1) << 24)) >> 24);
			else if(Opcode == 2)
				registerTable[Rd] = ReadIOAddress(address, 2);
			else
				registerTable[Rd] = (uint32_t)(((int)(ReadIOAddress(address, 2) << 16)) >> 16);
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((address >> 24) >= 8 && (address >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			if((registerTable[Rb] >> 24) >= 8 && (registerTable[Rb] >> 24) <= 0xD)
			{
				registerTable[Rb] &= ~0x070000000;
				registerTable[Rb] -= 0x5FC0000;
			}
			else
			{
				registerTable[Ro] &= ~0x070000000;
				registerTable[Ro] -= 0x5FC0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(address >= 0x06010000)
			{
				if((registerTable[Rb] >> 24) == 6)
					registerTable[Rb] += 0x3F0000;
				else
					registerTable[Ro] += 0x3F0000;
			}
			else
			{
				if((registerTable[Rb] >> 24) == 6)
					registerTable[Rb] -= 0x3F0000;
				else
					registerTable[Ro] -= 0x3F0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 0xE)
		{
			//save
			if((registerTable[Rb] >> 24) == 0xE)
				registerTable[Rb] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			else
				registerTable[Ro] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}
		while(1);
	}
	else if ((instruction >> 13) == 3)
	{
		//mExecutionState.instructionProc = &ARM7TDMI::Instruction_Thumb_9;
		uint32_t Opcode = (instruction >> 11) & 3;
		uint32_t imm = (instruction >> 6) & 31;
		uint32_t Rb = (instruction >> 3) & 7;
		uint32_t address = registerTable[Rb] + (Opcode <= 1 ? imm * 4 : imm);
#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(address, (uint16_t*)(0x06202000 + 64));
#endif
		if((address >> 24) == 4 || (address >> 24) == 0xD || (address >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			uint32_t Rd = instruction & 7;
			if(Opcode == 0)
				WriteIOAddress(address, registerTable[Rd], 4);
			else if(Opcode == 1)
				registerTable[Rd] = ReadIOAddress(address, 4);
			else if(Opcode == 2)
				WriteIOAddress(address, registerTable[Rd] & 0xFF, 1);
			else
				registerTable[Rd] = ReadIOAddress(address, 1);
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((address >> 24) >= 8 && (address >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			registerTable[Rb] &= ~0x070000000;
			registerTable[Rb] -= 0x5FC0000;
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			//registerTable[Rb] += 0x3F0000;

			if(address >= 0x06010000)
			{
				registerTable[Rb] += 0x3F0000;
			}
			else
			{
				registerTable[Rb] -= 0x3F0000;
			}

			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 0xE)
		{
			//save
			registerTable[Rb] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}
		while(1);
	}
	else if ((instruction >> 12) == 8)
	{
		//mExecutionState.instructionProc = &ARM7TDMI::Instruction_Thumb_10;
		uint32_t imm = (instruction >> 6) & 31;
		uint32_t Rb = (instruction >> 3) & 7;
		uint32_t address = registerTable[Rb] + imm * 2;
#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(address, (uint16_t*)(0x06202000 + 64));
#endif
		if((address >> 24) == 4 || (address >> 24) == 0xD || (address >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			if(((instruction >> 11) & 1) == 0)//write
			{
				WriteIOAddress(address, registerTable[instruction & 7] & 0xFFFF, 2);
			}
			else
			{
				registerTable[instruction & 7] = ReadIOAddress(address, 2);
			}
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((address >> 24) >= 8 && (address >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			registerTable[Rb] &= ~0x070000000;
			registerTable[Rb] -= 0x5FC0000;
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(address >= 0x06010000)
			{
				registerTable[Rb] += 0x3F0000;
			}
			else
			{
				registerTable[Rb] -= 0x3F0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((address >> 24) == 0xE)
		{
			//save
			registerTable[Rb] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}
		while(1);
	}
	else if ((instruction >> 12) == 12)
	{
		//mExecutionState.instructionProc = &ARM7TDMI::Instruction_Thumb_15;
			
		uint32_t Rb = (instruction >> 8) & 7;
		uint32_t base_address = registerTable[Rb];
#ifdef DEBUG_ADDRESSES
		PrintDebug(ok_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(base_address, (uint16_t*)(0x06202000 + 64));
#endif
		if((base_address >> 24) == 4 || (base_address >> 24) == 0xD || (base_address >> 24) == 6)
		{
			//io register
			//manually execute the opcode
			uint32_t L = (instruction >> 11) & 1;
			uint32_t RList = instruction & 0xFF;

			int nrregisters = count_ones_8(RList);
			uint32_t start_address = base_address;
			uint32_t end_address = base_address + 4 * nrregisters;
			registerTable[Rb] = end_address;
			uint32_t cur = start_address;
			int i;
			if(L)//load
			{
				for(i = 0; i < 8; i++)
				{
					if(!((RList >> i) & 1))
						continue;
					registerTable[i] = ReadIOAddress(cur, 4);
					cur += 4;
				}
			}
			else
			{
				for(i = 0; i < 8; i++)
				{
					if(!((RList >> i) & 1))
						continue;
					WriteIOAddress(cur, registerTable[i], 4);
					cur += 4;
				}
			}
			//return 0 to signal that the original opcode should be skipped
			return 0;
		}
		else if((base_address >> 24) >= 8 && (base_address >> 24) <= 0xD)
		{
			//cartridge
			//fix address by subtracting 0x08000000 - 0x02040000 = 0x05FC0000
			registerTable[Rb] &= ~0x070000000;
			registerTable[Rb] -= 0x5FC0000;
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((base_address >> 24) == 6)
		{
			//oam
			//fix address by adding 0x06400000 - 0x06010000 = 0x3F0000
			//I am not sure if this is the best solution, as a game might use this address again to copy something to the bg vram. Maybe that needs some special protection for the memory between 0x06010000 and 0x06400000
			//to fix that
			if(base_address >= 0x06010000)
			{
				registerTable[Rb] += 0x3F0000;
			}
			else
			{
				registerTable[Rb] -= 0x3F0000;
			}
			//we should return 1 to signal that the original opcode should be reexecuted with fixed registers
			return 1;
		}
		else if((base_address >> 24) == 0xE)
		{
			//save
			registerTable[Rb] -= 0x0E000000 - ((uint32_t)EEPROM_VRAM_BLOCK);
			return 1;
		}
		while(1);
	}
	else
	{
		//?
		PrintDebug(unk_string, (uint16_t*)0x06202000);
		PrintHexDebug((uint32_t)instructionAddress, (uint16_t*)(0x06202000 + 32));
		PrintHexDebug(instruction, (uint16_t*)(0x06202000 + 64));
		while(1);
	}
	while(1);
}