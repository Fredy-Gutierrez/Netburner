#include <arp.h>
#include <http.h>
#include <init.h>
#include <netinterface.h>
#include <serial.h>
#include <stdlib.h>

/**
 *  The use of config objects should be done at a global scope.  They can contain any number of member,
 *  which should be given a default value and an name used as an identifier.  The name value for each
 *  member variable should be unique, otherwise it can lead to issues inside the config tree.
 */
static config_string gSingleString{appdata, "MyString", "SName"};
config_int gMyOwnVal{appdata, 199, "MyOwnValue"};

const char *AppName = "BasicConfigVariable";

/**
 *  UpdateConfigString
 *
 *  Takes a config_string, prints the current value, and updates it from
 *  user input.
 */

void UpdateConfigString(config_string &confStr)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confStr.GetNameValue(confName);
    iprintf("%s old value: %s\r\n", confName.c_str(), confStr.c_str());
    char newVal[80];
    iprintf("   Enter new value: ");
    fgets(newVal, 80, stdin);
    newVal[strlen(newVal) - 1] = '\0';   // Replace return character with null
    iprintf("\r\n");

    // Assign the string value
    confStr = newVal;
    iprintf("%s new value: %s\r\n", confName.c_str(), confStr.c_str());

    // Now force it to save
    SaveConfigToStorage();
}

/**
 *  UpdateConfigInt
 *
 *  Takes a config_int, prints the current value, and updates it from
 *  user input.
 */
void UpdateConfigInt(config_int &confInt)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confInt.GetNameValue(confName);
    iprintf("%s old value: %d\r\n", confName.c_str(), int(confInt));
    char newVal[25];
    iprintf("     Enter new value: ");
    fgets(newVal, 25, stdin);
    iprintf("\r\n");

    // Assign the int value
    confInt = atoi(newVal);
    iprintf("%s new value: %d\r\n", confName.c_str(), int(confInt));

    // Now force it to save
    SaveConfigToStorage();
}

/**
 *  ShowCommandList
 *
 *  Prints out a list of all available commands
 */
void ShowCommandList()
{
    iprintf("\r\nCommand List\r\n");
    iprintf("------------\r\n");
    iprintf("   C) Show Config Tree\r\n");
    iprintf("   S) Show and Set gSingleString\r\n");
    iprintf("   V) Show and Set gMyOwnVal\r\n");
    iprintf("   ?) Show Command List\r\n");
}

/**
*  ProcessCommand
*
*  Process serial port menu commands
*/
void ProcessCommand(char cmd)
{
    switch (toupper(cmd))
    {
        case 'C':
        {
            ShowTree();
            break;
        }
        case 'S':
        {
            UpdateConfigString(gSingleString);
            break;
        }
        case 'V':
        {
            UpdateConfigInt(gMyOwnVal);
            break;
        }

        case '?':
        default:
        {
            ShowCommandList();
            break;
        }
    }
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    // Setup everything the device needs to get going
    init();                   // Initialize network stack
    WaitForActiveNetwork();   // Wait for DHCP address

    iprintf("Config Tree Demo built at %s on %s\r\n'?' for commands\r\n", __TIME__, __DATE__);
    iprintf("IP:            %hI\r\n", InterfaceIP(GetFirstInterface()));
    iprintf("AutoIP :       %hI\r\n", InterfaceAutoIP(GetFirstInterface()));
    iprintf("Gateway:       %hI\r\n", InterfaceGate(GetFirstInterface()));

    ShowCommandList();

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);

        if (charavail())
        {
            char c = getchar();
            iprintf("Read char %c\r\n", c);
            ProcessCommand(c);
        }
    }
}
