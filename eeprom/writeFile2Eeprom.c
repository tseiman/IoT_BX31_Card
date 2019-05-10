/**
 * by Thomas Schmidt Fr 09 Mai 2019 22:59:50 CEST
 * Tool to write the IoT EEPROM directly on an MangOH board.
 * EEPROM Write needs to be enabled (see IoT Board Readme and tool Readme)
 *
 **/


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/limits.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>


#define DEFAULT_MANGOH_RED_IOT_I2C_BUS_ID	5
#define I2C_BASE_PATH				"/dev/i2c-"
#define MAX_SN_LEN 				32
#define EEPROM_SIZE				4096

#define SERIAL_NR_START_OFFSET			70
#define SERIAL_NR_STOP_OFFSET			101





int writeEeprom(char *fileName, char* deviceName, char *serialStr) {
    FILE *infile;
    unsigned char buffer[1] = "\0";
    int lenOfSnStr = strnlen(serialStr, MAX_SN_LEN);
    int snPos = 0;
    int device;
    unsigned char data[3];

    if ((infile = fopen(fileName, "rb")) < 0) {
	perror("ERROR: cannot open image file.");
	return 0;
    } 

    printf("\n");
    
    if ((device = open(deviceName, O_RDWR)) < 0) {
	perror("Can not open I2C Bus");
	return 200;
    }
    if (ioctl(device, I2C_SLAVE, 0x53) < 0) {
	perror("Can not connect to Device");
	return 201;
    }

    for(int i = 0; i < EEPROM_SIZE; ++i) {
	fread(&buffer, sizeof(unsigned char), 1, infile);
	if(i >= SERIAL_NR_START_OFFSET && i <= SERIAL_NR_STOP_OFFSET && snPos <= lenOfSnStr) {
	    buffer[0] = serialStr[snPos];
	    ++snPos;
	} 


	data[0]= (unsigned char) (i >> 8);	// High Byte Address
	data[1]= (unsigned char) i; 		// Low Byte Address
	data[2]= (unsigned char) buffer[0];	// Data Byte
	write(device,data,3);
	printf("\033[sWrite %d%% : addr %02x %02x , data= %x\033[u", i*100/EEPROM_SIZE+1, data[0], data[1], data[2]);
	usleep(5000);
    }
    printf("\n");

    close(device);
    fclose(infile);

    return 1;

} 


int checkEepromImgFileHeader(char *fileName) {
    FILE *fp;
    struct stat st;
    stat(fileName, &st);
    if(st.st_size != EEPROM_SIZE) return 0;
    unsigned char buffer[1] = "\0";

    if ((fp = fopen(fileName, "rb")) < 0) {
	perror("ERROR: cannot open image file.");
	return 0;
    } 
    fread(&buffer, sizeof(unsigned char), 1, fp);
    if(buffer[0] != 0xaa) return 0;

    fread(&buffer, sizeof(unsigned char), 1, fp);
    if(buffer[0] != 0x55) return 0;

    fclose(fp);
    return 1;
}



int main (int argc, char **argv) {

    char deviceName[PATH_MAX];
    char *inputFile = NULL;
    int busID = DEFAULT_MANGOH_RED_IOT_I2C_BUS_ID;
    int c;
    int noInteractive = 0;
    char securityQuestion;
    char serialString[MAX_SN_LEN];
    opterr = 0;

    memset(serialString,0,MAX_SN_LEN);

    while ((c = getopt (argc, argv, "b:f:s:hy")) != -1) {
	switch (c) {
    	    case 'b':
		for (int i=0;i< strlen(optarg); i++) {
    		    if (!isdigit(optarg[i])) {
        		printf ("ERROR: input for BusID is not a number\n");
        		return 1;
    		    }
		}
    		busID = atoi(optarg);
    		break;
    	    case 'f':
    		inputFile = optarg;
    		break;
    	    case 's':
    		if(strlen(optarg) > MAX_SN_LEN - 1) {
		    fprintf (stderr, "ERROR: serial number string too long - max len is %d - but actually given len is %d\n", MAX_SN_LEN -1 , (int)strlen(optarg));
		    return 1;
		}
		strncpy(serialString,optarg,MAX_SN_LEN -1);
    		break;
    	    case 'y':
    		noInteractive = 1;
    		break;
    	    case 'h':
    		printf("usage writeFile2Eeprom [-b <busID>] -f <eeprom image input file> [-y] -s <IoT card serial Number> \n");
    		printf("    -b <busID>                     optional,  the I2C bus ID - on MangOH red it is 5 (default)\n");
    		printf("    -f <eeprom image input file>   mandatory, the image which should be written to eeprom\n");
    		printf("    -y 	                           optional,  it will not ask if you are really sure\n");
    		printf("    -s <IoT Serial Number>         mandatory, a string with max 31 bytes with the serial information\n");
		printf("                                   of the IoT card. BX31 IoT card SN should have following format:\n");
		printf("                                   YYYYMMDDhhmmss_<HW Version>_<NumberInBatch> e.g.: 20190506200700_1.0_001\n");
		return 0;
    	    case '?':
    		if (optopt == 'f' || optopt == 'b') {
		    fprintf (stderr, "ERROR: Option -%c requires an argument.\n", optopt);
    		} else if (isprint (optopt)) {
        	    fprintf (stderr, "ERROR: Unknown option `-%c'.\n", optopt);
    		} else {
        	    fprintf (stderr, "ERROR: Unknown option character `\\x%x'.\n", optopt);
		}
		return 1;
    	    default:
    		abort ();
        }
    }

    if(access( inputFile , F_OK ) == -1 ) {
	fprintf(stderr,"Error: File %s - not found or readable.\n", inputFile);
	return 2;
    }

    if(strnlen(serialString,MAX_SN_LEN) < 1) {
	fprintf(stderr,"Error: Serial string %s - not correct.\n", serialString);
	return 4;

    }

    sprintf(deviceName, "%s%d", I2C_BASE_PATH, busID);

    if(! checkEepromImgFileHeader(inputFile)) {
	fprintf(stderr,"Error: File %s - doesn't seem to be a IoT card EEPROm image file.\n", inputFile);
	return 5;
    }

    printf("Please check carefully:\n");
    printf("  input file        =  %s\n", inputFile);
    printf("  I2C device Name   =  %s\n", deviceName);
    printf("  Device Serial ID  =  %s\n", serialString);
    
    if(!noInteractive) {
	do {
	    printf("Are you really sure to write to this device ? y/n: "); 
	    fflush(stdin);
    	    scanf(" %c",&securityQuestion);
	} while ( securityQuestion != 'n' && securityQuestion != 'y');
	if(securityQuestion == 'n') return 0; 
    }
    
    if(! writeEeprom(inputFile, deviceName, serialString)) { 
	fprintf(stderr,"Error: Failed to process image.\n");
	return 254;
    }

    printf("Successfuly wrote image.\n");
    
}

