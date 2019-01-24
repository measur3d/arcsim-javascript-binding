#ifndef GEOMETRY_API_BLOB
#define GEOMETRY_API_BLOB

#include <blob/blob_formats/formats.hpp>

#include <typeinfo>


namespace Geometry
{
    class Blob
    {

    public:
        // FIXME: this structure is replicated from BinBlob interface
        //        create BinBlob automatically or convert BinBuffer to be a
        //        stream

        class BinBuffer {
        public:
            uint64_t len;
            const char* buffer;
        };


        template<typename T>
        using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)> >;
        typedef deleted_unique_ptr<BinBuffer> BinBlob_UniquePtr;

        static inline BinBlob_UniquePtr BuildSafeBlobPtr()
            {
                return BinBlob_UniquePtr( nullptr,
                                          [](BinBuffer* bb) { delete [] bb->buffer; delete bb; }
                                          );                
            }
                            
        template< class CURR_FORMAT, class PREV_FORMAT, typename = void >
        struct UpConvertHelper;

        template< class CURR_FORMAT, class PREV_FORMAT>
        struct UpConvertHelper<CURR_FORMAT, PREV_FORMAT,
                               typename std::enable_if< std::is_same< typename CURR_FORMAT::CAST_FROM , PREV_FORMAT >::value >::type >
        {
            void operator()(CURR_FORMAT& current, const blob_formats::format_base* other)
                {
                    const PREV_FORMAT* prev_ptr = dynamic_cast<const PREV_FORMAT*>( other );
                    current = *prev_ptr;
                }

        };

        template< class CURR_FORMAT, class PREV_FORMAT>
        struct UpConvertHelper<CURR_FORMAT, PREV_FORMAT,
                               typename std::enable_if< !std::is_same< typename CURR_FORMAT::CAST_FROM , PREV_FORMAT >::value >::type >
        {
            void operator()(CURR_FORMAT& current, const blob_formats::format_base* other)
                {
                    typedef typename CURR_FORMAT::CAST_FROM INTERMEDIATE_FORMAT;

                    std::unique_ptr< INTERMEDIATE_FORMAT > f_inter = std::make_unique<INTERMEDIATE_FORMAT>();
                    INTERMEDIATE_FORMAT* inter_ptr = f_inter.get();
                    UpConvertHelper< INTERMEDIATE_FORMAT, PREV_FORMAT >()(*inter_ptr, other );
                    UpConvertHelper< CURR_FORMAT, INTERMEDIATE_FORMAT>()( current, inter_ptr );
                }

        };


    private:
        const uint16_t version_major;
        const uint16_t version_minor;
        uint16_t orig_version_major;
        uint16_t orig_version_minor;
        typedef blob_formats::format_0_3 CURRENT_FORMAT;
        std::unique_ptr<CURRENT_FORMAT> blob;

    public:
        /*
        *   Constructors
        */

        Blob()
            : version_major( CURRENT_FORMAT::version_major() ),
              version_minor( CURRENT_FORMAT::version_minor() ),
              blob( std::make_unique<CURRENT_FORMAT>() )
        {}

        /*
        *   Field Accessors / Setters
        *
        *   These will likely change as the blob format changes and receives future updates
        */

        uint16_t FileVersionMajor()
        {
            return orig_version_major;
        }

        uint16_t FileVersionMinor()
        {
            return orig_version_minor;
        }

        uint16_t FormatVersionMajor()
        {
            return version_major;
        }

        uint16_t FormatVersionMinor()
        {
            return version_minor;
        }

        std::string& Name()
        {
            return blob->name;
        }

        const std::string& Name() const
        {
            return blob->name;
        }

        uint32_t NumVertices() const
        {
            return blob->n_vertices;
        }

        uint32_t NumFaces() const
        {
            return blob->n_faces;
        }

        uint32_t NumPieces() const
        {
            return blob->n_pieces;
        }

        uint32_t NumCurves() const
        {
            return blob->n_curves;
        }

        uint32_t NumTexChannels() const
        {
            return blob->n_texture_channels;
        }

        bool Has2DCoordinates() const
        {
            return blob->include_2D_coords;
        }

        void Set3DVertices(const std::vector< std::array< float, 3 > >& vertices)
        {
            blob->vertices_3D = vertices;
            blob->n_vertices = static_cast<uint32_t>(vertices.size());
        }

        void Set2DVertices(const std::vector< std::array< float, 2 > >& vertices)
        {
            blob->vertices_2D = vertices;
            if( vertices.size() > 0 )
                blob->include_2D_coords = true;
            else
                blob->include_2D_coords = false;

                // If this doesn't match the 3D vertex count, the self-check will catch it...
            blob->n_vertices = static_cast<uint32_t>(vertices.size());
        }

        void SetNumTexChannels( uint32_t channels )
        {
            blob->texture_channels.resize( channels );
            blob->n_texture_channels = channels;
        }

        void SetTexChannel(uint32_t channel, const std::vector< std::array< float, 2 > >& coords )
        {
            blob->texture_channels.at( channel ) = coords;
        }

        void SetFaces(const std::vector< std::array< uint32_t, 3> >& faces)
        {
            blob->faces = faces;
            blob->n_faces = static_cast<uint32_t>(faces.size());
        }

        void SetNumPieces( uint32_t num_pieces)
        {
            blob->pieces.resize( num_pieces );
            blob->n_pieces = num_pieces;
        }

        void SetPiece(uint32_t piece_id, const std::string& name, const std::vector<uint32_t>& vertices )
        {
            blob->pieces.at(piece_id).name = name;
            blob->pieces.at(piece_id).vertices = vertices;
        }

        void SetNumCurves( uint32_t num_curves )
        {
            blob->curves.resize( num_curves );
            blob->n_curves = num_curves;
        }

        void SetCurve( uint32_t curve_id, const std::string& name, uint32_t piece_id, const std::vector< uint32_t >& vertices )
        {
            blob->curves.at(curve_id).name = name;
            blob->curves.at(curve_id).piece_id = piece_id;
            blob->curves.at(curve_id).vertices = vertices;
        }

        void SetGeomData( std::string data_name, bool face_centric, const std::vector< double >& data )
        {

            blob->geom_data[data_name] = std::pair< bool, std::vector< double > >( face_centric, data );
        }

        const std::vector< std::array< float, 3 > >& Get3DVertices() const
        {
            return blob->vertices_3D;
        }

        const std::vector< std::array< float, 2 > >& Get2DVertices() const
        {
            return blob->vertices_2D;
        }

        const std::vector< std::array< float, 2 > >& GetTexChannel(uint32_t channel) const
        {
            return blob->texture_channels.at( channel );
        }

        const std::vector< std::array< uint32_t, 3> >& GetFaces() const
        {
            return blob->faces;
        }

        void GetPiece(uint32_t piece_id, std::string& name, std::vector<uint32_t>& vertices) const
        {
            name =  blob->pieces.at(piece_id).name;
            vertices =  blob->pieces.at(piece_id).vertices;
        }

        void GetCurve( uint32_t curve_id, std::string& name, uint32_t& piece_id, std::vector< uint32_t >& vertices ) const
        {
            name = blob->curves.at(curve_id).name;
            piece_id = blob->curves.at(curve_id).piece_id;
            vertices = blob->curves.at(curve_id).vertices;
        }

        std::vector< std::string > GetGeomDataNames()
        {
            std::vector< std::string > names;
            for (const auto& it : blob->geom_data)
                names.push_back( it.first );
            return names;
        }

        void GetGeomData( std::string data_name, bool& face_centric, std::vector< double >& data )
        {
            face_centric = blob->geom_data.at(data_name).first;
            data = blob->geom_data.at(data_name).second;
        }


        /*
        *   Validation / Load / Save Routines
        */

        bool operator==( const Blob& other ) const
        {
            return *(blob) == *(blob);
        }



        bool SelfCheck() const
        {
            return blob->self_check();
        }

        template<typename BufferType>
        void Load(const BufferType& blobdata)
        {
            std::stringstream data;

                // Load binary data into a buffer container
            data.write( blobdata.buffer, blobdata.len );
                // ... and return to the beginning
            data.seekg(0);

            Load<std::stringstream::char_type, std::stringstream::traits_type>( data );
        }

        template <typename charT, typename traits>
        void Load( std::basic_istream<charT, traits>& in_stream)
        {
            std::stringstream errout;
            blob = std::make_unique<CURRENT_FORMAT>();

            try {
                    // Trigger exceptions if we encounter a bad buffer state
                in_stream.exceptions( std::ios_base::failbit | std::ios_base::badbit | std::ios_base::eofbit );


                    // Read and check version
                {
                    uint16_t loaded_version_major = 0, loaded_version_minor = 0;
                    in_stream.read( reinterpret_cast<char*>(&loaded_version_major),  sizeof( uint16_t ) );
                    in_stream.read( reinterpret_cast<char*>(&loaded_version_minor),  sizeof( uint16_t ) );
                    if( loaded_version_major != version_major ||
                        loaded_version_minor != version_minor ){
                        if( loaded_version_major == 0 && loaded_version_minor == 1 ){
                            std::unique_ptr<blob_formats::format_0_1> format_0_1_blob = std::make_unique<blob_formats::format_0_1>();
                            format_0_1_blob->Load( in_stream );
                            format_0_1_blob->self_check();
                            UpConvertHelper<CURRENT_FORMAT, blob_formats::format_0_1>()( *blob.get(), format_0_1_blob.get() );
                        }
                        else if( loaded_version_major == 0 && loaded_version_minor == 2 ){
                            std::unique_ptr<blob_formats::format_0_2> format_0_2_blob = std::make_unique<blob_formats::format_0_2>();
                            format_0_2_blob->Load( in_stream );
                            format_0_2_blob->self_check();
                            UpConvertHelper<CURRENT_FORMAT, blob_formats::format_0_2>()( *blob.get(), format_0_2_blob.get() );
                        }
                        else{
                            errout << "Version mismatch. Expecting " << version_major << "." <<
                                version_minor << ", but got " << loaded_version_major << "." << loaded_version_minor;
                            throw BlobError::IO(errout.str());
                        }
                    }
                    else{
                        blob->Load( in_stream );
                    }
                    orig_version_major = loaded_version_major;
                    orig_version_minor = loaded_version_minor;
                }
            }
            catch( std::ios_base::failure &fail) {
                errout << "Failed to read Blob object: " << "Caught an ios_base::failure.\n"
                        << "Explanatory string: " << fail.what() << '\n'
                        << "Error code: " << fail.code();
                throw BlobError::IO(errout.str());
            }

            SelfCheck();
        }

        BinBlob_UniquePtr Save() const
        {

            std::stringstream data;
            std::stringstream errout;

            Save( data );

                // Allocate the BinBlob according to the amount of data written to the stringstream
                // The following is safe as long as the buffer we are allocating never escapes this
                // function unless wrapped by the BinBlob unique_ptr with custom deleter. This deleter
                // will take care of the deallocation once the client is finished with the BinBlob.
            std::streamoff buffer_size = data.tellp();
            if( buffer_size != std::streamoff(-1)){
                unsigned long len = static_cast<unsigned long>( buffer_size );
                char* buffer = new char[ len ];
                data.seekg(0);
                data.read( buffer, len );
                BinBlob_UniquePtr output_blob = BuildSafeBlobPtr();
                output_blob = std::make_unique<BinBuffer>();
                output_blob->buffer = buffer;
                output_blob->len = len;                
                return output_blob;
            }
            else{
                errout << "Failed to write Blob object: Error in reading buffer size.";
                throw BlobError::IO(errout.str());
            }
        }

        template <typename charT, typename traits>
        void Save( std::basic_ostream<charT,traits>& out_stream) const
        {
            SelfCheck();

            std::stringstream errout;

            try {
                    // Trigger exceptions if we encounter a bad buffer state
                out_stream.exceptions( std::ios_base::failbit | std::ios_base::badbit | std::ios_base::eofbit );

                out_stream.write( reinterpret_cast<const char*>(&version_major), sizeof( uint16_t ));
                out_stream.write( reinterpret_cast<const char*>(&version_minor), sizeof( uint16_t ));

                blob->Save( out_stream );

            }
            catch( std::ios_base::failure &fail) {
                errout << "Failed to write Blob object: " << "Caught an ios_base::failure.\n"
                        << "Explanatory string: " << fail.what() << '\n'
                        << "Error code: " << fail.code();
                throw BlobError::IO(errout.str());
            }
        }


    };
}


    /*
     *   Display Routines
     */


template <typename charT, typename traits>
std::basic_ostream<charT,traits> & operator << (std::basic_ostream<charT,traits> & out, const Geometry::Blob& blob)
{
    out << "Blob [ " << blob.Name() << " ]: ";
    out << blob.NumVertices();
    if( blob.Has2DCoordinates() )
        out << " vertices (w/ 2D), ";
    else
        out << " vertices, ";
    out << blob.NumFaces() << " faces, ";
    out << blob.NumPieces() << " pieces, ";
    out << blob.NumCurves() << " curves, ";
    out << blob.NumTexChannels() << " texture channels";


    return out;
}

#endif // GEOMETRY_API_BLOB
