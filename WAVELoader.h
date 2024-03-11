//*****************************************************************************************//
//**                                                                                     **//
//**                                WAVELoader                                           **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_WAVELoader_Header
#define Class_WAVELoader_Header

#include <stdint.h>
#include <memory>

enum WAVE_Format {
	PCM,
	IEEE_FLOAT,
	DRM,
	EXTENSIBLE,
	ALAW,
	MULAW,
	ADPCM,
	MPEG,
	DOLBY_AC3_SPDIF,
	WMASPDIF
};

enum WAVE_numChannels {
	MONAURAL,
	STEREO
};

struct WAVE_Output {
	WAVE_Format format = {};
	WAVE_numChannels numChannels = {};
	uint32_t SamplesPerSec = 0;
	uint32_t AvgBytesPerSec = 0;
	uint16_t BlockAlign = 0;
	uint16_t BitsPerSample = 0;
	uint16_t cbSize = 0;//�g���p�����[�^�T�C�Y(���j�APCM�̏ꍇ�͖��g�p)
	std::unique_ptr<uint8_t[]> cb;//�g���p�����[�^(���j�APCM�̏ꍇ�͖��g�p)
	uint32_t wave_size = 0;
	std::unique_ptr<uint8_t[]> wave_data;//�g�`�f�[�^
};

class WAVELoader {

private:
	struct RIFF {
		uint32_t chunk_size = 0;//�t�@�C���S�̃T�C�Y����RIFF��WAVE�̃o�C�g��(8Byte)����������
		uint32_t format = 0;//WAV�t�@�C���̏ꍇ "WAVE" (0x57415645)�ŌŒ�
	};
	//�ǂݍ��݂̓��g���G���f�B�A��
	struct FMT {
		uint32_t chunk_size = 0;//���j�APCM�� 16(0x10000000) ���̑� 16+�g���p�����[�^
		uint16_t audioformat = 0;//���j�APCM 1(0x0100)��
		uint16_t numChannels = 0;//���m���� 1(0x0100)�A�X�e���I 2(0x0200)
		uint32_t SamplesPerSec = 0;//�T���v�����O���g�� 8kHz (0x401F0000) 44.1kHz (0x44AC0000)
		uint32_t AvgBytesPerSec = 0;//1�b�Ԃ̃o�C�g������
		uint16_t BlockAlign = 0;//�u���b�N�A���C�����g(�f�[�^�̍ŏ��P��)
		uint16_t BitsPerSample = 0;//�ʎq���r�b�g�� 8�r�b�g 8(0x0800) 16�r�b�g 16(0x1000)
		uint16_t cbSize = 0;//�g���p�����[�^�T�C�Y(���j�APCM�̏ꍇ�͖��g�p)
		std::unique_ptr<uint8_t[]> cb;//�g���p�����[�^(���j�APCM�̏ꍇ�͖��g�p)
	};

	struct DATA {
		uint32_t chunk_size = 0;//�g�`�f�[�^�̃T�C�Y(���t�@�C���T�C�Y?126)
		std::unique_ptr<uint8_t[]>wave_data;//�g�`�f�[�^
	};

	class bytePointer {
	private:
		std::unique_ptr<uint8_t[]> byte;
		uint32_t index = 0;
		uint32_t size = 0;

	public:
		void setPointer(uint32_t size, uint8_t* byte);
		uint32_t getIndex() const;
		void setIndex(uint32_t index);
		uint8_t* getPointer();
		uint32_t convertUCHARtoUINTstr();
		uint32_t convertUCHARtoUINT();
		uint16_t convertUCHARtoUSHORT();
		bool EndOfFile() const;
	};

public:
	WAVE_Output loadWAVE(const char* pass, char* errorMessage = nullptr);
	WAVE_Output loadWAVEInByteArray(uint8_t* byteArray, uint32_t byteSize, char* errorMessage = nullptr);
};

#endif
