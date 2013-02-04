#include "stdafx.h"
#include "filters.h"

#define INITGUID
#include <Guiddef.h>
#include "moreuuids.h"

#define AVICHECKBYTES L"0,4,,52494646,8,4,,41564919;0,4,,52494646,8,4,,41564920;0,4,,52494646,8,4,,41564958;0,4,,52494646,8,4,,414D5620"
#define MP4CHECKBYTES L"4,4,,66747970;4,4,,6d6f6f76;4,4,,6d646174;4,4,,736b6970;4,12,ffffffff00000000ffffffff,77696465027fe3706d646174;3,3,,000001"
#define MPEGCHECKBYTES L"0,16,FFFFFFFFF100010001800001FFFFFFFF,000001BA2100010001800001000001BB;0,5,FFFFFFFFC0,000001BA40;0,1,,47,188,1,,47,376,1,,47;4,1,,47,196,1,,47,388,1,,47;0,4,,54467263,1660,1,,47;0,8,fffffc00ffe00000,4156000055000000"

//////////////////////////////////////////////////////////////////////////
//Source
SourceFilter gf_gabestMp4 = {TRUE, &CLSID_GabestMp4, NULL, MERIT_NORMAL, FLT_FILE, FT_SRC, 0, NULL, L"MP4splitter.ax", NULL,
	L"mp4,move", MP4CHECKBYTES};
SourceFilter gf_babestFlv = {TRUE, &CLSID_GabestAvi, NULL, MERIT_NORMAL, FLT_FILE, FT_SRC, 0, NULL, L"AviSplitter.ax", NULL, 
	L"avi", AVICHECKBYTES};
SourceFilter gf_gabestMpeg = {TRUE, &CLSID_GabestMpeg, NULL, MERIT_NORMAL, FLT_FILE, FT_SRC, 0, NULL, L"MpegSplitter.ax", NULL,
	L"avi", MPEGCHECKBYTES};
SourceFilter gf_gabestFlv = {TRUE, &CLSID_GabestFlv, NULL, MERIT_NORMAL, FLT_FILE, FT_SRC, 0, NULL, L"FlvSplitter.ax", NULL,
	L"flv", L"0,4,,464C5601"};

//////////////////////////////////////////////////////////////////////////
// Audio Decoder

// ffdshow More Audio Decoder
REGPINTYPES ffdshowMoreAudioMediaTypes [] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_AMVA},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC_FRAMED},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_IMA4},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_ima4},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_ASWF}
};

Filter	gf_ffdshowMoreAudioDecoder = {TRUE, &CLSID_ffdshowAudioDecoder, L"ffdshow More Audio Decoder", MERIT_NORMAL, FLT_FILE, FT_ADEC,
		_countof(ffdshowMoreAudioMediaTypes), ffdshowMoreAudioMediaTypes, L"ffdshow.ax"};

//////////////////////////////////////////////////////////////////////////
// Video Decoder
REGPINTYPES avcMediaTypes[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_H264},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_h264},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_AVC1},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_avc1},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_X264},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_x264}
};
Filter	gf_avcDecoder = {TRUE, &CLSID_PreferredAVCDecoder, L"MPC H264 Decoder", MERIT_NORMAL+2, FLT_FILE, FT_VDEC,
		_countof(avcMediaTypes), avcMediaTypes, L"AVCVideoDec.ax"};

//////////////////////////////////////////////////////////////////////////
// Audio Effect
REGPINTYPES	audioMediaTypes[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL}
};

Filter	gf_audioSwitcher = {TRUE, &CLSID_AudioSwitcher, L"Audio Switcher", MERIT_NORMAL, FLT_REG, FT_AEFFECT,
		_countof(audioMediaTypes), audioMediaTypes, L"audioSwitcher.ax"};

//////////////////////////////////////////////////////////////////////////
// Video Effect
REGPINTYPES videoMediaTypes[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL}
};

Filter	gf_vobSubTitle = {TRUE, &CLSID_VobSubtitle, L"JayGuo VobSub Filter", MERIT_PREFERRED+1, FLT_FILE, FT_VEFFECT,
		_countof(videoMediaTypes), videoMediaTypes, L"VSFilter.dll"};

//////////////////////////////////////////////////////////////////////////
// Audio Renderer
Filter	gf_defaultAudioRenderer = {TRUE, &CLSID_PreferredAudioRenderer, L"Audio Renderer", MERIT_PREFERRED + 1, FLT_INTERNAL, FT_ARENDER,
		_countof(audioMediaTypes), audioMediaTypes, NULL};
Filter	gf_defaultDirectSound = {TRUE, &CLSID_DSoundRender, L"Default DirectSound Device", MERIT_PREFERRED, FLT_REG, FT_ARENDER,
		_countof(audioMediaTypes), audioMediaTypes, L"quartz.dll"};
Filter	gf_defaultWaveOut = {TRUE, &CLSID_AudioRender, L"Default WaveOut Device", MERIT_PREFERRED - 1, FLT_REG, FT_ARENDER,
		_countof(audioMediaTypes), audioMediaTypes, L"quartz.dll"};

//////////////////////////////////////////////////////////////////////////
// Video Renderer
REGPINTYPES vrMediaTypes[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL}
};

Filter	gf_defaultVideoRenderer = {TRUE, &CLSID_PreferredVideoRenderer, L"Video Render", MERIT_PREFERRED, FLT_REG, FT_VRENDER,
		_countof(vrMediaTypes), vrMediaTypes, L"quartz.dll"};

Filter gf_VMR7Windowless = {TRUE, &CLSID_VMR7Windowless, L"Video Mixing Renderer 7(Windowless)", MERIT_PREFERRED, FLT_REG, FT_VRENDER,
	_countof(vrMediaTypes), vrMediaTypes, L"quartz.dll"};

Filter	gf_VMR9Windowless = {TRUE, &CLSID_VMR9Windowless, L"Video Mixing Renderer 9(Windowless)", MERIT_PREFERRED, FLT_REG, FT_VRENDER,
		_countof(vrMediaTypes), vrMediaTypes, L"quartz.dll"};

Filter gf_EnhancedVideoRenderer = {TRUE, &CLSID_EnhancedVideoRenderer, L"Enhanced Video Renderer", MERIT_PREFERRED, FLT_REG, FT_VRENDER,
		_countof(vrMediaTypes), vrMediaTypes, L"quartz.dll"};

Filter gf_VideoRendererDefault = {TRUE, &CLSID_VideoRendererDefault, L"Video Renderer Default", MERIT_PREFERRED, FLT_REG, FT_VRENDER,
		_countof(vrMediaTypes), vrMediaTypes, L"quartz.dll"};

Filter gf_VideoRender = {TRUE, &CLSID_VideoRenderer, L"Video Renderer", MERIT_PREFERRED, FLT_REG, FT_VRENDER,
		_countof(vrMediaTypes), vrMediaTypes, L"quartz.dll"};


Filter*	g_filters[] = {
	//Source Filter.
	(Filter*)&gf_gabestFlv, (Filter*)&gf_gabestMp4, (Filter*)&gf_gabestMpeg,
	//Audio Decoder
	&gf_ffdshowMoreAudioDecoder,
	//Video Decoder
	&gf_avcDecoder,
	//Audio Effect
	&gf_audioSwitcher,
	//Video Effect
	&gf_vobSubTitle,
	//Audio Renderer
	&gf_defaultAudioRenderer, &gf_defaultDirectSound, &gf_defaultWaveOut,
	//Video Renderer
	&gf_defaultVideoRenderer, &gf_VMR9Windowless, &gf_VMR7Windowless, &gf_EnhancedVideoRenderer,
	&gf_VideoRender, &gf_VideoRendererDefault,
	//End
	NULL
};
