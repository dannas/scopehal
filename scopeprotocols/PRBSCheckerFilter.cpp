/***********************************************************************************************************************
*                                                                                                                      *
* libscopeprotocols                                                                                                    *
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

#include "../scopehal/scopehal.h"
#include "PRBSCheckerFilter.h"
#include "PRBSGeneratorFilter.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

PRBSCheckerFilter::PRBSCheckerFilter(const string& color)
	: Filter(OscilloscopeChannel::CHANNEL_TYPE_DIGITAL, color, CAT_ANALYSIS)
	, m_polyname("Polynomial")
{
	CreateInput("Data");
	CreateInput("Clock");

	m_parameters[m_polyname] = FilterParameter(FilterParameter::TYPE_ENUM, Unit(Unit::UNIT_COUNTS));
	m_parameters[m_polyname].AddEnumValue("PRBS-7", PRBSGeneratorFilter::POLY_PRBS7);
	m_parameters[m_polyname].AddEnumValue("PRBS-9", PRBSGeneratorFilter::POLY_PRBS9);
	m_parameters[m_polyname].AddEnumValue("PRBS-11", PRBSGeneratorFilter::POLY_PRBS11);
	m_parameters[m_polyname].AddEnumValue("PRBS-15", PRBSGeneratorFilter::POLY_PRBS15);
	m_parameters[m_polyname].AddEnumValue("PRBS-23", PRBSGeneratorFilter::POLY_PRBS23);
	m_parameters[m_polyname].AddEnumValue("PRBS-31", PRBSGeneratorFilter::POLY_PRBS31);
	m_parameters[m_polyname].SetIntVal(PRBSGeneratorFilter::POLY_PRBS7);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Factory methods

bool PRBSCheckerFilter::ValidateChannel(size_t i, StreamDescriptor stream)
{
	if(stream.m_channel == NULL)
		return false;

	if( (i < 2) && (stream.m_channel->GetType() == OscilloscopeChannel::CHANNEL_TYPE_DIGITAL) )
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Accessors

string PRBSCheckerFilter::GetProtocolName()
{
	return "PRBS Checker";
}

void PRBSCheckerFilter::SetDefaultName()
{
	Unit rate(Unit::UNIT_BITRATE);

	string prefix = "";
	switch(m_parameters[m_polyname].GetIntVal())
	{
		case PRBSGeneratorFilter::POLY_PRBS7:
			prefix = "PRBS7";
			break;

		case PRBSGeneratorFilter::POLY_PRBS9:
			prefix = "PRBS9";
			break;

		case PRBSGeneratorFilter::POLY_PRBS11:
			prefix = "PRBS11";
			break;

		case PRBSGeneratorFilter::POLY_PRBS15:
			prefix = "PRBS15";
			break;

		case PRBSGeneratorFilter::POLY_PRBS23:
			prefix = "PRBS23";
			break;

		case PRBSGeneratorFilter::POLY_PRBS31:
		default:
			prefix = "PRBS31";
			break;
	}

	m_hwname = prefix + "Check(" + GetInputDisplayName(0).c_str() + ")";
	m_displayname = m_hwname;
}

bool PRBSCheckerFilter::NeedsConfig()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actual decoder logic

void PRBSCheckerFilter::Refresh()
{
	if(!VerifyAllInputsOK())
	{
		SetData(NULL, 0);
		return;
	}

	//Sample the input data stream
	//TODO: allow single rate clocks too?
	auto din = GetDigitalInputWaveform(0);
	auto clkin = GetDigitalInputWaveform(1);
	DigitalWaveform data;
	SampleOnAnyEdges(din, clkin, data);

	auto poly = static_cast<PRBSGeneratorFilter::Polynomials>(m_parameters[m_polyname].GetIntVal());

	//Figure out how many bits of state we need
	size_t statesize = poly;

	//Need at least the state size worth of data bits to do a meaningful check
	auto len = data.m_samples.size();
	if(len < statesize)
	{
		SetData(NULL, 0);
		return;
	}

	//Create the output "error found" waveform
	auto dout = dynamic_cast<DigitalWaveform*>(GetData(0));
	if(!dout)
	{
		dout = new DigitalWaveform;
		SetData(dout, 0);
	}
	dout->m_timescale = 1;
	dout->m_triggerPhase = 0;
	dout->m_startTimestamp = data.m_startTimestamp;
	dout->m_startFemtoseconds = data.m_startFemtoseconds;
	dout->m_densePacked = true;
	dout->Resize(len);

	//Read the first N bits of state into the seed
	uint32_t prbs = 0;
	for(size_t i=0; i<statesize; i++)
	{
		prbs = (prbs << 1) | data.m_samples[i];

		dout->m_offsets[i] = data.m_offsets[i];
		dout->m_durations[i] = data.m_durations[i];
		dout->m_samples[i] = 0;
	}

	//Start checking actual data bits
	for(size_t i=statesize; i<len; i++)
	{
		dout->m_offsets[i] = data.m_offsets[i];
		dout->m_durations[i] = data.m_durations[i];

		bool value = PRBSGeneratorFilter::RunPRBS(prbs, poly);
		dout->m_samples[i] = (value != data.m_samples[i]);
	}
}
