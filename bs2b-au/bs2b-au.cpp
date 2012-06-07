// This file is based on the file stereo simulator plugin ('stereo.cpp') 
// in the AU version of the MDA plugins [1,2]

// [1] http://mda.smartelectronix.com/,
// [2] http://sourceforge.net/projects/mda-vst/

// Copyright (c) 2008 Paul Kellett, modifications (c) 2012 Leonard Wolf
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



#include "AUEffectBase.h"

#include "bs2b-common.h"
#include "bs2bclass.h"


//--------------------------------------------------------------------------------
enum
{
	kParam_Level_Feed,
	kParam_Level_Fcut,
	kNumParams
};

const SInt16 kNumInputs = 2;
const SInt16 kNumOutputs = 2;

const double kBufferSize_Seconds = 0.1;


//--------------------------------------------------------------------------------
class BS2B : public AUEffectBase
{
public:
	BS2B(AudioComponentInstance inComponentInstance);
    
	virtual OSStatus Initialize();
	virtual void Cleanup();
	virtual OSStatus Reset(AudioUnitScope inScope, AudioUnitElement inElement);
	virtual OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo);
	virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef * outStrings);
	virtual bool SupportsTail()
    {	return true;	}
	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
	virtual OSStatus Version()
    {	return PLUGIN_VERSION;	}
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);
    
private:
    bs2b_base bs2b;
};



//--------------------------------------------------------------------------------
AUDIOCOMPONENT_ENTRY(AUBaseFactory, BS2B)  // needed for lion: TN2276

//--------------------------------------------------------------------------------
BS2B::BS2B(AudioComponentInstance inComponentInstance)
: AUEffectBase(inComponentInstance, false)
{    
	for (AudioUnitParameterID i=0; i < kNumParams; i++)
	{
		AudioUnitParameterInfo paramInfo;
		OSStatus status = GetParameterInfo(kAudioUnitScope_Global, i, paramInfo);
		if (status == noErr)
			SetParameter(i, paramInfo.defaultValue);
	}
}

//--------------------------------------------------------------------------------
OSStatus	BS2B::Initialize()
{
	OSStatus status = AUEffectBase::Initialize();
    
	if (status == noErr)
	{
        // What does this do?
		const AudioUnitElement elem = 0;
		Reset(kAudioUnitScope_Global, elem);
        
		if ( GetStreamFormat(kAudioUnitScope_Input, elem).mChannelsPerFrame != GetStreamFormat(kAudioUnitScope_Output, elem).mChannelsPerFrame )
		{
			if ( ProcessesInPlace() )
			{
				SetProcessesInPlace(false);
				PropertyChanged(kAudioUnitProperty_InPlaceProcessing, kAudioUnitScope_Global, elem);
			}
		}
	}
    
	return status;
}

// still do something sensible with stereo inputs? Haas?

//--------------------------------------------------------------------------------
void BS2B::Cleanup()
{
	AUEffectBase::Cleanup();
}

//--------------------------------------------------------------------------------
OSStatus BS2B::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	OSStatus status = AUEffectBase::Reset(inScope, inElement);
	return status;
}

//--------------------------------------------------------------------------------
OSStatus BS2B::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo & outParameterInfo)
{
	OSStatus status = noErr;
    
	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;
    
	switch (inParameterID)
	{
		case kParam_Level_Feed:
			FillInParameterName(outParameterInfo, CFSTR("Crossfeed level"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
			outParameterInfo.minValue = BS2B_MINFEED/10.0;
			outParameterInfo.maxValue = BS2B_MAXFEED/10.0;
			outParameterInfo.defaultValue = 4.5;
			break;
            
		case kParam_Level_Fcut:
			FillInParameterName(outParameterInfo, CFSTR("Cutoff frequency"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
			outParameterInfo.minValue = BS2B_MINFCUT;
			outParameterInfo.maxValue = BS2B_MAXFCUT;
			outParameterInfo.defaultValue = 700;
			break;
        default:
			status = kAudioUnitErr_InvalidParameter;
			break;
	}
    
	return status;
}

//--------------------------------------------------------------------------------
OSStatus BS2B::GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings)
{
    // ??
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;
    
	switch (inParameterID)
	{
        return noErr;
	}
    
	return kAudioUnitErr_InvalidPropertyValue;
}

//--------------------------------------------------------------------------------
UInt32 BS2B::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
	static AUChannelInfo plugChannelInfo[] = { {kNumInputs, kNumOutputs}, {kNumOutputs, kNumOutputs} };
    
	if (outChannelInfo != NULL)
		*outChannelInfo = plugChannelInfo;
    
	return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}

//--------------------------------------------------------------------------------
OSStatus BS2B::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
                                    const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
    //const float * in[kNumInputs];
	const float * in1 = (float*)(inBuffer.mBuffers[0].mData); 
	const float * in2 = (float*)(inBuffer.mBuffers[1].mData);
    
	float * out[kNumOutputs];
    //for (SInt16 i=0; i < kNumOutputs; i++)    // FIXME: why doesn't this work? --> learn C++!
    //    in[i] = (float*)(inBuffer.mBuffers[i].mData);
	for (SInt16 i=0; i < kNumOutputs; i++)
		out[i] = (float*)(outBuffer.mBuffers[i].mData);
        
    bs2b.set_level_fcut(GetParameter(kParam_Level_Fcut));
    bs2b.set_level_feed(10.0*GetParameter(kParam_Level_Feed));
    
    for (UInt32 samp=0; samp < inFramesToProcess; samp++)
		{
            float sample[2];
            sample[0] = in1[samp];
            sample[1] = in2[samp];
            bs2b.cross_feed(sample);
            out[0][samp] = sample[0];
            out[1][samp] = sample[1];
		}
    
	return noErr;
}
