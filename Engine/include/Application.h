

namespace Engine
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

		void PushLayer();
		void PopLayer();
	};
}