#include "cbsvpgproperties.h"
#include <wx/object.h>

class svPGPropBase;

IMPLEMENT_ABSTRACT_CLASS(svPGPropBase, wxObject)

IMPLEMENT_ABSTRACT_CLASS2(svPGPeripheryProp, wxStringProperty, svPGPropBase)

svPGPeripheryProp::svPGPeripheryProp(SVDPeriphery &per) : wxStringProperty( per.GetName(),
                                                                            per.GetName() ) ,
                                                          svPGPropBase(&per, this)
{
    m_addr      = per.GetBaseAddress();
    m_baseAddr  = per.GetBaseAddress();    // Base address is the periphery
    m_offset    = 0;                    // periphery has 0 offset
    //m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = 0; // We have to determine the size of the register...

    SetHelpString( per.GetDesc() );


    auto itr = per.GetRegistersBegin();
    for ( ; itr != per.GetRegistersEnd(); ++itr)
    {
        SVDRegister* reg = dynamic_cast<SVDRegister*>((*itr).get());
        if(reg == nullptr)
            continue;

        m_size += reg->GetSize() / 8;  // Size is in bits

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
    for (unsigned int i = 0;i < GetChildCount() ; ++i )
    {
        svPGPropBase* child = dynamic_cast<svPGPropBase*>(Item(i));
        if(child == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_("svPGPeripheryProp::SetData: ") + Item(i)->GetName() + _(" child of ") + GetName() + _(" can not be casted to svPGPropBase*"));
            continue;
        }
       // child->SetData(data, start);
    }
}

IMPLEMENT_ABSTRACT_CLASS2(svPGRegisterProp, wxStringProperty, svPGPropBase)
svPGRegisterProp::svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per) : wxStringProperty( reg.GetName(),
                                                                                                        reg.GetName() ) ,
                                                                                      svPGPropBase(&per, this)
{
    m_addr      = per.GetBaseAddress() + reg.GetAddressOfset();
    m_baseAddr  = per.GetBaseAddress();    // Base address is the periphery
    m_offset    = reg.GetAddressOfset();  // periphery has 0 offset
    //m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = reg.GetSize() / 8; // We have to determine the size of the register...
    //ctor
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
            // String
            prop = new svPGValueProp(*field);
        }

        AddChild(prop);
    }

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
        //wxString att = child->GetAttribute(wxT("TYPE"),wxEmptyString);
        bool isKind = child->IsKindOf( CLASSINFO(svPGEnumFieldProp) );
        //if(att == wxT("ENUM"))
        if(isKind)
        {
            svPGEnumFieldProp* en = (svPGEnumFieldProp*) child;
            if(en == nullptr)
                continue;
            en->Populate();
        }
    }
}

void svPGRegisterProp::SetValueFromString(const wxString& str, int flags)
{
    wxCharBuffer buff = str.To8BitData();
    size_t size = std::min(str.length(), (size_t) 8);

    uint64_t data = 0;

    memcpy(&data, buff, size);
    int count = GetChildCount();
    for(int i = 0; i < count; ++i)
    {
        svPGPropBase* prop = dynamic_cast<svPGPropBase*>(Item(i));
        prop->SetData(data);
    }

    SetValue(wxString::Format(wxT("0x%llx"), data ));

}

wxVariant svPGRegisterProp::ChildChanged( wxVariant& thisValue,
                                    int childIndex,
                                    wxVariant& childValue ) const
{
    wxLongLong value = thisValue.GetLongLong();
    wxLongLong child_val = childValue.GetLongLong();
    return wxVariant(value);    // dummy
}

void svPGRegisterProp::RefreshChildren()
{

}

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

    wxString description;
    description << field.GetDesc();

    SetHelpString(description);

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

     SetAttribute( wxT("TYPE"), wxT("ENUM") );


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
    data = (data & m_mask) >> m_bitOffset;
    auto itr = m_elements.begin();
    for(; itr != m_elements.end(); ++itr)
    {
        if(itr->value == data)
            SetValue(itr->index);
    }
}


IMPLEMENT_ABSTRACT_CLASS2(svPGBitProp, wxBoolProperty, svPGPropBase)
IMPLEMENT_ABSTRACT_CLASS2(svPGValueProp, wxStringProperty, svPGPropBase)
