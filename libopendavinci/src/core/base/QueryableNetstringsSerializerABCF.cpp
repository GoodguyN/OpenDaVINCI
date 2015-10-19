/**
 * OpenDaVINCI - Portable middleware for distributed components.
 * Copyright (C) 2008 - 2015 Christian Berger, Bernhard Rumpe
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "core/base/QueryableNetstringsSerializerABCF.h"
#include "core/base/Serializable.h"

namespace core {
    namespace base {

        using namespace std;

        QueryableNetstringsSerializerABCF::QueryableNetstringsSerializerABCF() :
            m_buffer() {}

        QueryableNetstringsSerializerABCF::~QueryableNetstringsSerializerABCF() {}


        uint8_t QueryableNetstringsSerializerABCF::encodeVarInt(ostream &out, uint64_t value) {
            // We will write at least one byte.
            uint8_t size = 1;

            // Encode as little endian like in Protobuf's case.
            value = htole64(value);

            char byte = 0;
            while (value > 0x7f) {
                // If the value to be written occupies more than 7 bits, we need to encode it using the MSB flag.
                byte = (static_cast<uint8_t>(value & 0x7f)) | 0x80;
                out.put(byte);
                // Remove the seven bits that we have already written.
                value >>= 7;
                // Start next byte.
                size++;
            }
            // Write final value.
            byte = (static_cast<uint8_t>(value)) & 0x7f;
            out.put(byte);

            return size;
        }

        void QueryableNetstringsSerializerABCF::getSerializedData(ostream &o) {
            // Write magic number.
            uint16_t magicNumber = 0xABCF;
            magicNumber = htons(magicNumber);
            o.write(reinterpret_cast<const char *>(&magicNumber), sizeof(uint16_t));

            // Write length.
            uint64_t length = static_cast<uint32_t>(m_buffer.str().length());
            encodeVarInt(o, length);

            // Write payload.
            o << m_buffer.str();

            // Write End-Of-Data for checking corruptness.
            o << ",";
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const Serializable &s) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            stringstream buffer;
            buffer << s;

            uint32_t size = static_cast<uint32_t>(buffer.str().length());
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            m_buffer << buffer.str();
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const bool &b) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(b));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
            m_buffer.write(reinterpret_cast<const char *>(&b), sizeof(const bool));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const char &c) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(c));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
            m_buffer.write(&c, sizeof(const char));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const unsigned char &uc) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(uc));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
            m_buffer.write(reinterpret_cast<const char *>(&uc), sizeof(const unsigned char));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const int8_t &i) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(i));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            int8_t _i = i;
            m_buffer.write(reinterpret_cast<const char *>(&_i), sizeof(const int8_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const int16_t &i) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(i));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            int16_t _i = i;
            _i = htons(_i);
            m_buffer.write(reinterpret_cast<const char *>(&_i), sizeof(const int16_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const uint16_t &ui) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(ui));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            uint16_t _ui = ui;
            _ui = htons(_ui);
            m_buffer.write(reinterpret_cast<const char *>(&_ui), sizeof(const uint16_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const int32_t &i) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(i));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            int32_t _i = i;
            _i = htonl(_i);
            m_buffer.write(reinterpret_cast<const char *>(&_i), sizeof(const int32_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const uint32_t &ui) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(ui));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            uint32_t _ui = ui;
            _ui = htonl(_ui);
            m_buffer.write(reinterpret_cast<const char *>(&_ui), sizeof(const uint32_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const int64_t &i) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(i));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            int64_t _i = i;
            _i = __htonll(_i);
            m_buffer.write(reinterpret_cast<const char *>(&_i), sizeof(const int64_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const uint64_t &ui) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(ui));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            uint64_t _ui = ui;
            _ui = __htonll(_ui);
            m_buffer.write(reinterpret_cast<const char *>(&_ui), sizeof(const uint64_t));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const float &f) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(f));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            float _f = f;
            _f = Serializer::htonf(_f);
            m_buffer.write(reinterpret_cast<const char *>(&_f), sizeof(const float));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const double &d) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t size = static_cast<uint32_t>(sizeof(d));
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

            double _d = d;
            _d = Serializer::htond(_d);
            m_buffer.write(reinterpret_cast<const char *>(&_d), sizeof(const double));
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const string &s) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t stringLength = s.length();
            uint32_t size = static_cast<uint32_t>(stringLength + sizeof(uint32_t)); // String's length plus the length of the type for (string's length).
            size = htonl(size);
            m_buffer.write(reinterpret_cast<const char *>(&size), sizeof(uint32_t));

            uint32_t _stringLength = stringLength;
            _stringLength = htonl(_stringLength);
            m_buffer.write(reinterpret_cast<const char *>(&_stringLength), sizeof(uint32_t));
            m_buffer.write(s.c_str(), stringLength);
        }

        void QueryableNetstringsSerializerABCF::write(const uint32_t &id, const void *data, const uint32_t &size) {
            uint32_t _id = id;
            _id = htonl(_id);
            m_buffer.write(reinterpret_cast<const char *>(&_id), sizeof(uint32_t));

            uint32_t realSize = size;
            realSize = htonl(realSize);
            m_buffer.write(reinterpret_cast<const char*>(&realSize), sizeof(uint32_t));
            m_buffer.write(reinterpret_cast<const char*>(data), size);
        }

    }
} // core::base
