#include <stdio.h>

struct pdp8cpu{

    unsigned short ACL;
    unsigned short PC;
    unsigned short MA;
    unsigned short MB;
    unsigned short SR;
    unsigned char IR;

};


typedef enum {
    AND = 0,
    TAD = 1,
    ISZ =2,
    DCA =3,
    JMS =4,
    JMP =5
} opCode;

char * opcode_label[] = {
    "AND","TAD","ISZ","DCA","JMS","JMP","IOT","OPR"
};

struct opcode{
    unsigned char opcode;
    unsigned char indirect;
    unsigned char zeropage;
    unsigned char address;
};

struct pdp8cpu cpu;

void decode(unsigned short word)
{
    opCode op = word >> 9;
    unsigned char indirect = (word & 0x100)>>8;
    unsigned char zeropage = (word & 0x80)>>7;
    unsigned char address = (word & 0x7F);


    printf("\n=> %s ",opcode_label[op]);


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

    for(int i=0;i<5;i++){
        decode (program[i]);
    }
}
