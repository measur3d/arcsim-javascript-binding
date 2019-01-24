#ifndef GEOMETRY_BLOB_FORMATS_
#define GEOMETRY_BLOB_FORMATS_

#include <stdexcept>
#include <vector>
#include <array>
#include <memory>
#include <sstream>
#include <cstring>
#include <string>
#include <cstdint>
#include <functional>
#include <iostream>
#include <unordered_set>
#include <map>
#include <algorithm>


namespace Geometry
{
    namespace BlobError 
    {
        class Consistency : public std::runtime_error
        {
        public:

            Consistency()
                : std::runtime_error("")
            {}


            Consistency(const std::string& s)
                : std::runtime_error(s)
            {}
        };


        class IO : public std::runtime_error
        {
        public:

            IO() :
                std::runtime_error("")
            {}


            IO(const std::string& s)
                : std::runtime_error(s)
            {}
        };
    }


    namespace blob_formats
    {

        struct format_base
        {

            virtual bool self_check() const
            {
                return false;
            }

            virtual void Load( std::istream& in )
            {}


            virtual void Save( std::ostream& out ) const
            {}

            virtual bool operator==( const format_base& other) const
            {
                return true;
            }

            template<class PREV_FORMAT>
            format_base& operator=( const PREV_FORMAT& other)
            {
                throw BlobError::IO( "Cannot perform this conversion type." );
            }

            std::string read_string( std::istream& in )
            {
                uint32_t string_len;
                std::string str;
                std::unique_ptr<char[]> buffer;
                in.read( reinterpret_cast<char*>(&string_len), sizeof(uint32_t) );
                buffer = std::make_unique<char[]>(string_len+1); // Add one, as the format does not use null terminated strings...
                memset( buffer.get(), 0, string_len+1); // Make sure our string is zeroed, for safety.
                in.read( buffer.get(), string_len );
                str = std::string( buffer.get() );
                return str;
            }

            void write_string( std::ostream& out, std::string data ) const
            {
                uint32_t string_len = static_cast<uint32_t>(data.size());
                out.write( reinterpret_cast<char*>(&string_len), sizeof(uint32_t) );
                out.write( data.c_str(), string_len ); // Note! This is simply the number of characters. It does not include a null terminator!
            }

            static void tokenize(std::string str, std::string del, std::vector<std::string> &token_v) 
            {
                std::size_t start = str.find_first_not_of(del), end=start;
                while (start != std::string::npos){
                        // Find next occurence of delimiter
                    end = str.find_first_of(del, start);
                        // Push back the token found into vector
                    token_v.push_back(str.substr(start, end-start));
                        // Skip all occurences of the delimiter to find new start
                    start = str.find_first_not_of(del, end);
                }
            }

        };

    }

}

//TODO: this needs to be revised since not standard method of inclusion

#include <blob/blob_formats/format_0_1.hpp>
#include <blob/blob_formats/format_0_2.hpp>
#include <blob/blob_formats/format_0_3.hpp>


#endif // GEOMETRY_BLOB_FORMATS_
