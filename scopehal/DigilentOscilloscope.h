/***********************************************************************************************************************
*                                                                                                                      *
* libscopehal v0.1                                                                                                     *
*                                                                                                                      *
* Copyright (c) 2012-2022 Andrew D. Zonenberg and contributors                                                         *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

#ifndef DigilentOscilloscope_h
#define DigilentOscilloscope_h

#include "RemoteBridgeOscilloscope.h"

/**
	@brief DigilentOscilloscope - driver for talking to the scopehal-waveforms-bridge daemon (wfmserver)
 */
class DigilentOscilloscope : public RemoteBridgeOscilloscope
{
public:
	DigilentOscilloscope(SCPITransport* transport);
	virtual ~DigilentOscilloscope();

	//not copyable or assignable
	DigilentOscilloscope(const DigilentOscilloscope& rhs) =delete;
	DigilentOscilloscope& operator=(const DigilentOscilloscope& rhs) =delete;

public:

	//Device information
	virtual unsigned int GetInstrumentTypes();

	virtual void FlushConfigCache();

	//Channel configuration
	virtual std::vector<OscilloscopeChannel::CouplingType> GetAvailableCouplings(size_t i);
	virtual double GetChannelAttenuation(size_t i);
	virtual void SetChannelAttenuation(size_t i, double atten);
	virtual int GetChannelBandwidthLimit(size_t i);
	virtual void SetChannelBandwidthLimit(size_t i, unsigned int limit_mhz);
	virtual OscilloscopeChannel* GetExternalTrigger();
	virtual bool CanEnableChannel(size_t i);

	//Triggering
	virtual Oscilloscope::TriggerMode PollTrigger();
	virtual bool AcquireData();
	virtual void PushTrigger();

	//Timebase
	virtual std::vector<uint64_t> GetSampleRatesNonInterleaved();
	virtual std::vector<uint64_t> GetSampleRatesInterleaved();
	virtual std::set<InterleaveConflict> GetInterleaveConflicts();
	virtual std::vector<uint64_t> GetSampleDepthsNonInterleaved();
	virtual std::vector<uint64_t> GetSampleDepthsInterleaved();
	virtual bool IsInterleaving();
	virtual bool SetInterleaving(bool combine);

	//ADC configuration
	virtual std::vector<AnalogBank> GetAnalogBanks();
	virtual AnalogBank GetAnalogBank(size_t channel);
	virtual bool IsADCModeConfigurable();
	virtual std::vector<std::string> GetADCModeNames(size_t channel);
	virtual size_t GetADCMode(size_t channel);
	virtual void SetADCMode(size_t channel, size_t mode);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Logic analyzer configuration

	virtual std::vector<DigitalBank> GetDigitalBanks();
	virtual DigitalBank GetDigitalBank(size_t channel);
	virtual bool IsDigitalHysteresisConfigurable();
	virtual bool IsDigitalThresholdConfigurable();
	virtual float GetDigitalHysteresis(size_t channel);
	virtual float GetDigitalThreshold(size_t channel);
	virtual void SetDigitalHysteresis(size_t channel, float level);
	virtual void SetDigitalThreshold(size_t channel, float level);

	enum Series
	{
		SERIES_ANALOG_DISCOVERY,
		SERIES_ANALOG_DISCOVERY_2,
		SERIES_DIGITAL_DISCOVERY,
		SERIES_ANALOG_DISCOVERY_PRO,

		SERIES_UNKNOWN
	};

protected:
	void IdentifyHardware();

	std::string GetChannelColor(size_t i);

	//hardware analog channel count, independent of LA option etc
	size_t m_analogChannelCount;
	size_t m_digitalChannelBase;
	size_t m_digitalChannelCount;

	//OscilloscopeChannel* m_extTrigChannel;

	//Most SCPI API calls are write only, so we have to maintain all state clientside.
	//This isn't strictly a cache anymore since it's never flushed!
	std::map<size_t, double> m_channelAttenuations;

	Series m_series;

public:

	static std::string GetDriverNameInternal();
	OSCILLOSCOPE_INITPROC(DigilentOscilloscope)
};

#endif
