#pragma once
#include <cstdint>
#include <Mui_Base.h>
#define HEAD_LENGTH 1024
class Wav {
public:
	struct WAV_HEADER {
		char             RIFF[4] = { 'R','I','F','F' };              //RIFF��ʶ
		unsigned long    ChunkSize;                                  //�ļ���С-8
		char             WAVE[4] = { 'W','A','V','E' };              //WAVE��
		char             fmt[4] = { 'f','m','t',' ' };               //fmt��
		unsigned long    Subchunk1Size;                              //fmt���С
		unsigned short   AudioFormat;                                //�����ʽ
		unsigned short   NumOfChan;                                  //������
		unsigned long    SamplesPerSec;                              //������
		unsigned long    bytesPerSec;                                //ÿ�����ֽ���
		unsigned short   blockAlign;                                 //�������ֽ�
		unsigned short   bitsPerSample;                              //������λ��
		char             Subchunk2ID[4] = { 'd','a','t','a' };       //���ݿ�
		unsigned long    Subchunk2Size;                              //���ݿ��С
		WAV_HEADER(unsigned long cs = 36, unsigned long sc1s = 16, unsigned short af = 1, unsigned short nc = 1, unsigned long sr = 22050, unsigned long bps = 44100, unsigned short ba = 2, unsigned short bips = 16, unsigned long sc2s = 0) :ChunkSize(cs), Subchunk1Size(sc1s), AudioFormat(af), NumOfChan(nc), SamplesPerSec(sr), bytesPerSec(bps), blockAlign(ba), bitsPerSample(bips), Subchunk2Size(sc2s) {}
	};
	using iterator = int16_t*;
	Wav(unsigned long cs = 36, unsigned long sc1s = 16, unsigned short af = 1, unsigned short nc = 1, unsigned long sr = 22050, unsigned long bps = 44100, unsigned short ba = 2, unsigned short bips = 16, unsigned long sc2s = 0) :header({
			cs,
			sc1s,
			af,
			nc,
			sr,
			bps,
			ba,
			bips,
			sc2s
		}), Data(nullptr), StartPos(44) {
		dataSize = 0;
		SData = nullptr;
	}
	Wav(unsigned long sr, unsigned long length, const void* data) :header({
			36,
			16,
			1,
			1,
			sr,
			sr * 2,
			2,
			16,
			length
		}), Data(new char[length + 1]), StartPos(44)
	{
		header.ChunkSize = 36 + length;
		memcpy(Data, data, length);
		SData = reinterpret_cast<int16_t*>(Data);
		dataSize = length / 2;
	}
	Wav(const wchar_t* Path);
	Wav(const Wav& input);
	Wav(Wav&& input) noexcept;
	Wav& operator=(const Wav& input) = delete;
	Wav& operator=(Wav&& input) noexcept;
	~Wav() { destory(); }
	Wav& cat(const Wav& input);
	bool isEmpty() const { return this->header.Subchunk2Size == 0; }
	const char* getData() const { return Data; }
	char* getData() { return Data; }
	WAV_HEADER getHeader() const { return header; }
	WAV_HEADER& Header() { return header; }
	void destory() const { delete[] Data; }
	void changeData(const void* indata, long length, int sr)
	{
		delete[] Data;
		Data = new char[length];
		memcpy(Data, indata, length);
		header.ChunkSize = 36 + length;
		header.Subchunk2Size = length;
		header.SamplesPerSec = sr;
		header.bytesPerSec = 2 * sr;
	}
	int16_t& operator[](const size_t index) const
	{
		if (index < dataSize)
			return *(SData + index);
		return *(SData + dataSize - 1);
	}
	iterator begin() const
	{
		return reinterpret_cast<int16_t*>(Data);
	}
	iterator end() const
	{
		return reinterpret_cast<int16_t*>(Data + header.Subchunk2Size);
	}
	Mui::UIResource getRes() const
	{
		Mui::UIResource out;
		out.size = header.ChunkSize + 8;
		out.data = new Mui::_m_byte[out.size + 44];
		int64_t pos = 0;
		memcpy(out.data + pos, header.RIFF, 4);
		pos += 4;
		memcpy(out.data + pos, &header.ChunkSize, 4);
		pos += 4;
		memcpy(out.data + pos, header.WAVE, 4);
		pos += 4;
		memcpy(out.data + pos, header.fmt, 4);
		pos += 4;
		memcpy(out.data + pos, &header.Subchunk1Size, 4);
		pos += 4;
		memcpy(out.data + pos, &header.AudioFormat, 2);
		pos += 2;
		memcpy(out.data + pos, &header.NumOfChan, 2);
		pos += 2;
		memcpy(out.data + pos, &header.SamplesPerSec, 4);
		pos += 4;
		memcpy(out.data + pos, &header.bytesPerSec, 4);
		pos += 4;
		memcpy(out.data + pos, &header.blockAlign, 2);
		pos += 2;
		memcpy(out.data + pos, &header.bitsPerSample, 2);
		pos += 2;
		memcpy(out.data + pos, header.Subchunk2ID, 4);
		pos += 4;
		memcpy(out.data + pos, &header.Subchunk2Size, 4);
		pos += 4;
		memcpy(out.data + pos, Data, header.Subchunk2Size);
		return out;
	}
	int64_t getDataLen()const
	{
		return static_cast<int64_t>(dataSize);
	}
private:
	WAV_HEADER header;
	char* Data;
	int16_t* SData;
	size_t dataSize;
	int StartPos;
};
