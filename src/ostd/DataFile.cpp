#include "DataFile.hpp"
#include "Utils.hpp"
#include <utility>

//TODO: Add all errors

namespace ostd
{
	DataBlock::DataBlock(void)
	{
		type = eDataBlockType::Invalid;
		name = "";
		value = "null";
		invalidate();
		setTypeName("ox::DataBlock");
	}

	int32_t DataBlock::asInt(void) const
	{
		if (type != eDataBlockType::Int)
			return 0;
		return StringEditor(value).toInt();
	}

	float DataBlock::asFloat(void) const
	{
		if (type != eDataBlockType::Float)
			return 0;
		return StringEditor(value).toFloat();
	}

	String DataBlock::asString(void) const
	{
		return StringEditor(value).trim().str();
	}

	std::vector<int32_t> DataBlock::asIntArray(void) const
	{
		std::vector<int32_t> arr;
		StringEditor _value = value;
		if (type != eDataBlockType::IntArray || _value.contains("\""))
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asIntArray(...) -> 1");
			// return arr;
		}
		if (!_value.trim().startsWith("(") || !_value.trim().endsWith(")"))
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asIntArray(...) -> 2");
			// return arr;
		}
		_value = _value.substr(1, _value.len() - 1);
		auto st = _value.tokenize(",");
		while (st.hasNext())
			arr.push_back(StringEditor(st.next()).toInt());
		return arr;
	}

	std::vector<String> DataBlock::asStringArray(void) const
	{
		std::vector<String> arr;
		StringEditor _value = value;
		if (type != eDataBlockType::StringArray || !_value.contains("\""))
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asStringArray(...) -> 1");
			// return arr;
		}
		if (!_value.trim().startsWith("(") || !_value.trim().endsWith(")"))
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asStringArray(...) -> 2");
			// return arr;
		}
		_value = _value.substr(1, _value.len() - 1);
		auto st = _value.tokenize(",");
		StringEditor token = "";
		while (st.hasNext())
		{
			token = st.next();
			if (!token.startsWith("\"") || !token.endsWith("\""))
			{
				// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asStringArray(...) -> 3");
				// continue;
			}
			token = token.substr(1, token.len() - 1);
			arr.push_back(token.str());
		}
		return arr;
	}

	String DataBlock::asLiteral(void) const
	{
		if (type != eDataBlockType::Literal)
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asLiteral(...)");
			// return "";
		}
		return StringEditor(value).trim().str();
	}

	Color DataBlock::asColor(void) const
	{
		if (type != eDataBlockType::IntArray)
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockType, "omnia::common::DataBlock::asColor(...)");
			// return Color::Transparent;
		}
		std::vector<int32_t> v = asIntArray();
		if (v.size() != 4)
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataBlockValue, "omnia::common::DataBlock::asColor(...): Color-array must contain exactly 4 items.");
			// return Color::Transparent;
		}
		return Color(v[0], v[1], v[2], v[3]);
	}



	void DataFile::load(String filePath)
	{
		if (StringEditor(filePath).trim().str() == "")
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataFilePath, StringBuilder("in omnia::common::DataFile::load(...); path is empty").get());
			// invalidate();
			return;
		}
		setValid(true);
		setTypeName("ox::DataFile");
		m_dataFilePath = filePath;
		std::vector<String> lines;
		if (!Utils::readFile(filePath, lines))
		{
			// ErrorHandler::instance().pushError(Errors::InvalidDataFilePath, StringBuilder("in omnia::common::DataFile::load(...); path = ").add(filePath).get());
			// invalidate();
			return;
		}
		m_rawContent = "";
		for (auto& line : lines)
			m_rawContent += line + "\n";
		loadDefines(m_rawContent);
		String newData = replaceDefines(m_rawContent);
		auto st = StringEditor(newData).tokenize("]");
		StringEditor token = "";
		while (st.hasNext()) //Priority round
		{
			token = st.next();
			if (!token.startsWith("["))
			{
				// ErrorHandler::instance().pushError(Errors::InvalidDataFileBlock, "omnia::common::DataFile::load(...); Priority-round; All DataFile blocks must start with a '[' character");
				continue;
			}
			token = token.substr(1);
			token.trim();
			if (token.startsWith("*"))
			{
				token = token.substr(1);
				token.trim();
				if (!token.contains(":"))
					continue;
				StringEditor inst = token.substr(0, token.indexOf(":"));
				inst.trim();
				StringEditor data = token.substr(token.indexOf(":") + 1);
				data.trim();
				DFD_LIST paramList;
				auto st2 = data.tokenize(";");
				String param = "";
				while (st2.hasNext())
				{
					param = st2.next();
					DataBlock p = parseParameter(param);
					if (p.isValid())
						paramList.push_back(p);
					// else
					// 	ErrorHandler::instance().pushError(Errors::UnableToParseDataBlock, StringBuilder("omnia::common::DataFile::load(...); Priority-round; param=").add(param).add("; inst=").add(inst).get());
				}
				onDataBlockRead(inst.toLower().str(), paramList, paramList.size());
			}
		}
		st.cycle();
		while (st.hasNext()) //Main Round
		{
			token = st.next();
			if (!token.startsWith("["))
			{
				// ErrorHandler::instance().pushError(Errors::InvalidDataFileBlock, "omnia::common::DataFile::load(...); Main-round; All DataFile blocks must start with a '[' character");
				continue;
			}
			token = token.substr(1);
			token.trim();
			if (token.startsWith("$") || token.startsWith("*") || token.startsWith("#") || token.startsWith("@") || token.startsWith("!"))
				continue;
			else if (token.startsWith(">"))
			{
				token = token.substr(1);
				token.trim();
				std::cout << token << "\n";
				continue;
			}
			else
			{
				if (!token.contains(":"))
					continue;
				StringEditor inst = token.substr(0, token.indexOf(":"));
				inst.trim();
				StringEditor data = token.substr(token.indexOf(":") + 1);
				data.trim();
				DFD_LIST paramList;
				auto st2 = data.tokenize(";");
				String param = "";
				while (st2.hasNext())
				{
					param = st2.next();
					DataBlock p = parseParameter(param);
					if (p.isValid())
						paramList.push_back(p);
					// else
					// 	ErrorHandler::instance().pushError(Errors::UnableToParseDataBlock, StringBuilder("omnia::common::DataFile::load(...); Main-round; param=").add(param).add("; inst=").add(inst).get());
				}
				onDataBlockRead(inst.toLower().str(), paramList, paramList.size());
			}
		}
	}

	void DataFile::addLocal(String name, int32_t value)
	{
		DataBlock fv;
		fv.type = eDataBlockType::Int;
		fv.name = name;
		fv.value = StringEditor().addi(value).str();
		m_localDefs.push_back(fv);
	}

	void DataFile::addLocal(String name, String value, bool literal)
	{
		DataBlock fv;
		fv.type = (literal ? eDataBlockType::Literal : eDataBlockType::String);
		fv.name = name;
		fv.value = value;
		m_localDefs.push_back(fv);
	}

	const DataBlock DataFile::getLocal(String name)
	{
		for (auto& fv : m_localDefs)
		{
			if (StringEditor(fv.name).trim().str() == StringEditor(name).trim().str())
				return std::as_const(fv);
		}
		DataBlock fv;
		// ErrorHandler::instance().pushError(Errors::LocalDataBlockNotFound, StringBuilder("in omnia::common::DataFile::getLocal(...); name=").add(name).get());
		return std::as_const(fv);
	}

	void DataFile::addGlobal(String name, int32_t value)
	{
		DataBlock fv;
		fv.type = eDataBlockType::Int;
		fv.name = name;
		fv.value = StringEditor().addi(value).str();
		s_globalDefs.push_back(fv);
	}

	void DataFile::addGlobal(String name, String value, bool literal)
	{
		DataBlock fv;
		fv.type = (literal ? eDataBlockType::Literal : eDataBlockType::String);
		fv.name = name;
		fv.value = value;
		s_globalDefs.push_back(fv);
	}

	const DataBlock DataFile::getGlobal(String name)
	{
		for (auto& fv : s_globalDefs)
		{
			if (StringEditor(fv.name).trim().str() == StringEditor(name).trim().str())
				return std::as_const(fv);
		}
		DataBlock fv;
		// ErrorHandler::instance().pushError(Errors::GlobalDataBlockNotFound, StringBuilder("in omnia::common::DataFile::getLocal(...); name=").add(name).get());
		return fv;//std::as_const(fv);
	}

	void DataFile::loadDefines(String data)
	{
		auto st = StringEditor(data).tokenize("]");
		StringEditor token = "";
		while (st.hasNext())
		{
			token = st.next();
			if (!token.startsWith("["))
			{
				// ErrorHandler::instance().pushError(Errors::InvalidDataFileBlock, "omnia::common::DataFile::loadDefines(...); All DataFile Defines must start with a '[' character");
				continue;
			}
			token = token.substr(1);
			token.trim();
			if (token.startsWith("#")) //Local var
			{
				if (!token.contains("="))
				{
					// ErrorHandler::instance().pushError(Errors::InvalidDataBlockDefineValue, "omnia::common::DataFile::loadDefines(...); Local DataFile Defines must contain an assignment operator and a valid Value");
					continue;
				}
				String name = "";
				StringEditor value = "";
				name = token.substr(1, token.indexOf("="));
				name = StringEditor(name).trim().str();
				value = token.substr(token.indexOf("=") + 1);
				value.trim();
				if (StringEditor(value).startsWith("%"))
				{
					value = StringEditor(value).substr(1);
					value.trim();
					addLocal(name, value.str(), true);
				}
				else if (!value.startsWith("\"") && !value.endsWith("\""))
					addLocal(name, value.toInt());
				else if (value.startsWith("\"") && value.endsWith("\""))
				{
					value = value.substr(1, value.len() - 1);
					addLocal(name, value.str());
				}
				else
				{
					// ErrorHandler::instance().pushError(Errors::InvalidDataBlockDefineValue, "omnia::common::DataFile::loadDefines(...); Unknown value type in Local Define assignment");
					continue;
				}
			}
			else if (token.startsWith("@")) //Global var
			{
				if (!token.contains("="))
				{
					// ErrorHandler::instance().pushError(Errors::InvalidDataBlockDefineValue, "omnia::common::DataFile::loadDefines(...); Global DataFile Defines must contain an assignment operator and a valid Value");
					continue;
				}
				String name = "";
				StringEditor value = "";
				name = token.substr(1, token.indexOf("="));
				name = StringEditor(name).trim().str();
				value = token.substr(token.indexOf("=") + 1);
				value.trim();
				if (value.startsWith("%"))
				{
					value = value.substr(1);
					value.trim();
					DataFile::addGlobal(name, value.str(), true);
				}
				else if (!value.startsWith("\"") && !value.endsWith("\""))
					DataFile::addGlobal(name, value.toInt());
				else if (value.startsWith("\"") && value.endsWith("\""))
				{
					value = value.substr(1, value.len() - 1);
					DataFile::addGlobal(name, value.str());
				}
				else
				{
					// ErrorHandler::instance().pushError(Errors::InvalidDataBlockDefineValue, "omnia::common::DataFile::loadDefines(...); Unknown value type in Global Define assignment");
					continue;
				}
			}
		}
	}

	String DataFile::replaceDefines(String data)
	{
		StringEditor newData = data;
		for (auto& def : m_localDefs)
		{
			if (def.type == eDataBlockType::String)
				newData = newData.replaceAll(def.name, StringEditor("\"").add(def.value).add("\"").str());
			else if (def.type == eDataBlockType::Int)
				newData = newData.replaceAll(def.name, def.value);
			else if (def.type == eDataBlockType::Literal)
				newData = newData.replaceAll(def.name, def.value);
		}
		for (auto& def : s_globalDefs)
		{
			if (def.type == eDataBlockType::String)
				newData = newData.replaceAll(def.name, StringEditor("\"").add(def.value).add("\"").str());
			else if (def.type == eDataBlockType::Int)
				newData = newData.replaceAll(def.name, def.value);
			else if (def.type == eDataBlockType::Literal)
				newData = newData.replaceAll(def.name, def.value);
		}
		return newData.str();
	}

	DataBlock DataFile::parseParameter(String _param)
	{
		DataBlock db;
		StringEditor param = _param;
		if (!param.contains("="))
		{
			// ErrorHandler::instance().pushError(Errors::DataBlockParamAssignMissing, "omnia::common::DataFile::parseParameter(...);");
			return db;
		}
		param = param.trim();
		String name = "";
		StringEditor value = "";
		name = param.substr(0, param.indexOf("="));
		name = StringEditor(name).trim().str();
		value = param.substr(param.indexOf("=") + 1);
		value.trim();
		db.name = StringEditor(name).toLower().str();
		db.value = value.str();
		if (value.startsWith("\""))
		{
			if (!value.endsWith("\""))
			{
				db.invalidate();
				// ErrorHandler::instance().pushError(Errors::DataBlockUnmatchedDoubleQuote, "omnia::common::DataFile::parseParameter(...);");
				return db;
			}
			value = value.substr(1, value.len() - 1);
			db.type = eDataBlockType::String;
			db.value = value.str();
		}
		else if (value.startsWith("("))
		{
			if (!value.endsWith(")"))
			{
				db.invalidate();
				// ErrorHandler::instance().pushError(Errors::DataBlockUnmatchedParenth, "omnia::common::DataFile::parseParameter(...);");
				return db;
			}
			db.type = (value.contains("\"") ? eDataBlockType::StringArray : eDataBlockType::IntArray);
		}
		else if (value.toLower().startsWith("f"))
		{
			value = value.substr(1);
			value.trim();
			db.value = value.str();
			db.type = eDataBlockType::Float;
		}
		else
			db.type = eDataBlockType::Int;
		db.validate();
		return db;
	}
}
