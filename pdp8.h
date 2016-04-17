
int startPDP8();
void registerKeyboardInput(char (*key_in)(void)); //callback into main to check if there is a key
void registerTeletypeOutput(void (*tty_out)(char c));//callback to give char out
