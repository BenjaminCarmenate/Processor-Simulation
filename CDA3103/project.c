// Group Members: Benjamin Carmenate, Taylor McGruder, Katerina Lypsky, Justis Nazirbage
// CDA 3103 Project
// 4/21/2022

#include "spimcore.h"

/* ALU */
/* 10 Points */
// this function determines exactly what ALU operation to perform
// base on the ALU control, operations were given on the table in the pdf
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
  unsigned Z = 0;
  *Zero = 0;

  // switches through alu operations with the alu control code
  switch (ALUControl)
  {
    // add
    case 0b000:
      Z = A + B;
      *ALUresult = Z;
      break;
    // sub
    case 0b001:
      Z = A - B;
      *ALUresult = Z;
      break;
    // A < B signed
    case 0b010:
      Z = 0;
      if((int)A < (int)B)
        Z = 1;
      *ALUresult = Z;
      break;
    // A < B unsigned
    case 0b011:
      Z = 0;
      if(A < B)
        Z = 1;
      *ALUresult = Z;
      break;
    // A bitwise and B
    case 0b100:
      Z = A & B;
      *ALUresult = Z;
      break;
    // A bitwise or B
    case 0b101:
      Z = A | B;
      *ALUresult = Z;
      break;
    // B left shifted 16
    case 0b110:
      Z = (B << 16);
      *ALUresult = Z;
      break;
    // inverse of A
    case 0b111:
      Z = ~A;
      *ALUresult = Z;
      break;
  }

  if(*ALUresult == 0)
    *Zero = 1;
}

/* instruction fetch */
/* 10 Points */
// checks if PC is a valid address which means multiple of 4 and not out of
// bounds and stroes that instruction from memory in the instruction variable
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
  // halt checks
  if(PC % 4 != 0)
    return 1;
  if(PC < 0b00000000000000000000 || PC > 0b11111111111111111111)
    return 1;

  *instruction = Mem[(PC>>2)];
  return 0;
}


/* instruction partition */
/* 10 Points */
// partitions the instruction into its different segments by bitshifting to he position
// the instruction is at and then masking out the rest of the bits.
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
  *op = (instruction >> 26) & 0b111111;
  *r1 = (instruction >> 21) & 0b11111;
  *r2 = (instruction >> 16) & 0b11111;
  *r3 = (instruction >> 11) & 0b11111;
  *funct = instruction & 0b00000000000000000000000000111111;
  *offset = instruction & 0b00000000000000001111111111111111;
  *jsec = instruction & 0b00000001111111111111111111111111;
}



/* instruction decode */
/* 15 Points */
// essentially sets permissions for each individual instruction
// permissions for the R-type instructions are the same
// but permissions for I-type and J-type differ, the permissions are determined
// by following the provided diagram and seeing what multiplexers need to be turned
// on and off to perform the desired operations.
int instruction_decode(unsigned op,struct_controls *controls)
{
  controls->RegDst = 0;
  controls->Jump = 0;
  controls->Branch = 0;
  controls->MemRead = 0;
  controls->MemtoReg = 0;
  controls->ALUOp = 0;
  controls->MemWrite = 0;
  controls->ALUSrc = 0;
  controls->RegWrite = 0;


  switch (op) {
    // r-type
    case 0b000000:
      controls->RegDst = 1;
      controls->RegWrite = 1;
      controls->ALUOp = 0b111;
      break;
    //addi
    case 0b001000:
      controls->ALUOp = 0b000;
      controls->RegWrite = 1;
      controls->ALUSrc = 1;
      break;
    // Jump
    case 0b000010:
      controls->Jump = 1;
      controls->ALUOp = 0b000;
      break;
    //sw
    case 0b101011:
      controls->MemWrite = 1;
      controls->ALUSrc = 1;
      break;
    //lw
    case 0b100011:
      controls->RegWrite = 1;
      controls->MemtoReg = 1;
      controls->MemRead = 1;
      controls->ALUSrc = 1;
      break;
    //lui
    case 0b001111:
      controls->ALUOp = 0b110;
      controls->RegWrite = 1;
      controls->ALUSrc = 1;
      break;
    //beq
    case 0b000100:
      controls->Branch;
      controls->ALUOp = 0b001;
      break;
    //sltiu
    case 0b001011:
      controls->ALUOp = 0b011;
      controls->ALUSrc = 1;
      controls->RegWrite = 1;
      break;
    //slti
    case 0b101000:
      controls->ALUOp = 0b010;
      controls->ALUSrc = 1;
      controls->RegWrite = 1;
      break;
    //unknown instruction halts program
    default:
      return 1;
  }
  return 0;
}

/* Read Register */
/* 5 Points */
// reads the registers into data1 and data2
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
  *data1 = Reg[r1];
  *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
// turns an unsigned integer to a 32 bit signed integer
void sign_extend(unsigned offset,unsigned *extended_value)
{
  unsigned check = offset & 0b1000000000000000;
  if(check >= 1)
    *extended_value = offset | 0b11111111111111110000000000000000;
  else
    *extended_value = offset & 0b00000000000000001111111111111111;
}

/* ALU operations */
/* 10 Points */
// this function determines if the op code is an I-type or R-type
// if it is an r-type it switches through the funct codes and performs the correct
// ALU operations
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    if(ALUOp != 0b111)
    {
      if(ALUSrc = 1)
        ALU(data1, extended_value, ALUOp, ALUresult, Zero);
      else
        ALU(data1, data2, ALUOp, ALUresult, Zero);
    }
    else
    {
      // switches through the r type by using their funct codes
      switch (funct) {
        //add
        case 0b100000:
          ALU(data1, data2, 0b000, ALUresult, Zero);
          break;
        //sub
        case 0b100010:
          ALU(data1, data2, 0b001, ALUresult, Zero);
          break;
        //and
        case 0b100100:
          ALU(data1, data2, 0b100, ALUresult, Zero);
          break;
        //or
        case 0b100101:
          ALU(data1, data2, 0b101, ALUresult, Zero);
          break;
        //slt
        case 0b101010:
          ALU(data1, data2, 0b010, ALUresult, Zero);
          break;
        //sltu
        case 0b101011:
          ALU(data1, data2, 0b011, ALUresult, Zero);
          break;
      }
  }
  return 0;
}

/* Read / Write Memory */
/* 10 Points */
// this function reads or writes to memory
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
  // checks if ALUresult is valid in either case
  if(MemRead == 1 || MemWrite == 1)
  {
    if((ALUresult % 4) != 0)
      return 1;
    if(ALUresult < 0b00000000000000000000 || ALUresult > 0b11111111111111111111)
      return 1;
  }

  //on mem read it stores the Mem addressed at the ALU result in memdata
  if(MemRead == 1)
    *memdata = Mem[ALUresult >> 2];
  //on mem write stores the memory addressed at ALUresult in data2
  if(MemWrite == 1)
    Mem[ALUresult >> 2] = data2;

  return 0;
}


/* Write Register */
/* 10 Points */
// this function handles writing to the register either from the ALU or from memory, and handles writing to both registers.
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
  if(RegWrite == 1)
  {
    // write to r2
    if(RegDst == 0)
    {
      // write from mem
      if(MemtoReg == 1)
        Reg[r2] = memdata;
      // write from ALU
      else
        Reg[r2] = ALUresult;
    }
    // write to r3
    else
    {
      if(MemtoReg == 1)
        Reg[r3] = memdata;
      else if(RegDst == 1)
        Reg[r3] = ALUresult;
    }
  }
}

/* PC update */
/* 10 Points */
// this funtion updates PC by 4 on every run through and in some cases jumps or branches
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
  *PC = *PC + 4;

  if(Jump == 1)
  {
    // checks if jumping to a valid address
    if((jsec % 4) != 0)
      return;
    if(jsec < 0b00000000000000000000 || jsec > 0b11111111111111111111)
      return;
    // if so jumps
    *PC = (*PC & 0b11110000000000000000000000000000) | (jsec << 2);
  }

  if(Branch == 1 && Zero == 1)
    *PC = *PC + (extended_value << 2);

}
