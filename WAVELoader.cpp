//*****************************************************************************************//
//**                                                                                     **//
//**                                WAVELoader                                           **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "WAVELoader.h"

namespace {
	const uint32_t RIFF_id = 0x52494646;
	const uint32_t fmt_id = 0x666D7420;
	const uint32_t data_id = 0x64617461;

	const uint32_t WAVE_f = 0x57415645;

	void setEr(char* errorMessage, char* inMessage) {
		if (errorMessage) {
			int size = (int)strlen(inMessage) + 1;
			memcpy(errorMessage, inMessage, size);
		}
	}
}

void WAVELoader::bytePointer::setPointer(uint32_t Size, uint8_t* Byte) {
	size = Size;
	byte = std::make_unique<uint8_t[]>(size);
	memcpy(byte.get(), Byte, size);
}

uint32_t WAVELoader::bytePointer::getIndex() const {
	return index;
}

void WAVELoader::bytePointer::setIndex(uint32_t Index) {
	index = Index;
}

uint8_t* WAVELoader::bytePointer::getPointer() {
	uint8_t* ret = &byte[index];
	index++;
	return ret;
}

uint32_t WAVELoader::bytePointer::convertUCHARtoUINTstr() {
	uint32_t ret = ((uint32_t)byte[index + 0] << 24) | ((uint32_t)byte[index + 1] << 16) |
		((uint32_t)byte[index + 2] << 8) | ((uint32_t)byte[index + 3]);

	index += 4;
	return ret;
}

uint32_t WAVELoader::bytePointer::convertUCHARtoUINT() {
	uint32_t ret = ((uint32_t)byte[index + 0]) | ((uint32_t)byte[index + 1] << 8) |
		((uint32_t)byte[index + 2] << 16) | ((uint32_t)byte[index + 3] << 24);

	index += 4;
	return ret;
}

uint16_t WAVELoader::bytePointer::convertUCHARtoUSHORT() {
	uint16_t ret = ((uint16_t)byte[index + 0]) | ((uint16_t)byte[index + 1] << 8);

	index += 2;
	return ret;
}

bool WAVELoader::bytePointer::EndOfFile() const {
	return size <= index;
}

WAVE_Output WAVELoader::loadWAVE(const char* pass, char* errorMessage) {

	FILE* fp = fopen(pass, "rb");
	if (fp == NULL) {
		setEr(errorMessage, "File read error");
		return WAVE_Output();
	}
	unsigned int count = 0;
	while (fgetc(fp) != EOF) {
		count++;
	}
	fseek(fp, 0, SEEK_SET);//最初に戻す
	std::unique_ptr<uint8_t[]> byte = std::make_unique<uint8_t[]>(count);
	fread(byte.get(), sizeof(uint8_t), count, fp);
	fclose(fp);
	return loadWAVEInByteArray(byte.get(), count, errorMessage);
}

WAVE_Output WAVELoader::loadWAVEInByteArray(uint8_t* byteArray, uint32_t byteSize, char* errorMessage) {

	std::unique_ptr<bytePointer> bp = std::make_unique<bytePointer>();
	bp->setPointer(byteSize, byteArray);

	RIFF riff = {};
	FMT fmt = {};
	DATA data = {};

	//チャンクデータ読み込み
	while (!bp->EndOfFile()) {

		uint32_t identifier = bp->convertUCHARtoUINTstr();

		switch (identifier) {

		case RIFF_id:
		{
			riff.chunk_size = bp->convertUCHARtoUINT();
			riff.format = bp->convertUCHARtoUINTstr();
			if (riff.format != WAVE_f) {
				setEr(errorMessage, "File is not WAVE.");
				return WAVE_Output();
			}
			break;
		}
		case fmt_id:
		{
			fmt.chunk_size = bp->convertUCHARtoUINT();
			uint32_t tmpIndex = bp->getIndex();
			fmt.audioformat = bp->convertUCHARtoUSHORT();
			fmt.numChannels = bp->convertUCHARtoUSHORT();
			fmt.SamplesPerSec = bp->convertUCHARtoUINT();
			fmt.AvgBytesPerSec = bp->convertUCHARtoUINT();
			fmt.BlockAlign = bp->convertUCHARtoUSHORT();
			fmt.BitsPerSample = bp->convertUCHARtoUSHORT();
			if (fmt.chunk_size > 16) {
				fmt.cbSize = bp->convertUCHARtoUSHORT();
				fmt.cb = std::make_unique<uint8_t[]>(fmt.cbSize);
				memcpy(fmt.cb.get(), bp->getPointer(), fmt.cbSize);
			}
			bp->setIndex(tmpIndex + fmt.chunk_size);
		}
		break;

		case data_id:
		{
			data.chunk_size = bp->convertUCHARtoUINT();
			uint32_t tmpIndex = bp->getIndex();
			data.wave_data = std::make_unique<uint8_t[]>(data.chunk_size);
			memcpy(data.wave_data.get(), bp->getPointer(), data.chunk_size);
			bp->setIndex(tmpIndex + data.chunk_size);
		}
		break;

		default:
		{
			uint32_t size = bp->convertUCHARtoUINT();
			uint32_t tmpIndex = bp->getIndex();
			bp->setIndex(tmpIndex + size);
		}
		break;

		}
	}

	WAVE_Output out = {};

	switch (fmt.audioformat) {

	case 1://リニアPCM
		out.format = PCM;
		break;

	case 6://A-law
		out.format = ALAW;
		break;

	case 7://μ-law
		out.format = MULAW;
		break;
	}

	switch (fmt.numChannels) {

	case 1://モノラル
		out.numChannels = MONAURAL;
		break;

	case 2://ステレオ
		out.numChannels = STEREO;
		break;
	}

	out.SamplesPerSec = fmt.SamplesPerSec;
	out.AvgBytesPerSec = fmt.AvgBytesPerSec;
	out.BlockAlign = fmt.BlockAlign;
	out.BitsPerSample = fmt.BitsPerSample;
	out.cbSize = fmt.cbSize;
	if (out.cbSize > 0) {
		out.cb = std::move(fmt.cb);
	}
	out.wave_size = data.chunk_size;
	out.wave_data = std::move(data.wave_data);

	setEr(errorMessage, "OK");

	return out;
}