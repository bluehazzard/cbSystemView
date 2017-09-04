#include "cbsvpgproperties.h"
#include <wx/object.h>

#include <cbcolourmanager.h>
#include <logmanager.h>

class svPGPropBase;

IMPLEMENT_ABSTRACT_CLASS(svPGPropBase, wxObject)

IMPLEMENT_ABSTRACT_CLASS2(svPGPeripheryProp, wxStringProperty, svPGPropBase)


wxString svPGPropBase::GetDataReadable() const
{
    wxString output;
    switch(m_rep)
    {
    case REP_BIN:
        output = wxT("0b") + printBits(GetBitSize(), &m_data);
        break;
    case REP_HEX:
        output.Printf(wxT("0x%llx"), m_data );
        break;
    case REP_DEC:
        output.Printf(wxT("%lli"), (int64_t) m_data );
        break;
    case REP_UDEC:
        output.Printf(wxT("%llu"), m_data );
        break;
    case REP_FLOAT:
        output.Printf(wxT("%f"), *(reinterpret_cast<const float*>(&m_data)) );
        break;
    case REP_CHAR:
        output.Printf(wxT("%c"), *(reinterpret_cast<const char*>(&m_data)) );
    }

    return output;
}

void svPGPropBase::SetRepresentation(ValueRepresentation rep)
{
    if(CanRepresent(rep));
    {
        m_rep = rep;
        UpdateView();
    }
};

svPGPeripheryProp::svPGPeripheryProp(SVDPeriphery &per) : wxStringProperty( per.GetName(),
                                                                            per.GetName() ) ,
                                                          svPGPropBase(&per, this)
{
    m_addr      = per.GetBaseAddress();
    m_baseAddr  = per.GetBaseAddress(); // Base address is the periphery
    m_offset    = 0;                    // periphery has 0 offset
    //m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = 0;                    // We have to determine the size of the periphery...

    SetHelpString( per.GetDesc() );


    auto itr = per.GetRegistersBegin();
    for ( ; itr != per.GetRegistersEnd(); ++itr)
    {
        SVDRegister* reg = dynamic_cast<SVDRegister*>((*itr).get());
        if(reg == nullptr)
            continue;

        m_size      += reg->GetSize() / 8;  // Size is in bytes
        m_bitSize   += reg->GetSize();      // site in bits

        svPGRegisterProp* prop = new svPGRegisterProp(*reg, per);
        AddChild(prop);
    }
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
    wxCharBuffer buff = str.To8BitData();
    size_t in_size = str.length();
    size_t start = 0;


    for (unsigned int i = 0;i < GetChildCount() ; ++i )
    {
        svPGPropBase* child = dynamic_cast<svPGPropBase*>(Item(i));
        if(child == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_("svPGPeripheryProp::SetDataFromBinary: ") + Item(i)->GetName() + _(" child of ") + GetName() + _(" can not be casted to svPGPropBase*"));
            continue;
        }

        size_t out_size = child->GetSize();

        if(out_size > in_size-start)
            out_size = in_size - start;

        char* target_buff = new char[out_size];

        memcpy(target_buff, buff.data() + start, out_size);
        start += out_size;
        dynamic_cast<svPGPropBase*>(Item(i))->SetDataFromBinary(wxString::From8BitData(target_buff,out_size));
        delete[] target_buff;
    }
}


IMPLEMENT_ABSTRACT_CLASS2(svPGRegisterProp, wxStringProperty, svPGPropBase)
svPGRegisterProp::svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per) : wxStringProperty ( reg.GetName(),
                                                                                                        reg.GetName() ) ,
                                                                                      svPGPropBase(&per, this)
{
    SetBitFlag(m_repCap, REP_HEX, REP_BIN); // We can represent the Data in hex or binary, other values make no sense?
    m_rep = REP_HEX;

    m_data      = per.GetResetValue();
    m_resetValue= per.GetResetValue();
    m_resetMask = per.GetResetMask();
    m_addr      = per.GetBaseAddress() + reg.GetAddressOfset();
    m_baseAddr  = per.GetBaseAddress();    // Base address is the periphery
    m_offset    = reg.GetAddressOfset();  // periphery has 0 offset
    //m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = reg.GetSize() / 8;    // We have to determine the size of the register...
    m_bitSize   = reg.GetSize();        // Register size is in bits
    wxString desc   = reg.GetDesc();



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

    desc << wxString::Format(wxT("\nAddress: 0x%llx\n"), m_addr);
    desc << wxString::Format(wxT("\nReset value: 0x%llx\n"), m_resetValue & m_resetMask);

    SetDescription(desc);
    SetHelpString(desc);

}

svPGRegisterProp::~svPGRegisterProp()
{
    //dtor
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

void svPGRegisterProp::SetDataFromBinary(const wxString& str)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    SetData(data);
}

void svPGRegisterProp::SetDataFromString(const wxString& str)
{
    long long num;
    wxString number;
    if(str.StartsWith(wxT("0x"),&number))
    {
        number.ToLongLong(&num,16);
    }
    else
    {
        str.ToLongLong(&num,10);
    }
    //SetData((uint64_t) num);
    m_data = num & m_mask;
    SetData(m_data);
    UpdateView();
}

// Property value...
void svPGRegisterProp::SetValueFromString(const wxString& str, int flags)
{
    SetDataFromString(str);
}

#if wxCHECK_VERSION(3,0,0)
wxVariant svPGRegisterProp::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
#else
void svPGRegisterProp::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
#endif // wxCHECK_VERSION
{
// TODO (bluehazzard#1#): Implement this properly

    #if wxCHECK_VERSION(3,0,0)
    wxLongLong value = thisValue.GetLongLong();
    wxLongLong child_val = childValue.GetLongLong();
    #else
    long value = thisValue.GetLong();
    long child_val = childValue.GetLong();
    #endif // wxCHECK_VERSION


    wxPGProperty *child = Item(childIndex);
    svPGPropBase *base = dynamic_cast<svPGPropBase*>(child);
    if(base != nullptr)
    {
        uint64_t mask = base->GetMask();
        uint64_t offset = base->GetBitOffset();

        // To set the value we have to clear the old bits with the inverted mask
        value = value & ~mask;
        // Now we can set the new value. The value returned from the Child is non shifted so we have to shift it
        value |= (value << offset);
    }
    else if (child->IsKindOf( CLASSINFO(svPGEnumFieldProp) ))
    {

    }
    else if (child->IsKindOf( CLASSINFO(svPGValueProp) ))
    {

    }
    else if (child->IsKindOf( CLASSINFO(svPGBitProp) ))
    {

    }
    else if (child->IsKindOf( CLASSINFO(svPGEnumFieldProp) ))
    {

    }


    #if wxCHECK_VERSION(3,0,0)
    return wxVariant(value);    // dummy
    #endif // wxCHECK_VERSION
}

void svPGRegisterProp::RefreshChildren()
{

}

void svPGRegisterProp::UpdateView()
{
    SetValue(GetDataReadable());
}

void svPGRegisterProp::SetData( uint64_t data )
{
    m_data = data;
    int count = GetChildCount();
    for(int i = 0; i < count; ++i)
    {
        svPGPropBase* prop = dynamic_cast<svPGPropBase*>(Item(i));
        prop->SetData(m_data);
    }

    UpdateView();
};


//void svPGRegisterProp::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue)  const
//{

//}

IMPLEMENT_ABSTRACT_CLASS2(svPGEnumFieldProp, wxEnumProperty, svPGPropBase)

svPGEnumFieldProp::svPGEnumFieldProp(SVDField &field) : wxEnumProperty(field.GetName(),
                                                                       field.GetName() ) ,
                                                        svPGPropBase(&field, this)
{
    m_data          = field.GetResetValue();
    m_mask          = field.m_bitRange.GetMask();
    m_size          = ceil(field.m_bitRange.GetWidth() / 8.0);  // Size in bytes
    m_bitSize       = field.m_bitRange.GetWidth();
    m_bitOffset     = field.m_bitRange.GetOffset();
    m_resetValue    = field.GetResetValue();
    m_resetMask     = field.GetResetMask();

    wxString desc = field.GetDesc();
    desc << wxT("\n\n");
    desc << wxString::Format(wxT("Bit size: %lld\n"), m_bitSize);
    desc << wxString::Format(wxT("Bit offset: %lld\n"), m_bitOffset);

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

void svPGEnumFieldProp::SetData( uint64_t data)
{
    m_data = data;
    UpdateView();
}

void svPGEnumFieldProp::SetDataFromBinary(const wxString& str)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    SetData(data);
}

void svPGEnumFieldProp::SetDataFromString(const wxString& str)
{
    auto itr = m_elements.begin();
    for(; itr != m_elements.end(); ++itr)
    {
        if(itr->index == str)
            m_data = itr->value;
    }

    UpdateView();
}


void svPGEnumFieldProp::UpdateView()
{
    auto itr = m_elements.begin();
    for(; itr != m_elements.end(); ++itr)
    {
        if(itr->value == m_data)
            SetValue(itr->index);
    }
}

void svPGValueProp::UpdateView()
{
    SetValue(GetDataReadable());
}

void svPGValueProp::SetDataFromBinary(const wxString& str)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    SetData(data);
}

void svPGValueProp::SetDataFromString(const wxString& str)
{
    long long num;
    wxString number;
    if(str.StartsWith(wxT("0x"),&number))
    {
        number.ToLongLong(&num,16);
    }
    else
    {
        str.ToLongLong(&num,10);
    }
    //SetData((uint64_t) num);
    m_data = num & m_mask;
    UpdateView();
}

void svPGValueProp::SetData( uint64_t data )
{
    m_data = (data & m_mask)>>m_bitOffset;
    UpdateView();
};


void svPGBitProp::SetDataFromString(const wxString& str)
{
    long tmp = 0;
    bool number = str.ToLong(&tmp,10);

    if( (number && tmp > 0) || str == wxT("1") || str == wxT("0x1") || str == wxT("true") || str == wxT("TRUE"))
        SetValueFromInt(1);
    else if (str == wxT("0") || str == wxT("false") || str == wxT("FALSE"))
        SetValueFromInt(0);
}

void svPGBitProp::SetDataFromBinary(const wxString& str)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    SetData(data);
}


wxString printBits(size_t const size, uint64_t data)
{
    unsigned char byte;
    wxString ret;
    ret.clear();
    int i, j;

    j = size -1;

    for ( ;j>=0;j--)
    {
            byte = (data >> j) & 1;
            if(byte)
                ret << wxT("1");
            else
                ret << wxT("0");
    }

    return ret;
}

IMPLEMENT_ABSTRACT_CLASS2(svPGBitProp, wxBoolProperty, svPGPropBase)
IMPLEMENT_ABSTRACT_CLASS2(svPGValueProp, wxStringProperty, svPGPropBase)
