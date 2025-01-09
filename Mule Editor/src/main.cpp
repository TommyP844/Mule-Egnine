
#include "spdlog/spdlog.h"

#include "mule/Application/Application.h"

// Layers
#include "EditorLayer.h"

int main(int argc, char** argv)
{
    Mule::Application* app = new Mule::Application();

    app->PushLayer<EditorLayer>();

    app->Run();

    delete app;
    return 0;
}