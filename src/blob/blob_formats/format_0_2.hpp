#ifndef GEOMETRY_BLOB_FORMAT_0_2_
#define GEOMETRY_BLOB_FORMAT_0_2_


namespace Geometry
{
    namespace blob_formats
    {
        struct format_0_2 : public format_base
        {
            typedef format_0_1 CAST_FROM;
            typedef std::array<float, 3> VEC3D;
            typedef std::array<float, 2> VEC2D;
            typedef std::array<uint32_t, 3> TRIANGLE;

            struct PIECE
            {
                std::string name;
                std::vector< uint32_t > vertices;
                bool operator==(const PIECE& other) const
                {
                    return name == other.name &&
                        std::equal( vertices.begin(), vertices.end(), other.vertices.begin() );
                }

            };

            struct CURVE
            {
                std::string name;
                uint32_t piece_id;
                std::vector< uint32_t > vertices;
                bool operator==(const CURVE& other) const
                {
                    return name == other.name &&
                        piece_id == other.piece_id &&
                        std::equal( vertices.begin(), vertices.end(), other.vertices.begin() );
                }

            };

            typedef std::vector< VEC3D > VERTEX3D_ARRAY;
            typedef std::vector< VEC2D > VERTEX2D_ARRAY;
            typedef std::vector< TRIANGLE > TRIANGLE_ARRAY;
            typedef std::vector< PIECE > PIECE_ARRAY;
            typedef std::vector< CURVE > CURVE_ARRAY;

            std::string name;
            uint32_t n_texture_channels;
            bool include_2D_coords;
            uint32_t n_vertices;
            uint32_t n_faces;
            uint32_t n_pieces;
            uint32_t n_curves;
            VERTEX3D_ARRAY vertices_3D;
            VERTEX2D_ARRAY vertices_2D;
            std::vector< VERTEX2D_ARRAY > texture_channels;
            TRIANGLE_ARRAY faces;
            PIECE_ARRAY pieces;
            CURVE_ARRAY curves;

            static uint32_t version_major()
            {
                return 0;
            }
            static uint32_t version_minor()
            {
                return 2;
            }

            format_0_2& operator=( const CAST_FROM& other )
            {
                name = other.name;
                n_texture_channels = other.n_texture_channels;
                include_2D_coords = other.include_2D_coords;
                n_vertices = other.n_vertices;
                n_faces = other.n_faces;
                n_pieces = other.n_pieces;
                n_curves = other.n_curves;
                vertices_3D = other.vertices_3D;
                vertices_2D = other.vertices_2D;
                texture_channels = other.texture_channels;
                faces = other.faces;
                for( uint32_t c = 0; c < n_curves; c++){
                    CURVE curve;
                    curve.name = other.curves.at(c).name;
                    curve.piece_id = other.curves.at(c).piece_id;
                    curve.vertices = other.curves.at(c).vertices;
                    curves.push_back(curve);
                }

                    // Build reverse vertex / face map for acceleration
                std::map< uint32_t, std::vector<uint32_t> > verts2faces;
                for( uint32_t f = 0; f < n_faces; f++)
                    for( auto v : faces.at(f) )
                        verts2faces[v].push_back( f );


                for( uint32_t p = 0; p < n_pieces; p++){
                    PIECE piece;
                    piece.name = other.pieces.at(p);
                        // Generate interior vertices....
                    std::unordered_set<uint32_t> vert_set;
                    std::vector< uint32_t > vert_queue;
                    for(const auto& curve : curves )
                        if( curve.piece_id == p ){
                            vert_set.insert( curve.vertices.begin(), curve.vertices.end() );
                        }

                    vert_queue.push_back( *vert_set.begin() );
                    while( vert_queue.size() > 0 ){
                        uint32_t vert = vert_queue.back();
                        vert_queue.pop_back();
                        const auto& triangles = verts2faces.at(vert);
                        for( auto triangle : triangles )
                            for( auto v : faces.at(triangle) ){
                                auto res = vert_set.insert( v );
                                if(res.second)
                                    vert_queue.push_back(v);
                            }
                    }
                    piece.vertices.insert( piece.vertices.begin(), vert_set.begin(), vert_set.end() );
                    std::sort( piece.vertices.begin(), piece.vertices.end() );
                    pieces.push_back( piece );
                }
                return *this;
            }



            virtual bool operator==( const format_0_2& other) const
            {
                bool is_equal = true;
                is_equal = is_equal && name == other.name;
                is_equal = is_equal && n_texture_channels == other.n_texture_channels;
                is_equal = is_equal && include_2D_coords == other.include_2D_coords;
                is_equal = is_equal && n_vertices == other.n_vertices;
                is_equal = is_equal && n_faces == other.n_faces;
                is_equal = is_equal && n_pieces == other.n_pieces;
                is_equal = is_equal && n_curves == other.n_curves;
                is_equal = is_equal && std::equal( vertices_3D.begin(), vertices_3D.end(), other.vertices_3D.begin());
                is_equal = is_equal && std::equal( vertices_2D.begin(), vertices_2D.end(), other.vertices_2D.begin());
                is_equal = is_equal && std::equal( texture_channels.begin(), texture_channels.end(), other.texture_channels.begin());
                is_equal = is_equal && std::equal( faces.begin(), faces.end(), other.faces.begin());
                is_equal = is_equal && std::equal( pieces.begin(), pieces.end(), other.pieces.begin());
                is_equal = is_equal && std::equal( curves.begin(), curves.end(), other.curves.begin());
                return is_equal;
            }

            virtual bool self_check() const
            {

                std::stringstream errout;


                if( vertices_3D.size()  != n_vertices ){
                    errout << "Self-Check failed! 3D vertices have an incorrect length of " << vertices_3D.size() << " and should be " << n_vertices ;
                    throw BlobError::Consistency(errout.str());
                }

                if( include_2D_coords ){
                    if (vertices_2D.size() != n_vertices){
                        errout << "Self-Check failed! 2D vertices have an incorrect length of " << vertices_2D.size() << " and should be " << n_vertices ;
                        throw BlobError::Consistency(errout.str());
                    }
                }
                else
                    if (vertices_2D.size() != 0 )
                        throw BlobError::Consistency("Self-Check failed! 2D not included, but 2D vertices have data.");

                if (faces.size()!= n_faces){
                    errout << "Self-Check failed! Faces have an incorrect length of " << faces.size() << " and should be " << n_faces ;
                    throw BlobError::Consistency(errout.str());
                }

                for( uint32_t f = 0; f < faces.size(); f++ ){
                    for( uint32_t v = 0; v < faces[f].size(); v++ ){
                        if( faces[f][v] >= n_vertices ){
                            errout << "Self-Check failed! Face " << f << ", Index " << v << " references a vertex beyond the number of known vertices.";
                            throw BlobError::Consistency( errout.str() );
                        }
                    }
                }

                if(pieces.size() != n_pieces){
                    errout << "Self-Check failed! Pieces have an incorrect length of " << pieces.size() << " and should be " << n_pieces ;
                    throw BlobError::Consistency(errout.str());
                }

                for( auto&& piece : pieces ){
                    for( uint32_t v = 0; v < piece.vertices.size(); v++ )
                        if( piece.vertices.at(v) >= n_vertices ){
                            errout << "Self-Check failed! Piece '" << piece.name << "' vertex at position " << v << " is out of bounds" ;
                            throw BlobError::Consistency( errout.str() );
                        }
                }

                if (curves.size() != n_curves){
                    errout << "Self-Check failed! Curves have an incorrect length of " << curves.size() << " and should be " << n_curves ;
                    throw BlobError::Consistency(errout.str());
                }


                if (texture_channels.size() != n_texture_channels){
                    errout << "Self-Check failed! Number of texture channels is " << texture_channels.size() << " and should be " << n_texture_channels ;
                    throw BlobError::Consistency(errout.str());
                }


                for( uint32_t c = 0; c < texture_channels.size(); c++ )
                    if( texture_channels.at(c).size() != n_vertices ){
                        errout << "Self-Check failed! Texture channel " << c << " has an incorrect length of " <<  texture_channels.at(c).size();
                        throw BlobError::Consistency(errout.str());
                    }


                for( auto&& curve : curves ){
                    if( curve.piece_id >= n_pieces ){
                        errout << "Self-Check failed! Curve '" << curve.name << "' is attached to invalid piece index " << curve.piece_id ;
                        throw BlobError::Consistency( errout.str() );
                    }
                    if( curve.vertices.size() <= 0 ){
                        errout << "Self-Check failed! Curve '" << curve.name << "' has zero vertices" ;
                        throw BlobError::Consistency( errout.str() );
                    }
                    for( uint32_t v = 0; v < curve.vertices.size(); v++ )
                        if( curve.vertices.at(v) >= n_vertices ){
                            errout << "Self-Check failed! Curve '" << curve.name << "' vertex at position " << v << " is out of bounds" ;
                            throw BlobError::Consistency( errout.str() );
                        }
                }

                return true;

            }

            virtual void Load( std::istream& data )
            {
                    // Read name
                {
                    name = read_string( data );
                }

                    // Read Tex channel count
                {
                    data.read( reinterpret_cast<char*>(&n_texture_channels), sizeof( uint32_t ) );
                }

                    // Read 2D data inclusion
                {
                    uint32_t has_2d_data;
                    data.read( reinterpret_cast<char*>(&has_2d_data), sizeof( uint32_t ) );
                    include_2D_coords = has_2d_data > 0;
                }

                    // Skip reserved space
                {
                    uint32_t a,b;
                    data.read( reinterpret_cast<char*>(&a), sizeof(uint32_t ) );
                    data.read( reinterpret_cast<char*>(&b), sizeof(uint32_t ) );
                }


                    // Read Element counts
                {
                    data.read( reinterpret_cast<char*>(&n_vertices), sizeof(uint32_t) );
                    data.read( reinterpret_cast<char*>(&n_faces), sizeof(uint32_t) );
                    data.read( reinterpret_cast<char*>(&n_pieces), sizeof(uint32_t) );
                    data.read( reinterpret_cast<char*>(&n_curves), sizeof(uint32_t) );
                }

                    // Skip reserved space
                {
                    uint32_t a,b;
                    data.read( reinterpret_cast<char*>(&a), sizeof(uint32_t ) );
                    data.read( reinterpret_cast<char*>(&b), sizeof(uint32_t ) );
                }

                    // Read 3D vertices
                {
                    for( uint32_t v = 0; v < n_vertices; v++ ){
                        VEC3D vertex;
                        data.read( reinterpret_cast<char*>( vertex.data() ), sizeof(VEC3D) );
                        vertices_3D.push_back( vertex );
                    }
                }

                    // Read 2D vertices
                {
                    if( include_2D_coords ){
                        for( uint32_t v = 0; v < n_vertices; v++ ){
                            VEC2D vertex;
                            data.read( reinterpret_cast<char*>( vertex.data() ), sizeof(VEC2D) );
                            vertices_2D.push_back( vertex );
                        }
                    }
                    else
                        vertices_2D.resize(0);
                }

                    // Read Texture Coords
                {
                    for( uint32_t c = 0 ; c < n_texture_channels; c++ ){
                        VERTEX2D_ARRAY tex_channel;
                        for( uint32_t v = 0; v < n_vertices; v++ ){
                            VEC2D vertex;
                            data.read( reinterpret_cast<char*>( vertex.data() ), sizeof(VEC2D) );
                            tex_channel.push_back( vertex );
                        }
                        texture_channels.push_back( tex_channel );
                    }
                }

                    // Read Faces
                {
                    for( uint32_t v = 0; v < n_faces; v++ ){
                        TRIANGLE tri;
                        data.read( reinterpret_cast<char*>( tri.data() ), sizeof(TRIANGLE) );
                        faces.push_back( tri );
                    }
                }

                    // Read Pieces (Names Only)
                {
                    for( uint32_t p = 0; p < n_pieces; p++){
                        PIECE piece;
                        piece.name = read_string( data );
                        pieces.push_back( piece );
                    }
                }

                    // Read Curves
                {
                    for( uint32_t c = 0; c < n_curves; c++ ){
                        CURVE curve;
                        curve.name = read_string(data);
                        data.read( reinterpret_cast<char*>( &(curve.piece_id) ), sizeof( uint32_t ) );
                        uint32_t curve_verts;
                        data.read( reinterpret_cast<char*>( &curve_verts ), sizeof( uint32_t ) );
                        for( uint32_t v = 0; v < curve_verts; v++ ){
                            uint32_t vert_idx;
                            data.read( reinterpret_cast<char*>( &vert_idx ), sizeof( uint32_t ) );
                            curve.vertices.push_back( vert_idx );
                        }
                        curves.push_back( curve );
                    }
                }

                    // Skip reserved space
                {
                    uint32_t a; // Read 8 times for 256 bytes of reserved space
                    for( int i = 0; i < 8; i++ )
                        data.read( reinterpret_cast<char*>(&a), sizeof(uint32_t ) );
                }

                    // Read Pieces (Vertices)
                {
                    for( uint32_t p = 0; p < n_pieces; p++){
                        PIECE& piece = pieces.at(p);
                        uint32_t piece_verts;
                        data.read( reinterpret_cast<char*>( &piece_verts ), sizeof( uint32_t ) );
                        for( uint32_t v = 0; v < piece_verts; v++ ){
                            uint32_t vert_idx;
                            data.read( reinterpret_cast<char*>( &vert_idx ), sizeof( uint32_t ) );
                            piece.vertices.push_back( vert_idx );
                        }
                    }
                }

            }

            virtual void Save( std::ostream& data ) const
            {
                    // Write name
                {
                    write_string( data, name );
                }

                    // Write Tex channel count
                {
                    data.write( reinterpret_cast<const char*>(&n_texture_channels), sizeof( uint32_t ) );
                }

                    // Write 2D data inclusion
                {
                    uint32_t has_2d_data = include_2D_coords ? 1 : 0;
                    data.write( reinterpret_cast<const char*>(&has_2d_data), sizeof( uint32_t ) );

                }

                    // Skip reserved space
                {
                    uint32_t a=0x0,b=0x0;
                    data.write( reinterpret_cast<const char*>(&a), sizeof(uint32_t ) );
                    data.write( reinterpret_cast<const char*>(&b), sizeof(uint32_t ) );
                }


                    // Write Element counts
                {
                    data.write( reinterpret_cast<const char*>(&n_vertices), sizeof(uint32_t) );
                    data.write( reinterpret_cast<const char*>(&n_faces), sizeof(uint32_t) );
                    data.write( reinterpret_cast<const char*>(&n_pieces), sizeof(uint32_t) );
                    data.write( reinterpret_cast<const char*>(&n_curves), sizeof(uint32_t) );
                }

                    // Skip reserved space
                {
                    uint32_t a=0x0,b=0x0;
                    data.write( reinterpret_cast<const char*>(&a), sizeof(uint32_t ) );
                    data.write( reinterpret_cast<const char*>(&b), sizeof(uint32_t ) );
                }

                    // Write 3D vertices
                {
                    for( uint32_t v = 0; v < n_vertices; v++ ){
                        const VEC3D& vertex = vertices_3D.at(v);
                        data.write( reinterpret_cast<const char*>( vertex.data() ), sizeof(VEC3D) );
                    }
                }

                    // Write 2D vertices
                {
                    if( include_2D_coords ){
                        for( uint32_t v = 0; v < n_vertices; v++ ){
                            const VEC2D& vertex = vertices_2D.at(v);
                            data.write( reinterpret_cast<const char*>( vertex.data() ), sizeof(VEC2D) );
                        }
                    }
                }

                    // Write Texture Coords
                {
                    for( uint32_t c = 0 ; c < n_texture_channels; c++ ){
                        const VERTEX2D_ARRAY& tex_channel = texture_channels.at(c);
                        for( uint32_t v = 0; v < n_vertices; v++ ){
                            const VEC2D& vertex = tex_channel.at(v);
                            data.write( reinterpret_cast<const char*>( vertex.data() ), sizeof(VEC2D) );
                        }
                    }
                }

                    // Write Faces
                {
                    for( uint32_t v = 0; v < n_faces; v++ ){
                        const TRIANGLE& tri = faces.at(v);
                        data.write( reinterpret_cast<const char*>( tri.data() ), sizeof(TRIANGLE) );
                    }
                }

                    // Write Pieces (Names Only)
                {
                    for( uint32_t p = 0; p < n_pieces; p++){
                        const PIECE& piece = pieces.at(p);
                        write_string( data, piece.name );
                    }
                }

                    // Write Curves
                {
                    for( uint32_t c = 0; c < n_curves; c++ ){
                        const CURVE& curve = curves.at(c);
                        write_string( data, curve.name );
                        data.write( reinterpret_cast<const char*>( &(curve.piece_id) ), sizeof( uint32_t ) );
                        uint32_t curve_verts = static_cast<uint32_t>(curve.vertices.size());
                        data.write( reinterpret_cast<const char*>( &curve_verts ), sizeof( uint32_t ) );
                        for( uint32_t v = 0; v < curve_verts; v++ ){
                            const uint32_t& vert_idx = curve.vertices.at(v);
                            data.write( reinterpret_cast<const char*>( &vert_idx ), sizeof( uint32_t ) );
                        }
                    }
                }

                    // Skip reserved space
                {
                    uint32_t a=0x0; // Write 8 times for 256 bytes of reserved space
                    for( int i = 0; i < 8; i++ )
                        data.write( reinterpret_cast<char*>(&a), sizeof(uint32_t ) );
                }

                    // Write Pieces (Vertices)
                {
                    for( uint32_t p = 0; p < n_pieces; p++){
                        const PIECE& piece = pieces.at(p);
                        uint32_t piece_verts = static_cast<uint32_t>(piece.vertices.size());
                        data.write( reinterpret_cast<const char*>( &piece_verts ), sizeof( uint32_t ) );
                        for( uint32_t v = 0; v < piece_verts; v++ ){
                            const uint32_t& vert_idx = piece.vertices.at(v);
                            data.write( reinterpret_cast<const char*>( &vert_idx ), sizeof( uint32_t ) );
                        }
                    }
                }

            }
        };
    }
}


#endif // GEOMETRY_BLOB_FORMAT_0_2_
