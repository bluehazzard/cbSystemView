#ifndef CBSVPGPROPERTIES_H
#define CBSVPGPROPERTIES_H

#include <cstdint>
#include "cbsvdfilereader.h"
#include <map>
#include <algorithm>
#include <sdk.h>
#include <wx/propgrid/manager.h>
#include <wx/defs.h>
#include <wx/object.h>
#include <debuggermanager.h>


typedef std::vector<uint8_t> data_vetor;

class svPGPropBase : public wxObject
{
        DECLARE_ABSTRACT_CLASS(svPGPropBase);
    public:
        svPGPropBase(const  SVDRegisterProperties* base, wxPGProperty* prop)
        {
            wxString desc = base->GetDesc();
            if(base->GetAccessRight() == SVD_ACCESS_READ)
            {
                prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
                desc += _T(" (Read only)");
            }
            prop->SetHelpString( desc );
        };

        virtual ~svPGPropBase()     {};
        virtual void SetData( uint64_t data ) {};
        virtual void Populate() {};

        uint64_t GetAddress()   {return m_addr; };
        uint64_t GetSize()      {return m_size; };

    protected:
        uint64_t m_baseAddr;
        uint64_t m_addr;
        uint64_t m_offset;
        uint64_t m_size;

};


class svPGPeripheryProp : public wxStringProperty, public svPGPropBase
{
        DECLARE_ABSTRACT_CLASS(svPGPeripheryProp);
    public:
        svPGPeripheryProp(SVDPeriphery &per);
        virtual ~svPGPeripheryProp();

        virtual void SetData(uint64_t data );
        void Populate();
    protected:

    private:
};

class svPGRegisterProp : public wxStringProperty, public svPGPropBase
{
        DECLARE_ABSTRACT_CLASS(svPGRegisterProp);
    public:
        svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per);
        virtual ~svPGRegisterProp();

        void SetData( uint64_t data )    {};
        void Populate();

        virtual wxVariant ChildChanged( wxVariant& thisValue,
                                    int childIndex,
                                    wxVariant& childValue ) const;

        //void ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue)  const;
        void SetValueFromString(const wxString& str, int flags = 0);


        virtual void RefreshChildren();

    protected:

    private:

};

struct svPGEnumFieldElement
{
    svPGEnumFieldElement(wxString _name, wxString _description, wxString _text, int _index, uint64_t _value) :
    name(_name), description(_description), text(_text), index(_index), value(_value)    {};

    wxString name;
    wxString description;
    wxString text;
    int      index;
    uint64_t value;
};

class svPGEnumFieldProp : public wxEnumProperty, public svPGPropBase
{
        DECLARE_ABSTRACT_CLASS(svPGEnumFieldProp);
    public:
        svPGEnumFieldProp(SVDField &field);
        virtual ~svPGEnumFieldProp()    {};

        void Populate();

        void SetData( uint64_t data );
    protected:

    private:

        uint64_t m_data;
        uint64_t m_mask;
        uint64_t m_resetValue;
        uint64_t m_resetMask;
        uint64_t m_bitSize;
        uint64_t m_bitOffset;

        std::vector<svPGEnumFieldElement> m_elements;

};

class svPGBitProp : public wxBoolProperty, public svPGPropBase
{
        DECLARE_ABSTRACT_CLASS(svPGBitProp);
    public:
        svPGBitProp(const SVDField &field) : wxBoolProperty( field.GetName(),
                                                             field.GetName() ) ,
                                            svPGPropBase(&field, this)
        {
             SetAttribute( wxPG_BOOL_USE_CHECKBOX, true );
             m_mask = field.m_bitRange.GetMask();
        };
        virtual ~svPGBitProp()      {};

        void SetData( uint64_t data )
        {
            cbDebuggerPlugin* dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
            //dbg->Log(wxString::Format(wxT("bit value = %d"), (data & m_mask) ));
            SetValueFromInt((data & m_mask));

        };

        void Populate() {};

    protected:

    private:

        uint64_t m_data;
        uint64_t m_mask;
        uint64_t m_resetValue;
        uint64_t m_resetMask;
        uint64_t m_bitSize;
        uint64_t m_bitOffset;
};

class svPGValueProp : public wxStringProperty, public svPGPropBase
{

        DECLARE_ABSTRACT_CLASS(svPGValueProp);

    public:
        svPGValueProp(const SVDField &field) : wxStringProperty( field.GetName(),
                                                                 field.GetName() ),
                                               svPGPropBase(&field, this)
        {
             SetHelpString( field.GetDesc() );
             m_mask = field.m_bitRange.GetMask();
             m_bitOffset = field.m_bitRange.GetOffset();
        };
        virtual ~svPGValueProp()    {};

        void SetData( uint64_t data )     { SetValueFromInt((data & m_mask)>>m_bitOffset); };
        void Populate() {};
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
