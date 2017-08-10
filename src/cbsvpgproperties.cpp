#include "cbsvpgproperties.h"
#include <wx/object.h>

class svPGPropBase;

IMPLEMENT_ABSTRACT_CLASS(svPGPropBase, wxObject)

IMPLEMENT_ABSTRACT_CLASS2(svPGPeripheryProp, wxStringProperty, svPGPropBase)

svPGPeripheryProp::svPGPeripheryProp(const SVDPeriphery &per) : wxStringProperty(per.m_name, per.m_name)
{
    m_addr      = per.m_baseAddress;
    m_baseAddr  = per.m_baseAddress;    // Base address is the periphery
    m_offset    = 0;                    // periphery has 0 offset
    //m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = 0; // We have to determine the size of the register...

    SetHelpString(per.m_description);


    auto itr = per.m_registers.begin();
    for ( ; itr != per.m_registers.end(); ++itr)
    {
        SVDRegister* reg = dynamic_cast<SVDRegister*>((*itr).get());
        if(reg == nullptr)
            continue;

        m_size += reg->m_size / 8;  // Size is in bits

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
svPGRegisterProp::svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per) : wxStringProperty(reg.m_name, reg.m_name)
{
    m_addr      = per.m_baseAddress + reg.m_addressOffset;
    m_baseAddr  = per.m_baseAddress;    // Base address is the periphery
    m_offset    = reg.m_addressOffset;  // periphery has 0 offset
    //m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = reg.m_size / 8; // We have to determine the size of the register...
    //ctor

    SetHelpString(reg.m_description);

    auto itr = reg.m_fields.begin();
    for ( ; itr != reg.m_fields.end(); ++itr)
    {
        SVDField* field = dynamic_cast<SVDField*>((*itr).get());
        if(field == nullptr)
            continue;

        if((field->m_bitRange.GetWidth() == 1) && (field->m_enumerated_value.m_values.size() == 0))
        {
            // Flag
            svPGBitProp* prop = new svPGBitProp(*field);
            if(field->m_access == SVD_ACCESS_READ)
                prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
            AddChild(prop);
        }
        else if(field->m_enumerated_value.m_values.size() != 0)
        {
            // Enumeration
            svPGEnumFieldProp* prop = new svPGEnumFieldProp(*field);
            if(field->m_access == SVD_ACCESS_READ)
                prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
            AddChild(prop);
        }
        else
        {
            // String
            svPGValueProp* prop = new svPGValueProp(*field);
            if(field->m_access == SVD_ACCESS_READ)
                prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
            AddChild(prop);
        }

    }

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

void svPGRegisterProp::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue)  const
{

}

IMPLEMENT_ABSTRACT_CLASS2(svPGEnumFieldProp, wxEnumProperty, svPGPropBase)

svPGEnumFieldProp::svPGEnumFieldProp(const SVDField &field) : wxEnumProperty(field.m_name, field.m_name)
{
    m_data          = field.m_resetValue;
    m_mask          = field.m_bitRange.GetMask();
    m_size          = ceil(field.m_bitRange.GetWidth() / 8.0);  // Size in bytes
    m_bitSize       = field.m_bitRange.GetWidth();
    m_bitOffset     = field.m_bitRange.GetOffset();
    m_resetValue    = field.m_resetValue;
    m_resetMask     = field.m_resetMask;

    SetHelpString(field.m_description);

    // Find best length for value and description part
    auto itr = field.m_enumerated_value.m_values.begin();
    size_t name_length = 0;
    size_t desc_length = 0;
    for (; itr != field.m_enumerated_value.m_values.end(); ++itr)
    {
        name_length = std::max((*itr)->m_name.length(), name_length);
        desc_length = std::max((*itr)->m_description.length(), desc_length);
    }

    name_length += 4;

    itr = field.m_enumerated_value.m_values.begin();
    for (; itr != field.m_enumerated_value.m_values.end(); ++itr)
    {
        wxString name = (*itr)->m_name;
        for(size_t i = name.length(); i < name_length; ++i)
            name += _(" ");

        m_elements.push_back(svPGEnumFieldElement((*itr)->m_name,
                                                  (*itr)->m_description,
                                                  name + wxT(": ") + (*itr)->m_description,
                                                  -1,
                                                  (*itr)->m_value));
    }

     SetAttribute( wxT("TYPE"), wxT("ENUM") );


}

void svPGEnumFieldProp::Populate()
{
    auto itr = m_elements.begin();
    for (; itr != m_elements.end(); ++itr)
    {
        (*itr).index = AppendChoice((*itr).text);
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
