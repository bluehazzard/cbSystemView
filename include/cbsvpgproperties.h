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
#include <bitset>


template <typename A, typename V>
void SetBitFlag(A& bitfield, V flag)
{
    bitfield.set(static_cast<int>(flag));
    return;
}

template <typename A, typename ...Args>
void SetBitFlag(A& bitfield, Args... flags)
{
    using expander = int[];
    (void) expander{ (SetBitFlag(bitfield,flags), void(), 0)... };
    return;
}


/** \brief This is the base class for all properties.
 *
 * It contains the data represented in binary form as uint64_t.
 * The data for all over register is calculated on demand.
 * Register contain the data as it is represented on the device
 * Properties under the register (bits and flags ecc.) contain the
 * data adjusted by offset and mask. So if you call GetData() on a bit
 * you will get "1" or "0" also if the bit position of the flag is BIT3
 * in the register
 */
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


        /** \brief If the property is a Enum property it gets populated in this function
         *
         *  If a sub property can be of the type enum this function has to be called on
         *  every sub property
         * \return virtual void
         *
         */
        virtual void Populate() {};

        uint64_t GetAddress() const  { return m_addr; };

        /** \brief Get the site of the register/periphery in bytes
         *
         * \return uint64_t
         *
         */
        uint64_t GetSize()    const    { return m_size; };

        /** \brief Get the uint64_t data. Only usable for registers and below
         * To get the data from a periphery you have to iterate over it and get all register
         *
         * \return virtual uint64_t
         *
         */
        virtual uint64_t GetData()  const    { return m_data; };

        uint64_t GetMask()      const { return m_mask; };             /**< Get bit mask for data */
        uint64_t GetResetVal()  const { return m_resetValue; };       /**< Get reset value for data */
        uint64_t GetResetMask() const { return m_resetMask; };        /**< Get bit mask for reset value */
        uint64_t GetBitSize()   const { return m_bitSize; };          /**< Get bit size of the data*/
        uint64_t GetBitOffset() const { return m_bitOffset; };        /**< Get bit offset (<< operation) */

        /** \brief Set data (only for registers and below
         *
         * This function has to be overwritten from the properties to interpreted the data and hand it to his sub fields
         * \param data uint64_t
         * \return virtual void
         *
         */
        virtual void SetData(uint64_t data)     { m_data = data; };


        /** \brief Set the data from a wxString encoded with From8BitData()
         *
         *
         * \param str const wxString&   String created with wxString::To8BitData()
         * \return virtual void
         *
         */
        virtual void SetDataFromBinary(const wxString& str)    = 0;

        /** \brief Set the data from a wxString encoded in human readable format
         *
         *
         * \param str const wxString&   String to extract data from (ex. "0xDEADBEEF" )
         * \return virtual void
         *
         */
        virtual void SetDataFromString(const wxString& str)    = 0;

        enum ValueRepresentation
        {
            REP_HEX,        /**< Interpret data as hexadecimal value */
            REP_DEC,        /**< Interpret data as signed integer value */
            REP_UDEC,       /**< Interpret data as unsigned integer value */
            REP_BIN,        /**< Interpret data as binary value */
            REP_CHAR,       /**< Interpret data as ASCII character */
            REP_FLOAT,      /**< Interpret data as float (works only on 32Bit register) value */
            REP_LAST_FLAG   /**< DO NOT USE!!  This is the last value in the enum for calculation purpose */
        };


        /** \brief Return the data as human readable string. The format is set by @ref SetRepresentation()
         *
         * \return virtual wxString
         *
         */
        virtual wxString GetDataReadable() const;

        virtual wxString GetDataReadable(uint64_t data,uint64_t bitsize, ValueRepresentation rep) const;

        virtual void UpdateView()               {};

        virtual wxString GetDescription()                       { return m_desc; };
        virtual void SetDescription(const wxString& desc)       { m_desc = desc; };

        /** \brief Check if this property value can be represented as @ref rep
         *
         * \param rep
         * \return bool CanRepresent(ValueRepresentation rep)     true if the representation is valid
         *
         */
        bool CanRepresent(ValueRepresentation rep)                      { return m_repCap.test(static_cast<int>(rep)); };
        virtual void SetRepresentation(ValueRepresentation rep);
        virtual ValueRepresentation GetRepresentation()  const          { return m_rep; };

    protected:
        uint64_t m_baseAddr;    /**< Base address of the periphery/register. If it is a fag the address of the register is stored  */
        uint64_t m_addr;        /**< Base address of the periphery/register. If it is a fag the address of the register is stored  */
        uint64_t m_offset;      /**< Offset from base address ?needed?  */
        uint64_t m_size;        /**< size of the periphery/register/flag in bytes can be 0 for bit size < 8  */

        uint64_t m_data;        /**< actual data */
        uint64_t m_mask;        /**< mask for data */
        uint64_t m_resetValue;  /**< reset value for data */
        uint64_t m_resetMask;   /**< reset mask for data */
        uint64_t m_bitSize;     /**< bit size in data*/
        uint64_t m_bitOffset;   /**< bit offset in data */

        std::bitset<REP_LAST_FLAG> m_repCap;    /**< Flag list for all ValueRepresentation */
        ValueRepresentation m_rep;              /**< current representation  */

        wxString m_desc;

};


class svPGPeripheryProp : public wxStringProperty, public svPGPropBase
{
        DECLARE_ABSTRACT_CLASS(svPGPeripheryProp);
    public:

        svPGPeripheryProp(SVDPeriphery &per);
        virtual ~svPGPeripheryProp();

        virtual void SetData(uint64_t data );
        void SetValueFromString(const wxString& str, int flags);

        virtual void SetDataFromBinary(const wxString& str);
        virtual void SetDataFromString(const wxString& str);        /**< Not supported in this property */

        void Populate();
    protected:

    private:
};

class svPGRegisterProp : public wxStringProperty , public svPGPropBase
{
        DECLARE_ABSTRACT_CLASS(svPGRegisterProp);
    public:
        svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per);
        virtual ~svPGRegisterProp();

        void SetData( uint64_t data );
        uint64_t GetDataFromChildren()    const;
        void Populate();
        void UpdateView();

        #if wxCHECK_VERSION(3,0,0)
        virtual wxVariant ChildChanged( wxVariant& thisValue,
                                    int childIndex,
                                    wxVariant& childValue ) const;
        #else
        virtual void ChildChanged( wxVariant& thisValue,
                                    int childIndex,
                                    wxVariant& childValue ) const;
        #endif // wxCHECK_VERSION

        //void ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue)  const;
        void SetValueFromString(const wxString& str, int flags = 0);
        virtual void SetDataFromString(const wxString& str);
        virtual void SetDataFromBinary(const wxString& str);

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
        virtual void SetDataFromBinary(const wxString& str);
        virtual void SetDataFromString(const wxString& str);

        void UpdateView();

    protected:

    private:

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


             m_mask         = field.m_bitRange.GetMask();
             m_bitOffset    = field.m_bitRange.GetOffset();
             m_bitSize      = 1;
             m_resetMask    = field.m_bitRange.GetMask();
             m_resetValue   = field.GetResetValue();

             wxString desc = field.GetDesc();
             desc << wxT("\n\n");
             desc << wxString::Format(wxT("Bit size: %lld\n"), m_bitSize);
             desc << wxString::Format(wxT("Bit offset: %lld\n"), m_bitOffset);

             SetDescription(desc);
             SetHelpString(desc);
        };
        virtual ~svPGBitProp()      {};

        void SetData( uint64_t data )
        {
            SetValueFromInt((data & m_mask));
        };

        virtual void SetDataFromString(const wxString& str);
        virtual void SetDataFromBinary(const wxString& str);

        void Populate() {};

    protected:

    private:

};

class svPGValueProp : public wxStringProperty, public svPGPropBase
{

        DECLARE_ABSTRACT_CLASS(svPGValueProp);

    public:

        svPGValueProp(const SVDField &field) : wxStringProperty( field.GetName(),
                                                                 field.GetName() ),
                                               svPGPropBase(&field, this)
        {
            SetBitFlag(m_repCap, REP_HEX, REP_BIN, REP_DEC, REP_UDEC, REP_FLOAT, REP_CHAR);
            m_rep = REP_HEX;

            wxString desc =  field.GetDesc();

            m_mask = field.m_bitRange.GetMask();
            m_bitOffset = field.m_bitRange.GetOffset();
            m_bitSize = field.m_bitRange.GetWidth();
            m_size = field.GetSize() / 8;


            desc << wxString::Format(wxT("\nBit size: %lld"), m_bitSize);
            desc << wxString::Format(wxT("\nBit offset: %lld"), m_bitOffset);

            SetHelpString(desc);

        };

        virtual ~svPGValueProp()    {};

        virtual void SetDataFromString(const wxString& str);
        virtual void SetDataFromBinary(const wxString& str);

        void Populate() {};
        void SetData( uint64_t data );
        void UpdateView();


    protected:

    private:
};

wxString printBits(size_t const size, uint64_t ptr);


#endif // CBSVPGPROPERTIES_H
