#include "Graphics/UI/Scene.h"

#include <RmlUi/Core.h>

namespace Mule::UI
{

	Scene::Scene()
	{
		Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(1280, 720));
		if (!context) return;

		Rml::ElementDocument* doc = context->LoadDocument("assets/ui/main.rml");
		if (doc)
			doc->Show();
	}
}