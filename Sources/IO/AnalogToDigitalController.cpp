#include "AnalogToDigitalController.h"

#include <sys/mman.h>
#include "ADCMacros.h"
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include <stdlib.h>

volatile uint32_t *map; // pointer to /dev/mem

namespace LibBBB  {

AnalogToDigitalController::AnalogToDigitalController()
{
	if ( initializeMemoryMap() )
	{
		exit( EXIT_FAILURE );
		fprintf( stderr, "Could not initialize virtual memory\n");
	}

	if ( initializeMemoryMapADC() )
	{
		exit( EXIT_FAILURE );
		fprintf( stderr, "Could not initialize the ADC's virtual memory\n");
	}
}

AnalogToDigitalController::~AnalogToDigitalController()
{

}

AnalogToDigitalController& AnalogToDigitalController::instance()
{
    static AnalogToDigitalController inst;
    return inst;
}

int AnalogToDigitalController::rawADCValue( Channel::Enum channel )
{
    // clear the FIFO buffer just in case it's not empty
    int output;
    while(map[ (FIFO0COUNT-MMAP_OFFSET)/4] & FIFO_COUNT_MASK )
    {
        output = map[(ADC_FIFO0DATA-MMAP_OFFSET)/4] & ADC_FIFO_MASK;
    }

    // enable step for the right pin
	map[ (ADC_STEPENABLE-MMAP_OFFSET)/4] |= ( 0x01 << ( channel ) );

    // wait for sample to appear in the FIFO buffer
	while( !( map[(FIFO0COUNT-MMAP_OFFSET)/4] & FIFO_COUNT_MASK ) )
    {	}

    // return the the FIFO0 data register
    output =  map[(ADC_FIFO0DATA-MMAP_OFFSET)/4] & ADC_FIFO_MASK;
    return output;
}

float AnalogToDigitalController::adcVoltage( Channel::Enum channel )
{
    int raw_adc = rawADCValue( channel );
    return raw_adc * 1.8 / 4095.0;
}

float AnalogToDigitalController::batteryVoltage()
{
	return ( adcVoltage( (Channel::Enum)LIPO_ADC_CH ) * V_DIV_RATIO ) + LIPO_OFFSET;
}

int AnalogToDigitalController::initializeMemoryMap()
{
    int devMemFileDescriptor = open( "/dev/mem", O_RDWR );
    errno = 0;
    if ( devMemFileDescriptor == -1 )
    {
        return 1;
    }

    map = (uint32_t*)mmap( NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, devMemFileDescriptor, MMAP_OFFSET );
    if ( map == MAP_FAILED )
    {
        close( devMemFileDescriptor );
        return 1;
    }

    return 0;
}

int AnalogToDigitalController::initializeMemoryMapADC()
{
    map[(CM_WKUP+CM_WKUP_ADC_TSC_CLKCTRL-MMAP_OFFSET)/4] |= MODULEMODE_ENABLE;

    // waiting for adc clock module to initialize
    while ( !( map[ (CM_WKUP+CM_WKUP_ADC_TSC_CLKCTRL-MMAP_OFFSET)/4] & MODULEMODE_ENABLE) )
    {
        usleep(10);
    }

    map[(ADC_CTRL-MMAP_OFFSET)/4] &= !0x01;

    // make sure STEPCONFIG write protect is off
    map[(ADC_CTRL-MMAP_OFFSET)/4] |= ADC_STEPCONFIG_WRITE_PROTECT_OFF;

    // set up each ADCSTEPCONFIG for each ain pin
    map[(ADCSTEPCONFIG1-MMAP_OFFSET)/4] = 0x00<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY1-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG2-MMAP_OFFSET)/4] = 0x01<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY2-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG3-MMAP_OFFSET)/4] = 0x02<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY3-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG4-MMAP_OFFSET)/4] = 0x03<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY4-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG5-MMAP_OFFSET)/4] = 0x04<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY5-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG6-MMAP_OFFSET)/4] = 0x05<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY6-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG7-MMAP_OFFSET)/4] = 0x06<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY7-MMAP_OFFSET)/4]  = 0<<24;
    map[(ADCSTEPCONFIG8-MMAP_OFFSET)/4] = 0x07<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
    map[(ADCSTEPDELAY8-MMAP_OFFSET)/4]  = 0<<24;

    // enable the ADC
    map[(ADC_CTRL-MMAP_OFFSET)/4] |= 0x01;

    // clear the FIFO buffer
    int output;
    while ( map[(FIFO0COUNT-MMAP_OFFSET)/4] & FIFO_COUNT_MASK )
    {
        output = map[(ADC_FIFO0DATA-MMAP_OFFSET)/4] & ADC_FIFO_MASK;
        output = output;
    }

    return 0;
}
} // namespace LibBBB
