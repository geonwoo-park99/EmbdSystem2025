#include "button.h"

int main(void)
{
    BUTTON_MSG_T messageRxData;
    int msgID = buttonInit();
    int returnValue;
    while(1)
    {
        returnValue = msgrcv(msgID, &messageRxData, sizeof(int), 0, IPC_NOWAIT);
        if(returnValue<0)   break;
    }
    while(1)
    {
        returnValue = msgrcv(msgID, &messageRxData, sizeof(int), 0, 0);
        if (returnValue < 0) printf("Recive failed");
        switch(messageRxData.keyInput)
        {
            case KEY_VOLUMEUP: printf("Volume up key):"); break;
            case KEY_HOME: printf("Home key):"); break;
            case KEY_SEARCH: printf("Search key):"); break;
            case KEY_BACK: printf("Back key):"); break;
            case KEY_MENU: printf("Menu key):"); break;
            case KEY_VOLUMEDOWN: printf("Volume down key):"); break;
        }
        printf("\r\n");
    }
    buttonExit();
    return 0;
}
