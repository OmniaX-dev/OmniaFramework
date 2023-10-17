#ifndef __DATAFILE_HPP__
#define __DATAFILE_HPP__

#include <ostd/Color.hpp>
#include <ostd/BaseObject.hpp>

#include <vector>

#define DATABLOCK_TRUE 1
#define DATABLOCK_FALSE 0
#define DFD_LIST std::vector<ostd::DataBlock>

namespace ostd
{
	enum class eDataBlockType
	{
		Invalid = 0,
		Int,
		Float,
		String,
		IntArray,
		StringArray,
		Literal
	};

	struct DataBlock : public BaseObject
	{
		public:
			DataBlock(void);
			int32_t asInt(void) const;
			float asFloat(void) const;
			inline bool asBool(void) const { return asInt() == DATABLOCK_TRUE; }
			String asString(void) const;
			std::vector<int32_t> asIntArray(void) const;
			std::vector<String> asStringArray(void) const;
			String asLiteral(void) const;
			Color asColor(void) const;

		private:
			inline DataBlock(bool __invalid) { if (__invalid) invalidate(); }

		public:
			eDataBlockType type;
			String name;
			String value;
	};

	class DataFile : public BaseObject
	{
		public:
			inline DataFile(void) { invalidate(); }
			inline DataFile(String filePath) { load(filePath); }
			inline virtual ~DataFile(void) = default;
			virtual void onDataBlockRead(String instr, DFD_LIST data, int32_t paramCount) = 0;
			void load(String filePath);
			void addLocal(String name, int32_t value);
			void addLocal(String name, String value, bool literal = false);
			const DataBlock getLocal(String name);

			static void addGlobal(String name, int32_t value);
			static void addGlobal(String name, String value, bool literal = false);
			static const DataBlock getGlobal(String name);

		protected:
			void loadDefines(String data);
			String replaceDefines(String data);
			DataBlock parseParameter(String param);

		protected:
			String m_dataFilePath;
			String m_rawContent;
			bool m_hasAutoLoadInfo;
		
		private:
			DFD_LIST m_localDefs;
			inline static DFD_LIST s_globalDefs;
	};
}

#endif
