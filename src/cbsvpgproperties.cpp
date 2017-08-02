#include "cbsvpgproperties.h"

svPGPeripheryProp::svPGPeripheryProp(SVDPeriphery &per) : wxStringProperty(per.m_name, per.m_name)
{
    m_addr      = per.m_baseAddress;
    m_baseAddr  = per.m_baseAddress;    // Base address is the periphery
    m_offset    = 0;                    // periphery has 0 offset
    m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = 0; // We have to determine the size of the register...
    //ctor
}

svPGPeripheryProp::~svPGPeripheryProp()
{
    //dtor
}

void svPGPeripheryProp::SetData(data_vetor &data, data_vetor::iterator& start )
{
    for (unsigned int i = 0;i < GetChildCount() ; ++i )
    {
        svPGPropBase* child = dynamic_cast<svPGPropBase>(Item(i));
        if(child == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_("svPGPeripheryProp::SetData: ") + Item(i)->GetName() + _(" child of ") + GetName() + _(" can not be casted to svPGPropBase*"));
            continue;
        }
        child->SetData(data, start);
    }
}

svPGRegisterProp::svPGRegisterProp(SVDRegister &reg) : wxStringProperty(reg.m_name, reg.m_name)
{
    m_addr      = reg.m_baseAddress;
    m_baseAddr  = reg.m_baseAddress;    // Base address is the periphery
    m_offset    = 0;                    // periphery has 0 offset
    m_mask      = 0xFFFFFFFFFFFFFFFF;
    m_size      = 0; // We have to determine the size of the register...
    //ctor
}

svPGRegisterProp::~svPGRegisterProp()
{
    //dtor
}

typedef

svPGFieldProp::svPGFieldProp(SVDField &field) : wxEnumProperty(field.m_name, field.m_name)
{
    m_data          = field.m_resetValue;
    m_mask          = field.m_bitRange.GetMask();
    m_size          = ceil(field.m_bitRange.GetWidth() / 8.0);  // Size in bytes
    m_bitSize       = field.m_bitRange.GetWidth();
    m_bitOffset     = field.m_bitRange.GetOffset();
    m_resetValue    = field.m_resetValue;
    m_resetMask     = field.m_resetMask;

    // Find best length for value and description part
    auto itr = field.m_enumerated_value.m_values.begin();
    size_t name_length = 0;
    size_t desc_length = 0;
    for (; itr != field.m_enumerated_value.m_values.end(); ++itr)
    {
        name_length = std::max((*value_itr)->m_name, name_length);
        desc_length = std::max((*value_itr)->m_description, desc_length);
    }

    name_length += 4;

    itr = field.m_enumerated_value.m_values.begin();
    for (; itr != field.m_enumerated_value.m_values.end(); ++itr)
    {
        wxString name = (*value_itr)->m_name;
        for(size_t i = name.length(); i < name_length+ ++i)
            name += _(" ");

        AppendChoice(name + wxT(": ") + (*value_itr)->m_description);
    }


}

void svPGFieldProp::SetData(data_vetor &data, data_vetor::iterator& start )
{
    if(start == data.end())
        return;
    for unsigned int i = 0; i < m_size; ++i)
    {

    }
}
