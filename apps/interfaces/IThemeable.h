namespace blokusUi
{
	class IThemeable
	{
	public:
		virtual ~IThemeable() = default;

		virtual void updateThemedResources() = 0;
	};
}