#include <stdio.h>

typedef enum {
    AND = 0,
    TAD = 1,
    ISZ = 2,
    DCA = 3,
    JMS = 4,
    JMP = 5,
    IOT = 6,
    OPR = 7
} opCode;

typedef enum {
    FETCH = 0,
    EXEC  = 1,
    DEFER = 2,
    BREAK = 3
} majorState;

struct pdp8cpu{

    unsigned short ACL;
    unsigned short PC;
    unsigned short MA;
    unsigned short MB;
    unsigned short SR;
    unsigned char IR;

    //current state
    majorState state;
};


char * opcode_label[] = {
    "AND","TAD","ISZ","DCA","JMS","JMP","IOT","OPR"
};

//le hardware 
struct pdp8cpu cpu;
unsigned short memory[0xFFF]; //4096 mots de memoire (pas d'extension :))

void decode(unsigned short word)
{
    opCode op = word >> 9;
    unsigned char indirect = (word & 0x100)>>8;
    unsigned char zeropage = (word & 0x80)>>7;
    unsigned char address = (word & 0x7F);

    printf("\n=> %s IND %d ZP %d ADDR:%o \n",opcode_label[op],indirect,zeropage,address);

}

void dumpCpu()
{
    printf("ACL:%o PC:%o SR:%o\n",cpu.ACL,cpu.PC,cpu.SR);
}

void dumpMemory(unsigned short addr)
{
    printf("%04o:",addr);
    for(int i = 0;i<8;i++){
        printf("\t%04o",memory[addr+i]);
    }
    printf("\n");
}

void loadAddress()
{
    //put content of SR into PC
    cpu.PC = cpu.SR&0xFFF;
}

void deposit()
{
    //deposit SR in mem[PC]
    memory[cpu.PC] = cpu.SR;
}

void singleInstruction()
{


}

void writeMem(unsigned short location, unsigned short value)
{

}

void ANDY(unsigned short value)
{
    cpu.ACL = cpu.ACL&value;
}

void TADY(unsigned short value)
{
    cpu.ACL = cpu.ACL+value;
}

void ISZY(unsigned short value)
{

}

void DCAY(unsigned short value)
{

}

void JMSY(unsigned short value)
{

}

void JMPY(unsigned short value)
{

}

void version()
{
    printf("====================================\n");
    printf("= LITTLECODESHOP DEC PDP8 EMULATOR =\n");
    printf("====================================\n");
}

int main(int argc, char ** argv){

    unsigned short program[] = {
        07001,
        02361,
        05356,
        05355,
        00000
    };

    version();

    //load the program 
    cpu.SR = 05555; //address de base
    loadAddress();
    cpu.SR = 07001;
    deposit();
    cpu.SR = 05556; 
    loadAddress();
    cpu.SR = 02361;
    deposit();
    cpu.SR = 05557; 
    loadAddress();
    cpu.SR = 05356;
    deposit();
    cpu.SR = 05560; 
    loadAddress();
    cpu.SR = 05355;
    deposit();
    cpu.SR = 05561; 
    loadAddress();
    cpu.SR = 00000;
    deposit();

    for(int i = 0;i<4;i++){
        decode(memory[05555+i]);
        dumpCpu();
    }

    dumpMemory(05555);

}
