#pragma once

#include <string>

#include "Core/Common.h"

namespace blokusUi
{
    struct XmlConfigReader
	{
		static core::flat_hash_map<std::string, std::string> loadConfiguration(
			const std::string& _resourcePath,
			const std::string& _elementName);
	};
}
