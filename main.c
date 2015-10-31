//Reference card :
//http://homepage.cs.uiowa.edu/~jones/pdp8/refcard/74.html
#include <stdio.h>
#include <stdlib.h>

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

int focal_loaded = 0;
struct pdp8cpu{

    unsigned short ACL;
    unsigned short PC;
    unsigned short MA;
    unsigned short MB;
    unsigned short SR;
    unsigned char IR;

    unsigned char HALT;

    //current state
    majorState state;
};

struct teletypeASR33 {

    unsigned char tti_buffer; //8 bit buffer contains the last typed char
    unsigned char kbd_flag; //1 when there is something ready in tti
    unsigned char tto_buffer; //8 bit buffer next char to be printed
    unsigned char prt_flag; //1 when ready to print next char

};

struct tapereader750c {
    unsigned char reader_flag;
    unsigned char reader_buffer;
    char * file_buffer;
    long buffer_pos;
    long buffer_size;
};



char * opcode_label[] = {
    "AND","TAD","ISZ","DCA","JMS","JMP","IOT","OPR"
};

//le hardware 
struct pdp8cpu cpu;
struct teletypeASR33 teletype;
struct tapereader750c tapereader;
unsigned short memory[0xFFF]; //4096 mots de memoire (pas d'extension :))

void loadtext(char * filename){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    unsigned short addr,val;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);
        sscanf(line,"%ho\t%ho\n",&addr,&val);
        printf("ADDR :%04o \tVAL :%04o\n",addr,val);
        memory[addr] = val;

    }

    fclose(fp);
    if (line)
        free(line);
}

void loadfile(char * filename){
    //load a file in the reader 

    FILE * pFile;
    long lSize;
    char * buffer;
    size_t result;

    pFile = fopen ( filename , "rb" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    result = fread (buffer,1,lSize,pFile);
    if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

    /* the whole file is now loaded in the memory buffer. */

    // terminate
    fclose (pFile);
    //set the tape reader info
    tapereader.file_buffer = buffer;
    tapereader.buffer_pos = 0;
    tapereader.buffer_size = lSize;
}

void dumpCpu()
{
    printf("\nAC:%04o L:%01o PC:%o SR:%o\n",cpu.ACL,(cpu.ACL&010000),cpu.PC,cpu.SR);
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

    //printf("real address :%04o\n",addr);

    //si indirect je dois voir si il faut pas faire un autoindex
    if(indirect&&(addr>=010)&&(addr<=017)){
        printf("AUTOINDEX AT %04o",addr);
        memory[addr] = memory[addr]+1;
    }
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
    printf("JE DEPOSE %04o sur %04o",(cpu.ACL&07777),addr);
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
            if(teletype.kbd_flag)
            {
                cpu.PC++;
            }
            break;
        case 06032:
            printf(">>>>>KCC<<<<<\n");
            teletype.kbd_flag = 0;
            cpu.ACL&=010000;
            break;
        case 06034:
            {
                printf(">>>>>KRS<<<<<\n");
            unsigned short tmp = (unsigned short)teletype.tti_buffer;
            cpu.ACL = (cpu.ACL&017400)|tmp;}
            break;
        case 06036:
            {
                printf(">>>>>KRB<<<<<\n");
            teletype.kbd_flag = 0;
            cpu.ACL&=010000;
            unsigned short tmp = (unsigned short)teletype.tti_buffer;
            cpu.ACL = (cpu.ACL&017400)|tmp;}
            break;
        case 06041:
            printf(">>>>>TSF<<<<<\n");
            teletype.prt_flag = 1;
            if(teletype.prt_flag)
            {
                cpu.PC++;
            }
            break;
        case 06042:
            printf(">>>>>TCF<<<<<\n");
            teletype.prt_flag = 0;
            break;
        case 06044:
            teletype.tto_buffer = (unsigned char)cpu.ACL;
            printf(">>>>>TPC<<<<< CHAR : %c\n",teletype.tto_buffer);
            break;
        case 06046:
            teletype.prt_flag = 0;
            teletype.tto_buffer = (unsigned char)cpu.ACL;
            printf(">>>>>TLS<<<<< CHAR : %c %d\n",teletype.tto_buffer,teletype.tto_buffer);
            break;
        case 06011:
            printf("RSF\n");
            if(tapereader.reader_flag){
                cpu.PC++;
            }
            break;
        case 06012:
            {
                unsigned short tmp = (unsigned short)tapereader.reader_buffer;
                cpu.ACL = (cpu.ACL&017400)|tmp;
                //printf("RRB %c\n",tapereader.reader_buffer);
            }
            break;
        case 06014:
            {
                printf("RFC\n");
                tapereader.reader_flag = 0;
                if(tapereader.buffer_pos<tapereader.buffer_size){
                    tapereader.reader_buffer = tapereader.file_buffer[tapereader.buffer_pos++];
                    tapereader.reader_flag = 1;
                    //printf("READING BYTE -> %02x\n",tapereader.reader_buffer);
                }
                else{
                focal_loaded = 1;
                }
            }
            break;
        case 06016:
            {
                printf("RFC RRB\n");
                tapereader.reader_flag = 0;
                if(tapereader.buffer_pos<tapereader.buffer_size){
                    tapereader.reader_buffer = tapereader.file_buffer[tapereader.buffer_pos++];
                    tapereader.reader_flag = 1;
                    printf("READING BYTE -> %02x\n",tapereader.reader_buffer);
                }
                else{
                focal_loaded = 1;
                }
                unsigned short tmp = (unsigned short)tapereader.reader_buffer;
                cpu.ACL = (cpu.ACL&017400)|tmp;
                printf("RRB %c\n",tapereader.reader_buffer);
            }
            break;

        default:
            break;
    
    }

}


void OPRGRP1(unsigned short value)
{
    //printf("group 1\n");
    if(value&0200){ //CLA
        cpu.ACL&=010000;
    }
    if(value&0100){ //CLL
        cpu.ACL&=07777;
    }
    if(value&040){ //CMA
        cpu.ACL ^= 07777;
    }
    if(value&020){ //CML
        cpu.ACL ^= 1<<12;
    }
    if(value&01){ //IAC
        cpu.ACL++;
    }
    if(value&010){ //RAR
        if(value&02){ //2
            cpu.ACL = ((cpu.ACL<<11)|(cpu.ACL>>2))&07777;
        }else{
            cpu.ACL = ((cpu.ACL<<12)|(cpu.ACL>>1))&07777;
        }
    }
    if(value&04){ //RAL
        //one or 2 ??
        if(value&02){ //2
            cpu.ACL = ((cpu.ACL<<2)|(cpu.ACL>>11))&07777;
        }else{
            cpu.ACL = ((cpu.ACL<<1)|(cpu.ACL>>12))&07777;
        }
    }
}

void OPRGRP2(unsigned short value)    
{
    if((value&01)==0){
        //printf("group 2\n");
        if(value==07410){//inconditional skip !
            cpu.PC++;
            return;
        }
        if(value&0100){ //SMA/SPA
            if(value&010){//SPA
                printf("SPA\n");
                if((cpu.ACL&04000)){
                    cpu.PC++;
                }
            }else{//SMA
                printf("SMA\n");
                if((cpu.ACL&04000)==0){
                    cpu.PC++;
                }
            }
        }
        if(value&040){ //SZA/SNA
            if(value&010){//SNA
                if(cpu.ACL&0777){
                    cpu.PC++;
                }
            }else{//SZA
                if((cpu.ACL&0777)==0){
                    cpu.PC++;
                }
            }
        }
        if(value&020){ //SNL
            if(value&010){//SZL
                if((cpu.ACL&010000)==0){
                    cpu.PC++;
                }
            }else{//SNL
                if(cpu.ACL&010000){
                    cpu.PC++;
                }
            }
        }
        if(value&0200){ //CLA
            cpu.ACL&=010000;
        }
        if(value&04){ //OSR
            cpu.ACL |= cpu.SR;
        }
        if(value&02){ //HLT
            printf("HALT");
            cpu.HALT = 1;
        }
    } else {
        printf("EAE GROUP INSTRUCTIONS\n");
    }
}

void OPRV(unsigned short value)
{
    if((value&0400)==0){
        OPRGRP1(value);
    }
    if(value&0400){
        OPRGRP2(value);
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
    dumpCpu();
    //fetch
    unsigned short next_op = memory[cpu.PC++];
    //execute ??
    execute(next_op);


}

void version()
{
    printf("====================================\n");
    printf("= LITTLECODESHOP DEC PDP8 EMULATOR =\n");
    printf("====================================\n");
}

int main(int argc, char ** argv){

    unsigned short program[] = {
    };

    version();

    //RIM LOADER
    cpu.SR = 07756;
    loadAddress();
    cpu.SR = 06014;                      /* 7756, RFC */
    deposit();
    cpu.SR = 06011;                      /* 7757, LOOP, RSF */
    deposit();
    cpu.SR = 05357;                      /* JMP .-1 */
    deposit();
    cpu.SR = 06016;                      /* RFC RRB */
    deposit();
    cpu.SR = 07106;                      /* CLL RTL*/
    deposit();
    cpu.SR = 07006;                      /* RTL */
    deposit();
    cpu.SR = 07510;                      /* SPA*/
    deposit();
    cpu.SR = 05374;                      /* JMP 7774 */
    deposit();
    cpu.SR = 07006;                      /* RTL */
    deposit();
    cpu.SR = 06011;                      /* RSF */
    deposit();
    cpu.SR = 05367;                      /* JMP .-1 */
    deposit();
    cpu.SR = 06016;                      /* RFC RRB */
    deposit();
    cpu.SR = 07420;                      /* SNL */
    deposit();
    cpu.SR = 03776;                      /* DCA I 7776 */
    deposit();
    cpu.SR = 03376;                      /* 7774, DCA 7776 */
    deposit();
    cpu.SR = 05357;                      /* JMP 7757 */
    deposit();
    cpu.SR = 00000;                      /* 7776, 0 */
    deposit();
    cpu.SR = 00000;                      /* 7777, JMP 7701 */
    deposit();
    

    //load the program //increment memory  
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


    ////// PROGRAM UART
    cpu.SR = 07470;
    loadAddress();
    cpu.SR = 07200;
    deposit();
    cpu.SR = 07001;
    deposit();
    cpu.SR = 07002;
    deposit();
    cpu.SR = 07001;
    deposit();
    cpu.SR = 06046;
    deposit();
    cpu.SR = 07402;
    deposit();


    ////////// TTY TEST OUTPUT
    cpu.SR = 00000;
    loadAddress();
    cpu.SR = 07200;
    deposit();
    cpu.SR = 06046;
    deposit();
    cpu.SR = 06041;
    deposit();
    cpu.SR = 05002;
    deposit();
    cpu.SR = 07001;
    deposit();
    cpu.SR = 06046;
    deposit();
    cpu.SR = 05002;
    deposit();

    //////// ADDITION
    cpu.SR = 0200;
    loadAddress();
    cpu.SR = 07200;
    deposit();
    cpu.SR = 01205;
    deposit();
    cpu.SR = 01206;
    deposit();
    cpu.SR = 07402;
    deposit();
    cpu.SR = 05577;
    deposit();
    cpu.SR = 00003;
    deposit();
    cpu.SR = 00004;
    deposit();



    cpu.SR = 07756; //address de base
    loadAddress();

    cpu.HALT = 0;

    //loadfile("loader.bin"); //load that paper tape in the machine
    loadtext("focal.txt");

    //Load focal
    //while(!focal_loaded){
    //    singleInstruction();
    //    dumpMemory(07776);
    //    dumpCpu();
    //    getchar();
    //}

    //start FOCAL :)
    printf("STARTING FOCAL\n");
    dumpMemory(0200);
//    cpu.SR = 0200;
//    loadAddress();
//    while(!cpu.HALT){
//        singleInstruction();
//    }

    
}
