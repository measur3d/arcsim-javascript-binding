
#include <translation/arcsim_translation.hpp>

#include <algorithm>

void fillMaps( std::map<std::string, uint32_t>& piece_map,
               std::vector< std::map< std::string, uint32_t> >& curve_map,
               const Geometry::Blob& blob){
  {      
      std::string name;
      std::vector<uint32_t> vertices_ms;
      for( uint32_t nPiece = 0; nPiece < blob.NumPieces(); ++nPiece ){
          blob.GetPiece( nPiece, name, vertices_ms );
          piece_map.insert( std::make_pair( name, nPiece ) );
      }
  }
  curve_map.resize( blob.NumPieces() );
  {
      std::string name;
      std::vector<uint32_t> vertices_ms;
      uint32_t piece_id;
      for( uint32_t nCurve = 0; nCurve < blob.NumCurves(); ++nCurve){
          blob.GetCurve( nCurve, name, piece_id, vertices_ms );
          curve_map.at( piece_id ).insert( std::make_pair( name, curve_map.at( piece_id ).size() ));
      }
  }
}

void LoadGeometry( ARCSim::GeometryT* geometry, const Geometry::Blob& blob)
{
    std::cout << "Load Geometry " << std::endl;
    geometry->vertices_ws = std::make_unique<ARCSim::WorldSpaceCoordinatesT>();
    for( auto vertex3D: blob.Get3DVertices() ){
        ARCSim::Vec3 v3( vertex3D[0], vertex3D[1], vertex3D[2] );
        geometry->vertices_ws->vertices.push_back( v3 );
    }
    
    if( blob.Has2DCoordinates() ){
        geometry->vertices_ms = std::make_unique<ARCSim::MaterialSpaceCoordinatesT>();
        for( auto vertex2D: blob.Get2DVertices() ){
            ARCSim::Vec2 v2( vertex2D[0], vertex2D[1] );
            geometry->vertices_ms->vertices.push_back( v2 );
        }    
    }
    
    for( uint32_t tChannel = 0; tChannel < blob.NumTexChannels(); ++tChannel){
        geometry->texture_channels.emplace_back( std::make_unique<ARCSim::MaterialSpaceCoordinatesT>() );
        const auto& channel = blob.GetTexChannel(tChannel);
        for( auto tex2D : channel ){
            ARCSim::Vec2 v2( tex2D[0], tex2D[1] );
            geometry->texture_channels[tChannel]->vertices.push_back( v2 );
        }
    }
    for( auto face: blob.GetFaces() ){
        std::unique_ptr<ARCSim::FaceT> faceT = std::make_unique<ARCSim::FaceT>();
        faceT->tri_ws = std::make_unique<ARCSim::Triangle>( face[0], face[1], face[2] );
        faceT->tri_ms = std::make_unique<ARCSim::Triangle>( face[0], face[1], face[2] );
        for( uint32_t tChannel = 0; tChannel < blob.NumTexChannels(); ++tChannel)
            faceT->tri_tx_chns.emplace_back( face[0], face[1], face[2] );
        
        geometry->faces.emplace_back( std::move( faceT ) );
    }
}

void SaveGeometry( const ARCSim::GeometryT* geometry, Geometry::Blob& blob)
{
    std::cout << "Save Geometry " << std::endl;
    
    std::vector< std::array< float, 3 > > worldspace_vertices;
    std::vector< std::array< float, 2 > > materialspace_vertices;
    std::vector< std::array< uint32_t, 3 > > worldspace_faces;
    
    if( geometry->vertices_ws )
        for( const auto& ws_vert: geometry->vertices_ws->vertices )
            worldspace_vertices.emplace_back( std::array< float, 3 > { ws_vert.x(), ws_vert.y(), ws_vert.z() } );            
    if( geometry->vertices_ms )
        for( const auto& ms_vert: geometry->vertices_ms->vertices )
            materialspace_vertices.emplace_back( std::array< float, 2 > { ms_vert.u(), ms_vert.v() } );            
    blob.Set3DVertices( worldspace_vertices );
    blob.Set2DVertices( materialspace_vertices );
    blob.SetNumTexChannels( geometry->texture_channels.size() );
    for( int channel = 0; channel < geometry->texture_channels.size(); ++channel ){
        materialspace_vertices.clear();
        for( const auto& ms_vert: geometry->texture_channels.at(channel)->vertices )
            materialspace_vertices.emplace_back( std::array< float, 2 > { ms_vert.u(), ms_vert.v() } );
        blob.SetTexChannel( channel, materialspace_vertices );
    }

    // Geometry has a richer face description, so we'll just use the worldspace ones..
    for( const auto& face : geometry->faces ){
        worldspace_faces.emplace_back( std::array< uint32_t, 3 > { face->tri_ws->a(), face->tri_ws->b(), face->tri_ws->c() } );
    }
    
    blob.SetFaces( worldspace_faces );        
}

void ARCSimTranslation::ConvertToFB( const Geometry::Blob& blob, ARCSim::GarmentFrameT& garmentFrame){
    
    garmentFrame.geometry = std::make_unique<ARCSim::GeometryT>();
    LoadGeometry( garmentFrame.geometry.get(), blob );
    for( uint32_t nPiece = 0; nPiece < blob.NumPieces(); ++nPiece ){
        std::string name;
        std::unique_ptr<ARCSim::PieceMapT> pieceMapT = std::make_unique<ARCSim::PieceMapT>();
        blob.GetPiece( nPiece, name, pieceMapT->vertices_ms );
    
        for( uint32_t nCurve = 0; nCurve < blob.NumCurves(); ++nCurve){
            std::unique_ptr<ARCSim::CurveMapT> curveMapT = std::make_unique<ARCSim::CurveMapT>();
            uint32_t piece_id;
            blob.GetCurve( nCurve, name, piece_id, curveMapT->vertices_ms);
            if( piece_id != nPiece )
                continue;
            pieceMapT->curve_maps.push_back( std::move( curveMapT ) );
        }
        garmentFrame.piece_maps.push_back( std::move( pieceMapT ) );
    }
}

void ARCSimTranslation::ConvertToFB( const Geometry::Blob& blob, const std::string& json, ARCSim::GarmentT& garment){

  Json::Value json_root; 
  std::stringstream json_stream;
  json_stream.str( json );
  json_stream >> json_root;

  std::map<std::string, uint32_t> piece_map;
  std::vector< std::map< std::string, uint32_t> > curve_map;

  fillMaps( piece_map, curve_map, blob );
  
  garment.name = blob.Name();
  garment.initial_geometry = std::make_unique<ARCSim::GarmentFrameT>();
  garment.initial_geometry->geometry = std::make_unique<ARCSim::GeometryT>();
  LoadGeometry( garment.initial_geometry->geometry.get(), blob );
  
  { // Generate a default fabric to use if nothing is specified per piece...
      std::unique_ptr<ARCSim::FabricT> fabricT = std::make_unique<ARCSim::FabricT>();
      fabricT->type = ARCSim::FabricType::AvametricV1;
      fabricT->avametric_v1_props = std::make_unique<ARCSim::AvametricV1FabricT>();
      fabricT->avametric_v1_props->density = 1.0f;
      fabricT->avametric_v1_props->stretch_c11 = 1.0f;
      fabricT->avametric_v1_props->stretch_c12 = 1.0f;
      fabricT->avametric_v1_props->stretch_c22 = 1.0f;
      fabricT->avametric_v1_props->stretch_c33 = 1.0f;
      fabricT->avametric_v1_props->bending = 1.0f;
      fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::GrayInterlock;
      garment.fabrics.push_back( std::move( fabricT ) );
  }

  if( blob.NumPieces() != json_root["pieces"].size() )
      throw 1;
  
  for( uint32_t nPiece = 0; nPiece < blob.NumPieces(); ++nPiece ){
      std::unique_ptr<ARCSim::PieceT> pieceT = std::make_unique<ARCSim::PieceT>();
      std::unique_ptr<ARCSim::PieceMapT> pieceMapT = std::make_unique<ARCSim::PieceMapT>();
      blob.GetPiece( nPiece, pieceT->name, pieceMapT->vertices_ms );
      
      Json::Value piece_root;
      bool piece_found = false;
      for( uint32_t jPiece = 0; jPiece < json_root["pieces"].size(); ++jPiece){          
          if( json_root["pieces"][jPiece]["name"] == pieceT->name ){
              piece_root = json_root["pieces"][jPiece];
              piece_found = true;
              break;
          }
      }
      if(!piece_found)
          throw 1;

      bool has_extrusion = !(piece_root["extrusion"].isNull());
      ARCSim::EdgeTreatment edgeStyle = ARCSim::EdgeTreatment::Block;
      if( has_extrusion ){
          if( piece_root["extrusion"]["type"].asString() == "round" )
              edgeStyle = ARCSim::EdgeTreatment::Round;
          else if( piece_root["extrusion"]["type"].asString() == "block" )
              edgeStyle = ARCSim::EdgeTreatment::Block;
          else if( piece_root["extrusion"]["type"].asString() == "double" )
              edgeStyle = ARCSim::EdgeTreatment::DoubleRound;
          else
              throw 1;
          
          pieceT->extrusion_thickness = piece_root["extrusion"]["thickness"].asFloat();
      }

      if(!piece_root["fabric"].isNull()){
          std::unique_ptr<ARCSim::FabricT> fabricT = std::make_unique<ARCSim::FabricT>();
          if( piece_root["fabric"]["type"] == "avametric_v1" ){
              fabricT->type = ARCSim::FabricType::AvametricV1;
              fabricT->avametric_v1_props = std::make_unique<ARCSim::AvametricV1FabricT>();
              fabricT->avametric_v1_props->density = piece_root["fabric"]["multipliers"]["density"].asFloat();
              fabricT->avametric_v1_props->stretch_c11 = piece_root["fabric"]["multipliers"]["stretch"][0].asFloat();
              fabricT->avametric_v1_props->stretch_c12 = piece_root["fabric"]["multipliers"]["stretch"][1].asFloat();
              fabricT->avametric_v1_props->stretch_c22 = piece_root["fabric"]["multipliers"]["stretch"][2].asFloat();
              fabricT->avametric_v1_props->stretch_c33 = piece_root["fabric"]["multipliers"]["stretch"][3].asFloat();
              fabricT->avametric_v1_props->bending = piece_root["fabric"]["multipliers"]["bend"].asFloat();

              if( piece_root["fabric"]["name"] == "gray-interlock")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::GrayInterlock;
              else if( piece_root["fabric"]["name"] == "11oz-black-denim")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::BlackDenim11oz;
              else if( piece_root["fabric"]["name"] == "ivory-rib-knit")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::IvoryRibKnit;
              else if( piece_root["fabric"]["name"] == "pink-ribbon-brown")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::PinkRibbonBrown;
              else if( piece_root["fabric"]["name"] == "aluminium")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::Aluminium;
              else if( piece_root["fabric"]["name"] == "royal-target")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::RoyalTarget;
              else if( piece_root["fabric"]["name"] == "camel-ponte-roma")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::CamelPonteRoma;
              else if( piece_root["fabric"]["name"] == "tango-red-jet-set")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::TangoRedJetSet;
              else if( piece_root["fabric"]["name"] == "white-dots-on-blk")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::WhiteDotsOnBlk;
              else if( piece_root["fabric"]["name"] == "white-swim-solid")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::WhiteSwimSolid;
              else if( piece_root["fabric"]["name"] == "isotropic")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::Isotropic;
              else if( piece_root["fabric"]["name"] == "navy-sparkle-sweat")
                  fabricT->avametric_v1_props->basetype = ARCSim::AvametricBaseMaterial::NavySparkleSweat;
              else
                  throw 1;
          }
          else if( piece_root["fabric"]["type"] == "gerber" ){
              fabricT->type = ARCSim::FabricType::Gerber;
              fabricT->gerber_props = std::make_unique<ARCSim::GerberFabricT>();
              fabricT->gerber_props->density = piece_root["fabric"]["parameters"]["density"].asFloat();
              fabricT->gerber_props->stretch_x = piece_root["fabric"]["parameters"]["stretchX"].asFloat();
              fabricT->gerber_props->stretch_y = piece_root["fabric"]["parameters"]["stretchY"].asFloat();
              fabricT->gerber_props->stretch_bias = piece_root["fabric"]["parameters"]["stretchBias"].asFloat();
              fabricT->gerber_props->bending_x = piece_root["fabric"]["parameters"]["bendX"].asFloat();
              fabricT->gerber_props->bending_y = piece_root["fabric"]["parameters"]["bendY"].asFloat();
              fabricT->gerber_props->bending_bias = piece_root["fabric"]["parameters"]["bendBias"].asFloat();
          }
          else if( piece_root["fabric"]["type"] == "gerber" ){
              fabricT->type = ARCSim::FabricType::SimpleAnisotropic;
              fabricT->simple_anisotropic_props = std::make_unique<ARCSim::SimpleAnisotropicFabricT>();
              fabricT->simple_anisotropic_props->density = piece_root["fabric"]["parameters"]["density"].asFloat();
              fabricT->simple_anisotropic_props->youngs_modulus_x = piece_root["fabric"]["parameters"]["youngs_modulus_X"].asFloat();
              fabricT->simple_anisotropic_props->youngs_modulus_y = piece_root["fabric"]["parameters"]["youngs_modulus_Y"].asFloat();
              fabricT->simple_anisotropic_props->poissons_ratio = piece_root["fabric"]["parameters"]["poissons_ratio"].asFloat();
              fabricT->simple_anisotropic_props->shear_modulus = piece_root["fabric"]["parameters"]["shear_modulus"].asFloat();
              fabricT->simple_anisotropic_props->bending_x = piece_root["fabric"]["parameters"]["bendX"].asFloat();
              fabricT->simple_anisotropic_props->bending_y = piece_root["fabric"]["parameters"]["bendY"].asFloat();
              fabricT->simple_anisotropic_props->bending_bias = piece_root["fabric"]["parameters"]["bendBias"].asFloat();
          }
          else
              throw 1;

          garment.fabrics.push_back( std::move( fabricT ) );
          pieceT->fabric = garment.fabrics.size()-1;
      } else
          pieceT->fabric = 0;
          
      
      for( uint32_t nCurve = 0; nCurve < blob.NumCurves(); ++nCurve){
          std::unique_ptr<ARCSim::CurveT> curveT = std::make_unique<ARCSim::CurveT>();
          std::unique_ptr<ARCSim::CurveMapT> curveMapT = std::make_unique<ARCSim::CurveMapT>();
          uint32_t piece_id;
          blob.GetCurve( nCurve, curveT->name, piece_id, curveMapT->vertices_ms);
          if( piece_id != nPiece )
              continue;

          Json::Value curve_root;
          bool curve_found = false;
          bool is_boundary = false;
          for( uint32_t jCurve = 0; jCurve < piece_root["boundary"].size(); ++jCurve){          
              if( piece_root["boundary"][jCurve]["name"] == curveT->name ){
                  curve_root = piece_root["boundary"][jCurve];
                  curve_found = true;
                  is_boundary = true;
                  break;
              }
          }
          if(!piece_found){
              for( uint32_t jCurve = 0; jCurve < piece_root["internals"].size(); ++jCurve){          
                  if( piece_root["internals"][jCurve]["name"] == curveT->name ){
                      curve_root = piece_root["internals"][jCurve];
                      curve_found = true;
                      break;
                  }
              }
          }
          if(!piece_found)
              throw 1;

          for( uint32_t cp_idx=0; cp_idx < curve_root["points"].size(); ++cp_idx ){
              curveT->control_points.emplace_back( curve_root["points"][cp_idx]["loc"][0].asFloat(),
                                                   curve_root["points"][cp_idx]["loc"][1].asFloat() );
          }
          if( curve_root["type"] == "bezier")
              curveT->type = ARCSim::CurveType::Bezier;
          else if( curve_root["type"] == "line" ||
                   curve_root["type"] == "polyline" )
              curveT->type = ARCSim::CurveType::Polyline;
          else
              throw 1;


          curveT->edge_treatment = edgeStyle;
          pieceT->curves.push_back( std::move( curveT ) );
          pieceMapT->curve_maps.push_back( std::move( curveMapT ) );
          if( is_boundary )
              pieceT->boundary.push_back( pieceT->curves.size() - 1 );
      }

      garment.pieces.push_back( std::move( pieceT ) );
      garment.initial_geometry->piece_maps.push_back( std::move( pieceMapT ) );
  }

  Json::Value sewing_root;
  sewing_root = json_root["sewing"];
  for( int seam_idx = 0; seam_idx < sewing_root.size(); ++seam_idx){
      std::unique_ptr<ARCSim::SeamT> seamT = std::make_unique<ARCSim::SeamT>();
      seamT->piece_a = piece_map.at(sewing_root[seam_idx]["first"]["piece"].asString());
      seamT->piece_b = piece_map.at(sewing_root[seam_idx]["second"]["piece"].asString());
      seamT->curve_a = curve_map.at(seamT->piece_a).at(sewing_root[seam_idx]["first"]["curve"].asString());
      seamT->curve_b = curve_map.at(seamT->piece_b).at(sewing_root[seam_idx]["second"]["curve"].asString());
      seamT->reversed = sewing_root[seam_idx]["reverse"].asBool();
      if( sewing_root[seam_idx].isMember("sewn_fold") )
          seamT->seam_angle = sewing_root[seam_idx]["sewn_fold"]["angle"].asFloat();
      garment.seams.push_back( std::move( seamT ) );
  }       

  
}
   

template<class T>
void ParseEdgeAttachments( T* ptr, const Json::Value& jsonConstraint,
                      const std::map<std::string, uint32_t>& piece_map,
                      const std::vector< std::map< std::string, uint32_t> >& curve_map) {
    ptr->cloth_attachment = std::make_unique<ARCSim::AttachmentsEdgeT>();
    for( int ea = 0; ea < jsonConstraint["edges"].size(); ++ea )
        ptr->cloth_attachment->attachments.emplace_back( piece_map.at(jsonConstraint["edges"][ea]["panel"].asString()),
                                                         curve_map.at(piece_map.at(jsonConstraint["edges"][ea]["panel"].asString()))
                                                         .at(jsonConstraint["edges"][ea]["edge"].asString()));    
}

template<class T>
void ParseBodyAttachments( T* ptr, const Json::Value& jsonConstraint ){
    if( jsonConstraint.isMember("obs_uv") ){
        ptr->body_attachment.type = ARCSim::ObstacleAttachment::AttachmentsUV;
        ptr->body_attachment.value = new ARCSim::AttachmentsUVT();
        ARCSim::AttachmentsUVT* uv_att = ptr->body_attachment.AsAttachmentsUV();
        uv_att->attachments.emplace_back( std::make_unique<ARCSim::AttachmentUVT>() );
        uv_att->attachments.at(0)->uv_loc = std::make_unique<ARCSim::Vec2>(jsonConstraint["obs_uv"][0].asFloat(),
                                                                           jsonConstraint["obs_uv"][1].asFloat());
        uv_att->attachments.at(0)->channel = 0;              
    } else {
        ptr->body_attachment.type = ARCSim::ObstacleAttachment::AttachmentsBary;
        ptr->body_attachment.value = new ARCSim::AttachmentsBaryT();
        ARCSim::AttachmentsBaryT* bary_att = ptr->body_attachment.AsAttachmentsBary();
        bary_att->attachments.emplace_back( std::make_unique<ARCSim::AttachmentBaryT>() );
        bary_att->attachments.at(0)->barycentric_coords = std::make_unique<ARCSim::Vec3>(jsonConstraint["obs_bary"][0].asFloat(),
                                                                                         jsonConstraint["obs_bary"][1].asFloat(),
                                                                                         jsonConstraint["obs_bary"][2].asFloat());              
        bary_att->attachments.at(0)->face = jsonConstraint["obs_face"].asInt();
    }
}


void ARCSimTranslation::ConvertToFB( const Geometry::Blob& blob, const std::string& json, std::vector<std::unique_ptr<ARCSim::ConstraintT> >& constraints){

  Json::Value json_root; 
  std::stringstream json_stream;
  json_stream.str( json );
  json_stream >> json_root;

  constraints.clear();

  std::map<std::string, uint32_t> piece_map;
  std::vector< std::map< std::string, uint32_t> > curve_map;

  fillMaps( piece_map, curve_map, blob );
  
  for(int constraint_id = 0; constraint_id < json_root["handles"].size(); ++constraint_id ){
      Json::Value jsonConstraint = json_root["handles"][constraint_id];

      std::unique_ptr<ARCSim::ConstraintT> constraint = std::make_unique<ARCSim::ConstraintT>();
      
      constraint->standard_props = std::make_unique<ARCSim::StandardConstraintPropertiesT>();
      constraint->standard_props->name = jsonConstraint.get("name", "").asString();
      constraint->standard_props->stiffness = jsonConstraint.get("stiffness", 1e3).asFloat();
      constraint->standard_props->start_frame = jsonConstraint.get("start_frame", 0).asInt();
      constraint->standard_props->end_frame = jsonConstraint.get("end_frame", 1000000).asInt();
      
      if( jsonConstraint["type"] == "node" ){
          constraint->type = ARCSim::ConstraintType::Node;
          constraint->node = std::make_unique<ARCSim::NodeConstraintT>();
          constraint->node->cloth_attachment = std::make_unique<ARCSim::AttachmentsNodeT>();
          for( int v = 0; v < jsonConstraint["vertices"].size(); ++v )
              constraint->node->cloth_attachment->attachments.emplace_back( jsonConstraint["vertices"][v].asInt() );               
      } else if(  jsonConstraint["type"] == "force" ){
          constraint->type = ARCSim::ConstraintType::Force;
          constraint->force = std::make_unique<ARCSim::ForceConstraintT>();
          constraint->force->direction = std::make_unique<ARCSim::Vec3>( jsonConstraint["direction"][0].asFloat(),
                                                                         jsonConstraint["direction"][1].asFloat(),
                                                                         jsonConstraint["direction"][2].asFloat() );
          if( jsonConstraint.isMember("edges") ){
              constraint->force->cloth_attachment.type = ARCSim::ClothAttachment::AttachmentsEdge;
              constraint->force->cloth_attachment.value = new ARCSim::AttachmentsEdgeT();
              ARCSim::AttachmentsEdgeT* edge_att = constraint->force->cloth_attachment.AsAttachmentsEdge();
              for( int ea = 0; ea < jsonConstraint["edges"].size(); ++ea )
                  edge_att->attachments.emplace_back( piece_map.at(jsonConstraint["edges"][ea]["panel"].asString()),
                                                      curve_map.at(piece_map.at(jsonConstraint["edges"][ea]["panel"].asString()))
                                                      .at(jsonConstraint["edges"][ea]["edge"].asString()));              
          } else {
              constraint->force->cloth_attachment.type = ARCSim::ClothAttachment::AttachmentsMs;
              constraint->force->cloth_attachment.value = new ARCSim::AttachmentsMsT();
              ARCSim::AttachmentsMsT* ms_att = constraint->force->cloth_attachment.AsAttachmentsMs();
              ms_att->attachments.emplace_back( std::make_unique<ARCSim::AttachmentMsT>() );
              ms_att->attachments.at(0)->ms_loc = std::make_unique<ARCSim::Vec2>(jsonConstraint["cloth_ms"][0].asFloat(),
                                                                                 jsonConstraint["cloth_ms"][1].asFloat());
          }

      } else if(  jsonConstraint["type"] == "pin" ){
          constraint->type = ARCSim::ConstraintType::Pin;
          constraint->pin = std::make_unique<ARCSim::PinConstraintT>();
          constraint->pin->slack = jsonConstraint["slack"].asFloat();
          constraint->pin->cloth_attachment = std::make_unique<ARCSim::AttachmentsMsT>();
          constraint->pin->cloth_attachment->attachments.emplace_back( std::make_unique<ARCSim::AttachmentMsT>() );
          constraint->pin->cloth_attachment->attachments.at(0)->ms_loc
              = std::make_unique<ARCSim::Vec2>(jsonConstraint["cloth_ms"][0].asFloat(),
                                               jsonConstraint["cloth_ms"][1].asFloat());
          ParseBodyAttachments( constraint->pin.get(), jsonConstraint );
              

      } else if(  jsonConstraint["type"] == "centering" || jsonConstraint["type"] == "collar"){
          constraint->type = ARCSim::ConstraintType::Centering;
          constraint->centering = std::make_unique<ARCSim::CenteringConstraintT>();
          ParseEdgeAttachments( constraint->centering.get(), jsonConstraint, piece_map, curve_map );
      } else if(  jsonConstraint["type"] == "barrier" ){
          constraint->type = ARCSim::ConstraintType::Barrier;
          constraint->barrier = std::make_unique<ARCSim::BarrierConstraintT>();
          constraint->barrier->normal = std::make_unique<ARCSim::Vec3>(jsonConstraint["normal"][0].asFloat(),
                                                                       jsonConstraint["normal"][1].asFloat(),
                                                                       jsonConstraint["normal"][2].asFloat());
          constraint->barrier->animate_normal = jsonConstraint.get("animate_normal",true).asBool();
          ParseEdgeAttachments( constraint->barrier.get(), jsonConstraint, piece_map, curve_map );
          ParseBodyAttachments( constraint->barrier.get(), jsonConstraint );
      } else if(  jsonConstraint["type"] == "elastic" ){
          constraint->type = ARCSim::ConstraintType::Elastic;
          constraint->elastic = std::make_unique<ARCSim::ElasticConstraintT>();
          constraint->elastic->target_length = jsonConstraint["target_length"].asFloat();
          ParseEdgeAttachments( constraint->elastic.get(), jsonConstraint, piece_map, curve_map );
      } else if(  jsonConstraint["type"] == "belt" ){
          constraint->type = ARCSim::ConstraintType::Belt;
          constraint->belt = std::make_unique<ARCSim::BeltConstraintT>();
          constraint->belt->slack = jsonConstraint["slack"].asFloat();
          constraint->belt->normal = std::make_unique<ARCSim::Vec3>(jsonConstraint["normal"][0].asFloat(),
                                                                    jsonConstraint["normal"][1].asFloat(),
                                                                    jsonConstraint["normal"][2].asFloat());
          constraint->belt->animate_normal = jsonConstraint.get("animate_normal",true).asBool();
          ParseEdgeAttachments( constraint->belt.get(), jsonConstraint, piece_map, curve_map );
          ParseBodyAttachments( constraint->belt.get(), jsonConstraint );
      } else if(  jsonConstraint["type"] == "regular_button" ){
          constraint->type = ARCSim::ConstraintType::Button;
          constraint->button = std::make_unique<ARCSim::ButtonConstraintT>();
          constraint->button->cloth_attachment = std::make_unique<ARCSim::AttachmentsMsT>();
          constraint->button->cloth_attachment->attachments.emplace_back( std::make_unique<ARCSim::AttachmentMsT>() );
          constraint->button->cloth_attachment->attachments.at(0)->ms_loc
              = std::make_unique<ARCSim::Vec2>(jsonConstraint["first"][0].asFloat(),
                                               jsonConstraint["first"][1].asFloat());
          constraint->button->cloth_attachment->attachments.emplace_back( std::make_unique<ARCSim::AttachmentMsT>() );
          constraint->button->cloth_attachment->attachments.at(1)->ms_loc
              = std::make_unique<ARCSim::Vec2>(jsonConstraint["second"][0].asFloat(),
                                               jsonConstraint["second"][1].asFloat());
      } else if(  jsonConstraint["type"] == "oriented_button" ){
          constraint->type = ARCSim::ConstraintType::OrientedButton;
          constraint->oriented_button = std::make_unique<ARCSim::OrientedButtonConstraintT>();
          constraint->oriented_button->cloth_attachment = std::make_unique<ARCSim::AttachmentsMsT>();
          constraint->oriented_button->cloth_attachment->attachments.emplace_back( std::make_unique<ARCSim::AttachmentMsT>() );
          constraint->oriented_button->cloth_attachment->attachments.at(0)->ms_loc
              = std::make_unique<ARCSim::Vec2>(jsonConstraint["first"][0].asFloat(),
                                               jsonConstraint["first"][1].asFloat());
          constraint->oriented_button->cloth_attachment->attachments.emplace_back( std::make_unique<ARCSim::AttachmentMsT>() );
          constraint->oriented_button->cloth_attachment->attachments.at(1)->ms_loc
              = std::make_unique<ARCSim::Vec2>(jsonConstraint["second"][0].asFloat(),
                                               jsonConstraint["second"][1].asFloat());
      }  
      constraints.push_back( std::move( constraint ) );
  }
}

void ARCSimTranslation::ConvertToFB( const Geometry::Blob& blob, const std::string& json, ARCSim::ObstacleT& body){
  Json::Value json_root; 
  std::stringstream json_stream;
  json_stream.str( json );
  json_stream >> json_root;
    
  body.name = blob.Name();
  body.initial_geometry = std::make_unique<ARCSim::GeometryT>();
  LoadGeometry( body.initial_geometry.get(), blob );
  
}

void ARCSimTranslation::ConvertFromFB( Geometry::Blob& blob, std::string& json, const ARCSim::GarmentT& garment){
    Json::Value json_root;
    json_root["version"] = std::string("0.2");
    json_root["handles"] = Json::Value(Json::arrayValue);
    const ARCSim::GarmentFrameT& frame = *garment.initial_geometry;    
    const ARCSim::GeometryT& geometry = *garment.initial_geometry->geometry;    
    SaveGeometry( &geometry, blob );
    blob.SetNumPieces( garment.pieces.size() );
    size_t piece_index = 0;
    size_t curve_index = 0;
    std::cout << "Converting Pieces" << std::endl;
    for( const auto& piece : garment.pieces ){
        std::cout << "Converting Piece " << piece->name << std::endl;        
        blob.SetNumCurves(curve_index+piece->curves.size());
        std::string piece_name = piece->name;
        std::vector<uint32_t> piece_vertices = frame.piece_maps.at(piece_index)->vertices_ms;
        blob.SetPiece( piece_index, piece_name, piece_vertices );
        Json::Value piece_root;
        piece_root["name"] = piece_name;
        piece_root["grain_direction"][0u] = 0;
        piece_root["grain_direction"][1] = 1;
        int piece_curve_index = 0;
        for( const auto& curve: piece->curves ){
            std::string curve_name = curve->name;
            std::vector<uint32_t> curve_vertices = frame.piece_maps.at(piece_index)->curve_maps.at(piece_curve_index)->vertices_ms;
            blob.SetCurve( curve_index, curve_name, piece_index, curve_vertices );
            curve_index++;
            piece_curve_index++;
        }
        std::cout << "Converting Boundaries" << std::endl;

        std::vector<bool> curve_is_boundary;
        curve_is_boundary.resize( piece->curves.size() );
        std::fill( curve_is_boundary.begin(), curve_is_boundary.end(), false );
        for( int boundary_index = 0; boundary_index < piece->boundary.size(); ++boundary_index ){
            const ARCSim::CurveT& curve = *piece->curves[ piece->boundary[boundary_index] ];
            curve_is_boundary[piece->boundary[boundary_index]] = true;
            piece_root["boundary"][boundary_index]["name"] = curve.name;
            if( curve.type == ARCSim::CurveType::Bezier )
                piece_root["boundary"][boundary_index]["type"] = "bezier";
            else
                piece_root["boundary"][boundary_index]["type"] = "polyline";
            for( int cp_idx = 0; cp_idx < curve.control_points.size(); ++cp_idx){
                piece_root["boundary"][boundary_index]["points"][cp_idx]["loc"][0u] = curve.control_points[cp_idx].u();
                piece_root["boundary"][boundary_index]["points"][cp_idx]["loc"][1] = curve.control_points[cp_idx].v();
            }
            for( int v_idx = 0; v_idx < frame.piece_maps.at(piece_index)->curve_maps.at(piece->boundary[boundary_index])->vertices_ms.size(); ++v_idx){
                piece_root["boundary"][boundary_index]["vertices"][v_idx] = frame.piece_maps.at(piece_index)->curve_maps.at(piece->boundary[boundary_index])->vertices_ms[v_idx];
            }
        }
        std::cout << "Converting Internals" << std::endl;
        piece_root["internals"] = Json::Value(Json::arrayValue);
        int json_internal_index = 0;
        for( int internal_index = 0; internal_index < piece->curves.size(); ++internal_index ){
            if( curve_is_boundary[internal_index] )
                continue;
            const ARCSim::CurveT& curve = *piece->curves[ internal_index ];
            piece_root["internals"][json_internal_index]["name"] = curve.name;
            if( curve.type == ARCSim::CurveType::Bezier )
                piece_root["internals"][json_internal_index]["type"] = "bezier";
            else
                piece_root["internals"][json_internal_index]["type"] = "polyline";
            for( int cp_idx = 0; cp_idx < curve.control_points.size(); ++cp_idx){
                piece_root["internals"][json_internal_index]["points"][cp_idx]["loc"][0u] = curve.control_points[cp_idx].u();
                piece_root["internals"][json_internal_index]["points"][cp_idx]["loc"][1] = curve.control_points[cp_idx].v();
            }
            for( int v_idx = 0; v_idx < frame.piece_maps.at(piece_index)->curve_maps.at(internal_index)->vertices_ms.size(); ++v_idx){
                piece_root["internals"][json_internal_index]["vertices"][v_idx] = frame.piece_maps.at(piece_index)->curve_maps.at(internal_index)->vertices_ms[v_idx];
            }
            json_internal_index++;
        }
        piece_index++;
        for( int fold_index = 0; fold_index < piece->folds.size(); ++fold_index ){
            const ARCSim::FoldT& fold = *(piece->folds[fold_index]);
            if( fold.type == ARCSim::FoldType::Simple ){
                piece_root["folds"][fold_index]["type"] = "simple";
                piece_root["folds"][fold_index]["angle"] = fold.angle;
                piece_root["folds"][fold_index]["curves"][0u] = piece->curves[fold.curve]->name;
            }
            else{
                piece_root["folds"][fold_index]["type"] = "graduated";
                piece_root["folds"][fold_index]["start_angle"] = fold.start_angle;
                piece_root["folds"][fold_index]["end_angle"] = fold.end_angle;
                piece_root["folds"][fold_index]["curves"][0u] = piece->curves[fold.curve]->name;
            }
        }
        std::cout << "Converting Fabric" << std::endl;
        
        const ARCSim::FabricT& fabric = *(garment.fabrics[piece->fabric]);
        if( fabric.type == ARCSim::FabricType::AvametricV1 ){
            const ARCSim::AvametricV1FabricT& avametricProps = *(fabric.avametric_v1_props);
            piece_root["fabric"]["type"] = "avametric_v1";
            switch( avametricProps.basetype ){
            case ARCSim::AvametricBaseMaterial::GrayInterlock:
                piece_root["fabric"]["name"] = "gray-interlock";
                break;
            case ARCSim::AvametricBaseMaterial::BlackDenim11oz:
                piece_root["fabric"]["name"] = "11oz-black-denim";
                break;
            case ARCSim::AvametricBaseMaterial::IvoryRibKnit:
                piece_root["fabric"]["name"] = "ivory-rib-knit";
                break;
            case ARCSim::AvametricBaseMaterial::PinkRibbonBrown:
                piece_root["fabric"]["name"] = "pink-ribbon-brown";
                break;
            case ARCSim::AvametricBaseMaterial::Aluminium:
                piece_root["fabric"]["name"] = "aluminium";
                break;
            case ARCSim::AvametricBaseMaterial::RoyalTarget:
                piece_root["fabric"]["name"] = "royal-target";
                break;
            case ARCSim::AvametricBaseMaterial::CamelPonteRoma:
                piece_root["fabric"]["name"] = "camel-ponte-roma";
                break;
            case ARCSim::AvametricBaseMaterial::TangoRedJetSet:
                piece_root["fabric"]["name"] = "tango-red-jet-set";
                break;
            case ARCSim::AvametricBaseMaterial::WhiteDotsOnBlk:
                piece_root["fabric"]["name"] = "white-dots-on-blk";
                break;
            case ARCSim::AvametricBaseMaterial::WhiteSwimSolid:
                piece_root["fabric"]["name"] = "white-swim-solid";
                break;
            case ARCSim::AvametricBaseMaterial::Isotropic:
                piece_root["fabric"]["name"] = "isotropic";
                break;
            case ARCSim::AvametricBaseMaterial::NavySparkleSweat:
                piece_root["fabric"]["name"] = "navy-sparkle-sweat";
                break;
            default:
                piece_root["fabric"]["name"] = "";
            };
            piece_root["fabric"]["multipliers"]["stretch"][0u] = avametricProps.stretch_c11;
            piece_root["fabric"]["multipliers"]["stretch"][1] = avametricProps.stretch_c12;
            piece_root["fabric"]["multipliers"]["stretch"][2] = avametricProps.stretch_c22;
            piece_root["fabric"]["multipliers"]["stretch"][3] = avametricProps.stretch_c33;
            piece_root["fabric"]["multipliers"]["bend"] = avametricProps.bending;
            piece_root["fabric"]["multipliers"]["density"] = avametricProps.density;
        }
        if( fabric.type == ARCSim::FabricType::Gerber ){
            const ARCSim::GerberFabricT& gerberProps = *(fabric.gerber_props);
            piece_root["fabric"]["type"] = "gerber";
            piece_root["fabric"]["name"] = std::to_string(piece->fabric);
            piece_root["fabric"]["parameters"]["stretchX"] = gerberProps.stretch_x;
            piece_root["fabric"]["parameters"]["stretchY"] = gerberProps.stretch_y;
            piece_root["fabric"]["parameters"]["stretchBias"] = gerberProps.stretch_bias;
            piece_root["fabric"]["parameters"]["bendX"] = gerberProps.bending_x;
            piece_root["fabric"]["parameters"]["bendY"] = gerberProps.bending_y;
            piece_root["fabric"]["parameters"]["bendBias"] = gerberProps.bending_bias;
            piece_root["fabric"]["parameters"]["density"] = gerberProps.density;                
        }
        if( fabric.type == ARCSim::FabricType::SimpleAnisotropic ){
            const ARCSim::SimpleAnisotropicFabricT& simpleProps = *(fabric.simple_anisotropic_props);
            piece_root["fabric"]["type"] = "simple_anisotropic";
            piece_root["fabric"]["name"] = std::to_string(piece->fabric);
            piece_root["fabric"]["parameters"]["youngs_modulus_X"] = simpleProps.youngs_modulus_x;
            piece_root["fabric"]["parameters"]["youngs_modulus_Y"] = simpleProps.youngs_modulus_y;
            piece_root["fabric"]["parameters"]["poissons_ratio"] = simpleProps.poissons_ratio;
            piece_root["fabric"]["parameters"]["shear_modulus"] = simpleProps.shear_modulus;
            piece_root["fabric"]["parameters"]["bendX"] = simpleProps.bending_x;
            piece_root["fabric"]["parameters"]["bendY"] = simpleProps.bending_y;
            piece_root["fabric"]["parameters"]["bendBias"] = simpleProps.bending_bias;
            piece_root["fabric"]["parameters"]["density"] = simpleProps.density; 
        }
        if( piece->curves[0]->edge_treatment == ARCSim::EdgeTreatment::Block)
            piece_root["extrusion"]["type"] = "block";
        if( piece->curves[0]->edge_treatment == ARCSim::EdgeTreatment::Round)
            piece_root["extrusion"]["type"] = "round";
        if( piece->curves[0]->edge_treatment == ARCSim::EdgeTreatment::DoubleRound)
            piece_root["extrusion"]["type"] = "double";
        piece_root["extrusion"]["thickness"] = piece->extrusion_thickness;
        json_root["pieces"].append( piece_root );
    }
    std::cout << "Converting Sewing" << std::endl;

    for( const auto& seam : garment.seams ){
        Json::Value seam_root;
        seam_root["sewn_fold"]["type"] = "simple";
        seam_root["sewn_fold"]["angle"] = seam->seam_angle;
        seam_root["reverse"] = seam->reversed;
        seam_root["first"]["piece"] = garment.pieces[seam->piece_a]->name;
        seam_root["second"]["piece"] = garment.pieces[seam->piece_b]->name;
        seam_root["first"]["curve"] = garment.pieces[seam->piece_a]->curves[seam->curve_a]->name;
        seam_root["second"]["curve"] = garment.pieces[seam->piece_b]->curves[seam->curve_b]->name;
        json_root["sewing"].append( seam_root );
    }
    std::stringstream json_stream;
    json_stream << json_root;
    //std::cout << json_root << std::endl;
    json = json_stream.str();
}

void ARCSimTranslation::ConvertFromFB( Geometry::Blob& blob, std::string& json, const ARCSim::GarmentT& garment, const std::vector<ARCSim::ConstraintT>& constraints){


}
    
void ARCSimTranslation::ConvertFromFB( Geometry::Blob& blob, std::string& json, const ARCSim::ObstacleFrameT& body){

    const ARCSim::GeometryT& geometry = *body.geometry;
    blob.Name() = "undefined";
    SaveGeometry( &geometry, blob );
    
    Json::Value json_root;
    json_root["name"] = "undefined";
    json_root["version"] = "0.1";
    std::stringstream json_stream;
    json_stream << json_root;
    json = json_stream.str();
}
