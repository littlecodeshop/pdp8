#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include "pdp8.h"

//////// CONIO STUFF ////////
struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

void teletype_out(char c){
    printf("%c",c);
}

char keyboard_in(){
         if(kbhit()){
            char c = getch();
            if(c==3) exit(0);
            return c;
        }
        return -1;
}

int main(int argc, char ** argv){

    int opt;
    int index;
    
    while((opt = getopt(argc,argv,"hf:"))!=-1){
        switch(opt){
            case 'f':
                printf("LOADING FILE %s",optarg);
            break;
            case 'h':
                help("pdp8");
                break;
            case 'v':
                version();
            break;
            default:
            break;
        }
    }

    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    

    
    // n'oublis pas ca -> pour passer en mode raw (pas de delay apres key)
    set_conio_terminal_mode();
    
    registerKeyboardInput(keyboard_in);
    //registerTeletypeOutput();
    
    //start the computer
    startPDP8();

}