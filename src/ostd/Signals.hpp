#ifndef __SIGNALS_HPP__
#define __SIGNALS_HPP__

#include <ostd/Types.hpp>
#include <ostd/BaseObject.hpp>
#include <vector>
#include <unordered_map>

namespace ostd
{
	//INFO: (Note to self) When adding new Builtin signals, the corresponding static list must
	//		be addeed to the ox::SignalHandler class, and the appropriate branch must be added
	//		to the ox::SignalHandler::emitSignal and the ox::SignalHandler::connect functions.
	//		Also, correct initialization in the ox::SignalHandler::init function must be added
	struct tBuiltinSignals
	{
		inline static constexpr uint32_t NoSignal 				=	0x0000;

		/** Builtin Signals **/
		inline static constexpr uint32_t KeyPressed 			=	0x0001;
		inline static constexpr uint32_t KeyReleased 			=	0x0002;
		inline static constexpr uint32_t MousePressed 			=	0x0003;
		inline static constexpr uint32_t MouseReleased 			=	0x0004;
		inline static constexpr uint32_t MouseMoved 			=	0x0005;

		inline static constexpr uint32_t OnGuiEvent				=	0x2001;

		inline static constexpr uint32_t WindowResized 			=	0x1001;
		/*********************/

		inline static constexpr uint32_t CustomSignalBase 		=	0xFF0000;
	};

	struct tSignalPriority
	{
		inline static constexpr uint8_t RealTime = 0;
		inline static constexpr uint8_t Normal = 1;
	};

	struct tSignal
	{
		const uint8_t priority;
		const uint32_t ID;
		bool handled { false };
		BaseObject& userData;

		inline tSignal(uint32_t id, BaseObject& _userData = BaseObject::InvalidRef(), uint8_t prio = tSignalPriority::Normal) : priority(prio), ID(id), userData(_userData) {  }
	};

	class SignalHandler
	{
		private: struct tSignalObjPair
		{
			BaseObject* object { nullptr };
			uint32_t signal_id { tBuiltinSignals::NoSignal };
		};
		private: struct tDelegateSignal
		{
			uint32_t id;
			BaseObject& ud;

			inline tDelegateSignal(uint32_t _id, BaseObject& _ud) : id(_id), ud(_ud) {  }
		};
		public:
			static void init(void);
			static void refresh(void);

			static void emitSignal(uint32_t signal_id, uint8_t prio = tSignalPriority::Normal, BaseObject& userData = BaseObject::InvalidRef());
			static void connect(BaseObject& object, uint32_t signal_id);

			inline static uint32_t newCustomSignal(uint32_t sub_id) { return tBuiltinSignals::CustomSignalBase + sub_id; }

		private:
			inline static std::vector<tSignalObjPair> m_customRecievers;
			inline static std::vector<tDelegateSignal> m_delegatedSignals;

			/** Builtin signal recievers lists **/
			inline static std::vector<tSignalObjPair> m_mousePressedRecievers;
			inline static std::vector<tSignalObjPair> m_mouseReleasedRecievers;
			inline static std::vector<tSignalObjPair> m_keyPressedRecievers;
			inline static std::vector<tSignalObjPair> m_keyReleasedRecievers;
			inline static std::vector<tSignalObjPair> m_mouseMovedRecievers;
			inline static std::vector<tSignalObjPair> m_windowResizedRecievers;
			inline static std::vector<tSignalObjPair> m_onGuiEventRecievers;
			/************************************/

			inline static constexpr uint16_t __SIGNAL_BUFFER_START_SIZE { 128 };
			inline static constexpr uint16_t __DELEGATED_SIGNALS_BUFFER_START_SIZE { 128 };
	};
	
	class WindowSizeObj : public BaseObject
	{
		public:
			const int32_t width;
			const int32_t height;

			inline WindowSizeObj(int32_t _w, int32_t _h) : width(_w), height(_h)
			{
				setTypeName("ox::WindowSizeObj");
				validate();
			}
	};

}

#endif
