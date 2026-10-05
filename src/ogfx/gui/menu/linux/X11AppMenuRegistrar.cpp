#include "X11AppMenuRegistrar.hpp"
#include <dbus/dbus.h>

namespace ogfx
{
	namespace gui
	{
		namespace
		{
			constexpr const char* kService = "com.canonical.AppMenu.Registrar";
			constexpr const char* kPath = "/com/canonical/AppMenu/Registrar";
			constexpr const char* kInterface = "com.canonical.AppMenu.Registrar";
		}

		bool X11AppMenuRegistrar::isAvailable(DBusConnection* connection)
		{
			if (!connection)
				return false;
			DBusError err;
			dbus_error_init(&err);
			dbus_bool_t has = dbus_bus_name_has_owner(connection, kService, &err);
			bool result = !dbus_error_is_set(&err) && has;
			dbus_error_free(&err);
			return result;
		}

		bool X11AppMenuRegistrar::registerWindow(DBusConnection* connection, u64 x11WindowId, const String& serviceName, const String& objectPath)
		{
			if (!connection)
				return false;
			DBusMessage* msg = dbus_message_new_method_call(kService, kPath, kInterface, "RegisterWindow");
			if (!msg)
				return false;
			u32 windowId = (u32)x11WindowId;
			const char* path = objectPath.c_str();
			dbus_message_append_args(msg, DBUS_TYPE_UINT32, &windowId, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID);

			DBusError err;
			dbus_error_init(&err);
			DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, msg, 2000, &err);
			dbus_message_unref(msg);
			bool ok = !dbus_error_is_set(&err);
			if (reply) dbus_message_unref(reply);
			dbus_error_free(&err);
			return ok;
		}

		void X11AppMenuRegistrar::unregisterWindow(DBusConnection* connection, u64 x11WindowId)
		{
			if (!connection)
				return;
			DBusMessage* msg = dbus_message_new_method_call(kService, kPath, kInterface, "UnregisterWindow");
			if (!msg)
				return;
			u32 windowId = (u32)x11WindowId;
			dbus_message_append_args(msg, DBUS_TYPE_UINT32, &windowId, DBUS_TYPE_INVALID);
			dbus_message_set_no_reply(msg, TRUE);
			dbus_connection_send(connection, msg, nullptr);
			dbus_message_unref(msg);
		}
	}
}
