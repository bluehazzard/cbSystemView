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
    (void) expander{ (SetBitFlag(bitfield, flags), void(), 0)... };
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

enum ValueRepresentation
{
    REP_HEX,        /**< Interpret data as hexadecimal value */
    REP_DEC,        /**< Interpret data as signed integer value */
    REP_UDEC,       /**< Interpret data as unsigned integer value */
    REP_BIN,        /**< Interpret data as binary value */
    REP_CHAR,       /**< Interpret data as ASCII character */
    REP_FLOAT,      /**< Interpret data as float (works only on 32Bit register) value */
    REP_BYTE_ARRAY,
    REP_LAST_FLAG   /**< DO NOT USE!!  This is the last value in the enum for calculation purpose */
};


class svPGBaseProp : public wxPGProperty
{
        DECLARE_DYNAMIC_CLASS(svPGBaseProp);
    public:
        svPGBaseProp() : wxPGProperty()
        {

        };

        svPGBaseProp(const wxString& name, const wxString& label) : wxPGProperty(name, label)
        {

        };

        svPGBaseProp(const  SVDRegisterProperties* base, wxPGProperty* prop)
        {
            wxString desc = base->GetDesc();

            if (base->GetAccessRight() == SVD_ACCESS_READ)
            {
                prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
                desc += _T(" (Read only)");
            }

            prop->SetHelpString(desc);

        };

        svPGBaseProp(const SVDPeriphery& per) : wxPGProperty(per.GetName(), per.GetName())
        {
            m_resetValue = per.GetResetValue();
            m_resetMask = per.GetResetMask();
            m_addr      = per.GetBaseAddress();
            m_baseAddr  = per.GetBaseAddress();    // Base address is the periphery
        }

        svPGBaseProp(const SVDPeriphery& per, const SVDRegister& reg) : wxPGProperty(reg.GetName(), reg.GetName())
        {
            m_resetValue = per.GetResetValue();
            m_resetMask = per.GetResetMask();
            m_addr      = per.GetBaseAddress() + reg.GetAddressOfset();
            m_baseAddr  = per.GetBaseAddress();    // Base address is the periphery
            m_offset    = reg.GetAddressOfset();  // periphery has 0 offset
            //m_mask      = 0xFFFFFFFFFFFFFFFF;
            m_size      = ceil(reg.GetSize() / 8.0);    // We have to determine the size of the register...
            m_bitSize   = reg.GetSize();        // Register size is in bits

        }

        svPGBaseProp(const SVDField& field) : wxPGProperty(field.GetName(), field.GetName())
        {
            m_mask      = field.m_bitRange.GetMask();
            m_bitOffset = field.m_bitRange.GetOffset();
            m_bitSize   = field.m_bitRange.GetWidth();
            m_size      = ceil(field.GetSize() / 8.0);
            m_resetValue = field.GetResetValue();
            m_resetMask = field.GetResetMask();
        }

        /** \brief If the property is a Enum property it gets populated in this function
        *
        *  If a sub property can be of the type enum this function has to be called on
        *  every sub property
        * \return virtual void
        *
        */
        virtual void Populate() {};

        bool CanRepresent(ValueRepresentation rep) const    { return m_repCap.test(rep); };
        ValueRepresentation GetRepresentation() const       { return m_rep; };
        void SetRepresentation(ValueRepresentation rep)     { m_rep = rep; GetGrid()->Update(); };

        virtual void GetDataToBinary(wxString& str);
        virtual void SetData(uint64_t data);
        virtual uint64_t GetData();

        std::bitset<REP_LAST_FLAG> m_repCap;    /**< Flag list for all ValueRepresentation */
        ValueRepresentation m_rep;              /**< current representation  */

        uint64_t GetMask()      const   { return m_mask; };             /**< Get bit mask for data */
        void SetMask(uint64_t msk)      { m_mask = msk; };           /**< Get bit mask for data */

        uint64_t GetResetVal()  const   { return m_resetValue; };       /**< Get reset value for data */
        void SetResetVal(uint64_t val)  { m_resetValue = val; };       /**< Get reset value for data */

        uint64_t GetResetMask() const   { return m_resetMask; };        /**< Get bit mask for reset value */
        void SetResetMask(uint64_t msk) { m_resetMask = msk; };        /**< Get bit mask for reset value */

        uint64_t GetBitSize()   const   { return m_bitSize; };          /**< Get bit size of the data*/
        void SetBitSize(uint64_t size)  { m_bitSize = size; };

        uint64_t GetBitOffset() const   { return m_bitOffset; };        /**< Get bit offset (<< operation) */
        void SetBitOffset(uint64_t of)  { m_bitOffset = of; };        /**< Get bit offset (<< operation) */

        uint64_t GetAddress() const     { return m_addr; };             /**< Get address*/
        void SetAddress(uint64_t addr)  { m_addr = addr; };             /**< Set address*/

        /** \brief Get the site of the register/periphery in bytes
        *
        * \return uint64_t
        *
        */
        uint64_t GetSize()    const    { return m_size; };
        void SetSize(uint64_t size)    { m_size = size; };

        /** \brief Set the data from a wxString encoded with From8BitData()
        *
        *
        * \param str const wxString&   String created with wxString::To8BitData()
        * \return virtual void
        *
        */
        virtual void SetDataFromBinary(const wxString& str);

        virtual wxString GetDescription()                       { return m_desc; };
        virtual void SetDescription(const wxString& desc)       { m_desc = desc; };


    private:

        uint64_t m_mask;        /**< mask for data */
        uint64_t m_resetValue;  /**< reset value for data */
        uint64_t m_resetMask;   /**< reset mask for data */
        uint64_t m_bitSize;     /**< bit size in data*/
        uint64_t m_bitOffset;   /**< bit offset in data */
        uint64_t m_baseAddr;    /**< Base address of the periphery/register. If it is a fag the address of the register is stored  */
        uint64_t m_addr;        /**< Base address of the periphery/register. If it is a fag the address of the register is stored  */
        uint64_t m_offset;      /**< Offset from base address ?needed?  */
        uint64_t m_size;        /**< size of the periphery/register/flag in bytes can be 0 for bit size < 8  */

        wxString m_desc;
};

class svPGData : public wxObject
{
        DECLARE_DYNAMIC_CLASS(svPGData);

    public:
        svPGData()
        {
            m_data = 0;
        }

        svPGData(const  SVDRegisterProperties* base, wxPGProperty* prop)
        {
            wxString desc = base->GetDesc();

            if (base->GetAccessRight() == SVD_ACCESS_READ)
            {
                prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
                desc += _T(" (Read only)");
            }

            prop->SetHelpString(desc);

        };

        svPGData(uint64_t dat)
        {
            m_data = dat;
        }

        virtual ~svPGData()     {};


        /** \brief Get the uint64_t data. Only usable for registers and below
         * To get the data from a periphery you have to iterate over it and get all register
         *
         * \return virtual uint64_t
         *
         */
        virtual uint64_t GetData() const { return m_data; };



        /** \brief Set data (only for registers and below
         *
         * This function has to be overwritten from the properties to interpreted the data and hand it to his sub fields
         * \param data uint64_t
         * \return virtual void
         *
         */
        virtual void SetData(uint64_t data)     { m_data = data; };

        /** \brief Set the data from a wxString encoded in human readable format
         *
         *
         * \param str const wxString&   String to extract data from (ex. "0xDEADBEEF" )
         * \return virtual void
         *
         */
        virtual void SetDataFromString(const wxString& str);


        /** \brief Return the data as human readable string. The format is set by @ref SetRepresentation()
         *
         * \return virtual wxString
         *
         */
        virtual wxString GetDataReadable(uint64_t data, uint64_t bitsize, ValueRepresentation rep) const;
        virtual wxString GetDataReadable(ValueRepresentation rep, uint64_t bitsize) const;

        void SetDataFromBinary(const wxString& str, const uint64_t& mask, const uint64_t offset);

        bool operator ==(const svPGData& right)
        {
            if (right.m_data     == m_data)
                return true;
            else
                return false;
        }

    protected:

        uint64_t m_data;        /**< actual data */



};

#if wxCHECK_VERSION(3,0,0)
WX_PG_DECLARE_VARIANT_DATA(svPGData);
#else
WX_PG_DECLARE_VARIANT_DATA(svPGDataVariantData,
                           svPGData,
                           wxEMPTY_PARAMETER_VALUE);
const svPGData& svPGDataRefFromVariant(const wxVariant& variant);
svPGData& svPGDataRefFromVariant(wxVariant& variant);
#endif // wxCHECK_VERSION


class svPGRegisterProp;

class svPGPeripheryProp : public svPGBaseProp
{
        WX_PG_DECLARE_PROPERTY_CLASS(svPGPeripheryProp);
    public:

        svPGPeripheryProp()                             {};
#if wxCHECK_VERSION(3,0,0)
        svPGPeripheryProp(SVDPeriphery &per);
#else
        svPGPeripheryProp(SVDPeriphery &per, wxPropertyGrid* parent);
#endif
        virtual ~svPGPeripheryProp();

        virtual void SetData(uint64_t data);
        void SetValueFromString(const wxString& str, int flags);

        virtual void SetDataFromBinary(const wxString& str);
        virtual void SetDataFromString(const wxString& str);        /**< Not supported in this property */

        virtual wxString ValueToString(wxVariant& value, int argFlags) const { return wxEmptyString; };
#if !wxCHECK_VERSION(3,0,0)
        virtual wxString GetValueAsString(int argFlags) const;
#endif

        virtual bool StringToValue(wxVariant& variant, const wxString& text, int argFlags) const override
        {
            return false;
        }

        virtual void RefreshChildren();

        void Populate();

        void SetRegisterChanged(svPGRegisterProp* reg);
        svPGRegisterProp* GetRegisterChanged();

    protected:

        svPGRegisterProp* m_RegisterChanged;

    private:

        wxString m_internalData;
};

class svPGRegisterProp : public svPGBaseProp
{
        WX_PG_DECLARE_PROPERTY_CLASS(svPGRegisterProp);
    public:
        svPGRegisterProp() : svPGBaseProp(wxEmptyString, wxEmptyString)     {};
        svPGRegisterProp(const SVDRegister &reg, const SVDPeriphery &per);
        virtual ~svPGRegisterProp();

        void SetData(uint64_t data);
        uint64_t GetDataFromChildren()    const;
        void Populate();

#if wxCHECK_VERSION(3,0,0)
        virtual wxVariant ChildChanged(wxVariant& thisValue,
                                       int childIndex,
                                       wxVariant& childValue) const;
#else
        virtual void ChildChanged(wxVariant& thisValue,
                                  int childIndex,
                                  wxVariant& childValue) const;
#endif // wxCHECK_VERSION

        //void ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue)  const;
        void SetValueFromString(const wxString& str, int flags = 0);
        virtual void SetDataFromBinary(const wxString& str);

        virtual wxString ValueToString(wxVariant& value, int argFlags) const;
#if !wxCHECK_VERSION(3,0,0)
        virtual wxString GetValueAsString(int argFlags) const;
#endif
        virtual bool StringToValue(wxVariant& variant, const wxString& text, int argFlags) const override
        {
            return false;
        }

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

#if !wxCHECK_VERSION(3, 0, 0)
class svPGEnumFieldProp : public svPGBaseProp, public wxEnumProperty
#else
class svPGEnumFieldProp : public svPGBaseProp
#endif
{
        WX_PG_DECLARE_PROPERTY_CLASS(svPGEnumFieldProp);

    public:
        svPGEnumFieldProp() : svPGBaseProp(wxEmptyString, wxEmptyString)              {};
        svPGEnumFieldProp(SVDField &field);
        virtual ~svPGEnumFieldProp()    {};

        void Populate();

        virtual int GetChoiceSelection() const;

        virtual wxString ValueToString(wxVariant& value, int argFlags) const;
#if !wxCHECK_VERSION(3,0,0)
        virtual wxString GetValueAsString(int argFlags) const;
#endif
        virtual bool StringToValue(wxVariant& variant, const wxString& text, int argFlags) const override;
        virtual bool IntToValue(wxVariant&  variant, int number, int argFlags = 0) const;

#if !wxCHECK_VERSION(3, 0, 0)
        virtual int GetChoiceInfo(wxPGChoiceInfo* choiceinfo);
#endif

    protected:

    private:

        std::vector<svPGEnumFieldElement> m_elements;

};


class svPGValueProp : public svPGBaseProp
{

        WX_PG_DECLARE_PROPERTY_CLASS(svPGValueProp);

    public:

        svPGValueProp() : svPGBaseProp(wxEmptyString, wxEmptyString)      {};
        svPGValueProp(const SVDField& field);

        virtual ~svPGValueProp()    {};

        virtual wxString ValueToString(wxVariant& value, int argFlags) const;
#if !wxCHECK_VERSION(3,0,0)
        virtual wxString GetValueAsString(int argFlags) const;
#endif
        virtual bool StringToValue(wxVariant& variant, const wxString& text, int argFlags) const;

    protected:

    private:
};


class svPGBitProp : public svPGBaseProp
{
        WX_PG_DECLARE_PROPERTY_CLASS(svPGBitProp);

    public:
        svPGBitProp() : svPGBaseProp(wxEmptyString, wxEmptyString)      {};
        svPGBitProp(const SVDField& field);
        virtual ~svPGBitProp()      {};

        wxString ValueToString(wxVariant& value, int argFlags) const;
#if !wxCHECK_VERSION(3,0,0)
        virtual wxString GetValueAsString(int argFlags) const;
        virtual int GetChoiceInfo(wxPGChoiceInfo *choiceinfo);
#endif
        bool StringToValue(wxVariant& variant, const wxString& text, int argFlags) const override;
        virtual bool IntToValue(wxVariant& variant, int number, int argFlags) const;
        virtual void SetChoiceSelection(int newValue);

        virtual int GetChoiceSelection() const;
        bool IsValueUnspecified() const;

        void SetDataFromBinary(const wxString& str);



    protected:

    private:

};

wxString printBits(size_t const size, uint64_t ptr);


#endif // CBSVPGPROPERTIES_H
