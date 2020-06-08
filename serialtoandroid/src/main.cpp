#include <init.h>
#include <serial.h>
#include <startnet.h>
#include <stdio.h>



const char * AppName="serialtoandroid";


void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);

    /*
     * The serial port drivers are initialized in polled mode by the monitor.
     * To enable them in interrupt driven mode we need to close and reopen
     * them with OpenSerial()
     */
    SerialClose(0);
    SerialClose(1);

    // Open the serial ports....
    int fd0 = OpenSerial(0, 115200, 1, 8, eParityNone);
    int fd1 = OpenSerial(1, 115200, 1, 8, eParityNone);

    // Now write something out both ports
    writestring(fd1, "Test1");
    writestring(fd0, "Test0");

    while (1)
    {
        // Set up a file set so we can select on the serial ports...
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd1, &read_fds);
        FD_SET(fd0, &read_fds);
        if (select(FD_SETSIZE, &read_fds, (fd_set *)0, (fd_set *)0, TICKS_PER_SECOND * 5))
        {
            if (FD_ISSET(fd1, &read_fds))
            {
#ifdef MOD5441X
                putleds(0x22);
#endif
                char buffer[40];
                int n = read(fd1, buffer, 40);
                write(fd0, buffer, n);
            }

            if (FD_ISSET(fd0, &read_fds))
            {
#ifdef MOD5441X
                putleds(0x22);
#endif
                char buffer[40];
                int n = read(fd0, buffer, 40);
                write(fd1, buffer, n);
            }
        }
        else
        {
            // WE timed out... nothing to send
        }
    }
}
