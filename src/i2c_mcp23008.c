#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef enum MCP230xxPinMode
{
    MCP23008_PM_INPUT,
    MCP23008_PM_OUTPUT
}MCP_PIN_MODE;

typedef enum MCP230xxRegisterAddresses
{
    MCP23017_IODIRA = 0x00,
    MCP23017_IODIRB = 0x01,
    MCP23017_GPPUA  = 0x0C,
    MCP23017_GPPUB  = 0x0D,
    MCP23017_GPIOA  = 0x12,
    MCP23017_GPIOB  = 0x13,
    MCP23017_OLATA  = 0x14,
    MCP23017_OLATB  = 0x15,
    MCP23008_IODIR  = 0x00,
    MCP23008_GPIO   = 0x09,
    MCP23008_GPPU   = 0x06,
    MCP23008_OLAT   = 0x0A,
}MCP_REG_ADDR;

int num_gpios;
int pullups;
unsigned char outputvalue;
unsigned char direction;
int i2cdev;
char *devfn = "/dev/i2c-0";
int addr = 0x20;  // The I2C address of the ADC

int init(int address, int num_gpios, int busnum, int debug)
{
    // Set default pin values -- all inputs with pull-ups disabled.
    // Current OLAT (output) value is polled, not set.
    direction = 0xFF;
    pullups = 0;
  
    // Attempt to open the i2c device 
    if( ( i2cdev = open( devfn, O_RDWR ) ) < 0 ) 
    {
        perror("Failed to open the i2c bus");
        exit(1);
    }

    // Tell the device which endpoint we want to talk to.
    if( ioctl( i2cdev, I2C_SLAVE, addr ) < 0 )
    {
        printf( "Failed to acquire bus access and/or talk to slave.\n" );
        /* ERROR HANDLING; you can check errno to see what went wrong */
       exit( 1 );
    }

    // Init the IO direction and pullup settings
    i2c_smbus_write_byte_data( i2cdev, MCP23008_IODIR, direction );
    i2c_smbus_write_byte_data( i2cdev, MCP23008_GPPU, pullups );

    // Read and intial value from the device.
    outputvalue = i2c_smbus_read_byte_data( i2cdev, MCP23008_OLAT );

    printf( "Initial Value Read: %d\n", outputvalue );
}

// Set single pin to either INPUT or OUTPUT mode
int config( int pin, int mode )
{
    if( mode == MCP23008_PM_INPUT )
    {
        direction |= (1 << pin);
    }
    else
    {
        direction &= ~(1 << pin);
    }

    // Update the register
    i2c_smbus_write_byte_data( i2cdev, MCP23008_IODIR, direction );

    return direction;
}

int pullup( int pin, int enable, int check )
{
    if( check )
    {
        if( ( direction & (1 << pin) ) != 0 )
        {
            fprintf(stderr, "Pin %d not set to input", pin );
            exit(1);
        }
    }

    if( enable )
    {
        pullups |= (1 << pin);
    }
    else
    {
        pullups &= ~(1 << pin);
    }

    i2c_smbus_write_byte_data( i2cdev, MCP23008_GPPU, pullups );

    return pullups;
}

int input( int pin, int check )
{
    int value;

    if( check )
    {
        if( ( direction & (1 << pin) ) != 0 )
        {
            fprintf(stderr, "Pin %d not set to input", pin );
            exit(1);
        }
    }

    value = i2c_smbus_read_byte_data( i2cdev, MCP23008_GPIO );

    return (value >> pin) & 1;
}

int output( int pin, int value )
{
    int new;

    if( value )
    {
        new = outputvalue | (1 << pin);
    }
    else
    {
        new = outputvalue & ~(1 << pin);
    }

    // Only write if pin value has changed:
    if( new != outputvalue )
    {
        outputvalue = new;
        i2c_smbus_write_byte_data( i2cdev, MCP23008_OLAT, new );
    }

    return new;
}

int i2ctest()
{
    int file;
    char *filename = "/dev/i2c-0";
    char buf[10] = {0};
    float data;
    char channel;
    int i;
   
    init( 0x20, 8, 0, 1 );

    // Set pins 0, 1, 2 as outputs
    config( 0, MCP23008_PM_OUTPUT );
    config( 1, MCP23008_PM_OUTPUT );
    config( 2, MCP23008_PM_OUTPUT );
    
    // Set pin 3 to input with the pullup resistor enabled
    pullup( 3, 1, 0 );

    // Read pin 3 and display the results
    printf( "%d: %x\n", 3, input( 3, 0 ) );
    
    // Python speed test on output 0 toggling at max speed
    while(1)
    {
        output( 0, 1 );
        output( 1, 0 );
        sleep(1);
        output( 0, 0 );
        output( 1, 1 );
        sleep(1);
    }
 
#if 0
    if( (file = open(filename, O_RDWR)) < 0) 
    {
        perror("Failed to open the i2c bus");
        exit(1);
    }

    int addr = 0x20;  // The I2C address of the ADC
    if( ioctl( file, I2C_SLAVE, addr ) < 0 )
    {
        printf( "Failed to acquire bus access and/or talk to slave.\n" );
        /* ERROR HANDLING; you can check errno to see what went wrong */
       exit( 1 );
    }

    for (i = 0; i<4; i++)
    {
        // Using I2C Read
        if( read( file, buf, 2 ) != 2 ) 
        {
            /* ERROR HANDLING: i2c transaction failed */
            printf("Failed to read from the i2c bus.\n");
            perror("Failed read"); 
            printf("\n\n");
        }
        else
        {
            data = (float)((buf[0] & 0b00001111)<<8)+buf[1];
            data = data/4096*5;
            channel = ((buf[0] & 0b00110000)>>4);
            printf("Channel %02d Data:  %04f\n",channel,data);
        }
    }
#endif
}

