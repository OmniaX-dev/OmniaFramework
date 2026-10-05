#include "DBusMenuServer.hpp"
#include <dbus/dbus.h>
#include <cstdio>

namespace ogfx
{
	namespace gui
	{
		namespace
		{
			constexpr const char* kInterface = "com.canonical.dbusmenu";
			constexpr const char* kLayoutStructSig = "(ia{sv}av)";

			// --- property marshaling -------------------------------------------------------

			void append_string_prop(DBusMessageIter* arrayIter, const char* key, const char* value)
			{
				DBusMessageIter entryIter, variantIter;
				dbus_message_iter_open_container(arrayIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
				dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
				dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "s", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
				dbus_message_iter_close_container(&entryIter, &variantIter);
				dbus_message_iter_close_container(arrayIter, &entryIter);
			}

			void append_bool_prop(DBusMessageIter* arrayIter, const char* key, bool value)
			{
				DBusMessageIter entryIter, variantIter;
				dbus_bool_t v = value ? TRUE : FALSE;
				dbus_message_iter_open_container(arrayIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
				dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
				dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "b", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_BOOLEAN, &v);
				dbus_message_iter_close_container(&entryIter, &variantIter);
				dbus_message_iter_close_container(arrayIter, &entryIter);
			}

			void append_int_prop(DBusMessageIter* arrayIter, const char* key, i32 value)
			{
				DBusMessageIter entryIter, variantIter;
				dbus_message_iter_open_container(arrayIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
				dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
				dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "i", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_INT32, &value);
				dbus_message_iter_close_container(&entryIter, &variantIter);
				dbus_message_iter_close_container(arrayIter, &entryIter);
			}

			// Appends the a{sv} properties dict for one Entry into whatever container iter
			// currently points at (a struct, when called from append_layout_struct).
			void append_entry_properties(DBusMessageIter* iter, const ContextMenu::Entry& e)
			{
				DBusMessageIter arrayIter;
				dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &arrayIter);

				if (e.isSeparator)
				{
					append_string_prop(&arrayIter, "type", "separator");
				}
				else
				{
					ostd::cpp_string label = e.text.cpp_str();
					append_string_prop(&arrayIter, "label", label.c_str());
					if (!e.enabled)
						append_bool_prop(&arrayIter, "enabled", false);
					if (!e.submenus.empty())
						append_string_prop(&arrayIter, "children-display", "submenu");
					if (e.checkable)
					{
						append_string_prop(&arrayIter, "toggle-type", "checkmark");
						append_int_prop(&arrayIter, "toggle-state", e.checked ? 1 : 0);
					}
				}
				dbus_message_iter_close_container(iter, &arrayIter);
			}

			// Appends one full (ia{sv}av) layout struct - recursing into children as nested
			// variants, exactly matching the DBusMenu wire format.
			void append_layout_struct(DBusMessageIter* iter, const ContextMenu::Entry& e)
			{
				DBusMessageIter structIter;
				dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, nullptr, &structIter);

				i32 id = e.id;
				dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT32, &id);
				append_entry_properties(&structIter, e);

				DBusMessageIter childrenIter;
				dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "v", &childrenIter);
				if (!e.isSeparator)
				{
					for (auto& child : e.submenus)
					{
						DBusMessageIter variantIter;
						dbus_message_iter_open_container(&childrenIter, DBUS_TYPE_VARIANT, kLayoutStructSig, &variantIter);
						append_layout_struct(&variantIter, child);
						dbus_message_iter_close_container(&childrenIter, &variantIter);
					}
				}
				dbus_message_iter_close_container(&structIter, &childrenIter);

				dbus_message_iter_close_container(iter, &structIter);
			}

			const ContextMenu::Entry* find_by_id(const stdvec<ContextMenu::Entry>& entries, i32 id)
			{
				for (auto& e : entries)
				{
					if (e.id == id)
						return &e;
					if (const ContextMenu::Entry* found = find_by_id(e.submenus, id))
						return found;
				}
				return nullptr;
			}

			void find_and_invoke(const stdvec<ContextMenu::Entry>& entries, i32 id, const std::function<void(i32)>& onActivate)
			{
				if (onActivate)
					onActivate(id);
			}
		}

		DBusMenuServer::~DBusMenuServer(void)
		{
			disconnect();
		}

		bool DBusMenuServer::connect(void)
		{
			DBusError err;
			dbus_error_init(&err);
			m_connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
			if (dbus_error_is_set(&err))
			{
				OX_WARN("DBusMenuServer: failed to connect to session bus: %s", err.message);
				dbus_error_free(&err);
				m_connection = nullptr;
				return false;
			}
			if (!m_connection)
				return false;

			const char* uniqueName = dbus_bus_get_unique_name(m_connection);
			m_serviceName = uniqueName ? String(uniqueName) : String("");

			static const DBusObjectPathVTable vtable = {
				nullptr,          // unregister_function
				&DBusMenuServer::handle_message,
				nullptr, nullptr, nullptr, nullptr
			};
			if (!dbus_connection_try_register_object_path(m_connection, m_objectPath.c_str(), &vtable, this, &err))
			{
				OX_WARN("DBusMenuServer: failed to register object path %s: %s", m_objectPath.c_str(), dbus_error_is_set(&err) ? err.message : "unknown error");
				dbus_error_free(&err);
				dbus_connection_unref(m_connection);
				m_connection = nullptr;
				return false;
			}
			return true;
		}

		void DBusMenuServer::disconnect(void)
		{
			if (!m_connection)
				return;
			dbus_connection_unregister_object_path(m_connection, m_objectPath.c_str());
			dbus_connection_unref(m_connection);
			m_connection = nullptr;
		}

		void DBusMenuServer::update(void)
		{
			if (!m_connection)
				return;
			dbus_connection_read_write(m_connection, 0);
			while (dbus_connection_dispatch(m_connection) == DBUS_DISPATCH_DATA_REMAINS) {  }
		}

		void DBusMenuServer::setMenu(const stdvec<ContextMenu::Entry>& topLevel)
		{
			m_topLevel = topLevel;
			m_revision++;

			if (!m_connection)
				return;
			DBusMessage* signal = dbus_message_new_signal(m_objectPath.c_str(), kInterface, "LayoutUpdated");
			u32 revision = m_revision;
			i32 parentId = 0;
			DBusMessageIter iter;
			dbus_message_iter_init_append(signal, &iter);
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &revision);
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &parentId);
			dbus_connection_send(m_connection, signal, nullptr);
			dbus_message_unref(signal);
		}

		DBusHandlerResult DBusMenuServer::handle_message(DBusConnection* connection, DBusMessage* message, void* userData)
		{
			return static_cast<DBusMenuServer*>(userData)->handle_message_impl(message);
		}

		DBusHandlerResult DBusMenuServer::handle_message_impl(DBusMessage* message)
		{
			const char* interface = dbus_message_get_interface(message);
			const char* member = dbus_message_get_member(message);
			if (!interface || !member)
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			DBusMessage* reply = nullptr;
			ostd::cpp_string iface(interface), memb(member);

			if (iface == kInterface)
			{
				if (memb == "GetLayout") reply = handle_get_layout(message);
				else if (memb == "GetGroupProperties") reply = handle_get_group_properties(message);
				else if (memb == "GetProperty") reply = handle_get_property(message);
				else if (memb == "Event") reply = handle_event(message);
				else if (memb == "EventGroup") reply = handle_event_group(message);
				else if (memb == "AboutToShow") reply = handle_about_to_show(message);
				else if (memb == "AboutToShowGroup") reply = handle_about_to_show_group(message);
			}
			else if (iface == "org.freedesktop.DBus.Properties")
			{
				if (memb == "Get") reply = handle_properties_get(message);
				else if (memb == "GetAll") reply = handle_properties_get_all(message);
			}
			else if (iface == "org.freedesktop.DBus.Introspectable" && memb == "Introspect")
			{
				reply = handle_introspect(message);
			}

			if (!reply)
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			if (!dbus_message_get_no_reply(message))
				dbus_connection_send(m_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		DBusMessage* DBusMenuServer::handle_get_layout(DBusMessage* msg)
		{
			i32 parentId = 0;
			i32 recursionDepth = -1;
			DBusMessageIter args;
			if (dbus_message_iter_init(msg, &args))
			{
				dbus_message_iter_get_basic(&args, &parentId);
				dbus_message_iter_next(&args);
				dbus_message_iter_get_basic(&args, &recursionDepth);
				// propertyNames (3rd arg) intentionally ignored - we always return every property.
			}

			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			u32 revision = m_revision;
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &revision);

			if (parentId == 0)
			{
				// Synthetic root - id 0, no properties of its own, children = the top-level tree.
				DBusMessageIter structIter;
				dbus_message_iter_open_container(&iter, DBUS_TYPE_STRUCT, nullptr, &structIter);
				i32 rootId = 0;
				dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT32, &rootId);
				DBusMessageIter emptyProps;
				dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "{sv}", &emptyProps);
				dbus_message_iter_close_container(&structIter, &emptyProps);
				DBusMessageIter childrenIter;
				dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "v", &childrenIter);
				for (auto& top : m_topLevel)
				{
					DBusMessageIter variantIter;
					dbus_message_iter_open_container(&childrenIter, DBUS_TYPE_VARIANT, kLayoutStructSig, &variantIter);
					append_layout_struct(&variantIter, top);
					dbus_message_iter_close_container(&childrenIter, &variantIter);
				}
				dbus_message_iter_close_container(&structIter, &childrenIter);
				dbus_message_iter_close_container(&iter, &structIter);
			}
			else if (const ContextMenu::Entry* found = find_by_id(m_topLevel, parentId))
			{
				append_layout_struct(&iter, *found);
			}
			else
			{
				// Unknown parentId - spec allows answering with an empty/childless struct.
				ContextMenu::Entry empty("");
				empty.id = parentId;
				append_layout_struct(&iter, empty);
			}
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_get_group_properties(DBusMessage* msg)
		{
			DBusMessageIter args;
			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			DBusMessageIter resultArray;
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(ia{sv})", &resultArray);

			if (dbus_message_iter_init(msg, &args))
			{
				DBusMessageIter idsIter;
				dbus_message_iter_recurse(&args, &idsIter);
				while (dbus_message_iter_get_arg_type(&idsIter) == DBUS_TYPE_INT32)
				{
					i32 id = 0;
					dbus_message_iter_get_basic(&idsIter, &id);
					if (const ContextMenu::Entry* found = find_by_id(m_topLevel, id))
					{
						DBusMessageIter entryStruct;
						dbus_message_iter_open_container(&resultArray, DBUS_TYPE_STRUCT, nullptr, &entryStruct);
						dbus_message_iter_append_basic(&entryStruct, DBUS_TYPE_INT32, &id);
						append_entry_properties(&entryStruct, *found);
						dbus_message_iter_close_container(&resultArray, &entryStruct);
					}
					if (!dbus_message_iter_next(&idsIter))
						break;
				}
			}
			dbus_message_iter_close_container(&iter, &resultArray);
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_get_property(DBusMessage* msg)
		{
			i32 id = 0;
			const char* name = "";
			dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &id, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter, variantIter;
			dbus_message_iter_init_append(reply, &iter);

			const ContextMenu::Entry* found = find_by_id(m_topLevel, id);
			ostd::cpp_string prop(name);
			if (found && prop == "label" && !found->isSeparator)
			{
				ostd::cpp_string label = found->text.cpp_str();
				const char* v = label.c_str();
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &v);
				dbus_message_iter_close_container(&iter, &variantIter);
			}
			else if (found && prop == "enabled")
			{
				dbus_bool_t v = found->enabled ? TRUE : FALSE;
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "b", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_BOOLEAN, &v);
				dbus_message_iter_close_container(&iter, &variantIter);
			}
			else
			{
				// Unknown id/property - an empty string variant is a harmless, valid fallback.
				const char* v = "";
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &v);
				dbus_message_iter_close_container(&iter, &variantIter);
			}
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_event(DBusMessage* msg)
		{
			i32 id = 0;
			const char* eventId = "";
			dbus_message_get_args(msg, nullptr, DBUS_TYPE_INT32, &id, DBUS_TYPE_STRING, &eventId, DBUS_TYPE_INVALID);
			if (ostd::cpp_string(eventId) == "clicked")
				find_and_invoke(m_topLevel, id, m_onActivate);
			return dbus_message_new_method_return(msg);
		}

		DBusMessage* DBusMenuServer::handle_event_group(DBusMessage* msg)
		{
			DBusMessageIter args;
			if (dbus_message_iter_init(msg, &args))
			{
				DBusMessageIter eventsIter;
				dbus_message_iter_recurse(&args, &eventsIter);
				while (dbus_message_iter_get_arg_type(&eventsIter) == DBUS_TYPE_STRUCT)
				{
					DBusMessageIter eventStruct;
					dbus_message_iter_recurse(&eventsIter, &eventStruct);
					i32 id = 0;
					const char* eventId = "";
					dbus_message_iter_get_basic(&eventStruct, &id);
					dbus_message_iter_next(&eventStruct);
					dbus_message_iter_get_basic(&eventStruct, &eventId);
					if (ostd::cpp_string(eventId) == "clicked")
						find_and_invoke(m_topLevel, id, m_onActivate);
					if (!dbus_message_iter_next(&eventsIter))
						break;
				}
			}
			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter, idErrors;
			dbus_message_iter_init_append(reply, &iter);
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "i", &idErrors);
			dbus_message_iter_close_container(&iter, &idErrors);
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_about_to_show(DBusMessage* msg)
		{
			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			dbus_bool_t needUpdate = FALSE; // we push LayoutUpdated proactively - nothing lazy to build
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &needUpdate);
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_about_to_show_group(DBusMessage* msg)
		{
			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter, a1, a2;
			dbus_message_iter_init_append(reply, &iter);
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "i", &a1);
			dbus_message_iter_close_container(&iter, &a1);
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "i", &a2);
			dbus_message_iter_close_container(&iter, &a2);
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_properties_get(DBusMessage* msg)
		{
			const char* iface = "";
			const char* prop = "";
			dbus_message_get_args(msg, nullptr, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &prop, DBUS_TYPE_INVALID);

			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter, variantIter;
			dbus_message_iter_init_append(reply, &iter);
			ostd::cpp_string p(prop);
			if (p == "Version")
			{
				u32 version = 3;
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "u", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_UINT32, &version);
			}
			else if (p == "TextDirection")
			{
				const char* v = "ltr";
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &v);
			}
			else if (p == "Status")
			{
				const char* v = "normal";
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &v);
			}
			else // IconThemePath and anything else default to an empty string array
			{
				dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "as", &variantIter);
				DBusMessageIter arr;
				dbus_message_iter_open_container(&variantIter, DBUS_TYPE_ARRAY, "s", &arr);
				dbus_message_iter_close_container(&variantIter, &arr);
			}
			dbus_message_iter_close_container(&iter, &variantIter);
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_properties_get_all(DBusMessage* msg)
		{
			DBusMessage* reply = dbus_message_new_method_return(msg);
			DBusMessageIter iter, arrayIter;
			dbus_message_iter_init_append(reply, &iter);
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &arrayIter);

			u32 version = 3;
			{
				DBusMessageIter entryIter, variantIter;
				const char* key = "Version";
				dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
				dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
				dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "u", &variantIter);
				dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_UINT32, &version);
				dbus_message_iter_close_container(&entryIter, &variantIter);
				dbus_message_iter_close_container(&arrayIter, &entryIter);
			}
			append_string_prop(&arrayIter, "TextDirection", "ltr");
			append_string_prop(&arrayIter, "Status", "normal");

			dbus_message_iter_close_container(&iter, &arrayIter);
			return reply;
		}

		DBusMessage* DBusMenuServer::handle_introspect(DBusMessage* msg)
		{
			static const char* xml =
				"<node>"
				"  <interface name=\"com.canonical.dbusmenu\">"
				"    <method name=\"GetLayout\">"
				"      <arg type=\"i\" direction=\"in\"/>"
				"      <arg type=\"i\" direction=\"in\"/>"
				"      <arg type=\"as\" direction=\"in\"/>"
				"      <arg type=\"u\" direction=\"out\"/>"
				"      <arg type=\"(ia{sv}av)\" direction=\"out\"/>"
				"    </method>"
				"    <method name=\"GetGroupProperties\">"
				"      <arg type=\"ai\" direction=\"in\"/>"
				"      <arg type=\"as\" direction=\"in\"/>"
				"      <arg type=\"a(ia{sv})\" direction=\"out\"/>"
				"    </method>"
				"    <method name=\"GetProperty\">"
				"      <arg type=\"i\" direction=\"in\"/>"
				"      <arg type=\"s\" direction=\"in\"/>"
				"      <arg type=\"v\" direction=\"out\"/>"
				"    </method>"
				"    <method name=\"Event\">"
				"      <arg type=\"i\" direction=\"in\"/>"
				"      <arg type=\"s\" direction=\"in\"/>"
				"      <arg type=\"v\" direction=\"in\"/>"
				"      <arg type=\"u\" direction=\"in\"/>"
				"    </method>"
				"    <method name=\"EventGroup\">"
				"      <arg type=\"a(isvu)\" direction=\"in\"/>"
				"      <arg type=\"ai\" direction=\"out\"/>"
				"    </method>"
				"    <method name=\"AboutToShow\">"
				"      <arg type=\"i\" direction=\"in\"/>"
				"      <arg type=\"b\" direction=\"out\"/>"
				"    </method>"
				"    <method name=\"AboutToShowGroup\">"
				"      <arg type=\"ai\" direction=\"in\"/>"
				"      <arg type=\"ai\" direction=\"out\"/>"
				"      <arg type=\"ai\" direction=\"out\"/>"
				"    </method>"
				"    <signal name=\"LayoutUpdated\">"
				"      <arg type=\"u\"/>"
				"      <arg type=\"i\"/>"
				"    </signal>"
				"    <signal name=\"ItemsPropertiesUpdated\">"
				"      <arg type=\"a(ia{sv})\"/>"
				"      <arg type=\"a(ias)\"/>"
				"    </signal>"
				"    <property name=\"Version\" type=\"u\" access=\"read\"/>"
				"    <property name=\"TextDirection\" type=\"s\" access=\"read\"/>"
				"    <property name=\"Status\" type=\"s\" access=\"read\"/>"
				"    <property name=\"IconThemePath\" type=\"as\" access=\"read\"/>"
				"  </interface>"
				"</node>";
			DBusMessage* reply = dbus_message_new_method_return(msg);
			dbus_message_append_args(reply, DBUS_TYPE_STRING, &xml, DBUS_TYPE_INVALID);
			return reply;
		}
	}
}
