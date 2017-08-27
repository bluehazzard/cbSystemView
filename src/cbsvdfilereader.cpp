#include "cbsvdfilereader.h"

#include <string.h>
#include <wx/tokenzr.h>

#include "tinywxuni.h"


wxString SVDToWxString(svdAccessRight r)
{
    switch(r)
    {
        case SVD_ACCESS_ND:             return wxString(wxT("ACCESS_NOT_DEFINED"));
        case SVD_ACCESS_READ:           return wxString(wxT("ACCESS_READ"));
        case SVD_ACCESS_WRITE:          return wxString(wxT("ACCESS_WRITE"));
        case SVD_ACCESS_READ_WRITE:     return wxString(wxT("ACCESS_READ_WRITE"));
        case SVD_ACCESS_WRITE_ONCE:     return wxString(wxT("ACCESS_WRITE_ONCE"));
        case SVD_ACCESS_READ_WRITE_ONCE: return wxString(wxT("ACCESS_READ_WRITE_ONCE"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdEndianType r)
{
    switch(r)
    {
        case SVD_ENDIAN_ND:         return wxString(wxT("ENDIAN_NOT_DEFINED"));
        case SVD_ENDIAN_LITTLE:     return wxString(wxT("ENDIAN_LITTLE"));
        case SVD_ENDIAN_BIG:        return wxString(wxT("ENDIAN_BIG"));
        case SVD_ENDIAN_SELECTABLE: return wxString(wxT("ENDIAN_SELECTABLE"));
        case SVD_ENDIAN_OTHER:      return wxString(wxT("ENDIAN_OTHER"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdProtectionLevel r)
{
    switch(r)
    {
        case SVD_PROTECTION_ND:         return wxString(wxT("PROTECTION_NOT_DEFINED"));
        case SVD_PROTECTION_SECURE:     return wxString(wxT("PROTECTION_SECURE"));
        case SVD_PROTECTION_NON_SECURE: return wxString(wxT("PROTECTION_NON_SECURE"));
        case SVD_PROTECTION_PRIVILEGED: return wxString(wxT("PROTECTION_PRIVILEGED"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdRegionAccessLevel r)
{
    switch(r)
    {
        case SVD_LEVEL_ND:          return wxString(wxT("LEVEL_NOT_DEFINED"));
        case SVD_LEVEL_NON_SECURE:  return wxString(wxT("LEVEL_NON_SECURE"));
        case SVD_LEVE_CALLABLE:     return wxString(wxT("LEVE_CALLABLE"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdAddressBlockUsage r)
{
    switch(r)
    {
        case SVD_USAGE_ND:         return wxString(wxT("USAGE_NOT_DEFINED"));
        case SVD_USAGE_REGISTERS:  return wxString(wxT("USAGE_REGISTERS"));
        case SVD_USAGE_BUFFER:     return wxString(wxT("USAGE_BUFFER"));
        case SVD_USAGE_RESERVED:   return wxString(wxT("USAGE_RESERVED"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdDataType r)
{
    switch(r)
    {
        case SVD_TYPE_ND:           return wxString(wxT("Not defined"));
        case SVD_TYPE_UINT8_T:      return wxString(wxT("uint8_t"));
        case SVD_TYPE_UINT16_T:     return wxString(wxT("uitn16_t"));
        case SVD_TYPE_UINT32_T:     return wxString(wxT("uint32_t"));
        case SVD_TYPE_UINT64_T:     return wxString(wxT("uint64_t"));
        case SVD_TYPE_INT8_T:       return wxString(wxT("int8_t"));
        case SVD_TYPE_INT16_T:      return wxString(wxT("itn16_t"));
        case SVD_TYPE_INT32_T:      return wxString(wxT("int32_t"));
        case SVD_TYPE_INT64_T:      return wxString(wxT("int64_t"));
        case SVD_TYPE_UINT8_T_P:    return wxString(wxT("*uint8_t"));
        case SVD_TYPE_UINT16_T_P:   return wxString(wxT("*uitn16_t"));
        case SVD_TYPE_UINT32_T_P:   return wxString(wxT("*uint32_t"));
        case SVD_TYPE_UINT64_T_P:   return wxString(wxT("*uint64_t"));
        case SVD_TYPE_INT8_T_P:     return wxString(wxT("*int8_t"));
        case SVD_TYPE_INT16_T_P:    return wxString(wxT("*itn16_t"));
        case SVD_TYPE_INT32_T_P:    return wxString(wxT("*int32_t"));
        case SVD_TYPE_INT64_T_P:    return wxString(wxT("*int64_t"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdWriteModifications r)
{
    switch(r)
    {
        case SVD_WRITE_MODIFICATION_ND:                 return wxString(wxT("MODIFICATION_NOT_DEFINED"));
        case SVD_WRITE_MODIFICATION_NONE:               return wxString(wxT("MODIFICATION_NONE"));
        case SVD_WRITE_MODIFICATION_MODIFY:             return wxString(wxT("MODIFICATION_MODIFY"));
        case SVD_WRITE_MODIFICATION_ONTE_TO_CLEAR:      return wxString(wxT("MODIFICATION_ONTE_TO_CLEAR"));
        case SVD_WRITE_MODIFICATION_ONTE_TO_SET:        return wxString(wxT("MODIFICATION_ONTE_TO_SET"));
        case SVD_WRITE_MODIFICATION_ONTE_TO_TOGGLE:     return wxString(wxT("MODIFICATION_ONTE_TO_TOGGLE"));
        case SVD_WRITE_MODIFICATION_ZERO_TO_CLEAR:      return wxString(wxT("MODIFICATION_ZERO_TO_CLEAR"));
        case SVD_WRITE_MODIFICATION_ZERO_TO_SET:        return wxString(wxT("MODIFICATION_ZERO_TO_SET"));
        case SVD_WRITE_MODIFICATION_ZERO_TO_TOGGLE:     return wxString(wxT("MODIFICATION_ZERO_TO_TOGGLE"));
        case SVD_WRITE_MODIFICATION_CLEAR:              return wxString(wxT("MODIFICATION_CLEAR"));
        case SVD_WRITE_MODIFICATION_SET:                return wxString(wxT("MODIFICATION_SET"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdReadModifications r)
{
    switch(r)
    {
        case SVD_READ_MODIFICATION_ND:              return wxString(wxT("MODIFICATION_NOT_DEFINED"));
        case SVD_READ_MODIFICATION_NONE:            return wxString(wxT("MODIFICATION_NONE"));
        case SVD_READ_MODIFICATION_MODIFY:          return wxString(wxT("MODIFICATION_MODIFY"));
        case SVD_READ_MODIFICATION_MODIFY_EXTERNAL: return wxString(wxT("MODIFICATION_MODIFY_EXTERNAL"));
        case SVD_READ_MODIFICATION_SET:             return wxString(wxT("MODIFICATION_SET"));
        case SVD_READ_MODIFICATION_CLEAR:           return wxString(wxT("MODIFICATION_CLEAR"));
    }
    return wxEmptyString;
};

wxString SVDToWxString(svdEnumUsage& r)
{
    switch(r)
    {
        case SVD_ENUM_ND:           return wxString(wxT("NOT_DEFINED"));
        case SVD_ENUM_READ_WRITE:   return wxString(wxT("READ_WRITE"));
        case SVD_ENUM_WRITE:        return wxString(wxT("WRITE"));
        case SVD_ENUM_READ:         return wxString(wxT("READ"));
    }
    return wxEmptyString;
};

const SVDRegisterProperties DefaultRegisterProperty = SVDRegisterProperties(0, SVD_ACCESS_READ_WRITE, wxT("n"), 0, 0);

SVDDevice::SVDDevice() : SVDRegisterProperties(DefaultRegisterProperty)
{
    m_vendor        = wxEmptyString;
    m_vendorID      = wxEmptyString;
    m_name          = wxEmptyString;
    m_series        = wxEmptyString;
    m_version       = wxEmptyString;
    m_description   = wxEmptyString;
    m_licenseText   = wxEmptyString;
    m_headerSystemFilename      = wxEmptyString;
    m_headerDefinitionsPrefix   = wxEmptyString;
    m_AddressUnitBits   = 0;
    m_width             = 0;
}

int SVDSAURegion::ReadFromNode(TiXmlElement* node)
{
    m_enabled = false;
    ReadFromXMLAttribute(node, "enable", m_enabled);
    m_name    = wxT("NoName");
    ReadFromXMLAttribute(node, "name", m_name);

    ReadFromXMLElement(node, "base", m_base);
    ReadFromXMLElement(node, "limit", m_limit);

    wxString tmp = wxT("n");
    ReadFromXMLElement(node, "protectionWhenDisabled", tmp);
    if(tmp == wxT("n"))
        m_access = SVD_LEVEL_NON_SECURE;
    else if(tmp == wxT("c"))
        m_access = SVD_LEVE_CALLABLE;

    return 0;
}


int SVDArray::ReadFromNode(TiXmlElement* node)
{
    m_dim           = 0;
    m_dimIncrement  = 0;
    m_dimName       = wxEmptyString;
    wxString dimIndexes = wxEmptyString;

    ReadFromXMLElement( node, "dim", m_dim);
    ReadFromXMLElement( node, "dimIncrement", m_dimIncrement);
    ReadFromXMLElement( node, "dimName", m_dimName);

    ReadFromXMLElement( node, "dimIndex" , dimIndexes );
    if(m_dim != 0)
    {
        m_dimIndex.clear();

        if(dimIndexes.Contains( wxT(",")) )
        {
            wxStringTokenizer tok(dimIndexes, wxT(","));
            while ( tok.HasMoreTokens() )
                m_dimIndex.push_back( tok.GetNextToken() );
        }
        else
        {
            wxString from = dimIndexes.BeforeFirst('-');
            wxString to   = dimIndexes.AfterLast('-');
            long l_from = 0, l_to = 0;
            if( !from.ToLong( &l_from ) || !to.ToLong( &l_to ))
            {
                // error
            }
            // create list with index names
            for( long i = l_from;i <= l_to;i++ )
            {
                m_dimIndex.push_back( wxString::Format( wxT("%d"), i ));
            }
        }

    }
    return 0;
}

int SVDSAURegionsConfig::ReadFromNode(TiXmlElement* node)
{
    m_enabled = false;
    ReadFromXMLAttribute(node, "enable", m_enabled);

    wxString tmp = wxT("n");
    ReadFromXMLElement(node, "protectionWhenDisabled", tmp);
    if(tmp == wxT("n"))
        m_ProtectionWhenDisabled = SVD_PROTECTION_NON_SECURE;
    else if(tmp == wxT("s"))
        m_ProtectionWhenDisabled = SVD_PROTECTION_SECURE;
    else if(tmp == wxT("p"))
        m_ProtectionWhenDisabled = SVD_PROTECTION_PRIVILEGED;

    TiXmlElement* element = node->FirstChildElement("region");
    m_regions.clear();
    while(element != nullptr)
    {
        SVDSAURegion tmp_region(m_ProtectionWhenDisabled == SVD_PROTECTION_NON_SECURE ? SVD_LEVEL_NON_SECURE : SVD_LEVE_CALLABLE ,m_enabled);
        tmp_region.ReadFromNode(element);
        m_regions.push_back(tmp_region);

        element = element->NextSiblingElement("region");
    }
    return 0;
}

int SVDRegisterProperties::ReadFromNode(TiXmlElement* node)
{

    ReadFromXMLElement(node, "size",        m_size);
    ReadFromXMLElement(node, "access"    ,  m_access);
    ReadFromXMLElement(node, "protection",  m_protection);
    ReadFromXMLElement(node, "resetValue",  m_resetValue);
    ReadFromXMLElement(node, "resetMask" ,  m_resetMask);

    return 0;
}

int SVDAddressBlock::ReadFromNode( TiXmlElement *node )
{
    ReadFromXMLElement(node, "offset",      m_offset);
    ReadFromXMLElement(node, "size",        m_size);
    ReadFromXMLElement(node, "usage",       m_usage);
    ReadFromXMLElement(node, "protection",  m_protection);

    return 0;
}

int SVDRegisterCluster::ReadFromNode(TiXmlElement* node)
{
    SVDRegisterBase::ReadFromNode(node);
    // Read Register or cluster elements

    ReadFromXMLElement(node, "alternateCluster", m_alternateCluster);
    ReadFromXMLElement(node, "headerStructName", m_headerStructName);

    TiXmlElement* element = node->FirstChildElement("cluster");
    m_register.clear();
    while(element != nullptr)
    {
        std::shared_ptr<SVDRegisterCluster> cluster = std::shared_ptr<SVDRegisterCluster>(new SVDRegisterCluster(*this));
        wxString derivedFrom;
        if (ReadFromXMLAttribute(element, "derivedFrom", derivedFrom))
        {
            ResolveDerivedFrom(m_register, *cluster.get(),derivedFrom);
        }
        cluster->ReadFromNode(element);
        if (ResolveArray(m_register,cluster) < 0)
        {
            // error resolving array
        }
        element = element->NextSiblingElement("cluster");
    }

    element = node->FirstChildElement("register");
    while(element != nullptr)
    {
        std::shared_ptr<SVDRegister> regist = std::shared_ptr<SVDRegister>( new SVDRegister(*this) );
        wxString derivedFrom;
        if(ReadFromXMLAttribute(element, "derivedFrom", derivedFrom))
        {
            ResolveDerivedFrom(m_register, *regist.get(), derivedFrom);
        }
        regist->ReadFromNode(element);
        if(ResolveArray(m_register, regist) < 0)
        {
            // error resolving array
        }
        element = element->NextSiblingElement("register");
    }

    return 0;
}

int SVDRegister::ReadFromNode(TiXmlElement* node)
{
    SVDRegisterBase::ReadFromNode(node);
    SVDFieldsBase::ReadFromNode(node);

    ReadFromXMLElement(node, "displayName",         m_displayName);
    ReadFromXMLElement(node, "alternateGroup",      m_alternateGroup);
    ReadFromXMLElement(node, "alternateRegister",   m_alternateRegister);
    ReadFromXMLElement(node, "dataType",            m_dataType);

    /*TiXmlElement* element = node->FirstChildElement("writeConstraint");
    if (element != nullptr)
    {
         m_writeConstraint.ReadFromNode(element);
    }*/

    //ReadFromXMLElement(node, "readAction",          m_readAction);
    TiXmlElement* element = node->FirstChildElement("fields");
    m_fields.clear();
    if(element)
    {
        element = element->FirstChildElement("field");
        while(element != nullptr)
        {
            std::shared_ptr<SVDField> field = std::shared_ptr<SVDField>(new SVDField(*this));
            wxString derivedFrom;
            if(ReadFromXMLAttribute(element,"derivedFrom",derivedFrom))
            {
                // TODO (bluehazzard#1#): This does not work, because the path has to be relative, and we do not support this... for ex. in periperhal A and registerX, derive from peripheralA.registerYY.fieldYY.
                ResolveDerivedFrom(m_fields, *field.get(),derivedFrom);
            }
            field->ReadFromNode(element);
            if(ResolveArray(m_fields,field) < 0)
            {
                 // error resolving array
                 return -1;
            }
            // TODO (bluehazzard#1#): The item displayName has also to be specialized with the array name
            element = element->NextSiblingElement("field");
        }
    }
    return 0;
}

int SVDWriteConstrainsRange::ReadFromNode(TiXmlElement* node)
{
    ReadFromXMLElement(node, "minimum", m_minimum);
    ReadFromXMLElement(node, "maximum", m_maximum);
    return 0;
}

int SVDWriteConstrains::ReadFromNode(TiXmlElement* node)
{
    ReadFromXMLElement(node, "writeAsRead", m_writeAsRead);
    ReadFromXMLElement(node, "writeAsRead", m_useEnumeratedValues);

    TiXmlElement* element = node->FirstChildElement("range");
    if (element != nullptr)
    {
        m_range.ReadFromNode(element);
    }
    return 0;
}

int SVDBitRange::ReadFromNode(TiXmlElement* node)
{
    TiXmlElement* element = node->FirstChildElement("bitOffset");
    if(element)
    {
        ReadFromXMLElement(node, "bitOffset", m_BitOffset);
        ReadFromXMLElement(node, "bitWidth", m_BitWidth);
        return 0;
    }
    element = node->FirstChildElement("lsb");
    if(element)
    {
        unsigned int lsb = 0, msb = 0;
        ReadFromXMLElement(node, "lsb", lsb);
        ReadFromXMLElement(node, "msb", msb);
        SetLSBMSB(lsb, msb);
        return 0;
    }
    element = node->FirstChildElement("bitRange");
    if(element)
    {
        wxString bitRange = wxEmptyString;
        ReadFromXMLElement(node, "bitRange", bitRange);

        if(bitRange != wxEmptyString)
        {
            wxString s_msb, s_lsb;
            s_msb = bitRange.BeforeFirst(':');
            s_lsb = bitRange.AfterFirst(':');
            s_msb = s_msb.AfterLast('[');
            s_lsb = s_lsb.BeforeFirst(']');
            long lsb = 0, msb = 0, result = 1;
            result &= s_msb.ToLong(&msb);
            result &= s_lsb.ToLong(&lsb);
            if(!result)
            {
                // ERROR:
                return -1;
            }

            SetLSBMSB(lsb, msb);
            return 0;
        }

        return -1;
    }
    return -1;
}

int SVDFieldsBase::ReadFromNode(TiXmlElement* node)
{
    SVDArray::ReadFromNode(node);
    SVDRegisterBase::ReadFromNode(node);

    m_writeConstraint.ReadFromNode(node);

    ReadFromXMLElement(node, "description",         m_description);
    ReadFromXMLElement(node, "modifiedWriteValues", m_modifiedWriteValues);
    ReadFromXMLElement(node, "readAction",          m_readAction);

    return 0;
}

int SVDField::ReadFromNode(TiXmlElement* node)
{
    SVDFieldsBase::ReadFromNode(node);
    m_derivedFrom = wxEmptyString;
    ReadFromXMLAttribute(node, "derivedFrom", m_derivedFrom);

    ReadFromXMLElement(node, "name",        m_name);
    ReadFromXMLElement(node, "description", m_description);

    m_bitRange.ReadFromNode(node);
    TiXmlElement* element = node->FirstChildElement("enumeratedValues");
    if (element)
    {
        m_enumerated_value.ReadFromNode(element);
    }
    return 0;
}

int SVDInterrupt::ReadFromNode(TiXmlElement* node)
{
    ReadFromXMLElement(node, "name",        m_name);
    ReadFromXMLElement(node, "description", m_description);
    ReadFromXMLElement(node, "value",       m_value);

    return 0;
}

int SVDEnumeratedValues::ReadFromNode(TiXmlElement* node)
{
    ReadFromXMLElement(node, "name",           m_name);
    ReadFromXMLElement(node, "headerEnumName", m_headerEnumName);
    ReadFromXMLElement(node, "usage",          m_usage);


    m_values.clear();

    TiXmlElement* element = node->FirstChildElement("enumeratedValue");
    while(element != nullptr)
    {
        std::shared_ptr<SVDEnumeratedValue> value = std::shared_ptr<SVDEnumeratedValue>(new SVDEnumeratedValue());
        wxString derivedFrom;
        if(ReadFromXMLAttribute(element,"derivedFrom",derivedFrom))
        {
            ResolveDerivedFrom(m_values, *value.get(),derivedFrom);
        }
        value->ReadFromNode(element);
        m_values.push_back(value);
        element = element->NextSiblingElement("enumeratedValue");
    }
    return 0;
}

int SVDEnumeratedValue::ReadFromNode(TiXmlElement* node)
{
    wxString val;
    ReadFromXMLElement(node, "name",        m_name);
    ReadFromXMLElement(node, "description", m_description);

    m_isDefault = false;
    bool is_val = ReadFromXMLElement(node, "value",     m_value);
    bool is_def = ReadFromXMLElement(node, "isDefault", m_isDefault);;
    if( is_val == false && is_def == false)
        m_dont_care = true;
    else
        m_dont_care = false;
    return 0;
}

int SVDPeriphery::ReadFromNode(TiXmlElement* node)
{
    SVDArray::ReadFromNode( node );
    SVDRegisterProperties::ReadFromNode( node );

    m_derivedFrom = wxEmptyString;
    ReadFromXMLAttribute(node, "derivedFrom", m_derivedFrom);

    ReadFromXMLElement(node, "name",                m_name);
    ReadFromXMLElement(node, "version",             m_version);
    ReadFromXMLElement(node, "description",         m_description);
    ReadFromXMLElement(node, "alternatePeripheral", m_alternatePeripherial);
    ReadFromXMLElement(node, "groupName",           m_groupName);
    ReadFromXMLElement(node, "prependToName",       m_prependToName);
    ReadFromXMLElement(node, "appendToName",        m_appendToName);
    ReadFromXMLElement(node, "headerStructName",    m_headerStructName);
    ReadFromXMLElement(node, "disableCondition",    m_disableCondition);
    ReadFromXMLElement(node, "baseAddress",         m_baseAddress);

    TiXmlElement* element = node->FirstChildElement("addressBlock");
    m_addressBlocks.clear();
    while(element != nullptr)
    {
        SVDAddressBlock tmp_addressBlock;
        tmp_addressBlock.ReadFromNode(element);
        m_addressBlocks.push_back(tmp_addressBlock);
        element = element->NextSiblingElement("addressBlock");
    }

    element = node->FirstChildElement("interrupt");
    m_interrupts.clear();
    while(element != nullptr)
    {
        SVDInterrupt tmp_interrupt;
        tmp_interrupt.ReadFromNode(element);
        m_interrupts.push_back(tmp_interrupt);

        element = element->NextSiblingElement("interrupt");
    }

    TiXmlElement* register_element = node->FirstChildElement("registers");
    if(register_element)
    {
        element = register_element->FirstChildElement("cluster");
        m_registers.clear();
        while(element != nullptr)
        {
            std::shared_ptr<SVDRegisterCluster> cluster = std::shared_ptr<SVDRegisterCluster>(new SVDRegisterCluster(*this));
            wxString derivedFrom;
            if(ReadFromXMLAttribute(element,"derivedFrom",derivedFrom))
            {
                ResolveDerivedFrom(m_registers, *cluster.get(),derivedFrom);
            }
            cluster->ReadFromNode(element);
            if(ResolveArray(m_registers,cluster) < 0)
            {
                // error resolving array
            }
            element = element->NextSiblingElement("cluster");
        }

        element = register_element->FirstChildElement("register");
        while(element != nullptr)
        {
            std::shared_ptr<SVDRegister> regist = std::shared_ptr<SVDRegister>(new SVDRegister(*this));
            wxString derivedFrom;
            if(ReadFromXMLAttribute(element,"derivedFrom",derivedFrom))
            {
                ResolveDerivedFrom(m_registers, *regist.get(),derivedFrom);
            }
            regist->ReadFromNode(element);
            if(ResolveArray(m_registers,regist) < 0)
            {
                // error resolving array
                return -1;
            }
            element = element->NextSiblingElement("register");
        }
    }
    return 0;
}


int SVDRegisterBase::ReadFromNode( TiXmlElement *node )
{
    SVDArray::ReadFromNode( node );
    SVDRegisterProperties::ReadFromNode( node );

    ReadFromXMLElement(node, "name",            m_name);
    ReadFromXMLElement(node, "description",     m_description);
    ReadFromXMLElement(node, "addressOffset",   m_addressOffset);
    return 0;
}


int SVDDevice::ReadFromNode(TiXmlElement* node)
{
    SVDRegisterProperties::ReadFromNode(node);
    ReadFromXMLElement(node, "vendor",      m_vendor);
    ReadFromXMLElement(node, "vendorID",    m_vendorID);
    ReadFromXMLElement(node, "name",        m_name);
    ReadFromXMLElement(node, "series",      m_series);
    ReadFromXMLElement(node, "version",     m_version);
    ReadFromXMLElement(node, "description", m_description);
    ReadFromXMLElement(node, "licenseText", m_licenseText);
    ReadFromXMLElement(node, "headerSystemFilename",    m_headerSystemFilename);
    ReadFromXMLElement(node, "headerDefinitionsPrefix", m_headerDefinitionsPrefix);
    ReadFromXMLElement(node, "addressUnitBits",         m_AddressUnitBits);
    ReadFromXMLElement(node, "width",       m_width);

    TiXmlElement* element = node->FirstChildElement("cpu");
    if(element)
        m_cpu.ReadFromNode(element);

    element = node->FirstChildElement("peripherals");
    m_peripherals.clear();
    if(element)
    {
        TiXmlElement* peripheral = element->FirstChildElement("peripheral");
        while(peripheral != nullptr)
        {
            std::shared_ptr<SVDPeriphery>  pheri = std::shared_ptr<SVDPeriphery>(new SVDPeriphery(*this));
            wxString derivedFrom;
            if(ReadFromXMLAttribute(peripheral,"derivedFrom",derivedFrom))
            {
                ResolveDerivedFrom(m_peripherals, *pheri.get(),derivedFrom);
            }
            pheri->ReadFromNode(peripheral);
            if(ResolveArray(m_peripherals,pheri) < 0)
            {
                // error resolving array
            }
            peripheral = peripheral->NextSiblingElement("peripheral");

        }
    }
    return 0;
}

SVDPeriphery::SVDPeriphery(const SVDPeriphery& base_prop)           : SVDRegisterProperties(base_prop)
{
    // copy all attributes
};



int SVDCpu::ReadFromNode(TiXmlElement* node)
{
    ReadFromXMLElement(node,"name", m_name);

    TiXmlElement* element = node->FirstChildElement("revision");
    if(element)
        m_revision = SVDHWRevision(element->GetText());

    ReadFromXMLElement(node,"endian",        m_endian);
    ReadFromXMLElement(node,"mpuPresent",    m_mpuPresent);
    ReadFromXMLElement(node,"fpuPresent",    m_fpuPresent);
    ReadFromXMLElement(node,"fpuDP",         m_fpuDP);
    ReadFromXMLElement(node,"icachePresent", m_iCachePresent);
    ReadFromXMLElement(node,"dcachePresent", m_dCachePresent);
    ReadFromXMLElement(node,"itcmPresent",   m_itcmPresent);
    ReadFromXMLElement(node,"dtcmPresent",   m_dtcmPresent);
    ReadFromXMLElement(node,"vtorPresent",   m_vtorPresent);
    ReadFromXMLElement(node,"nvicPrioBits",  m_nvicPriorityBits);
    ReadFromXMLElement(node,"vendorSystickConfig", m_vendorSystickConfig);
    ReadFromXMLElement(node,"deviceNumInterrupts", m_deviceNumInterrupts);
    ReadFromXMLElement(node,"sauNumRegions", m_sauNumRegions);

    element = node->FirstChildElement("sauRegionsConfig");
    m_sauRegionsConfig.clear();
    while(element != nullptr)
    {
        SVDSAURegionsConfig  tmp_region_config;
        tmp_region_config.ReadFromNode(element);
        m_sauRegionsConfig.push_back(tmp_region_config);

        element = element->NextSiblingElement("sauRegionsConfig");
    }
    return 0;
}

cbSVDFileReader::cbSVDFileReader()
{
    //ctor
}

cbSVDFileReader::~cbSVDFileReader()
{
    //dtor
}

int cbSVDFileReader::LoadSVDFile(const wxString& path, SVDDevice* device)
{
    TiXmlDocument doc;
    if (!TinyXML::LoadDocument(path, &doc))
        return -1;


    //PGDeviceProperty *device = new PGDeviceProperty();

    TiXmlElement* root;
    TiXmlNode* device_node;
    TiXmlElement* elem;
    root = doc.RootElement();
    device_node = root->NextSibling( "device" );

    if (root)
    {
        device->ReadFromNode( root );
    }
    return 0;
}
