#include "ManagerPRUs.h"

#include "PRUDefinitions.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define FILE_ERROR ( -1 )

#include <stdexcept>
#include <stdio.h>

namespace LibBBB  {

ManagerPRUs::ManagerPRUs()
	: _pruRunning( false )
	, _pruSharedMemoryPointer( NULL )

{
	// if there is no problem binding, set the running flag to true.
	_pruRunning = bindOrUnbindPRU( true ) ? false : true;

	// map the virtual address space
	mapVirtualAddressSpace();
}

ManagerPRUs::~ManagerPRUs()
{
	// unbind on exit
	bindOrUnbindPRU( false );
}

ManagerPRUs& ManagerPRUs::instance()
{
	static ManagerPRUs inst;
	return inst;
}

int ManagerPRUs::bindOrUnbindPRU( bool bind )
{
	// make a fd
	int fileDescriptor;

	// check whether the PRU is being bound or unbound
	const char* path = bind ? PRU_BIND_PATH : PRU_UNBIND_PATH;

	// open the path
	fileDescriptor = open( path, O_WRONLY );

	// check if the file was opened properly
	if ( fileDescriptor == FILE_ERROR )
	{
		// if not, just throw because the app cannot run without the
		// PRU, so the error needs fixed ASAP
		throw std::invalid_argument( "Cannot open PRU path!!!!" );
	}

	// if the rproc is not running, this file may not exist,
	// so test if the uevent file exist.
	if ( access( PRU0_UEVENT, F_OK ) != 0 )
	{
		// open the correct binding path, and check for success
		if ( write( fileDescriptor, PRU0_NAME, strlen( PRU0_NAME ) ) < 0 )
		{
			// if not, just throw because the app cannot run without the
			// PRU, so the error needs fixed ASAP
			throw std::invalid_argument( "Cannot access PRU0!!!!" );
		}
	}
//	else TODO: Why is this getting here?
//	{
//		printf("0 access failed\n");
//	}

	// if the rproc is not running, this file may not exist,
	// so test if the uevent file exist.
	if ( access( PRU1_UEVENT, F_OK ) != 0 )
	{
		// open the correct binding path, and check for success
		if ( write( fileDescriptor, PRU1_NAME, strlen( PRU1_NAME ) ) < 0 )
		{
			// if not, just throw because the app cannot run without the
			// PRU, so the error needs fixed ASAP
			throw std::invalid_argument( "Cannot access PRU1!!!!" );
		}
	}
//	else TODO: Why is this getting here?
//	{
//		printf("1 access failed\n" );
//	}

	// close the file
	close( fileDescriptor );

	// set the running flag
	_pruRunning = bind;

	return 0;
}

int ManagerPRUs::mapVirtualAddressSpace()
{
	// create the pointer
	uint32_t* pru;

	// open the /dev/mem file
	int memFileDescriptor = open( "/dev/mem", O_RDWR | O_SYNC );

	// check if the file was opened properly
	if ( memFileDescriptor == FILE_ERROR )
	{
		// if not, just throw because the app cannot run without the
		// PRU, so the error needs fixed ASAP
		throw std::invalid_argument( "Cannot open /dev/mem!!!!" );
	}

	// map the memory and cast the void pointer into a uint32_t pointer
	pru = static_cast< uint32_t* >( mmap( 0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, memFileDescriptor, PRU_ADDR ) );

	// check to see if the memory was mapped correctly
	if ( pru == MAP_FAILED )
	{
		// if not, just throw because the app cannot run without the
		// PRU, so the error needs fixed ASAP
		throw std::invalid_argument( "Cannot map /dev/mem!!!!" );
	}

	// close the fd
	close( memFileDescriptor );

	// set up the shared memory pointer
	_pruSharedMemoryPointer = pru + PRU_SHAREDMEM / 4;

	// set 36 bytes of _pruSharedMemoryPointer to zero
	memset( _pruSharedMemoryPointer, 0, 9 * 4 );

	// return zero
	return 0;
}


int ManagerPRUs::restartPRUs()
{
	int returnValue = 0;

	// bind then unbind the PRUs
	returnValue |= bindOrUnbindPRU( false );
	returnValue |= bindOrUnbindPRU( true );
	return returnValue;
}

int ManagerPRUs::setPRUState( bool on )
{
	// if the state is being set to the current state
	if ( _pruRunning == on )
	{
		// return error
		return -1;
	}

	// otherwise, set the desired state
	return bindOrUnbindPRU( on );
}

uint32_t* const ManagerPRUs::sharedMemoryPointer()
{
	return _pruSharedMemoryPointer;
}

const uint32_t* const ManagerPRUs::sharedMemoryPointer() const
{
	return _pruSharedMemoryPointer;
}
} // namespace LibBBB
