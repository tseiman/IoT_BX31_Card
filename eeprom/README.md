**eeprom_iot_bx31_card.yaml** is a template file from which the IoT Card EEPROM image can be generated.

The image is only one time flashed to give the IoT Module it's identity and configuration parameters which might be read by a Legato Application or Kernel driver.

The yaml to EEPROM bin image conversion tool can be found from here:

https://github.com/mangOH/mangOH/tree/master/tools

The **writeFile2Eeprom** tool has to be cross compiled in a Legato legs environment and is meant to run directly on a WP module - in case the EEPROM is not flashed during production.

Clone and `make` Legato according to the advises from here: https://github.com/legatoproject/legato-af .

Run the Legato bin/legs environment. Change to this directory and check the Makefile. 
Adjust the Variables:

* $(WP76XX_SYSROOT)
* $(WP76XX_TOOLCHAIN_PREFIX)
* $(WP76XX_TOOLCHAIN_DIR)

At the moment it is set to be compiled for Sierra Wireless WP76XX modules. You might adjust it e.g. to
$(WP77XX_TOOLCHAIN_DIR) or similar for an other module series.

Compile the **writeFile2Eeprom** by `make` in this folder. 

Copy the compiled **writeFile2Eeprom** e.g. by `~> scp writeFile2Eeprom root@192.168.2.2:~`.

Insert a new, unflashed IoT card into the IoT slot of a MangOH board. 
Enable the EEPROM to write. For that the WP Signal of the EEPROM needs to be pulled down. On BX31 IoT Board hardware revision 1.0 the wire 
between R5 and the EEPROM has to be conencted to GND. On newer hardware revisions there is a testpoint which needs to be connected to GND.

Run **writeFile2Eeprom** with a correct set of parameters:

```
writeFile2Eeprom [-b <busID>] -f <eeprom image input file> [-y] -s <IoT card serial Number>
    -b <busID>                     optional,  the I2C bus ID - on MangOH red it is 5 (default)
    -f <eeprom image input file>   mandatory, the image which should be written to eeprom
    -y 	                           optional,  it will not ask if you are really sure
    -s <IoT Serial Number>         mandatory, a string with max 31 bytes with the serial information
                                   of the IoT card. BX31 IoT card SN should have following format:
                                   YYYYMMDDhhmmss_<HW Version>_<NumberInBatch> e.g.: 20190506200700_1.0_001
```

e.g.: `~> writeFile2Eeprom -b 5 -f eeprom_iot_bx31_card.img -s 20190506200700_1.0_001`
