#pragma once

namespace blokusUI
{
	class IThemeable
	{
	public:
		virtual ~IThemeable() = default;

		virtual void updateThemedResources() = 0;
	};
}