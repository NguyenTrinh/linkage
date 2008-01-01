
/*
 *	This file was automatically generated by dbusxx-xml2cpp; DO NOT EDIT!
 */

#ifndef __dbusxx__DBusGlue_hh__ADAPTOR_MARSHAL_H
#define __dbusxx__DBusGlue_hh__ADAPTOR_MARSHAL_H

#include <dbus-c++/dbus.h>

namespace org {
namespace linkage {

class Torrent
: public ::DBus::InterfaceAdaptor
{
public:

    Torrent()
    : ::DBus::InterfaceAdaptor("org.linkage.Torrent")
    {
        register_method(Torrent, GetName, _GetName_stub);
        register_method(Torrent, GetState, _GetState_stub);
        register_method(Torrent, GetRates, _GetRates_stub);
        register_method(Torrent, GetTransfered, _GetTransfered_stub);
        register_method(Torrent, GetProgress, _GetProgress_stub);
        register_method(Torrent, GetPosition, _GetPosition_stub);
        register_method(Torrent, Start, _Start_stub);
        register_method(Torrent, Stop, _Stop_stub);
        register_method(Torrent, Remove, _Remove_stub);
    }

    ::DBus::IntrospectedInterface* const introspect() const 
    {
        static ::DBus::IntrospectedArgument GetName_args[] = 
        {
            { "name", "s", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetState_args[] = 
        {
            { "state", "s", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetRates_args[] = 
        {
            { "rates", "(uu)", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetTransfered_args[] = 
        {
            { "rates", "(tt)", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetProgress_args[] = 
        {
            { "progress", "d", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetPosition_args[] = 
        {
            { "position", "u", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument Start_args[] = 
        {
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument Stop_args[] = 
        {
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument Remove_args[] = 
        {
            { "erase_data", "b", true },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedMethod Torrent_methods[] = 
        {
            { "GetName", GetName_args },
            { "GetState", GetState_args },
            { "GetRates", GetRates_args },
            { "GetTransfered", GetTransfered_args },
            { "GetProgress", GetProgress_args },
            { "GetPosition", GetPosition_args },
            { "Start", Start_args },
            { "Stop", Stop_args },
            { "Remove", Remove_args },
            { 0, 0 }
        };
        static ::DBus::IntrospectedMethod Torrent_signals[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedProperty Torrent_properties[] = 
        {
            { 0, 0, 0, 0 }
        };
        static ::DBus::IntrospectedInterface Torrent_interface = 
        {
            "org.linkage.Torrent",
            Torrent_methods,
            Torrent_signals,
            Torrent_properties
        };
        return &Torrent_interface;
    }

public:

    /* properties exposed by this interface, use
     * property() and property(value) to get and set a particular property
     */

public:

    /* methods exported by this interface,
     * you will have to implement them in your ObjectAdaptor
     */
    virtual ::DBus::String GetName(  ) = 0;
    virtual ::DBus::String GetState(  ) = 0;
    virtual ::DBus::Struct< ::DBus::UInt32, ::DBus::UInt32 > GetRates(  ) = 0;
    virtual ::DBus::Struct< ::DBus::UInt64, ::DBus::UInt64 > GetTransfered(  ) = 0;
    virtual ::DBus::Double GetProgress(  ) = 0;
    virtual ::DBus::UInt32 GetPosition(  ) = 0;
    virtual void Start(  ) = 0;
    virtual void Stop(  ) = 0;
    virtual void Remove( const ::DBus::Bool& erase_data ) = 0;

public:

    /* signal emitters for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual interface method)
     */
    ::DBus::Message _GetName_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::String argout1 = GetName();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _GetState_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::String argout1 = GetState();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _GetRates_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::Struct< ::DBus::UInt32, ::DBus::UInt32 > argout1 = GetRates();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _GetTransfered_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::Struct< ::DBus::UInt64, ::DBus::UInt64 > argout1 = GetTransfered();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _GetProgress_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::Double argout1 = GetProgress();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _GetPosition_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::UInt32 argout1 = GetPosition();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _Start_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        Start();
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
    ::DBus::Message _Stop_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        Stop();
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
    ::DBus::Message _Remove_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::Bool argin1; ri >> argin1;
        Remove(argin1);
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
};

} } 
namespace org {
namespace linkage {

class Interface
: public ::DBus::InterfaceAdaptor
{
public:

    Interface()
    : ::DBus::InterfaceAdaptor("org.linkage.Interface")
    {
        register_method(Interface, Open, _Open_stub);
        register_method(Interface, Add, _Add_stub);
        register_method(Interface, GetVisible, _GetVisible_stub);
        register_method(Interface, SetVisible, _SetVisible_stub);
        register_method(Interface, Quit, _Quit_stub);
    }

    ::DBus::IntrospectedInterface* const introspect() const 
    {
        static ::DBus::IntrospectedArgument Open_args[] = 
        {
            { "file", "s", true },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument Add_args[] = 
        {
            { "file", "s", true },
            { "path", "s", true },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetVisible_args[] = 
        {
            { "visible", "b", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument SetVisible_args[] = 
        {
            { "visible", "b", true },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument Quit_args[] = 
        {
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedMethod Interface_methods[] = 
        {
            { "Open", Open_args },
            { "Add", Add_args },
            { "GetVisible", GetVisible_args },
            { "SetVisible", SetVisible_args },
            { "Quit", Quit_args },
            { 0, 0 }
        };
        static ::DBus::IntrospectedMethod Interface_signals[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedProperty Interface_properties[] = 
        {
            { 0, 0, 0, 0 }
        };
        static ::DBus::IntrospectedInterface Interface_interface = 
        {
            "org.linkage.Interface",
            Interface_methods,
            Interface_signals,
            Interface_properties
        };
        return &Interface_interface;
    }

public:

    /* properties exposed by this interface, use
     * property() and property(value) to get and set a particular property
     */

public:

    /* methods exported by this interface,
     * you will have to implement them in your ObjectAdaptor
     */
    virtual void Open( const ::DBus::String& file ) = 0;
    virtual void Add( const ::DBus::String& file, const ::DBus::String& path ) = 0;
    virtual ::DBus::Bool GetVisible(  ) = 0;
    virtual void SetVisible( const ::DBus::Bool& visible ) = 0;
    virtual void Quit(  ) = 0;

public:

    /* signal emitters for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual interface method)
     */
    ::DBus::Message _Open_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::String argin1; ri >> argin1;
        Open(argin1);
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
    ::DBus::Message _Add_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::String argin1; ri >> argin1;
        ::DBus::String argin2; ri >> argin2;
        Add(argin1, argin2);
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
    ::DBus::Message _GetVisible_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::Bool argout1 = GetVisible();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _SetVisible_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        ::DBus::Bool argin1; ri >> argin1;
        SetVisible(argin1);
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
    ::DBus::Message _Quit_stub( const ::DBus::CallMessage& call )
    {
        ::DBus::MessageIter ri = call.reader();

        Quit();
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
};

} } 
#endif//__dbusxx__DBusGlue_hh__ADAPTOR_MARSHAL_H
