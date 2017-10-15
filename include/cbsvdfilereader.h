#ifndef CBSVDFILEREADER_H
#define CBSVDFILEREADER_H

#include <sdk.h>
//#include <wx/propgrid/propgrid.h>
#include <vector>
#include <memory>
#include "tinywxuni.h"
#include <tinyxml.h>
#include <wx/string.h>
#include <algorithm>
#include <memory>
#include <globals.h>

enum svdAccessRight
{
    SVD_ACCESS_ND,
    SVD_ACCESS_READ,
    SVD_ACCESS_WRITE,
    SVD_ACCESS_READ_WRITE,
    SVD_ACCESS_WRITE_ONCE,
    SVD_ACCESS_READ_WRITE_ONCE
};

wxString SVDToWxString(svdAccessRight r);

enum svdEndianType
{
    SVD_ENDIAN_ND,
    SVD_ENDIAN_LITTLE,
    SVD_ENDIAN_BIG,
    SVD_ENDIAN_SELECTABLE,
    SVD_ENDIAN_OTHER
};

wxString SVDToWxString(svdEndianType r);

enum svdProtectionLevel
{
    SVD_PROTECTION_ND,
    SVD_PROTECTION_SECURE,
    SVD_PROTECTION_NON_SECURE,
    SVD_PROTECTION_PRIVILEGED
};

wxString SVDToWxString(svdProtectionLevel r);


enum svdRegionAccessLevel
{
    SVD_LEVEL_ND,
    SVD_LEVEL_NON_SECURE,
    SVD_LEVE_CALLABLE
};

wxString SVDToWxString(svdRegionAccessLevel r);

enum svdAddressBlockUsage
{
    SVD_USAGE_ND,
    SVD_USAGE_REGISTERS,
    SVD_USAGE_BUFFER,
    SVD_USAGE_RESERVED
};

wxString SVDToWxString(svdAddressBlockUsage r);

enum svdDataType
{
    SVD_TYPE_ND,
    SVD_TYPE_UINT8_T,
    SVD_TYPE_UINT16_T,
    SVD_TYPE_UINT32_T,
    SVD_TYPE_UINT64_T,
    SVD_TYPE_INT8_T,
    SVD_TYPE_INT16_T,
    SVD_TYPE_INT32_T,
    SVD_TYPE_INT64_T,
    SVD_TYPE_UINT8_T_P,
    SVD_TYPE_UINT16_T_P,
    SVD_TYPE_UINT32_T_P,
    SVD_TYPE_UINT64_T_P,
    SVD_TYPE_INT8_T_P,
    SVD_TYPE_INT16_T_P,
    SVD_TYPE_INT32_T_P,
    SVD_TYPE_INT64_T_P
};

wxString SVDToWxString(svdDataType r);

enum svdWriteModifications
{
    SVD_WRITE_MODIFICATION_ND,
    SVD_WRITE_MODIFICATION_NONE,
    SVD_WRITE_MODIFICATION_MODIFY,
    SVD_WRITE_MODIFICATION_ONTE_TO_CLEAR,
    SVD_WRITE_MODIFICATION_ONTE_TO_SET,
    SVD_WRITE_MODIFICATION_ONTE_TO_TOGGLE,
    SVD_WRITE_MODIFICATION_ZERO_TO_CLEAR,
    SVD_WRITE_MODIFICATION_ZERO_TO_SET,
    SVD_WRITE_MODIFICATION_ZERO_TO_TOGGLE,
    SVD_WRITE_MODIFICATION_CLEAR,
    SVD_WRITE_MODIFICATION_SET
};
wxString SVDToWxString(svdWriteModifications r);

enum svdReadModifications
{
    SVD_READ_MODIFICATION_ND,
    SVD_READ_MODIFICATION_NONE,
    SVD_READ_MODIFICATION_MODIFY,
    SVD_READ_MODIFICATION_MODIFY_EXTERNAL,
    SVD_READ_MODIFICATION_SET,
    SVD_READ_MODIFICATION_CLEAR
};

wxString SVDToWxString(svdReadModifications r);

enum svdEnumUsage
{
    SVD_ENUM_ND,
    SVD_ENUM_READ_WRITE,
    SVD_ENUM_WRITE,
    SVD_ENUM_READ
};

wxString SVDToWxString(svdEnumUsage r);

typedef uint64_t svd_address;

class SVDXMLnode
{
public:
    SVDXMLnode()    {};
    virtual ~SVDXMLnode()   {};
    virtual int ReadFromNode(TiXmlElement* node) { return 0; };
    virtual int WriteToNode(TiXmlElement* node)  { return 0; };
};


class SVDHWRevision
{
public:
    SVDHWRevision() {m_r  = 0; m_p=0;};

    SVDHWRevision(unsigned int r, unsigned int p) : m_r(r), m_p(p)
    {

    };

    SVDHWRevision(const char* s)
    {
        ReadFromString(s);
    };

    virtual ~SVDHWRevision()   {};


    int ReadFromString(const char* s)
    {
        size_t str_len = strlen(s);
        if(str_len < 2)
            return -1;
        char tmp[64];
        memset(tmp,0,64);

        if(s[0] == 'r')
        {
            unsigned int i = 1, a = 0;
            while(s[i] != 'p' && i < str_len && a < 64)
            {
                tmp[a] = s[i];
                ++a;
                ++i;
            }
            m_r = atoi(tmp);
            memset(tmp,0,64);
            ++i;
            while(i < str_len && a < 64)
            {
                tmp[a] = s[i];
                ++a;
                ++i;
            }
            m_p = atoi(tmp);

            return 0;
        }
        return -2;
    };

    wxString GetAsString()
    {
        return wxString() << wxT("r") << m_r << wxT("p") << m_p;
    };

    unsigned int GetR()     {return m_r;};
    unsigned int GetP()     {return m_p;};

    void SetR(unsigned int r)     {m_r = r;};
    void SetP(unsigned int p)     {m_p = p;};


private:

    unsigned int m_r;
    unsigned int m_p;
};

class SVDSAURegion : public SVDXMLnode
{
public:
    SVDSAURegion()
    {
        m_enabled = false;
        m_name = wxEmptyString;
        m_base  = 0;
        m_limit = 0;
        m_access = SVD_LEVEL_ND;
    };

    SVDSAURegion(svdRegionAccessLevel lvl, bool en)  : m_enabled(en), m_access(lvl) {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0;};

    wxString GetName()    const             {return m_name;};
    void     SetName(const wxString& name)  {m_name = name;};

    bool GetEnabled()    const              {return m_enabled;};
    void SetEnabled(bool en = true)        {m_enabled = en;};

    svd_address GetBaseAddress()    const   {return m_base;};
    void  SetBaseAddress(svd_address base)  {m_base = base;};

    svd_address GetLimit      ()    const   {return m_limit;};
    void  SetLimit(svd_address limit)   {m_limit = limit;};

    svdRegionAccessLevel    GetAccessLevel() {return m_access;};
    void  SetAccessLevel(svdRegionAccessLevel acc) {m_access = acc;};

private:

    bool          m_enabled;
    wxString      m_name;
    svd_address   m_base;
    svd_address   m_limit;
    svdRegionAccessLevel m_access;

};

class SVDSAURegionsConfig : public SVDXMLnode
{
public:
    SVDSAURegionsConfig()
    {
        m_enabled = false;
        m_ProtectionWhenDisabled = SVD_PROTECTION_ND;
    }

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node) { return 0;};

    bool IsEnabled()            {return m_enabled;};
    void SetEnabled(bool en)    {m_enabled = en;};

    svdProtectionLevel GetProtectionLvl()       {return m_ProtectionWhenDisabled;};
    void SetProtectionLvl(svdProtectionLevel lvl)       {m_ProtectionWhenDisabled = lvl;};

    std::vector<SVDSAURegion>::iterator GetRegionsBegin()   {return m_regions.begin();};
    std::vector<SVDSAURegion>::iterator GetRegionsEnd()     {return m_regions.end();};
    size_t GetRegionsCount()                                {return m_regions.size();};
    SVDSAURegion GetRegionsItem(size_t i)                   {return m_regions[i];};


private:
    bool m_enabled;
    svdProtectionLevel m_ProtectionWhenDisabled;
    std::vector<SVDSAURegion> m_regions;
};

class SVDArray : public SVDXMLnode
{
public:
    SVDArray()
    {
        m_dim = 0;
        m_dimIncrement = 0;
        m_dimName = wxEmptyString;
    };

    virtual ~SVDArray()    {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node) { return 0; };

    virtual int CalculateOffset(int offset) = 0;

    unsigned int GetDim()           {return m_dim;};
    svd_address  GetDimIncrement()  {return m_dimIncrement;};
    wxString     GetDimName()       {return m_dimName;};

    std::vector<wxString>::iterator GetDimIndexBegin()   {return m_dimIndex.begin();};
    std::vector<wxString>::iterator GetDimIndexEnd()     {return m_dimIndex.end();};
    size_t GGetDimIndexCount()                           {return m_dimIndex.size();};
    wxString GetDimIndexItem(size_t i)                   {return m_dimIndex[i];};

public:
    unsigned int    m_dim;
    svd_address     m_dimIncrement;
    std::vector<wxString>    m_dimIndex;
    wxString        m_dimName;
};

class SVDCpu : public SVDXMLnode
{
public:

    SVDCpu()
    {
        m_name      = wxEmptyString;
        m_revision  = SVDHWRevision();
        m_endian    = SVD_ENDIAN_ND;
        m_mpuPresent    = false;
        m_fpuPresent    = false;
        m_fpuDP         = false;
        m_iCachePresent = false;
        m_dCachePresent = false;
        m_itcmPresent   = false;
        m_dtcmPresent   = false;
        m_vtorPresent   = false;
        m_nvicPriorityBits  = 0;
        m_vendorSystickConfig = false;
        m_deviceNumInterrupts = 0;
        m_sauNumRegions = 0;
    };

    virtual ~SVDCpu()    {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node) { return 0; };

    wxString        m_name;
    SVDHWRevision   m_revision;
    svdEndianType   m_endian;
    bool            m_mpuPresent;
    bool            m_fpuPresent;
    bool            m_fpuDP;
    bool            m_iCachePresent;
    bool            m_dCachePresent;
    bool            m_itcmPresent;
    bool            m_dtcmPresent;
    bool            m_vtorPresent;
    unsigned int    m_nvicPriorityBits;
    bool            m_vendorSystickConfig;
    unsigned int    m_deviceNumInterrupts;
    unsigned int    m_sauNumRegions;
    std::vector<SVDSAURegionsConfig> m_sauRegionsConfig;
};

class SVDAddressBlock : public SVDXMLnode
{
public:
    SVDAddressBlock()
    {
        m_offset    = 0;
        m_size      = 0;
        m_usage     = SVD_USAGE_ND;
        m_protection= SVD_PROTECTION_ND;
    }

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node) { return 0; };

    unsigned int            m_offset;
    unsigned int            m_size;
    svdAddressBlockUsage    m_usage;
    svdProtectionLevel      m_protection;
};

class SVDInterrupt : public SVDXMLnode
{
public:
    SVDInterrupt()
    {
        m_name          = wxEmptyString;
        m_description   = wxEmptyString;
        m_value         = 0;
    }

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node) { return 0; };

    wxString     m_name;
    wxString     m_description;
    unsigned int m_value;
};

class SVDRegisterProperties : public SVDXMLnode
{
public:
    /*SVDRegisterProperties()
    {
        m_size       = 0;
        m_access     = SVD_ACCESS_ND;
        m_protection = wxEmptyString;
        m_resetValue = 0;
        m_resetMask  = 0;
    };*/

    SVDRegisterProperties(unsigned int size, svdAccessRight access, wxString protection, uint64_t resetValue,
                            uint64_t resetMask)
    {
        m_size = size;
        m_access = access;
        m_protection = protection;
        m_resetValue = resetValue;
        m_resetMask  = resetMask;
    };

    SVDRegisterProperties(const SVDRegisterProperties& base)
    {
        m_size = base.m_size;
        m_access = base.m_access;
        m_protection = base.m_protection;
        m_resetValue = base.m_resetValue;
        m_resetMask  = base.m_resetMask;
    };

    virtual ~SVDRegisterProperties()    {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node) {return 0; };

    wxString GetName()              const  { return m_name; };
    wxString GetDesc()              const  { return m_description; };
    unsigned int   GetSize()        const  { return m_size; };
    svdAccessRight GetAccessRight() const  { return m_access; };
    wxString       GetProtection()  const  { return m_protection; };
    uint64_t       GetResetValue()  const  { return m_resetValue; };
    uint64_t       GetResetMask()   const  { return m_resetMask; };

    void SetName(wxString name)     { m_name = name; };
    void SetDesc(wxString desc)     { m_description = desc; };

protected:
    wxString        m_name;
    wxString        m_description;
    unsigned int    m_size;
    svdAccessRight  m_access;
    wxString        m_protection;
    uint64_t        m_resetValue;
    uint64_t        m_resetMask;
};

extern const SVDRegisterProperties DefaultRegisterProperty;

class SVDRegisterBase : public SVDRegisterProperties, public SVDArray
{
public:
    SVDRegisterBase(const SVDRegisterProperties& base) : SVDRegisterProperties(base)
    {
        m_addressOffset = 0;
    };
    virtual ~SVDRegisterBase()  {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    int CalculateOffset(int offset)         { return m_addressOffset = offset * m_dimIncrement; };

    svd_address GetAddressOfset() const     { return m_addressOffset; };
    void SetAddressOfset(svd_address off)   { m_addressOffset = off; };


    inline bool operator==(const SVDRegisterBase& rhs) const
    {
        return m_name == rhs.m_name;
    }
    inline bool operator==(const wxString& rhs) const
    {
        return m_name == rhs;
    }

protected:
    svd_address m_addressOffset;

};


class SVDWriteConstrainsRange
{
    public:
    SVDWriteConstrainsRange()
    {
        m_minimum = 0;
        m_maximum = 0;
    };

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };
    uint64_t    m_minimum;
    uint64_t    m_maximum;
};

class SVDWriteConstrains
{
public:

    SVDWriteConstrains()
    {
        m_writeAsRead = false;
        m_useEnumeratedValues = false;
    }

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    bool m_writeAsRead;
    bool m_useEnumeratedValues;
    SVDWriteConstrainsRange m_range;

};

class SVDBitRange : public SVDXMLnode
{
public:
    SVDBitRange()
    {
        m_BitOffset = 0;
        m_BitWidth = 0;
    };

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    unsigned int GetOffset() const   { return m_BitOffset; };
    unsigned int GetWidth()  const   { return m_BitWidth;  };
    uint64_t     GetMask()   const   {return (static_cast<uint64_t>(-(m_BitWidth != 0)) & (static_cast<uint64_t>(-1) >> ((sizeof(uint64_t) * 8 /*CHAR_BIT*/) - m_BitWidth))) << m_BitOffset;  }    // Kudos to https://stackoverflow.com/a/28703383
    unsigned int GetLSB()    const   { return m_BitOffset;};
    unsigned int GetMSB()    const   { return m_BitOffset + m_BitWidth -1;};
    wxString     GetRange()  const   { return wxString::Format(wxT("[%d;%d]"), GetMSB(), GetLSB()); };

    void SetOffset(unsigned int off)        { m_BitOffset = off;    };
    void SetWidth(unsigned int width)       { m_BitWidth = width;   };
    void SetMask(uint64_t mask)             { };
    void SetLSB(unsigned int lsb)           { m_BitOffset = lsb;     };
    void SetMSB(unsigned int msb)           { m_BitWidth = msb - m_BitOffset; };
    void SetLSBMSB(unsigned int lsb, unsigned int msb)           {m_BitOffset = lsb;  m_BitWidth = msb - lsb +1; };

    unsigned int m_BitOffset;
    unsigned int m_BitWidth;
};

class SVDFieldsBase : public SVDRegisterBase
{
    public:
    SVDFieldsBase(const SVDRegisterBase& base) : SVDRegisterBase(base)
    {
        m_modifiedWriteValues   = SVD_WRITE_MODIFICATION_ND;
        m_readAction            = SVD_READ_MODIFICATION_ND ;
    };

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    SVDWriteConstrains    GetWriteConstraint() const       { return m_writeConstraint; };
    svdWriteModifications GetModifiedWriteValues() const   { return m_modifiedWriteValues; };
    svdReadModifications  GetReadAction() const            { return m_readAction; };

    void SetWriteConstraint(SVDWriteConstrains wc)        { m_writeConstraint = wc; };
    void SetModifiedWriteValues(svdWriteModifications wm) { m_modifiedWriteValues = wm; };
    void SetReadAction(svdReadModifications rm)           { m_readAction = rm; };

private:
    SVDWriteConstrains      m_writeConstraint;
    svdWriteModifications   m_modifiedWriteValues;
    svdReadModifications    m_readAction;
};

class SVDEnumeratedValue : public SVDXMLnode
{
    public:
    SVDEnumeratedValue()
    {
        m_name          = wxEmptyString;
        m_description   = wxEmptyString;
        m_dont_care     = false;
        m_value         = 0;
        m_isDefault     = false;
    }

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    wxString GetName()                      { return m_name; };
    void SetName(wxString name)             { m_name = name; };

    wxString GetDescription()               { return m_description; };
    void SetDescription(wxString desc)      { m_description = desc; };

    bool GetDontCare()              { return m_dont_care; };
    void SetDontCare(bool c)        { m_dont_care = c; };
    bool GetDefault()               { return m_isDefault; };
    void SetDefault(bool d)         { m_isDefault = d; };
    uint64_t GetValue()             { return m_value; };
    void  SetValue(uint64_t v)      { m_value = v; };

private:
    wxString m_name;
    wxString m_description;
    bool     m_dont_care;
    uint64_t m_value;
    bool     m_isDefault;
};

class SVDEnumeratedValues : public SVDXMLnode
{
    public:
    SVDEnumeratedValues()
    {
        m_derivedFrom    = wxEmptyString;
        m_name           = wxEmptyString;
        m_headerEnumName = wxEmptyString;
        m_usage          = SVD_ENUM_READ_WRITE;
    }

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    wxString GetDerivedFrom() const     { return m_derivedFrom; };
    wxString GetName() const            { return m_name; };
    wxString GetHeaderEnumName() const  { return m_headerEnumName; };
    svdEnumUsage GetUsage() const       { return m_usage; }

    std::vector<std::shared_ptr<SVDEnumeratedValue>>::iterator GetValuesBegin() { return m_values.begin(); };
    std::vector<std::shared_ptr<SVDEnumeratedValue>>::iterator GetValuesEnd()   { return m_values.end(); };
    size_t GetValuesSize()   { return m_values.size(); };

private:
    wxString m_derivedFrom;
    wxString m_name;
    wxString m_headerEnumName;
    svdEnumUsage m_usage;
    std::vector<std::shared_ptr<SVDEnumeratedValue> > m_values;

};

class SVDField : public SVDFieldsBase
{
    public:
    SVDField(const SVDFieldsBase& base) : SVDFieldsBase(base)
    {
        m_derivedFrom    = wxEmptyString;
        m_access         = SVD_ACCESS_ND;
    };
    virtual ~SVDField() {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };




    wxString    m_derivedFrom;
    SVDBitRange m_bitRange;
    SVDEnumeratedValues m_enumerated_value;

};

class SVDRegister : public SVDFieldsBase
{
    public:
    SVDRegister(const SVDRegisterBase& base) : SVDFieldsBase(base)
    {
        m_displayName       = wxEmptyString;
        m_alternateGroup    = wxEmptyString;
        m_alternateRegister = wxEmptyString;
        m_dataType          = SVD_TYPE_ND;
    };
    virtual ~SVDRegister()  {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    wxString m_displayName;
    wxString m_alternateGroup;
    wxString m_alternateRegister;

    svdDataType             m_dataType;
    // This members are in SVDFieldsBase
    //svdWriteModifications   m_modifiedWriteValues;
    //SVDWriteConstrains      m_writeConstraint;
    //svdReadModifications    m_readAction;

    std::vector<std::shared_ptr<SVDField> > m_fields;
};

class SVDRegisterCluster : public SVDRegisterBase
{
    public:
    //SVDRegisterCluster()  {};
    SVDRegisterCluster(const SVDRegisterBase& base) : SVDRegisterBase(base)
    {
        m_alternateCluster = wxEmptyString;
        m_headerStructName = wxEmptyString;
    };
    virtual ~SVDRegisterCluster()   {};

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    wxString    m_alternateCluster;
    wxString    m_headerStructName;


    std::vector<std::shared_ptr<SVDRegisterBase> >          m_register;
    //std::vector<std::shared_ptr<SVDRegisterCluster> >   m_cluster;


};

class SVDPeriphery : public SVDRegisterProperties, public SVDArray
{
public:
    //SVDPeriphery()     {};
    SVDPeriphery(const SVDRegisterProperties& base_prop)   : SVDRegisterProperties(base_prop)
    {

        m_derivedFrom       = wxEmptyString;
        m_version           = wxEmptyString;
        m_alternatePeripherial = wxEmptyString;
        m_groupName         = wxEmptyString;
        m_prependToName     = wxEmptyString;
        m_appendToName      = wxEmptyString;
        m_headerStructName  = wxEmptyString;
        m_disableCondition  = wxEmptyString;
        m_baseAddress       = 0;
    };
    SVDPeriphery(const SVDPeriphery& base_prop);

    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    int CalculateOffset(int offset)         {return m_baseAddress = offset * m_dimIncrement; };


    wxString GetDerivedFrom() const           { return m_derivedFrom; };
    wxString GetVersion() const               { return m_version; };
    wxString GetAlternativePath() const       { return m_alternatePeripherial; };
    wxString GetGroupName() const             { return m_groupName; };
    wxString GetPrependToName() const         { return m_prependToName; };
    wxString GetAppendToName() const          { return m_appendToName; };
    wxString GetHeaderStructName() const      { return m_headerStructName; };
    wxString GetDisableCondition() const      { return m_disableCondition; };

    void SetDerivedFrom(wxString der)         { m_derivedFrom = der; };
    void SetVersion(wxString ver)             { m_version = ver; };
    void SetAlternativePath(wxString alt)     { m_alternatePeripherial = alt; };
    void SetGroupName(wxString groupn)        { m_groupName = groupn; };
    void SetPrependToName(wxString prepend)   { m_prependToName = prepend; };
    void SetAppendToName(wxString append)     { m_appendToName = append; };
    void SetHeaderStructName(wxString name)   { m_headerStructName = name; };
    void SetDisableCondition(wxString cond)   { m_disableCondition = cond; };

    svd_address GetBaseAddress() const        { return m_baseAddress; };
    void  GetBaseAddress(svd_address addr)    { m_baseAddress = addr; };

    std::vector<SVDAddressBlock>::iterator GetAddressBlocksBegin()  { return m_addressBlocks.begin(); };
    std::vector<SVDAddressBlock>::iterator GetAddressBlocksEnd()    { return m_addressBlocks.end(); };
    size_t GetAddressBlocksCount()                                  { return m_addressBlocks.size(); };

    std::vector<SVDInterrupt>::iterator GetInterruptsBegin()  { return m_interrupts.begin(); };
    std::vector<SVDInterrupt>::iterator GetInterruptsEnd()    { return m_interrupts.end(); };
    size_t GetInterruptsSize()                                { return m_interrupts.size(); };

    std::vector<std::shared_ptr<SVDRegisterBase>>::iterator GetRegistersBegin()  { return m_registers.begin(); };
    std::vector<std::shared_ptr<SVDRegisterBase>>::iterator GetRegistersEnd()    { return m_registers.end(); };
    size_t GetRegistersSize()                                { return m_registers.size(); };

private:


    wxString    m_derivedFrom;
    wxString    m_version;
    wxString    m_alternatePeripherial;
    wxString    m_groupName;
    wxString    m_prependToName;
    wxString    m_appendToName;
    wxString    m_headerStructName;
    wxString    m_disableCondition;
    svd_address m_baseAddress;

    std::vector<SVDAddressBlock>  m_addressBlocks;
    std::vector<SVDInterrupt>     m_interrupts;
    std::vector< std::shared_ptr<SVDRegisterBase> >      m_registers;

};

typedef std::vector<std::shared_ptr<SVDPeriphery> > SVDPeriperyVector;

class SVDDevice : public SVDRegisterProperties
{
public:
    SVDDevice();

    virtual ~SVDDevice()    {};
    int ReadFromNode(TiXmlElement* node);
    int WriteToNode(TiXmlElement* node)     { return 0; };

    wxString        GetVendor()                     {return m_vendor;};
    wxString        GetVendorID()                   {return m_vendorID;};
    wxString        GetSeries()                     {return m_series;};
    wxString        GetVersion()                    {return m_version;};
    wxString        GetLicenseText()                {return m_licenseText;};
    wxString        GetHeaderSystemFilename()       {return m_headerSystemFilename;};
    wxString        GetHeaderDefinitionsPrefix()    {return m_headerDefinitionsPrefix;};
    unsigned int    GetAddressUnitBits()            {return m_AddressUnitBits;};
    unsigned int    GetWidth()                      {return m_width;};


    void SetVendor(wxString vendor)                         {m_vendor = vendor;};
    void SetVendorID(wxString vendorId)                     {m_vendorID = vendorId;};
    void SetSeries(wxString series)                         {m_series = series;};
    void SetVersion(wxString version)                       {m_version = version;};
    void SetLicenseText(wxString text)                      {m_licenseText = text;};
    void SetHeaderSystemFilename(wxString header)           {m_headerSystemFilename = header;};
    void SetHeaderDefinitionsPrefix(wxString headerPrefix)  {m_headerDefinitionsPrefix = headerPrefix;};
    void SetAddressUnitBits(unsigned int unitBits )         {m_AddressUnitBits = unitBits;};
    void SetWidth(unsigned int setWidth)                    {m_width = setWidth;};

    SVDPeriperyVector::iterator GetPeriperyBegin()              {return m_peripherals.begin();};
    SVDPeriperyVector::iterator GetPeriperyEnd()                {return m_peripherals.end();};
    SVDPeriperyVector::value_type GetPeriperyAtIndex(size_t i)  {return m_peripherals[i];};
    size_t GetPeriperyCount()                                   {return m_peripherals.size();};

private:

    wxString        m_vendor;
    wxString        m_vendorID;
    wxString        m_series;
    wxString        m_version;
    wxString        m_licenseText;
    wxString        m_headerSystemFilename;
    wxString        m_headerDefinitionsPrefix;
    unsigned int    m_AddressUnitBits;
    unsigned int    m_width;

    SVDCpu          m_cpu;
    SVDPeriperyVector m_peripherals;
};

class SVDFile
{
public:
    SVDFile();
    virtual ~SVDFile()  {};
};


class cbSVDFileReader
{
    public:
        cbSVDFileReader();
        virtual ~cbSVDFileReader();

        int LoadSVDFile(const wxString& path, SVDDevice* device);

    protected:

    private:
};



/*template<typename T>
bool SVDConvertFromChar(const char* txt, T& type)
{
    if (txt != 0)
    {
        type = txt;
        return true;
    }
    return false;
}*/



//template<>
bool inline SVDConvertFromChar(const char* txt, svdAddressBlockUsage& type)
{
    if (txt)
    {
        wxString tmp = cbC2U(txt);

        if(tmp == wxT("registers") || tmp == wxT("0"))
            type = SVD_USAGE_REGISTERS;
        else if(tmp == wxT("buffer") || tmp == wxT("1"))
            type = SVD_USAGE_BUFFER;
        else if(tmp == wxT("buffer") || tmp == wxT("2"))
            type = SVD_USAGE_RESERVED;
        else
            return false;

        return true;
    }
    return false;
}


//template<>
bool inline SVDConvertFromChar(const char* txt, svdAccessRight& type)
{
    if (txt)
    {
        wxString tmp = cbC2U(txt);

        if(tmp == wxT("read-only") )
            type = SVD_ACCESS_READ;
        else if(tmp == wxT("write-only") )
            type = SVD_ACCESS_WRITE;
        else if(tmp == wxT("read-write") )
            type = SVD_ACCESS_READ_WRITE;
        else if(tmp == wxT("writeOnce") )
            type = SVD_ACCESS_WRITE_ONCE;
        else if(tmp == wxT("read-writeOnce") )
            type = SVD_ACCESS_READ_WRITE_ONCE;
        else
            return false;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, svdDataType& type)
{
    if (txt)
    {
        wxString tmp = cbC2U(txt);

        if(tmp == wxT("uint8_t") )
            type = SVD_TYPE_UINT8_T;
        else if(tmp == wxT("uint16_t") )
            type = SVD_TYPE_UINT16_T;
        else if(tmp == wxT("uint32_t") )
            type = SVD_TYPE_UINT32_T;
        else if(tmp == wxT("uint64_t") )
            type = SVD_TYPE_UINT64_T;
        else if(tmp == wxT("int8_t") )
            type = SVD_TYPE_INT8_T;
        else if(tmp == wxT("int16_t") )
            type = SVD_TYPE_INT16_T;
        else if(tmp == wxT("int32_t") )
            type = SVD_TYPE_INT32_T;
        else if(tmp == wxT("int64_t") )
            type = SVD_TYPE_INT64_T;
        else if(tmp == wxT("uint8_t *") )
            type = SVD_TYPE_UINT8_T_P;
        else if(tmp == wxT("uint16_t *") )
            type = SVD_TYPE_UINT16_T_P;
        else if(tmp == wxT("uint32_t *") )
            type = SVD_TYPE_UINT32_T_P;
        else if(tmp == wxT("uint64_t *") )
            type = SVD_TYPE_UINT64_T_P;
        else if(tmp == wxT("int8_t *") )
            type = SVD_TYPE_INT8_T_P;
        else if(tmp == wxT("int16_t *") )
            type = SVD_TYPE_INT16_T_P;
        else if(tmp == wxT("int32_t *") )
            type = SVD_TYPE_INT32_T_P;
        else if(tmp == wxT("int64_t *") )
            type = SVD_TYPE_INT64_T_P;
        else
            return false;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, svdWriteModifications& type)
{
    if (txt)
    {
        wxString tmp = cbC2U(txt);

        if(tmp == wxT("oneToClear") )
            type = SVD_WRITE_MODIFICATION_ONTE_TO_CLEAR;
        else if(tmp == wxT("oneToSet") )
            type = SVD_WRITE_MODIFICATION_ONTE_TO_SET;
        else if(tmp == wxT("oneToToggle") )
            type = SVD_WRITE_MODIFICATION_ONTE_TO_TOGGLE;
        else if(tmp == wxT("zeroToClear") )
            type = SVD_WRITE_MODIFICATION_ZERO_TO_CLEAR;
        else if(tmp == wxT("zeroToSet") )
            type = SVD_WRITE_MODIFICATION_ZERO_TO_SET;
        else if(tmp == wxT("zeroToToggle") )
            type = SVD_WRITE_MODIFICATION_ZERO_TO_TOGGLE;
        else if(tmp == wxT("clear") )
            type = SVD_WRITE_MODIFICATION_CLEAR;
        else if(tmp == wxT("set") )
            type = SVD_WRITE_MODIFICATION_SET;
        else if(tmp == wxT("modify") )
            type = SVD_WRITE_MODIFICATION_MODIFY;
        else
            type = SVD_WRITE_MODIFICATION_NONE;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, svdReadModifications& type)
{
    if (txt)
    {
        wxString tmp = cbC2U(txt);

        if(tmp == wxT("clear") )
            type = SVD_READ_MODIFICATION_CLEAR;
        else if(tmp == wxT("set") )
            type = SVD_READ_MODIFICATION_SET;
        else if(tmp == wxT("modify") )
            type = SVD_READ_MODIFICATION_MODIFY;
        else if(tmp == wxT("modifyExternal") )
            type = SVD_READ_MODIFICATION_MODIFY_EXTERNAL;
        else
            type = SVD_READ_MODIFICATION_NONE;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, wxString& type)
{
    if (txt)
    {
        type = cbC2U(txt);
        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, unsigned int& type)
{
    if (txt)
    {
        type = std::stoull(txt, 0, 0);
        return true;
    }
    return false;
}

//template<>
/*
bool inline SVDConvertFromChar(const char* txt, unsigned long& type)
{
    if (txt)
    {
        type = std::stoull(txt, 0, 0);
        return true;
    }
    return false;
}
*/
//template<>
bool inline SVDConvertFromChar(const char* txt, uint64_t& type)
{
    if (txt)
    {
        type = std::stoull(txt, 0, 0);
        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, bool& type)
{
    if (txt)
    {
        wxString str = cbC2U(txt);
        if (str == wxT("true") || str == wxT("TRUE") || str == wxT("True") ||
            str == wxT("1") )
            type = true;
        else
            type = false;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, svdProtectionLevel& type)
{
    if (txt)
    {
        wxString str = cbC2U(txt);
        if (str == wxT("s") )
            type = SVD_PROTECTION_SECURE;
        else if (str == wxT("s") )
            type = SVD_PROTECTION_NON_SECURE;
        else if (str == wxT("p") )
            type = SVD_PROTECTION_PRIVILEGED;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, svdEnumUsage& type)
{
    if (txt)
    {
        wxString str = cbC2U(txt);
        if (str == wxT("read-write") )
            type = SVD_ENUM_READ_WRITE;
        else if (str == wxT("write") )
            type = SVD_ENUM_WRITE;
        else if (str == wxT("read") )
            type = SVD_ENUM_READ;
        else
            return false;

        return true;
    }
    return false;
}

//template<>
bool inline SVDConvertFromChar(const char* txt, svdEndianType& type)
{
    if (txt)
    {
        wxString str = cbC2U(txt);
        if (str == wxT("little") )
            type = SVD_ENDIAN_LITTLE;
        else if (str == wxT("big") )
            type = SVD_ENDIAN_BIG;
        else if (str == wxT("selectable") )
            type = SVD_ENDIAN_SELECTABLE;
        else if (str == wxT("other") )
            type = SVD_ENDIAN_OTHER;
        else
            return false;

        return true;
    }
    return false;
}


/*template<>
bool SVDConvertFromChar(const char* txt, svdAccessRight& type)
{
    if (txt)
    {
        wxString str = cbC2U(txt);
        if (str == wxT("s") )
            type = SVD_ACCESS_READ;
        else if (str == wxT("s") )
            type = SVD_ACCESS_WRITE;
        else if (str == wxT("p") )
            type = SVD_PROTECTION_PRIVILEGED;

        return true;
    }
    return false;
}*/


template<typename T>
bool ReadFromXMLElement(TiXmlElement* node, const char* name, T& type)
{
    TiXmlElement* element = node->FirstChildElement(name);
    if (element)
    {
        return SVDConvertFromChar(element->GetText(), type);
    }
    return false;
}

template<typename T>
bool ReadFromXMLAttribute(TiXmlElement* node, const char* name, T& type)
{
    const char* tmp = node->Attribute(name);
    if (tmp)
    {
        return SVDConvertFromChar(tmp, type);
    }
    return false;
}

template<typename T, typename A>
int ResolveArray(T& arr, std::shared_ptr<A> node)
{
    if (node->m_dim <= 1)
    {
        arr.push_back(node);
        return 1;
    }
    if (  node->m_dimIndex.size() < node->m_dim &&
         !node->GetName().Contains(wxT("[%s]")) )
    {
        //ERROR:  Index does not correspond with the index naming
        return -1;

    }
    for(unsigned int i = 0; i < node->m_dim; i++)
    {
        std::shared_ptr<A> tmp(new A(*(node.get())));
        tmp->SetName( wxString::Format(node->GetName(), node->m_dimIndex[i].c_str()) );
        tmp->CalculateOffset(i);
        arr.push_back(tmp);
    }
    return 1;
}

template<typename T, typename A>
int ResolveDerivedFrom(T& arr, A& node, const wxString& derived_From_name)
{
    if(derived_From_name == wxEmptyString)
        return -1;
    typename T::iterator itr;
    for(itr = arr.begin(); itr != arr.end() ; ++itr)
    {
        if ( (*itr)->GetName() == derived_From_name )
            break;
    }

    if(itr == arr.end())
    {
        //ERROR: we have not found the base register
        return -2;
    }
    else
    {
        node = **itr;
        return 0;
    }
    return 0;
}

#endif // CBSVDFILEREADER_H
