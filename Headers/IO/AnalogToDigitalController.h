#ifndef ANALOGTODIGITALCONTROLLER_H
#define ANALOGTODIGITALCONTROLLER_H

namespace LibBBB  {

/**
 * @brief The AnalogToDigitalController class controls the BBB's ADC controller. It
 *		  provides all functionality to setup and get values for all ADC channels.
 */
class AnalogToDigitalController
{
public:

	/**
	 * @brief The Channel struct contains all of the valid ADC channels
	 */
	struct Channel
	{
		enum Enum
		{
			One = 1,
			Two,
			Three,
			Four,
			Five,
			Six,
			Seven
		};
	};

private:

	/**
	 * @brief AnalogToDigitalController private constructor to create a singleton.
	 */
	AnalogToDigitalController();

public:

	/**
	 * @brief ~AnalogToDigitalController do nothing destructor.
	 */
	~AnalogToDigitalController();

	/**
	 * @brief	instance method is the Singleton method of creating and accessing this
	 *			class. This ensures there can only be ONE instance of this class. If there
	 *			are two instances of this class created, it could mess with the virtual
	 *			memory mapping!!
	 * @note	https://en.wikipedia.org/wiki/Singleton_pattern
	 * @return reference to this class.
	 */
	static AnalogToDigitalController& instance();

	/**
	 * @brief rawADCValue gets the raw (unscaled) ADC value
	 * @param channel
	 * @return
	 */
	int rawADCValue( Channel::Enum channel );

	/**
	 * @brief adcVoltage gets the ADC channel's unscaled value
	 * @param channel
	 * @return
	 */
	float adcVoltage( Channel::Enum channel );

	/**
	 * @brief batteryVoltage gets the LiPo battery's voltage
	 * @return LiPo battery's voltage
	 */
	float batteryVoltage();

private:

	/**
	 * @brief initializeMemoryMap method initialzes the virtual memory space
	 * @note This must be called BEFORE initializeMemoryMapADC()
	 * @return false on success, true on failure
	 */
	int initializeMemoryMap();

	/**
	 * @brief initializeMemoryMapADC initialzes the virtual memory space of the ADC
	 * @note This must be called AFTER initializeMemoryMap()
	 * @return false on success, true on failure
	 */
	int initializeMemoryMapADC();
};

} // namespace LibBBB
#endif // ANALOGTODIGITALCONTROLLER_H
