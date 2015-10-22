//Reference card :
//http://homepage.cs.uiowa.edu/~jones/pdp8/refcard/74.html
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
    memory[cpu.PC++] = cpu.SR;
}



void writeMem(unsigned short location, unsigned short value)
{

}

unsigned short realAddress(unsigned short value)
{
    unsigned short indirect = (value & 0x100)>>8;
    unsigned short currentpage = (value & 0x80)>>7;
    unsigned short address = (value & 0x7F);

    //si zeropage donc les 5 premiers sont 00000 sinon il faut copier de PC
    unsigned short prefix = currentpage?(cpu.PC&07600):00;
    unsigned short addr = prefix|address;

    printf("real address :%04o\n",addr);

    //si indirect -> c'est un pointeur sinon on renvois l'address
    return indirect?memory[addr]:addr;

}

void ANDY(unsigned short value)
{
    unsigned short v = memory[realAddress(value)];
    cpu.ACL = cpu.ACL&v;
}

void TADY(unsigned short value)
{
    unsigned short v = memory[realAddress(value)];
    cpu.ACL = (cpu.ACL+v);
}

void ISZY(unsigned short value)
{
    unsigned short addr = realAddress(value);
    memory[addr] = (memory[addr]+1)&0777;
    if(memory[addr] == 0){
        cpu.PC++;
    }
}

void DCAY(unsigned short value)
{
    unsigned short addr = realAddress(value);
    memory[addr] = cpu.ACL;
    cpu.ACL = cpu.ACL&010000; //je masque le L bit car pas affecté !
}

void JMSY(unsigned short value)
{
    unsigned short addr = realAddress(value);
    memory[addr] = cpu.PC;
    cpu.PC = addr+1;
}

void JMPY(unsigned short value)
{
    unsigned short addr = realAddress(value);
    cpu.PC = addr;
}

void IOTV(unsigned short value)
{
    switch(value){
        case 06031:
            printf(">>>>>KSF<<<<<\n");
            break;
        case 06032:
            printf(">>>>>KCC<<<<<\n");
            break;
        case 06034:
            printf(">>>>>KRS<<<<<\n");
            break;
        case 06036:
            printf(">>>>>KRB<<<<<\n");
            break;
        case 06041:
            printf(">>>>>TSF<<<<<\n");
            break;
        case 06042:
            printf(">>>>>TCF<<<<<\n");
            break;
        case 06044:
            printf(">>>>>TPC<<<<<\n");
            break;
        case 06046:
            printf(">>>>>TLS<<<<<\n");
            break;

        default:
            break;
    
    }

}

void OPRV(unsigned short value)
{
    switch(value){
    
        case 07000:
            //NOP
            break;
        case 07001:
            //IAC
            cpu.ACL++;
            break;
        case 07004:
            //RAL
            cpu.ACL = ((cpu.ACL<<1)|(cpu.ACL>>12))&07777;
            break;
        case 07006:
            //RTL
            cpu.ACL = ((cpu.ACL<<2)|(cpu.ACL>>11))&07777;
            break;
        case 07010:
            //RAR
            cpu.ACL = ((cpu.ACL<<12)|(cpu.ACL>>1))&07777;
            break;
        case 07012:
            //RTR
            cpu.ACL = ((cpu.ACL<<11)|(cpu.ACL>>2))&07777;
            break;
        case 07020:
            //CML
            cpu.ACL ^= 1<<12;
            break;
        case 07040:
            //CMA
            break;
    }
    
}


void (*pdp8_exec[8])(unsigned short v) = {ANDY,TADY,ISZY,DCAY,JMSY,JMPY,IOTV,OPRV};

void execute(unsigned short word)
{
    opCode op = word >> 9;
    unsigned char indirect = (word & 0x100)>>8;
    unsigned char zeropage = (word & 0x80)>>7;
    unsigned char address = (word & 0x7F);

    printf("\n=> %s IND %d ZP %d ADDR:%o \n",opcode_label[op],indirect,zeropage,address);

    pdp8_exec[op](word);
}

void singleInstruction()
{
    //fetch
    unsigned short next_op = memory[cpu.PC++];
    //execute ??
    execute(next_op);

    dumpCpu();

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
    cpu.SR = 02361;
    deposit();
    cpu.SR = 05356;
    deposit();
    cpu.SR = 05355;
    deposit();
    cpu.SR = 00000;
    deposit();


    dumpMemory(05555);

    cpu.SR = 05555; //address de base
    loadAddress();

    while(1){
        singleInstruction();
        dumpMemory(05555);
    }
    
}
