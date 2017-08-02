#ifndef CBSVPGPROPERTIES_H
#define CBSVPGPROPERTIES_H

#include <cstdint>
#include "cbsvdfilereader.h"
#include <map>
#include <math>
#include <sdk>

typedef std::vector<uint8_t> data_vetor;

class svPGPropBase
{
    public:
        svPGPropBase()
        virtual ~svPGPropBase();
        virtual void SetData(data_vetor &data, data_vetor::iterator& start ) = 0;

    protected:
        uint64_t m_baseAddr;
        uint64_t m_addr;
        uint64_t m_offset;
        uint64_t m_size;

};


class svPGPeripheryProp : public wxStringProperty, svPGPropBase
{
    public:
        svPGPeripheryProp(SVDPeriphery &per);
        virtual ~svPGPeripheryProp();

        virtual void SetData(data_vetor &data, data_vetor::iterator& start );
    protected:

    private:
};

class svPGRegisterProp : public wxStringProperty, svPGPropBase
{
    public:
        svPGRegisterProp(SVDRegister &reg);
        virtual ~svPGRegisterProp();

        virtual void SetData(data_vetor &data, data_vetor::iterator& start );
    protected:

    private:

};

class svPGFieldProp : public wxEnumProperty, svPGPropBase
{
    public:
        svPGFieldProp(SVDField &field);
        virtual ~svPGFieldProp();

        virtual void SetData(data_vetor &data, data_vetor::iterator& start );
    protected:

    private:

        uint64_t m_data;
        uint64_t m_mask;
        uint64_t m_resetValue;
        uint64_t m_resetMask;
        uint64_t m_bitSize;
        uint64_t m_bitOffset;

        std::map<int, uint64_t> m_indexValueMap;

};

class svPGBitProp : public wxBoolProperty, svPGPropBase
{
    public:
        svPGBitProp(SVDField &field);
        virtual ~svPGBitProp();

        virtual void SetData(data_vetor &data, data_vetor::iterator& start );
    protected:

    private:

        uint64_t m_data;
        uint64_t m_mask;
        uint64_t m_resetValue;
        uint64_t m_resetMask;
        uint64_t m_bitSize;
        uint64_t m_bitOffset;
};

class svPGValueProp : public wxStringProperty, svPGPropBase
{
    public:
        svPGValueProp(SVDField &field);
        virtual ~svPGValueProp();

        virtual void SetData(data_vetor &data, data_vetor::iterator& start );
    protected:

    private:

        uint64_t m_data;
        uint64_t m_mask;
        uint64_t m_resetValue;
        uint64_t m_resetMask;
        uint64_t m_bitSize;
        uint64_t m_bitOffset;
};
#endif // CBSVPGPROPERTIES_H
