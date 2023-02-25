#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <cxxmidi/guts/endianness.hpp>
#include <cxxmidi/guts/utils.hpp>

class Midi
{
	enum class MidiEvent
	{
		ReleaseNote = 0x80,
		ClickNote = 0x90,
		KeyAfterTouch = 0xA0,
		Controller = 0xB0,
		ChangeInstrument = 0xC0,
		ChannelAfterTouch = 0xD0,
		Slide = 0xE0,
		SystemCode = 0xF0,
		META = 0xFF
	};
public:
	struct MidiHeader
	{
		char MThd[4] = { 'M','T','h','d' };
		unsigned long MThdLen = 6;
		unsigned short format = 0;
		unsigned short channels = 0;
		unsigned short time_base = 120;
		MidiHeader(unsigned short f = 0, unsigned short c = 0, unsigned short tb = 120) :format(f), channels(c), time_base(tb) {}
	};

	struct MidiMTrk
	{
		char MTrk[4] = { 'M','T','r','k' };
		unsigned long MTrkLen = 4;
		std::vector<uint8_t> Data;
		unsigned char meta_event[4] = { 0x00,0xff,0x2f,0x00 };
	};

	Midi() = default;
	Midi(unsigned short tb) :_header(0, 0, tb) {}

	void Write(const std::wstring& _path)
	{
		std::ofstream outfile(_path.c_str(), std::ios::out | std::ios::binary);
		if(outfile.is_open())
		{
			outfile.write((char*)(&_header), 14);
			for(auto& it : _tracks)
			{
				outfile.write(it.MTrk, 4);
				outfile.write((char*)(&it.MTrkLen), 4);
				outfile.write((char*)(it.Data.data()), int64_t(it.Data.size()));
				outfile.write((char*)(it.meta_event), 4);
			}
		}
	}

	void CreateTrack()
	{
		_tracks.push_back({});
		if(!_header.format && _header.channels == 1)
			_header.format = 1;
		++_header.channels;
	}

	MidiMTrk& operator[](size_t _index)
	{
		return _tracks[_index];
	}

	void SetBaseTime(unsigned short _bt)
	{
		_header.time_base = _bt;
	}

	void InsertEvent(uint8_t _track, uint64_t _delta_time, MidiEvent _event, uint8_t _id, uint8_t _vol)
	{
		if (_track > 0x0Fui8)
			return;
		size_t _offtime = 1;
		const auto _tit = reinterpret_cast<unsigned char*>(&_delta_time);
		if (_delta_time > 0x0fffffff)
			return;
		if (_delta_time > 0xffffff)
			_offtime = 4;
		else if (_delta_time > 0xffff)
			_offtime = 3;
		else if (_delta_time > 0xff)
			_offtime = 2;
		switch(_event)
		{
		case MidiEvent::ClickNote:
			{
				_tracks[_track].Data.insert(_tracks[_track].Data.end(), _tit, _tit + _offtime);
				_tracks[_track].Data.emplace_back(unsigned char(_event) | _track);
				_tracks[_track].Data.emplace_back(_id);
				_tracks[_track].Data.emplace_back(_vol);
				_tracks[_track].MTrkLen += _offtime + 3;
			}
		case MidiEvent::ReleaseNote:
			{
				_tracks[_track].Data.insert(_tracks[_track].Data.end(), _tit, _tit + _offtime);
				_tracks[_track].Data.emplace_back(unsigned char(_event) | _track);
				_tracks[_track].Data.emplace_back(_id);
				_tracks[_track].Data.emplace_back(0);
				_tracks[_track].MTrkLen += _offtime + 3;
			}
		case MidiEvent::META:
			{
				
			}
		default:
			{
				_tracks[_track].Data.insert(_tracks[_track].Data.end(), _tit, _tit + _offtime);
				_tracks[_track].Data.emplace_back(unsigned char(_event) | _track);
				_tracks[_track].Data.emplace_back(_id);
				_tracks[_track].MTrkLen += _offtime + 2;
			}
		}
	}
private:
	MidiHeader _header;
	std::vector<MidiMTrk> _tracks;
};

#include <chrono>  // NOLINT() CPP11_INCLUDES
#include <cxxmidi/track.hpp>

namespace cxxmidi {

	class File : public std::vector<Track> {
	public:
		inline File();
		inline explicit File(const char* path);

		inline void Load(const char* path);
		inline void SaveAs(const char* path) const;

		inline Track& AddTrack();
		inline File::size_type Tracks() const;

		inline uint16_t TimeDivision() const;
		inline void SetTimeDivision(uint16_t time_division);

		inline std::chrono::microseconds Duration() const;

	private:
		uint16_t time_division_;  // [ticks per quarternote]

		inline void ReadHeaderChunk(std::ifstream& file);
		inline void ReadUnknownChunk(std::ifstream& file);
		inline void ReadTrackChunk(std::ifstream& file);
		inline void ReadEvent(std::ifstream& file, Event* event, bool* track_continue,
			uint8_t* running_status);

		inline void SaveHeaderChunk(std::ofstream& output_file) const;
		inline void SaveTrackChunk(std::ofstream& output_file,
			const Track& track) const;
		inline uint32_t SaveEvent(std::ofstream& output_file, const Event& event,
			uint8_t* last_cmd) const;

		inline static uint32_t Write(std::ofstream& file, const uint8_t* c,
			std::streamsize size = 1);
	};

}  // namespace cxxmidi

namespace cxxmidi {

    uint32_t File::Write(std::ofstream& file, const uint8_t* c,
        std::streamsize size) {
        file.write(reinterpret_cast<const char*>(c), size);
        return size;
    }

    File::File() : time_division_(500) {}

    File::File(const char* path) : time_division_(500) { Load(path); }

    void File::SaveAs(const char* path) const {
        std::ofstream out_file(path, std::ios_base::binary);

        if (!out_file.good()) {
#ifndef CXXMIDI_QUIET
            std::cerr << "CxxMidi: could not open file " << path << std::endl;
#endif
            return;
        }

        // save header
        SaveHeaderChunk(out_file);

        // loop over tracks
        for (const auto& track : *this) SaveTrackChunk(out_file, track);

        out_file.close();
    }

    void File::SaveTrackChunk(std::ofstream& output_file,
        const Track& track) const {
        // save chunk id
        guts::endianness::WriteBe<uint32_t>(output_file, 0x4D54726B);  // "MTrk"

        // write dummy chunk size (we will get back here)
        auto size_pos = output_file.tellp();
        guts::endianness::WriteBe<uint32_t>(output_file, 0);

        uint32_t chunk_size = 0;  // chunk size
        uint8_t last_cmd = 0;

        for (const auto& event : track)
            chunk_size += SaveEvent(output_file, event, &last_cmd);

        // go back to chunk size
        output_file.seekp(size_pos);
        // save chunk size
        guts::endianness::WriteBe<uint32_t>(output_file, chunk_size);

        // go to end of file
        output_file.seekp(0, std::ios_base::end);
    }

    uint32_t File::SaveEvent(std::ofstream& output_file, const Event& event,
        uint8_t* last_cmd) const {
        uint32_t r = utils::SaveVlq(output_file, event.Dt());

        int skip_data_bytes;
        if (event.IsSysex()) {
            r += Write(output_file, &event.at(0));  // SysEx type

            uint8_t data_size = static_cast<uint8_t>(event.size()) - 1;
            r += utils::SaveVlq(output_file, data_size);

            skip_data_bytes = 1;
            *last_cmd = 0;
        }
        else if (event.IsMeta()) {
            r += Write(output_file, event.data(), 2);  // byte 0 and 1

            uint8_t dataSize = static_cast<uint8_t>(event.size()) - 2;
            r += Write(output_file, &dataSize);  // save length

            skip_data_bytes = 2;
            *last_cmd = 0;
        }
        else {
            const uint8_t nowCmd = event.at(0);
            skip_data_bytes = (*last_cmd == nowCmd);
            *last_cmd = nowCmd;  // save current command if we use running status
        }

        r += Write(output_file, event.data() + skip_data_bytes,
            event.size() - skip_data_bytes);
        return r;
    }

    void File::SaveHeaderChunk(std::ofstream& output_file) const {
        // save chunk id
        guts::endianness::WriteBe<uint32_t>(output_file, 0x4D546864);  // "MThd"

        // save header size
        guts::endianness::WriteBe<uint32_t>(output_file, 6);

        // save file type
        uint16_t fileType = (Tracks() > 1) ? 1 : 0;
        guts::endianness::WriteBe<uint16_t>(output_file, fileType);

        // save tracks numer
        guts::endianness::WriteBe<uint16_t>(output_file,
            static_cast<uint16_t>(Tracks()));

        // save time division
        guts::endianness::WriteBe<uint16_t>(output_file, time_division_);
    }


    Track& File::AddTrack() {
        push_back(Track());
        return back();
    }

    File::size_type File::Tracks() const { return size(); }

    uint16_t File::TimeDivision() const { return time_division_; }

    void File::SetTimeDivision(uint16_t time_division) {
        time_division_ = time_division;
    }

    void File::Load(const char* path) {
        clear();

        // open file
        std::ifstream file(path, std::ifstream::in | std::ifstream::binary);

        if (!file.is_open()) {
#ifndef CXXMIDI_QUIET
            std::cerr << "CxxMidi: couldn't open: " << path << std::endl;
#endif
            return;
        }

        // calculate file length
        file.seekg(0, std::ifstream::end);
        auto fileLength = file.tellg();
        file.seekg(0, std::ifstream::beg);

        // control counters
        unsigned int i = 0;
        unsigned int headers = 0;

        // loop over chunks while data still in buffer
        while (file.good() && (fileLength - file.tellg())) {
            uint32_t chunk_id = guts::endianness::ReadBe<uint32_t>(file);

            switch (chunk_id) {
            case 0x4D546864:  // "MThd"
                ReadHeaderChunk(file);
                headers++;
                break;

            case 0x4D54726B:  // "MTrk"
                ReadTrackChunk(file);
                break;

            default:
#ifndef CXXMIDI_QUIET
                std::cerr << "CxxMidi: ignoring unknown chunk: 0x" << std::hex
                    << static_cast<int>(chunk_id) << std::endl;
#endif
                ReadUnknownChunk(file);
                break;
            }

            if (!i++ && !headers) {
#ifndef CXXMIDI_QUIET
                std::cerr << "CxxMidi: no header chunk "
                    "(probably not a MIDI file)"
                    << std::endl;
#endif
                break;
            }
        }
    }

    void File::ReadHeaderChunk(std::ifstream& file) {
        // read and check header size
        if (guts::endianness::ReadBe<uint32_t>(file) != 6) {
#ifndef CXXMIDI_QUIET
            std::cerr << "CxxMidi: warning: unsupported MIDI file type" << std::endl;
#endif
        }

        // read file type
        uint16_t type = guts::endianness::ReadBe<uint16_t>(file);

#ifndef CXXMIDI_QUIET
        // check file type
        if ((type != 0) && (type != 1))
            std::cerr << "CxxMidi: warning: unsupported MIDI file type: " << type
            << std::endl;
#endif
        // type 0: single track
        // type 1: multi track

        // read tracks number
        uint16_t tracks_num = guts::endianness::ReadBe<uint16_t>(file);

        // reserve vector capacity
        reserve(tracks_num);

        // read time division
        time_division_ = guts::endianness::ReadBe<uint16_t>(file);

#ifndef CXXMIDI_QUIET
        // check time division
        if (time_division_ & 0x8000)
            std::cerr << "CxxMidi: warning: unsupported MIDI file time division"
            << std::endl;
#endif
    }

    void File::ReadUnknownChunk(std::ifstream& file) {
        // get chunk size
        uint32_t chunk_size = guts::endianness::ReadBe<uint32_t>(file);

        // skip chunk data
        file.seekg(chunk_size, std::ifstream::cur);
    }

    void File::ReadTrackChunk(std::ifstream& file) {
        // push back new track
        Track& track = AddTrack();

        // read track chunk size
        uint32_t chunk_size = guts::endianness::ReadBe<uint32_t>(file);
        // we will not use this size to read data (we wait for end event)

        uint8_t running_status = 0;  // start with no running status
        auto begin = file.tellg();
        bool track_continue = true;

        // read track data
        while (track_continue) {
            Event& event = track.AddEvent();

            uint32_t dt = utils::GetVlq(file);  // get delta time
            event.SetDt(dt);                    // save event delta time

            // read event data
            ReadEvent(file, &event, &track_continue, &running_status);
        }

#ifndef CXXMIDI_QUIET
        if (chunk_size != (file.tellg() - begin))
            std::cerr << "CxxMidi: warning: track data and track chunk size mismatch"
            << std::endl;
#endif
    }

    void File::ReadEvent(std::ifstream& file, Event* event, bool* track_continue,
        uint8_t* running_status) {
        uint8_t cmd = guts::endianness::ReadBe<uint8_t>(file);

        // check running status
        bool incomplete = false;
        if (cmd < 0x80) {
            incomplete = true;  // this flag says: do not read to much later
            event->push_back(*running_status);  // command from previous complete event
            event->push_back(cmd);
            cmd = *running_status;  // swap
        }
        else {
            *running_status = cmd;  // current command for this track
            event->push_back(cmd);
        }

        // control events
        switch (cmd & 0xf0) {
            // two parameter events
        case Message::kNoteOn:
        case Message::kNoteOff:
        case Message::kNoteAftertouch:
        case Message::kControlChange:
        case Message::kPitchWheel: {
            event->push_back(guts::endianness::ReadBe<uint8_t>(file));
            if (!incomplete)
                event->push_back(guts::endianness::ReadBe<uint8_t>(file));
        } break;

            // one parameter events
        case Message::kProgramChange:
        case Message::kChannelAftertouch: {
            if (!incomplete)
                event->push_back(guts::endianness::ReadBe<uint8_t>(file));
        } break;

        case 0xf0:  // META events or SysEx events
        {
            switch (cmd) {
            case Message::kMeta:  // META events
            {
                uint8_t meta_event_type = guts::endianness::ReadBe<uint8_t>(file);
                event->push_back(meta_event_type);

                switch (meta_event_type) {
                case Message::kSequenceNumber:  // size always 2
                case Message::kText:
                case Message::kCopyright:
                case Message::kTrackName:
                case Message::kInstrumentName:
                case Message::kLyrics:
                case Message::kMarker:
                case Message::kCuePoint:
                case Message::kChannelPrefix:  // size always 1
                case Message::kOutputCable:    // size always 1
                case Message::kEndOfTrack:     // size always 0
                case Message::kTempo:          // size always 3
                case Message::kSmpteOffset:    // size always 5
                case Message::kTimeSignature:
                case Message::kKeySignature: {
                    // read string length
                    uint8_t strLength = guts::endianness::ReadBe<uint8_t>(file);
                    // event_.push_back(strLength);

#ifndef CXXMIDI_QUIET
                    if ((meta_event_type == Message::kSequenceNumber) &&
                        (strLength != 2))
                        std::cerr << "CxxMidi: sequence number event size is not 2 but "
                        << strLength << std::endl;

                    if ((meta_event_type == Message::kChannelPrefix) &&
                        (strLength != 1))
                        std::cerr << "CxxMidi: channel prefix event size is not 1 but"
                        << strLength << std::endl;

                    if ((meta_event_type == Message::kOutputCable) &&
                        (strLength != 1))
                        std::cerr << "CxxMidi: output cable event size is not 1 but"
                        << strLength << std::endl;

                    if ((meta_event_type == Message::kTempo) && (strLength != 3))
                        std::cerr << "CxxMidi: tempo event size is not 3 but"
                        << strLength << std::endl;

                    if ((meta_event_type == Message::kSmpteOffset) &&
                        (strLength != 5))
                        std::cerr << "CxxMidi: SMPTE offset event size is not 5 but "
                        << strLength << std::endl;
#endif

                    if (meta_event_type == Message::kEndOfTrack) {
#ifndef CXXMIDI_QUIET
                        if (strLength != 0)
                            std::cerr << "CxxMidi: end of track event size is not 0 but "
                            << strLength << std::endl;
#endif
                        * track_continue = false;
                    }

                    // read string
                    for (int i = 0; i < strLength; i++)
                        event->push_back(guts::endianness::ReadBe<uint8_t>(file));
                } break;
                default: {
#ifndef CXXMIDI_QUIET
                    std::cerr << "CxxMidi: unknown meta event 0x" << std::hex
                        << meta_event_type << std::endl;
#endif
                } break;
                }  // switch meta_event_type
                break;
            }  // case 0xff, META events
            case Message::kSysExBegin:
            case Message::kSysExEnd: {
                uint32_t size = utils::GetVlq(file);
                for (unsigned int i = 0; i < size; i++)
                    event->push_back(guts::endianness::ReadBe<uint8_t>(file));
            } break;
            }  // switch cmd
        }    // case 0xf0
        break;
        default:
#ifndef CXXMIDI_QUIET
            std::cerr << "warning: unknown event " << static_cast<int>(cmd)
                << std::endl;
#endif
            break;
        }
    }

}  // namespace cxxmidi
