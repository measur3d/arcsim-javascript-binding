#ifndef ARCSIM_TRANSLATION_HPP_
#define ARCSIM_TRANSLATION_HPP_

#include <translation/flatbuffer_utils.hpp>
#include <translation/arcsim_serializers.hpp>
#include <translation/blob.hpp>

#include <json/json.h>

#include <array>
#include <string>
#include <sstream>
#include <memory>


class ARCSimTranslation{
public:
    static void ConvertToFB( const Geometry::Blob& blob, ARCSim::GarmentFrameT& garmentFrame);
    static void ConvertToFB( const Geometry::Blob& blob, const std::string& json, ARCSim::GarmentT& garment);
    static void ConvertToFB( const Geometry::Blob& blob, const std::string& json, std::vector<std::unique_ptr<ARCSim::ConstraintT> >& constraints);
    static void ConvertToFB( const Geometry::Blob& blob, const std::string& json, ARCSim::ObstacleT& body);

    static void ConvertFromFB( Geometry::Blob& blob, std::string& json, const ARCSim::GarmentT& garment);
    static void ConvertFromFB( Geometry::Blob& blob, std::string& json, const ARCSim::GarmentT& garment, const std::vector<ARCSim::ConstraintT>& constraints);
    static void ConvertFromFB( Geometry::Blob& blob, std::string& json, const ARCSim::ObstacleFrameT& body);
};


#endif
