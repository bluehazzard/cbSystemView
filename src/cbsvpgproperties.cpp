#include "cbsvpgproperties.h"
#include <wx/object.h>

#include <cbcolourmanager.h>
#include <logmanager.h>

class svPGData;

IMPLEMENT_DYNAMIC_CLASS(svPGData, wxObject)


IMPLEMENT_DYNAMIC_CLASS(svPGBaseProp, wxPGProperty)


#if wxCHECK_VERSION(3,0,0)
WX_PG_IMPLEMENT_VARIANT_DATA(svPGData)
#else
WX_PG_IMPLEMENT_VARIANT_DATA(svPGDataVariantData,
                             svPGData);

svPGData& svPGDataRefFromVariant( wxVariant& variant )
{
    wxASSERT_MSG( variant.GetType() == wxS(#svPGData),
                  wxString::Format(wxS("Variant type should have been '%s'")
                                   wxS("instead of '%s'"),
                                   wxS("svPGData"),
                                   variant.GetType().c_str()));
    svPGDataVariantData *data = (svPGDataVariantData*) variant.GetData();
    return data->GetValueRef();
}

const svPGData& svPGDataRefFromVariant( const wxVariant& variant )
{
    wxASSERT_MSG( variant.GetType() == wxS(#svPGData),
                  wxString::Format(wxS("Variant type should have been '%s'")
                                   wxS("instead of '%s'"),
                                   wxS("svPGData"),
                                   variant.GetType().c_str()));
    svPGDataVariantData *data =
        (svPGDataVariantData*) variant.GetData();
    return data->GetValue();
}
#endif // wxCHECK_VERSION


void svPGBaseProp::SetDataFromBinary(const wxString& str)
{
    svPGData& data= svPGDataRefFromVariant(m_value);
    data.SetDataFromBinary(str ,GetMask(), GetBitOffset() );
    wxVariant var;
    var << data;
    SetValue(var);//,0,0);
    //m_value = var;
}

void svPGBaseProp::GetDataToBinary(wxString& str)
{

}

void svPGBaseProp::SetData(uint64_t data)
{

}


wxString svPGData::GetDataReadable(ValueRepresentation rep, uint64_t bitsize) const
{
    return GetDataReadable(m_data, bitsize, rep);
}

wxString svPGData::GetDataReadable(uint64_t data,uint64_t bitsize, ValueRepresentation rep) const
{
    wxString output;
    switch(rep)
    {
    case REP_BIN:
        output = wxT("0b") + printBits(bitsize, data);
        break;
    case REP_HEX:
    {
        wxString format;
        format << wxT("0x%0") << bitsize/4 << wxT("llx");
        output.Printf(format, data );
        break;
    }
    case REP_DEC:
        output.Printf(wxT("%lli"), (int64_t) data );
        break;
    case REP_UDEC:
        output.Printf(wxT("%llu"), data );
        break;
    case REP_FLOAT:
        output.Printf(wxT("%f"), *(reinterpret_cast<const float*>(&data)) );
        break;
    case REP_CHAR:
        output.Printf(wxT("%c"), *(reinterpret_cast<const char*>(&data)) );
        break;
    case REP_BYTE_ARRAY:
    {
        for(size_t i = 0; i < bitsize / 8; ++i)
        {
            output += wxString::Format(wxT("0x%x "), *(reinterpret_cast<const char*>(&data + i)) );
        }
    }
    break;

    }

    return output;
}


void svPGData::SetDataFromBinary(const wxString& str, const uint64_t& mask, const uint64_t offset)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    m_data = (data & mask)>>offset;
}

void svPGData::SetDataFromString(const wxString& str)
{

}

WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGPeripheryProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               TextCtrl)

svPGPeripheryProp::svPGPeripheryProp(SVDPeriphery &per) : svPGBaseProp( per )

{
    svPGData value(per.GetResetValue());

    uint64_t size       = 0;
    uint64_t bitSize    = 0;

    wxString desc = per.GetDesc();

    desc << wxString::Format(wxT("\nAddress: 0x%llx\n"), GetAddress());
    desc << wxString::Format(wxT("Reset value: 0x%llx\n"), GetResetVal() & GetResetMask());

    auto itr = per.GetRegistersBegin();
    for ( ; itr != per.GetRegistersEnd(); ++itr)
    {
        SVDRegister* reg = dynamic_cast<SVDRegister*>((*itr).get());
        if(reg == nullptr)
            continue;

        size      += reg->GetSize() / 8;  // Size is in bytes
        bitSize   += reg->GetSize();      // site in bits

        svPGRegisterProp* prop = new svPGRegisterProp(*reg, per);
        AddChild(prop);
    }

    SetSize(size);
    SetBitSize(bitSize);

    wxVariant var;
    var << value;
    SetValue(var);

    //m_value << value;

    SetDescription(desc);
    SetHelpString(desc);
    //ctor
}

svPGPeripheryProp::~svPGPeripheryProp()
{
    //dtor
}


void svPGPeripheryProp::Populate()
{
    size_t child_count = GetChildCount();
    for(size_t i = 0; i < child_count; ++i)
    {
        svPGRegisterProp *child = (svPGRegisterProp*) (Item(i));
        if(child == nullptr)
            continue;
        child->Populate();
    }
}


void svPGPeripheryProp::SetData(uint64_t data )
{
    // Not supported...
    Manager::Get()->GetLogManager()->LogError(_("svPGPeripheryProp::SetData: Not supported use SetDataFromBinary()"));
}

void svPGPeripheryProp::SetDataFromString(const wxString& str)
{
    // Not supported...
    Manager::Get()->GetLogManager()->LogError(_("svPGPeripheryProp::SetDataFromString: Not supported use SetDataFromBinary()"));
}


void svPGPeripheryProp::SetDataFromBinary(const wxString& str)
{
    m_internalData = str;
    RefreshChildren();
}

void svPGPeripheryProp::SetRegisterChanged(svPGRegisterProp* reg)
{
    m_RegisterChanged = reg;
}

svPGRegisterProp* svPGPeripheryProp::GetRegisterChanged()
{
    return m_RegisterChanged;
}

void svPGPeripheryProp::RefreshChildren()
{
    wxCharBuffer buff = m_internalData.To8BitData();
    size_t in_size = m_internalData.length();
    size_t start = 0;

    for (unsigned int i = 0; i < GetChildCount() ; ++i )
    {
        svPGBaseProp* child = dynamic_cast<svPGBaseProp*>(Item(i));
        if(child == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_("svPGPeripheryProp::SetDataFromBinary: ") + Item(i)->GetName() + _(" child of ") + GetName() + _(" can not be casted to svPGData*"));
            continue;
        }

        //const svPGData& data = svPGDataRefFromVariant(child->GetValue());

        size_t out_size = child->GetSize();
        if(out_size > in_size-start)
            out_size = in_size - start;

        char* target_buff = new char[out_size];

        memcpy(target_buff, buff.data() + start, out_size);
        start += out_size;
        dynamic_cast<svPGBaseProp*>(Item(i))->SetDataFromBinary(wxString::From8BitData(target_buff,out_size));
        delete[] target_buff;
    }
}


//IMPLEMENT_DYNAMIC_CLASS(svPGRegisterProp, svPGBaseProp)

WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGRegisterProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               TextCtrl)

svPGRegisterProp::svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per) : svPGBaseProp(per, reg)
{
    SetBitFlag(m_repCap, REP_HEX, REP_BIN); // We can represent the Data in hex or binary, other values make no sense?
    m_rep = REP_HEX;

    svPGData value(reg.GetResetValue());

    wxString desc = reg.GetDesc();

    auto itr = reg.m_fields.begin();
    for ( ; itr != reg.m_fields.end(); ++itr)
    {
        SVDField* field = dynamic_cast<SVDField*>((*itr).get());
        if(field == nullptr)
            continue;

        wxPGProperty* prop;

        if((field->m_bitRange.GetWidth() == 1) && (field->m_enumerated_value.GetValuesSize() == 0))
        {
            // Flag
            prop = new svPGBitProp(*field);
        }
        else if(field->m_enumerated_value.GetValuesSize() != 0)
        {
            // Enumeration
            prop = new svPGEnumFieldProp(*field);
        }
        else
        {
            // Generic value for all other
            prop = new svPGValueProp(*field);
        }

        AddPrivateChild(prop);
    }

    desc << wxString::Format(wxT("\nAddress: 0x%llx\n"), GetAddress());
    desc << wxString::Format(wxT("Reset value: 0x%llx\n"), GetResetVal() & GetResetMask());

    SetDescription(desc);

    m_value << value;

    SetHelpString(desc);

}

svPGRegisterProp::~svPGRegisterProp()
{
    //dtor
}


uint64_t svPGRegisterProp::GetDataFromChildren()    const
{
    uint64_t ret = 0;
    size_t count = GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        const svPGData& data = svPGDataRefFromVariant(Item(i)->GetValue());
        svPGBaseProp* child = dynamic_cast<svPGBaseProp*>(Item(i));
        ret &= child->GetMask();
        ret |= data.GetData() << child->GetBitOffset();
    }

    return ret;
}

void svPGRegisterProp::Populate()
{

    size_t child_count = GetChildCount();
    for(size_t i = 0; i < child_count; ++i)
    {
        wxPGProperty *child = Item(i);
        if(child == nullptr)
            continue;
        bool isKind = child->IsKindOf( CLASSINFO(svPGEnumFieldProp) );
        if(isKind)
        {
            svPGEnumFieldProp* en = (svPGEnumFieldProp*) child;
            if(en == nullptr)
                continue;
            en->Populate();
        }
    }
}

wxString svPGRegisterProp::ValueToString( wxVariant& value, int argFlags ) const
{

    //uint64_t raw_data = GetDataFromChildren();
    svPGData& data = svPGDataRefFromVariant(value);
    //data.SetData(raw_data);
    return data.GetDataReadable(m_rep, GetBitSize());
}

void svPGRegisterProp::SetDataFromBinary(const wxString& str)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    SetData(data);
    RefreshChildren();
}


#if wxCHECK_VERSION(3,0,0)
wxVariant svPGRegisterProp::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
#else
void svPGRegisterProp::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
#endif // wxCHECK_VERSION
{
// TODO (bluehazzard#1#): Implement this properly
    svPGData& data = svPGDataRefFromVariant(thisValue);
    svPGData& childData = svPGDataRefFromVariant(childValue);

    svPGBaseProp* child = dynamic_cast<svPGBaseProp*>(Item(childIndex));
    uint64_t mask = child->GetMask();
    uint64_t offset = child->GetBitOffset();
    uint64_t CData = childData.GetData();

    uint64_t PData = data.GetData();
    PData &= ~ mask;
    PData |= (CData << offset) & mask;
    data.SetData(PData);

    wxVariant newVariant;
    newVariant << data;

#if wxCHECK_VERSION(3,0,0)
    return newVariant;    // dummy
#endif // wxCHECK_VERSION
}

void svPGRegisterProp::SetData( uint64_t data )
{
    svPGData d;//(svPGDataRefFromVariant(m_value));
    d.SetData(data);
    wxVariant tmp;
    tmp << d;
    SetValue(tmp);//,0,0);
    //m_value = tmp;
}

void svPGRegisterProp::RefreshChildren()
{
    svPGData& data = svPGDataRefFromVariant(m_value);
    int count = GetChildCount();
    for(int i = 0; i < count; ++i)
    {
        svPGBaseProp* child = dynamic_cast<svPGBaseProp*>(Item(i));
        svPGData child_data;
        if (child->GetValue().GetType() != wxT("svPGData"))
        {

            child_data.SetData( (child->GetValue().GetLong() & child->GetMask()) >> child->GetBitOffset());
        }
        else
        {
            //child_data = svPGData(svPGDataRefFromVariant(child->GetValue()));
            child_data = svPGData();
            child_data.SetData( (data.GetData() & child->GetMask()) >> child->GetBitOffset());
        }

        wxVariant var;
        var << child_data;
        child->SetValue(var);//,0,0);
        //SetValue(var,0,0);
        //m_value = var;
    }
}


//void svPGRegisterProp::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue)  const
//{

//}

WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGEnumFieldProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               ComboBox)

//IMPLEMENT_ABSTRACT_CLASS2(svPGEnumFieldProp, wxEnumProperty, svPGData)

svPGEnumFieldProp::svPGEnumFieldProp(SVDField &field) : svPGBaseProp(field )
{
    svPGData value(field.GetResetValue());

    wxString desc =  field.GetDesc();

    desc << wxString::Format(wxT("\nBit size: %lld\n"), GetBitSize() );
    desc << wxString::Format(wxT("Bit offset: %lld\n"), GetBitOffset() );

    SetDescription(desc);
    SetHelpString(desc);


    // Find best length for value and description part
    auto itr = field.m_enumerated_value.GetValuesBegin();
    size_t name_length = 0;
    size_t desc_length = 0;
    for (; itr != field.m_enumerated_value.GetValuesEnd(); ++itr)
    {
        name_length = std::max((*itr)->GetName().length(), name_length);
        desc_length = std::max((*itr)->GetDescription().length(), desc_length);
    }

    name_length += 4;

    itr = field.m_enumerated_value.GetValuesBegin();
    for (; itr != field.m_enumerated_value.GetValuesEnd(); ++itr)
    {
        wxString name = (*itr)->GetName();
        for(size_t i = name.length(); i < name_length; ++i)
            name += _(" ");

        m_elements.push_back( svPGEnumFieldElement((*itr)->GetName(),
                              (*itr)->GetDescription(),
                              name + wxT(": ") + (*itr)->GetDescription(),
                              -1,
                              (*itr)->GetValue()) );
    }

    m_value << value;

}

wxString svPGEnumFieldProp::ValueToString( wxVariant& value, int argFlags ) const
{
    wxString ret;
    if(value.GetType() == wxT("svPGData") )
    {
        const svPGData& data = svPGDataRefFromVariant(value);
        auto itr = m_elements.begin();
        for(; itr != m_elements.end(); ++itr)
        {
            if(itr->value == data.GetData())
            {
                ret = itr->text;
                break;
            }

        }
    }
    else
    {
        long sel = value.GetLong();
        if(sel >= 0 && sel < m_elements.size())
            ret = m_elements[sel].text;
    }
    return ret;

}

bool svPGEnumFieldProp::StringToValue( wxVariant& variant, const wxString& text, int argFlags )
{
    svPGData& data = svPGDataRefFromVariant(variant);
    uint64_t old_data = data.GetData();
    uint64_t num = data.GetData();

    auto itr = m_elements.begin();
    for(; itr != m_elements.end(); ++itr)
    {
        if( itr->text == text)
            num = itr->value;
    }


    data.SetData( num & (GetMask() >> GetBitOffset() ));  // Mask the data

    if(old_data == data.GetData())
        return true;
    else
        return false;
}

void svPGEnumFieldProp::Populate()
{
    auto itr = m_elements.begin();
    for (; itr != m_elements.end(); ++itr)
    {
#if wxCHECK_VERSION(3, 0, 0)
        (*itr).index = AddChoice((*itr).text);
#else
        (*itr).index = AppendChoice((*itr).text);
#endif
    }
}


int svPGEnumFieldProp::GetChoiceSelection() const
{
    if(m_value.GetType() == wxT("svPGData"))
    {
        const svPGData& data = svPGDataRefFromVariant(m_value);
        uint64_t da = data.GetData();
        auto itr = m_elements.begin();
        size_t i = 0;
        for(; itr != m_elements.end(); ++itr)
        {
            if( itr->value == da)
                return i;
            i++;
        }
    }
    else
        return m_value.GetInteger();

}


WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGValueProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               TextCtrl)

//IMPLEMENT_ABSTRACT_CLASS(svPGValueProp, svPGBaseProp)

svPGValueProp::svPGValueProp(const SVDField& field) : svPGBaseProp(field)
{
    SetBitFlag(m_repCap, REP_HEX, REP_BIN, REP_DEC, REP_UDEC, REP_FLOAT, REP_CHAR);
    m_rep = REP_HEX;

    svPGData value(field.GetResetValue());

    wxString desc =  field.GetDesc();

    desc << wxString::Format(wxT("\nBit size: %lld\n"), GetBitSize());
    desc << wxString::Format(wxT("Bit offset: %lld\n"), GetBitOffset());

    SetDescription(desc);
    SetHelpString(desc);

    m_value << value;
}

wxString svPGValueProp::ValueToString( wxVariant& value, int argFlags ) const
{
    const svPGData& data = svPGDataRefFromVariant(value);
    wxString ret = data.GetDataReadable(m_rep, GetBitSize());
    return ret;
}

bool svPGValueProp::StringToValue( wxVariant& variant, const wxString& text, int argFlags )
{
    svPGData& data = svPGDataRefFromVariant(variant);
    uint64_t old_data = data.GetData();


    long long num;
    wxString number;
    if(text.StartsWith(wxT("0x"),&number))
    {
        number.ToLongLong(&num,16);
    }
    else
    {
        text.ToLongLong(&num,10);
    }


    data.SetData( num & (GetMask() >> GetBitOffset() ));  // Mask the data

    if(old_data == data.GetData())
        return true;
    else
        return false;
}



WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGBitProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               CheckBox)

//IMPLEMENT_ABSTRACT_CLASS(svPGBitProp, svPGBaseProp)

svPGBitProp::svPGBitProp(const SVDField &field) : svPGBaseProp( field )
{
    SetBitFlag(m_repCap, REP_HEX, REP_BIN, REP_DEC, REP_UDEC, REP_FLOAT, REP_CHAR);
    m_rep = REP_HEX;

    svPGData value(field.GetResetValue());

    SetAttribute(wxT("UseCheckbox"),true);

    wxString desc =  field.GetDesc();

    desc << wxString::Format(wxT("\nBit size: %lld\n"), GetBitSize());
    desc << wxString::Format(wxT("Bit offset: %lld\n"), GetBitOffset());

    SetDescription(desc);
    SetHelpString(desc);

    m_value << value;

};

int svPGBitProp::GetChoiceSelection() const
{
    const svPGData& data = svPGDataRefFromVariant(m_value);
    if(data.GetData())
        return 1;
    else
        return 0;
}

wxString svPGBitProp::ValueToString( wxVariant& value, int argFlags ) const
{
    const svPGData& data = svPGDataRefFromVariant(value);
    if(data.GetData())
        return wxT("1");
    else
        return wxT("0");
    //wxString ret = data.GetDataReadable(m_rep);
    //return ret;
}

bool svPGBitProp::IsValueUnspecified() const
{
    return false;
}


void svPGBitProp::SetDataFromBinary(const wxString& str)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);

    svPGData& Data= svPGDataRefFromVariant(m_value);

    Data.SetData((data & GetMask()) >> GetBitOffset() );
    wxVariant var;
    var << Data;
    SetValue(var);//,0,0);
    //m_value = var;
}

bool svPGBitProp::StringToValue( wxVariant& variant, const wxString& text, int argFlags )
{
    svPGData& data = svPGDataRefFromVariant(variant);

    uint64_t old_data = data.GetData();
    long long number;

    if( (text.ToLongLong(&number) && number > 0) || text == wxT("1") || text == wxT("0x1") || text == wxT("true") || text == wxT("TRUE"))
        data.SetData(1);
    else if (text == wxT("0") || text == wxT("false") || text == wxT("FALSE"))
        data.SetData(0);

    if(old_data == data.GetData())
        return true;
    else
        return false;
}

wxString printBits(size_t const size, uint64_t data)
{
    unsigned char byte;
    wxString ret;
    ret.clear();
    int i, j;

    j = size -1;

    for ( ; j>=0; j--)
    {
        byte = (data >> j) & 1;
        if(byte)
            ret << wxT("1");
        else
            ret << wxT("0");
    }

    return ret;
}



