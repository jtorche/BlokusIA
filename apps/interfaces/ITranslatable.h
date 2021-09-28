#pragma once

namespace blokusUI
{
	class ITranslatable
	{
	public:
		virtual ~ITranslatable() = default;

		virtual void retranslate() = 0;
	};
}
