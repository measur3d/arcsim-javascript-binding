#ifndef FLATBUFFER_UTILS_HPP_
#define FLATBUFFER_UTILS_HPP_

#include <flatbuffers/flatbuffers.h>

#include <utility>



template<class T>
std::pair<uint8_t*, size_t> PackToBytestream(T* ptr, const char* identifier){
    static std::unique_ptr<flatbuffers::FlatBufferBuilder> fbb;
    fbb = std::make_unique<flatbuffers::FlatBufferBuilder>();

    typedef typename T::TableType TableType;
    flatbuffers::Offset<TableType> res = TableType::Pack(*fbb, ptr);
    if( identifier )
        fbb->Finish(res, identifier );
    else
        fbb->Finish(res, nullptr );
    
    uint8_t *buf = fbb->GetBufferPointer();
    size_t size = fbb->GetSize();
    return std::make_pair(buf, size);
}

template<class T>
T* UnPackFromBytestream(const uint8_t* buffer, size_t length, const char* identifier){
    typedef typename T::TableType TableType;
    
    auto verifier = flatbuffers::Verifier(buffer, length);
    bool bufferOk;
    if( identifier )
        bufferOk = verifier.VerifyBuffer<TableType>(identifier);
    else
        bufferOk = verifier.VerifyBuffer<TableType>(nullptr);
    if( !bufferOk )
        return nullptr;
    
    auto root = flatbuffers::GetRoot<TableType>(buffer);
    return root->UnPack();
}


#endif
