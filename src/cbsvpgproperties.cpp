/*
 * cbSystemView
 * Copyright (C) 2017  bluehazzard
 *
 * This program is free software:   you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; this program is ONLY licensed under
 * version 3 of the License, later versions are explicitly excluded.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0>.
*/

#include "cbsvpgproperties.h"
#include <wx/object.h>

#include <cbcolourmanager.h>
#include <logmanager.h>

#define LOG_UNEXPECTED_ERROR(E)    Manager::Get()->GetLogManager()->LogError( wxString::FromUTF8(__PRETTY_FUNCTION__) + wxT(": ") + E)

class svPGData;

IMPLEMENT_DYNAMIC_CLASS(svPGData, wxObject)


IMPLEMENT_DYNAMIC_CLASS(svPGBaseProp, wxPGProperty)



WX_PG_IMPLEMENT_VARIANT_DATA(svPGData)

bool getDataFromVariant(const wxVariant& variant, svPGData& data )
{
    uint64_t da = 0;
    wxString type = variant.GetType();
    if(type == wxPG_VARIANT_TYPE_BOOL)
    {
        if(variant.GetBool())
            data.SetData(1);
        else
            data.SetData(0);

        return true;
    }
    else if (type == wxPG_VARIANT_TYPE_LONG)
    {
         data.SetData(variant.GetLong());
         return true;
    }
    else if (type == wxT("svPGData"))
    {
        data = svPGDataRefFromVariant(variant);
        return true;
    }
    return false;
};


void svPGBaseProp::SetDataFromBinary(const wxString& str)
{
    svPGData& data= svPGDataRefFromVariant(m_value);
    data.SetDataFromBinary(str ,GetMask(), GetBitOffset() );
    wxVariant var;
    var << data;
    SetValue(var, 0, 0);
    //m_value = var;
}

void svPGBaseProp::GetDataToBinary(wxString& str)
{

}

void svPGBaseProp::SetData(uint64_t data)
{

}

uint64_t svPGBaseProp::GetData()
{
    if(m_value.GetType() != wxT("svPGData"))
        return 0;

    svPGData& data = svPGDataRefFromVariant(m_value);
    return data.GetData();
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

    //SetFlag(wxPG_PROP_CATEGORY);

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

        AppendChild(prop);
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
    LOG_UNEXPECTED_ERROR(wxT("Not supported in svPGPeripheryProp"));
}

void svPGPeripheryProp::SetDataFromString(const wxString& str)
{
    // Not supported...
    LOG_UNEXPECTED_ERROR(wxT("Not supported in svPGPeripheryProp"));
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
            LOG_UNEXPECTED_ERROR( Item(i)->GetName() + _(" child of ") + GetName() + _(" can not be casted to svPGBaseProp*"));
            continue;
        }

        size_t out_size = child->GetSize();
        if(out_size > in_size-start)
            out_size = in_size - start;

        char* target_buff = new char[out_size];

        memcpy(target_buff, buff.data() + start, out_size);
        start += out_size;
        child->SetDataFromBinary(wxString::From8BitData(target_buff,out_size));

        delete[] target_buff;
    }
}


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

        AddChild(prop);
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
    svPGData data;
    if(getDataFromVariant(value, data) == false)
        return wxEmptyString;

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


wxVariant svPGRegisterProp::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
{

    svPGData thisData;
    if(getDataFromVariant(thisValue, thisData) == false)
    {
        LOG_UNEXPECTED_ERROR(wxT("thisValue can not be converted to svPGData"));
        return wxNullVariant;
    }


    svPGData childData;
    if(getDataFromVariant(childValue, childData) == false)
    {
        LOG_UNEXPECTED_ERROR(wxT("thisValue can not be converted to childValue"));
        return wxNullVariant;
    }

    svPGBaseProp* child = dynamic_cast<svPGBaseProp*>(Item(childIndex));
    uint64_t mask   = child->GetMask();
    uint64_t offset = child->GetBitOffset();

    uint64_t PData = thisData.GetData();
    PData &= ~ mask;
    PData |= (childData.GetData() << offset) & mask;

    svPGData returnVal;
    returnVal.SetData(PData);

    wxVariant newVariant;
    newVariant << returnVal;
    thisValue  << returnVal;

    return newVariant;
}

void svPGRegisterProp::SetData( uint64_t data )
{
    svPGData d;
    d.SetData(data);
    wxVariant tmp;
    tmp << d;
    SetValue(tmp, 0, 0);
}

void svPGRegisterProp::RefreshChildren()
{
    if(m_value.GetType() != wxT("svPGData"))
        return;

    svPGData& data = svPGDataRefFromVariant(m_value);
    int count = GetChildCount();
    for(int i = 0; i < count; ++i)
    {
        svPGBaseProp* child = dynamic_cast<svPGBaseProp*>(Item(i));
        svPGData child_data;
        const wxString& type = child->GetValue().GetType();
        if (type == wxPG_VARIANT_TYPE_LONG )
        {
            child_data.SetData( child->GetValue().GetLong());
        }
        else if(type == wxT("svPGData") )
        {
            child_data = svPGDataRefFromVariant(child->GetValue());
        }
        else if(type == wxPG_VARIANT_TYPE_BOOL)
        {
            child_data.SetData( child->GetValue().GetBool() == true  ? 1 : 0);
        }


        uint64_t childMask = child->GetMask();
        uint64_t childOffset = child->GetBitOffset();

        uint64_t oldData = child_data.GetData();
        uint64_t newData = (data.GetData() & childMask) >>  childOffset;

        if(oldData != newData)
            child->ChangeFlag( wxPG_PROP_MODIFIED, true );

        svPGData newPGData(newData);
        wxVariant var;
        var << newPGData;
        child->SetValue(var, 0, 0);
        //SetValue(var,0,0);
        //m_value = var;
    }
}


WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGEnumFieldProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               ComboBox)

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

bool svPGEnumFieldProp::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    svPGData& data = svPGDataRefFromVariant(variant);
    uint64_t old_data = data.GetData();
    uint64_t num = data.GetData();

    auto itr = m_elements.begin();
    for(; itr != m_elements.end(); itr++)
    {
        if( itr->text == text)
        {
            num = itr->value;
            break;
        }

    }


    data.SetData( num & (GetMask() >> GetBitOffset() ));  // Mask the data

    if(old_data == data.GetData())
        return false;
    else
        return true;
}

void svPGEnumFieldProp::Populate()
{
    auto itr = m_elements.begin();
    for (; itr != m_elements.end(); ++itr)
    {
        (*itr).index = AddChoice((*itr).text);
    }
}

int svPGEnumFieldProp::GetChoiceSelection() const
{
    wxString type = m_value.GetType();
    if(type == wxT("svPGData"))
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
    else if(type == wxPG_VARIANT_TYPE_LONG)
    {
        return m_value.GetLong();
    }
    else if(type == wxPG_VARIANT_TYPE_BOOL)
    {
        return m_value.GetBool() == true ? 1 : 0;
    }
    return wxNOT_FOUND;
}


bool svPGEnumFieldProp::IntToValue( wxVariant &	variant, int number, int argFlags ) const
{
    wxString type = variant.GetType();
    if(type != wxT("svPGData"))
    {
        svPGData data;
        variant << data;
    }

    svPGData& data = svPGDataRefFromVariant(variant);

    if(number < 0 || number >= m_elements.size())
        return false; //todo: ASSERT

    uint64_t oldData = data.GetData();
    uint64_t newData = m_elements[number].value;
    data.SetData(newData);

    if(oldData != newData)
        return true;
    else
        return false;
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

bool svPGValueProp::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
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
        return false;
    else
        return true;
}



WX_PG_IMPLEMENT_PROPERTY_CLASS(svPGBitProp, svPGBaseProp,
                               svPGData, const svPGData&,
                               CheckBox)

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
    wxString type = m_value.GetType();
    if (type == wxT("svPGData"))
    {
        const svPGData& data = svPGDataRefFromVariant(m_value);
        if(data.GetData())
            return 1;
        else
            return 0;

    } else if(type == wxPG_VARIANT_TYPE_BOOL)
    {
        return m_value.GetBool();
    }

    return wxNOT_FOUND;
}

wxString svPGBitProp::ValueToString( wxVariant& value, int argFlags ) const
{
    const svPGData& data = svPGDataRefFromVariant(value);
    if(data.GetData())
        return wxT("1");
    else
        return wxT("0");
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
    SetValue(var, 0, 0);
}

bool svPGBitProp::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
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

bool svPGBitProp::IntToValue( wxVariant& variant, int number, int argFlags ) const
{
    wxString type = variant.GetType();
    if(type != wxT("svPGData"))
    {
        svPGData data;
        variant << data;
    }

    svPGData& data = svPGDataRefFromVariant(variant);
    if(number == 0)
        data.SetData(0);
    else
        data.SetData(1);
    return true;
}

void svPGBitProp::SetChoiceSelection(int newValue)
{
    svPGData& data = svPGDataRefFromVariant(m_value);
    if(newValue == 0)
        data.SetData(0);
    else
        data.SetData(1);
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



