# IoT BX31 Card

IoT card for Sierra Wireless MangOH (https://mangoh.io) boards.
Connects the Sierra Wirelss  BX3105 Module via UART to the (WP) Module.
The IoT card is easy to use by using the Legato (https://legato.io) AT command interpreter.

![BX31 IoT Card](https://raw.githubusercontent.com/tseiman/IoT_BX31_Card/master/doc/IoT_BX31.jpg)

![BX31 IoT Card](https://raw.githubusercontent.com/tseiman/IoT_BX31_Card/master/doc/IoT_BX31_board.png)


## Programming the EEPROM
After hardware production the EEPROM needs to be one time flashed with the Identity and configuration data of the IoT card.
Please see the eeprom folder for further information. 

## Basic testing on MangOH
Ensure that the UART of the WP module is configured correctly to handle the IoT module - e.g. by configuring it via `AT!MAPUART=17,1` command on the WP module (please cross check the WP AT User guide).
Ensure that the GPIO's are set correctly. On MangOH red this are GPIO 42 for Firmware update and GPIO 2 for enable. The BX31 IoT module supports update of the Firmware over UART by pulling GPIO 42 low (it is a Active Low Pin on BX3105). For normal operation _pull_GPIO_42_high_. To reset the card you need to pull "enable" GPIO 2  High and then pull it down again - so the BX3105 gets enabled.

Disable Firmware update GPIO from command line:
```
~# echo 42 >/sys/class/gpio/export
~# echo out >/sys/class/gpio/gpio42/direction
~# echo 0 > /sys/class/gpio/gpio42/value
```
Reset the Module:
```
~# echo 2 >/sys/class/gpio/export
~# echo out >/sys/class/gpio/gpio2/direction
~# echo 1 > /sys/class/gpio/gpio42/value
~# echo 0 > /sys/class/gpio/gpio42/value
```

Now you can connect to the UART by using microcom and it should react to BX31 AT commands:
```
~# microcom -X -s 115200 /dev/ttyHS0
ATI

BX3105

OK
```
You can exit microcom by pressing CTR-X.

## Using it the IoT card in Legato

Ensure the UART of the WP module is configured in the right way. On a WP76 this can be done with the `AT!MAPUART=17,1` command. 
For further Information check the AT command guide of the Module you plan to use. 
The IoT module will be then accessible over the `/dev/ttyHS0` UART in Legato.
You might do this setoup out of a Legato application by using the Legato AT interpreter API.

In case you plan to use the BX31 IoT module in an Legato environment and to control it from a Legato Application - ensure that you have read/write access to the device.
This can be configured in the **requires - devices** section of your Legato appliactions **.adef** file - e.g.:

```
requires:
{
    device:
    {
       [rw]    /dev/ttyHS0    /dev/ttyHS0
    }
}

```

Ensure in the same file that you bind the AT Interpreter API to your project:

```
bindings:
{
    bx31_atservice.BX31_ATServiceComponent.le_atClient -> atService.le_atClient
...    
}
```


Now you can use the API to control the BX31 IoT Board - see this snippeds as some example (more glue code might be needed):


```c
        fd = le_tty_Open(BX31_SERIAL_DEVICE, O_RDWR | O_NDELAY | O_NOCTTY | O_NONBLOCK); // opening the UART2 - which is connected to the IoT Board
        if (fd == -1) {
                LE_ERROR("failed to open UART device");
                exit(EXIT_FAILURE);
        }
        
       ...


        LE_ASSERT(le_tty_SetBaudRate(fd, LE_TTY_SPEED_115200) == LE_OK);        // assuming BX31 is on 115200
        LE_ASSERT(le_tty_SetFraming(fd, 'N', 8, 1) == LE_OK);                   // set UART framing to 8bit, No Parity, 1 Stop bit
        LE_ASSERT(le_tty_SetRaw(fd, 10, 30000) == LE_OK);                       // We need raw UART mode (the canonical echo's
                                                                                // back to peer)
       ...

        DevRef = le_atClient_Start(fd);

        cmdRef = le_atClient_Create();                                          // instantiate an AT interpreter client


        /* --- check if we're really talking to BX31 --- */
        LE_INFO("Check BX310x presence");                                       // We ask for device identification string

        LE_ASSERT(le_atClient_SetCommandAndSend
                  (&cmdRef, DevRef, "ATI", "", "OK|ERROR|+CME ERROR",
                   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);

        LE_ASSERT(le_atClient_GetFinalResponse
                  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

        memset(buffer, 0, 50);

        LE_ASSERT(le_atClient_GetFirstIntermediateResponse
                  (cmdRef, buffer, LE_ATDEFS_RESPONSE_MAX_BYTES) == LE_OK);

        if (strncmp(buffer, "BX310", 4) != 0) {                                 // and compare it with the expected
                LE_ERROR("This doesn seem to be a BX310x bluetooth module, "
                                "exiting. Response was \"%s\"", buffer);        // if the BT module is not answering we better exit


      ...


        LE_INFO("Disable WiFi on BX310x");                                      // here we use the BX just for BT scanning
        LE_ASSERT(le_atClient_SetCommandAndSend
                  (&cmdRef, DevRef, "AT+SRWCFG=0", "", "OK|ERROR|+CME ERROR",
                   LE_ATDEFS_COMMAND_DEFAULT_TIMEOUT) == LE_OK);


``` 

For further information please check the legato AT API https://docs.legato.io/17_09/c_atClient.html
There is also a project related to this IoT Board which is used to do a bluetooth scanning: 
https://github.com/tseiman/SWILegatoBTScanner_BX31_AVS





