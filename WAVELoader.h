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
	uint16_t cbSize = 0;//拡張パラメータサイズ(リニアPCMの場合は未使用)
	std::unique_ptr<uint8_t[]> cb;//拡張パラメータ(リニアPCMの場合は未使用)
	uint32_t wave_size = 0;
	std::unique_ptr<uint8_t[]> wave_data;//波形データ
};

class WAVELoader {

private:
	struct RIFF {
		uint32_t chunk_size = 0;//ファイル全体サイズからRIFFとWAVEのバイト数(8Byte)を引いた数
		uint32_t format = 0;//WAVファイルの場合 "WAVE" (0x57415645)で固定
	};
	//読み込みはリトルエンディアン
	struct FMT {
		uint32_t chunk_size = 0;//リニアPCMは 16(0x10000000) その他 16+拡張パラメータ
		uint16_t audioformat = 0;//リニアPCM 1(0x0100)等
		uint16_t numChannels = 0;//モノラル 1(0x0100)、ステレオ 2(0x0200)
		uint32_t SamplesPerSec = 0;//サンプリング周波数 8kHz (0x401F0000) 44.1kHz (0x44AC0000)
		uint32_t AvgBytesPerSec = 0;//1秒間のバイト数平均
		uint16_t BlockAlign = 0;//ブロックアライメント(データの最小単位)
		uint16_t BitsPerSample = 0;//量子化ビット数 8ビット 8(0x0800) 16ビット 16(0x1000)
		uint16_t cbSize = 0;//拡張パラメータサイズ(リニアPCMの場合は未使用)
		std::unique_ptr<uint8_t[]> cb;//拡張パラメータ(リニアPCMの場合は未使用)
	};

	struct DATA {
		uint32_t chunk_size = 0;//波形データのサイズ(総ファイルサイズ?126)
		std::unique_ptr<uint8_t[]>wave_data;//波形データ
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
