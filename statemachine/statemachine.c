//State Machine task source file with switch statement and state variables defined.
#include <stdio.h>
#include <stdlib.h>

enum state{STATE_NORMAL , STATE_FAULT, STATE_RUNNING , STATE_ERROR};



void* statemachine_thread(void* arg) {
while (1) {
        enum state current_state = STATE_NORMAL;
        // State machine switch statement
        switch (current_state) {
            case 1:
                //code
                break;
            case 2:
               //code
                break;
            case 3:
                 //code
                break;
            case 4:
                 //code
                break;
            default:
                printf("Invalid state\n");
                break;
    }
return 0;
}
}

















