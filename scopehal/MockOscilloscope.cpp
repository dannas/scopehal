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

/**
	@file
	@author Andrew D. Zonenberg
	@brief Implementation of MockOscilloscope
 */

#include "scopehal.h"
#include "OscilloscopeChannel.h"
#include "MockOscilloscope.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

MockOscilloscope::MockOscilloscope(const string& name, const string& vendor, const string& serial)
	: m_name(name)
	, m_vendor(vendor)
	, m_serial(serial)
	, m_extTrigger(NULL)
{
}

MockOscilloscope::~MockOscilloscope()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Information queries

bool MockOscilloscope::IsOffline()
{
	return true;
}

string MockOscilloscope::IDPing()
{
	return "";
}

string MockOscilloscope::GetTransportName()
{
	return "null";
}

string MockOscilloscope::GetTransportConnectionString()
{
	return "";
}

string MockOscilloscope::GetDriverNameInternal()
{
	return "mock";
}

unsigned int MockOscilloscope::GetInstrumentTypes()
{
	return INST_OSCILLOSCOPE;
}

string MockOscilloscope::GetName()
{
	return m_name;
}

string MockOscilloscope::GetVendor()
{
	return m_vendor;
}

string MockOscilloscope::GetSerial()
{
	return m_serial;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Triggering

Oscilloscope::TriggerMode MockOscilloscope::PollTrigger()
{
	//we never trigger
	return TRIGGER_MODE_STOP;
}

bool MockOscilloscope::AcquireData()
{
	//no new data possible
	return false;
}

void MockOscilloscope::ArmTrigger()
{
	//no-op, we never trigger
}

void MockOscilloscope::StartSingleTrigger()
{
	//no-op, we never trigger
}

void MockOscilloscope::Start()
{
	//no-op, we never trigger
}

void MockOscilloscope::Stop()
{
	//no-op, we never trigger
}

void MockOscilloscope::ForceTrigger()
{
	//no-op, we never trigger
}

bool MockOscilloscope::IsTriggerArmed()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization

void MockOscilloscope::LoadConfiguration(const YAML::Node& node, IDTable& table)
{
	//Load the channels
	auto& chans = node["channels"];
	for(auto it : chans)
	{
		auto& cnode = it.second;

		//Allocate channel space if we didn't have it yet
		size_t index = cnode["index"].as<int>();
		if(m_channels.size() < (index+1))
			m_channels.resize(index+1);

		//Configure the channel
		OscilloscopeChannel::ChannelType type = OscilloscopeChannel::CHANNEL_TYPE_COMPLEX;
		string stype = cnode["type"].as<string>();
		if(stype == "analog")
			type = OscilloscopeChannel::CHANNEL_TYPE_ANALOG;
		else if(stype == "digital")
			type = OscilloscopeChannel::CHANNEL_TYPE_DIGITAL;
		else if(stype == "trigger")
			type = OscilloscopeChannel::CHANNEL_TYPE_TRIGGER;
		auto chan = new OscilloscopeChannel(
			this,
			cnode["name"].as<string>(),
			type,
			cnode["color"].as<string>(),
			index,
			true);
		m_channels[index] = chan;

		//Create the channel ID
		table.emplace(cnode["id"].as<int>(), chan);
	}

	//Call the base class to configure everything
	Oscilloscope::LoadConfiguration(node, table);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Channel configuration. Mostly trivial stubs.

bool MockOscilloscope::IsChannelEnabled(size_t i)
{
	return m_channelsEnabled[i];
}

void MockOscilloscope::EnableChannel(size_t i)
{
	m_channelsEnabled[i] = true;
}

void MockOscilloscope::DisableChannel(size_t i)
{
	m_channelsEnabled[i] = false;
}

vector<OscilloscopeChannel::CouplingType> MockOscilloscope::GetAvailableCouplings(size_t /*i*/)
{
	vector<OscilloscopeChannel::CouplingType> ret;
	ret.push_back(OscilloscopeChannel::COUPLE_DC_1M);
	ret.push_back(OscilloscopeChannel::COUPLE_AC_1M);
	ret.push_back(OscilloscopeChannel::COUPLE_DC_50);
	ret.push_back(OscilloscopeChannel::COUPLE_GND);
	//TODO: other options? or none?
	return ret;
}

OscilloscopeChannel::CouplingType MockOscilloscope::GetChannelCoupling(size_t i)
{
	return m_channelCoupling[i];
}

void MockOscilloscope::SetChannelCoupling(size_t i, OscilloscopeChannel::CouplingType type)
{
	m_channelCoupling[i] = type;
}

double MockOscilloscope::GetChannelAttenuation(size_t i)
{
	return m_channelAttenuation[i];
}

void MockOscilloscope::SetChannelAttenuation(size_t i, double atten)
{
	m_channelAttenuation[i] = atten;
}

int MockOscilloscope::GetChannelBandwidthLimit(size_t i)
{
	return m_channelBandwidth[i];
}

void MockOscilloscope::SetChannelBandwidthLimit(size_t i, unsigned int limit_mhz)
{
	m_channelBandwidth[i] = limit_mhz;
}

float MockOscilloscope::GetChannelVoltageRange(size_t i, size_t stream)
{
	return m_channelVoltageRange[pair<size_t, size_t>(i, stream)];
}

void MockOscilloscope::SetChannelVoltageRange(size_t i, size_t stream, float range)
{
	m_channelVoltageRange[pair<size_t, size_t>(i, stream)] = range;
}

OscilloscopeChannel* MockOscilloscope::GetExternalTrigger()
{
	return m_extTrigger;
}

float MockOscilloscope::GetChannelOffset(size_t i, size_t stream)
{
	return m_channelOffset[pair<size_t, size_t>(i, stream)];
}

void MockOscilloscope::SetChannelOffset(size_t i, size_t stream, float offset)
{
	m_channelOffset[pair<size_t, size_t>(i, stream)] = offset;
}

vector<uint64_t> MockOscilloscope::GetSampleRatesNonInterleaved()
{
	//no-op
	vector<uint64_t> ret;
	return ret;
}

vector<uint64_t> MockOscilloscope::GetSampleRatesInterleaved()
{
	//no-op
	vector<uint64_t> ret;
	return ret;
}

set<Oscilloscope::InterleaveConflict> MockOscilloscope::GetInterleaveConflicts()
{
	//no-op
	set<Oscilloscope::InterleaveConflict> ret;
	return ret;
}

vector<uint64_t> MockOscilloscope::GetSampleDepthsNonInterleaved()
{
	//no-op
	vector<uint64_t> ret;
	return ret;
}

vector<uint64_t> MockOscilloscope::GetSampleDepthsInterleaved()
{
	//no-op
	vector<uint64_t> ret;
	return ret;
}

uint64_t MockOscilloscope::GetSampleRate()
{
	return 1;
}

uint64_t MockOscilloscope::GetSampleDepth()
{
	//FIXME
	return 1;
}

void MockOscilloscope::SetSampleDepth(uint64_t /*depth*/)
{
	//no-op
}

void MockOscilloscope::SetSampleRate(uint64_t /*rate*/)
{
	//no-op
}

void MockOscilloscope::SetTriggerOffset(int64_t /*offset*/)
{
	//FIXME
}

int64_t MockOscilloscope::GetTriggerOffset()
{
	//FIXME
	return 0;
}

bool MockOscilloscope::IsInterleaving()
{
	return false;
}

bool MockOscilloscope::SetInterleaving(bool /*combine*/)
{
	return false;
}

void MockOscilloscope::PushTrigger()
{
	//no-op
}

void MockOscilloscope::PullTrigger()
{
	//no-op
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Import a waveform from file

/**
	@brief Imports waveforms from Agilent/Keysight/Rigol binary capture files
 */
bool MockOscilloscope::LoadBIN(const string& path)
{
	LogTrace("Importing BIN file \"%s\"\n", path.c_str());
	LogIndenter li_f;

	string f = ReadFile(path);
	uint32_t fpos = 0;

	FileHeader fh;
	f.copy((char*)&fh, sizeof(FileHeader), fpos);
	fpos += sizeof(FileHeader);

	//Get vendor from file signature
	switch(fh.magic[0])
	{
		case 'A':
			m_vendor = "Agilent/Keysight";
			break;

		case 'R':
			m_vendor = "Rigol";
			break;

		default:
			LogError("Unknown file format");
			return false;
	}

	LogDebug("Vendor:    %s\n", m_vendor.c_str());
	//LogDebug("File size: %i bytes\n", fh.length);
	LogDebug("Waveforms: %i\n\n", fh.count);

	//Load waveforms
	for(size_t i=0; i<fh.count; i++)
	{
		LogDebug("Waveform %i:\n", (int)i+1);
		LogIndenter li_w;

		//Parse waveform header
		WaveHeader wh;
		f.copy((char*)&wh, sizeof(WaveHeader), fpos);
		fpos += sizeof(WaveHeader);

		// Only set name/serial on first waveform
		if (i == 0)
		{
			// Split hardware string
			int idx = 0;
			for(int c=0; c<24; c++)
			{
				if(wh.hardware[c] == ':')
				{
					idx = c;
					break;
				}
			}

			//Set oscilloscope metadata
			m_name.assign(wh.hardware, idx);
			m_serial.assign(wh.hardware + idx + 1, 24 - idx);
		}

		LogDebug("Samples:      %i\n", wh.samples);
		LogDebug("Buffers:      %i\n", wh.buffers);
		LogDebug("Type:         %i\n", wh.type);
		LogDebug("Duration:     %.*f us\n", 2, wh.duration * 1e6);
		LogDebug("Start:        %.*f us\n", 2, wh.start * 1e6);
		LogDebug("Interval:     %.*f ns\n", 2, wh.interval * 1e9);
		LogDebug("Origin:       %.*f us\n", 2, wh.origin * 1e6);
		LogDebug("Holdoff:      %.*f ms\n", 2, wh.holdoff * 1e3);
		LogDebug("Sample Rate:  %.*f Msps\n", 2, (1 / wh.interval) / 1e6);
		LogDebug("Frame:        %s\n", m_name.c_str());
		LogDebug("Serial:       %s\n\n", m_serial.c_str());

		// Create new channel
		auto chan = new OscilloscopeChannel(
			this,			//Parent scope
			wh.label,		//Channel name
			OscilloscopeChannel::CHANNEL_TYPE_ANALOG,
			GetDefaultChannelColor(i),
			units[wh.x],
			units[wh.y],
			i,				//Channel index
			true			//Is physical channel
		);
		AddChannel(chan);
		chan->SetDefaultDisplayName();

		//Create new waveform for channel
		auto wfm = new AnalogWaveform;
		wfm->m_timescale = wh.interval * 1e15;
		wfm->m_startTimestamp = 0;
		wfm->m_startFemtoseconds = 0;
		wfm->m_triggerPhase = 0;
		chan->SetData(wfm, 0);

		//Loop through waveform buffers
		float vmin = FLT_MAX;
		float vmax = -FLT_MAX;
		for(size_t j=0; j<wh.buffers; j++)
		{
			LogDebug("Buffer %i:\n", (int)j+1);
			LogIndenter li_b;

			//Parse waveform data header
			DataHeader dh;
			f.copy((char*)&dh, sizeof(DataHeader), fpos);
			fpos += sizeof(DataHeader);

    		LogDebug("Data Type:      %i\n", dh.type);
    		LogDebug("Sample depth:   %i bits\n", dh.depth*8);
    		LogDebug("Buffer length:  %i KB\n\n\n", dh.length/1024);

			//Loop through waveform samples
			float sample = 0;
			if (dh.type == 6)
			{
				//Integer samples (digital waveforms)
				uint8_t* sample_i = nullptr;
				for(size_t k=0; k<wh.samples; k++)
				{
					sample_i = (uint8_t*)(f.c_str() + fpos);
					sample = (float)*sample_i;

					//Push sample to waveform
					wfm->m_offsets.push_back(k);
					wfm->m_samples.push_back(sample);
					wfm->m_durations.push_back(1);

					//Update voltage min/max values
					vmax = max(vmax, sample);
					vmin = min(vmin, sample);

					fpos += dh.depth;
				}
			}
			else
			{
				//Float samples (analog waveforms)
				float* sample_f = nullptr;
				for(size_t k=0; k<wh.samples; k++)
				{
					sample_f = (float*)(f.c_str() + fpos);
					sample = *sample_f;

					//Push sample to waveform
					wfm->m_offsets.push_back(k);
					wfm->m_samples.push_back(sample);
					wfm->m_durations.push_back(1);

					//Update voltage min/max values
					vmax = max(vmax, sample);
					vmin = min(vmin, sample);

					fpos += dh.depth;
				}
			}
		}

		//Calculate offset and range
		chan->SetVoltageRange((vmax-vmin) * 1.5, 0);
		chan->SetOffset(-((vmax-abs(vmin)) / 2), 0);
	}

	return true;
}

/**
	@brief Calculate min/max of each channel and adjust gain/offset accordingly
 */
void MockOscilloscope::AutoscaleVertical()
{
	for(auto c : m_channels)
	{
		auto wfm = dynamic_cast<AnalogWaveform*>(c->GetData(0));
		if(!wfm)
			continue;
		if(wfm->m_samples.empty())
			continue;

		float vmin = wfm->m_samples[0];
		float vmax = vmin;

		for(auto s : wfm->m_samples)
		{
			vmin = min(vmin, (float)s);
			vmax = max(vmax, (float)s);
		}

		//Calculate bounds
		c->SetVoltageRange((vmax - vmin) * 1.05, 0);
		c->SetOffset( -( (vmax - vmin)/2 + vmin ), 0);
	}
}
