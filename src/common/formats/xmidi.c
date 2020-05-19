
#include "fmio.h"
#include "memory.h"
#include "xmidi.h"
#include "memory/endian.h"

uint16 processXMidiData(void *data, const uint32 dataLength, sSequence_t **ppCurSequence)
{

	int32 chunkSize = 0;

	const sIffChunk *iffdata = (sIffChunk *)data;

	if ( (uint32)iffdata->id == ID_FORM)
	{
  		chunkSize = ReadBE32(iffdata->size);
	} 
	else if((uint32)iffdata->id == ID_CAT)
	{
    	chunkSize = ReadBE32(iffdata->size);
	}

	return 0;
}

void *processXMidiForm(void *data, sSequence_t **ppCurSequence, int16 *iError)
{
	return 0;
}
void *processXMidiList(void *data, sSequence_t **ppCurSequence, int16 *iError)
{
	return 0;
}

void *processXMidiCat(void *data, sSequence_t **ppCurSequence, int16 *iError)
{
	return 0;
}

void *processXMidiProp(void *data, sSequence_t **ppCurSequence, int16 *iError)
{
	return 0;
}



