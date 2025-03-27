#include "Graphics/Context/ImGuiContext.h"

#include "Application/Events.h"

// Submodules
#include "Graphics/imguiImpl/imgui_impl_vulkan.h"
#include "Graphics/imguiImpl/imgui_impl_glfw.h"
#include "ImGuizmo.h"

namespace Mule
{
	static void check_vk_result(VkResult err)
	{
		if (err == VK_SUCCESS)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	ImGuiContext::ImGuiContext(Ref<GraphicsContext> graphicsContext)
		:
		mContext(graphicsContext),
		mFrameIndex(0)
	{
		mGraphicsQueue = graphicsContext->GetGraphicsQueue();
		mFrameBuffer = graphicsContext->GetSwapchainFrameBuffer();

		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].CommandPool = mGraphicsQueue->CreateCommandPool();
			mFrameData[i].CommandBuffer = mFrameData[i].CommandPool->CreateCommandBuffer();
			mFrameData[i].Fence = graphicsContext->CreateFence();
			mFrameData[i].Semaphore = graphicsContext->CreateSemaphore();
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.DisplaySize.x = graphicsContext->GetWindow()->GetWidth();
		io.DisplaySize.y = graphicsContext->GetWindow()->GetHeight();

		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

		ImGui_ImplGlfw_InitForVulkan(graphicsContext->GetWindow()->GetGLFWWindow(), false);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = graphicsContext->GetInstance();
		initInfo.PhysicalDevice = graphicsContext->GetPhysicalDevice();
		initInfo.Device = graphicsContext->GetDevice();
		initInfo.DescriptorPool = graphicsContext->GetDescriptorPool();
		initInfo.Queue = graphicsContext->GetGraphicsQueue()->GetHandle();
		initInfo.QueueFamily = graphicsContext->GetGraphicsQueue()->GetQueueFamilyIndex();
		initInfo.ImageCount = 2;
		initInfo.MinImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.Subpass = 0;
		initInfo.UseDynamicRendering = false;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = check_vk_result;
		initInfo.RenderPass = mFrameBuffer->GetRenderPass();
		ImGui_ImplVulkan_Init(&initInfo);

		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
		ImGui::GetStyle().Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.40f);
		ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 0.75f);
		ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.62f, 1.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.28f, 0.37f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.35f, 0.44f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.37f, 0.44f, 0.53f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SeparatorActive] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_DockingPreview] = ImVec4(0.28f, 0.56f, 1.00f, 0.70f);
		ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.90f, 0.55f, 0.00f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.28f, 0.56f, 1.00f, 0.35f);
		ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.35f);

		ImGui::GetStyle().FrameRounding = 5.f;

#pragma region KeyMap

		mKeyMap[(uint32_t)KeyCode::Key_SPACE] = ImGuiKey_Space;
		mKeyMap[(uint32_t)KeyCode::Key_APOSTROPHE] = ImGuiKey_Apostrophe;
		mKeyMap[(uint32_t)KeyCode::Key_COMMA] = ImGuiKey_Comma;
		mKeyMap[(uint32_t)KeyCode::Key_MINUS] = ImGuiKey_Minus;
		mKeyMap[(uint32_t)KeyCode::Key_PERIOD] = ImGuiKey_Period;
		mKeyMap[(uint32_t)KeyCode::Key_SLASH] = ImGuiKey_Slash;
		mKeyMap[(uint32_t)KeyCode::Key_0] = ImGuiKey_0;
		mKeyMap[(uint32_t)KeyCode::Key_1] = ImGuiKey_1;
		mKeyMap[(uint32_t)KeyCode::Key_2] = ImGuiKey_2;
		mKeyMap[(uint32_t)KeyCode::Key_3] = ImGuiKey_3;
		mKeyMap[(uint32_t)KeyCode::Key_4] = ImGuiKey_4;
		mKeyMap[(uint32_t)KeyCode::Key_5] = ImGuiKey_5;
		mKeyMap[(uint32_t)KeyCode::Key_6] = ImGuiKey_6;
		mKeyMap[(uint32_t)KeyCode::Key_7] = ImGuiKey_7;
		mKeyMap[(uint32_t)KeyCode::Key_8] = ImGuiKey_8;
		mKeyMap[(uint32_t)KeyCode::Key_9] = ImGuiKey_9;
		mKeyMap[(uint32_t)KeyCode::Key_SEMICOLON] = ImGuiKey_Semicolon;
		mKeyMap[(uint32_t)KeyCode::Key_EQUAL] = ImGuiKey_Equal;
		mKeyMap[(uint32_t)KeyCode::Key_A] = ImGuiKey_A;
		mKeyMap[(uint32_t)KeyCode::Key_B] = ImGuiKey_B;
		mKeyMap[(uint32_t)KeyCode::Key_C] = ImGuiKey_C;
		mKeyMap[(uint32_t)KeyCode::Key_D] = ImGuiKey_D;
		mKeyMap[(uint32_t)KeyCode::Key_E] = ImGuiKey_E;
		mKeyMap[(uint32_t)KeyCode::Key_F] = ImGuiKey_F;
		mKeyMap[(uint32_t)KeyCode::Key_G] = ImGuiKey_G;
		mKeyMap[(uint32_t)KeyCode::Key_H] = ImGuiKey_H;
		mKeyMap[(uint32_t)KeyCode::Key_I] = ImGuiKey_I;
		mKeyMap[(uint32_t)KeyCode::Key_J] = ImGuiKey_J;
		mKeyMap[(uint32_t)KeyCode::Key_K] = ImGuiKey_K;
		mKeyMap[(uint32_t)KeyCode::Key_L] = ImGuiKey_L;
		mKeyMap[(uint32_t)KeyCode::Key_M] = ImGuiKey_M;
		mKeyMap[(uint32_t)KeyCode::Key_N] = ImGuiKey_N;
		mKeyMap[(uint32_t)KeyCode::Key_O] = ImGuiKey_O;
		mKeyMap[(uint32_t)KeyCode::Key_P] = ImGuiKey_P;
		mKeyMap[(uint32_t)KeyCode::Key_Q] = ImGuiKey_Q;
		mKeyMap[(uint32_t)KeyCode::Key_R] = ImGuiKey_R;
		mKeyMap[(uint32_t)KeyCode::Key_S] = ImGuiKey_S;
		mKeyMap[(uint32_t)KeyCode::Key_T] = ImGuiKey_T;
		mKeyMap[(uint32_t)KeyCode::Key_U] = ImGuiKey_U;
		mKeyMap[(uint32_t)KeyCode::Key_V] = ImGuiKey_V;
		mKeyMap[(uint32_t)KeyCode::Key_W] = ImGuiKey_W;
		mKeyMap[(uint32_t)KeyCode::Key_X] = ImGuiKey_X;
		mKeyMap[(uint32_t)KeyCode::Key_Y] = ImGuiKey_Y;
		mKeyMap[(uint32_t)KeyCode::Key_Z] = ImGuiKey_Z;
		mKeyMap[(uint32_t)KeyCode::Key_LEFT_BRACKET] = ImGuiKey_LeftBracket;
		mKeyMap[(uint32_t)KeyCode::Key_BACKSLASH] = ImGuiKey_Backslash;
		mKeyMap[(uint32_t)KeyCode::Key_RIGHT_BRACKET] = ImGuiKey_RightBracket;
		mKeyMap[(uint32_t)KeyCode::Key_GRAVE_ACCENT] = ImGuiKey_GraveAccent;
		// mKeyMap[(uint32_t)KeyCode::Key_WORLD_1] = ;
		// mKeyMap[(uint32_t)KeyCode::Key_WORLD_2] = ;
		mKeyMap[(uint32_t)KeyCode::Key_ESCAPE] = ImGuiKey_Escape;
		mKeyMap[(uint32_t)KeyCode::Key_ENTER] = ImGuiKey_Enter;
		mKeyMap[(uint32_t)KeyCode::Key_TAB] = ImGuiKey_Tab;
		mKeyMap[(uint32_t)KeyCode::Key_BACKSPACE] = ImGuiKey_Backspace;
		mKeyMap[(uint32_t)KeyCode::Key_INSERT] = ImGuiKey_Insert;
		mKeyMap[(uint32_t)KeyCode::Key_DELETE] = ImGuiKey_Delete;
		mKeyMap[(uint32_t)KeyCode::Key_RIGHT] = ImGuiKey_RightArrow;
		mKeyMap[(uint32_t)KeyCode::Key_LEFT] = ImGuiKey_LeftArrow;
		mKeyMap[(uint32_t)KeyCode::Key_DOWN] = ImGuiKey_DownArrow;
		mKeyMap[(uint32_t)KeyCode::Key_UP] = ImGuiKey_UpArrow;
		mKeyMap[(uint32_t)KeyCode::Key_PAGE_UP] = ImGuiKey_PageUp;
		mKeyMap[(uint32_t)KeyCode::Key_PAGE_DOWN] = ImGuiKey_PageDown;
		mKeyMap[(uint32_t)KeyCode::Key_HOME] = ImGuiKey_Home;
		mKeyMap[(uint32_t)KeyCode::Key_END] = ImGuiKey_End;
		mKeyMap[(uint32_t)KeyCode::Key_CAPS_LOCK] = ImGuiKey_CapsLock;
		mKeyMap[(uint32_t)KeyCode::Key_SCROLL_LOCK] = ImGuiKey_ScrollLock;
		mKeyMap[(uint32_t)KeyCode::Key_NUM_LOCK] = ImGuiKey_NumLock;
		mKeyMap[(uint32_t)KeyCode::Key_PRINT_SCREEN] = ImGuiKey_PrintScreen;
		mKeyMap[(uint32_t)KeyCode::Key_PAUSE] = ImGuiKey_Pause;
		mKeyMap[(uint32_t)KeyCode::Key_F1] = ImGuiKey_F1;
		mKeyMap[(uint32_t)KeyCode::Key_F2] = ImGuiKey_F2;
		mKeyMap[(uint32_t)KeyCode::Key_F3] = ImGuiKey_F3;
		mKeyMap[(uint32_t)KeyCode::Key_F4] = ImGuiKey_F4;
		mKeyMap[(uint32_t)KeyCode::Key_F5] = ImGuiKey_F5;
		mKeyMap[(uint32_t)KeyCode::Key_F6] = ImGuiKey_F6;
		mKeyMap[(uint32_t)KeyCode::Key_F7] = ImGuiKey_F7;
		mKeyMap[(uint32_t)KeyCode::Key_F8] = ImGuiKey_F8;
		mKeyMap[(uint32_t)KeyCode::Key_F9] = ImGuiKey_F9;
		mKeyMap[(uint32_t)KeyCode::Key_F10] = ImGuiKey_F10;
		mKeyMap[(uint32_t)KeyCode::Key_F11] = ImGuiKey_F11;
		mKeyMap[(uint32_t)KeyCode::Key_F12] = ImGuiKey_F12;
		mKeyMap[(uint32_t)KeyCode::Key_F13] = ImGuiKey_F13;
		mKeyMap[(uint32_t)KeyCode::Key_F14] = ImGuiKey_F14;
		mKeyMap[(uint32_t)KeyCode::Key_F15] = ImGuiKey_F15;
		mKeyMap[(uint32_t)KeyCode::Key_F16] = ImGuiKey_F16;
		mKeyMap[(uint32_t)KeyCode::Key_F17] = ImGuiKey_F17;
		mKeyMap[(uint32_t)KeyCode::Key_F18] = ImGuiKey_F18;
		mKeyMap[(uint32_t)KeyCode::Key_F19] = ImGuiKey_F19;
		mKeyMap[(uint32_t)KeyCode::Key_F20] = ImGuiKey_F20;
		mKeyMap[(uint32_t)KeyCode::Key_F21] = ImGuiKey_F21;
		mKeyMap[(uint32_t)KeyCode::Key_F22] = ImGuiKey_F22;
		mKeyMap[(uint32_t)KeyCode::Key_F23] = ImGuiKey_F23;
		mKeyMap[(uint32_t)KeyCode::Key_F24] = ImGuiKey_F24;
		// mKeyMap[(uint32_t)KeyCode::Key_F25] = ImGuiKey_F25;
		mKeyMap[(uint32_t)KeyCode::Key_KP_0] = ImGuiKey_Keypad0;
		mKeyMap[(uint32_t)KeyCode::Key_KP_1] = ImGuiKey_Keypad1;
		mKeyMap[(uint32_t)KeyCode::Key_KP_2] = ImGuiKey_Keypad2;
		mKeyMap[(uint32_t)KeyCode::Key_KP_3] = ImGuiKey_Keypad3;
		mKeyMap[(uint32_t)KeyCode::Key_KP_4] = ImGuiKey_Keypad4;
		mKeyMap[(uint32_t)KeyCode::Key_KP_5] = ImGuiKey_Keypad5;
		mKeyMap[(uint32_t)KeyCode::Key_KP_6] = ImGuiKey_Keypad6;
		mKeyMap[(uint32_t)KeyCode::Key_KP_7] = ImGuiKey_Keypad7;
		mKeyMap[(uint32_t)KeyCode::Key_KP_8] = ImGuiKey_Keypad8;
		mKeyMap[(uint32_t)KeyCode::Key_KP_9] = ImGuiKey_Keypad9;
		mKeyMap[(uint32_t)KeyCode::Key_KP_DECIMAL] = ImGuiKey_KeypadDecimal;
		mKeyMap[(uint32_t)KeyCode::Key_KP_DIVIDE] = ImGuiKey_KeypadDivide;
		mKeyMap[(uint32_t)KeyCode::Key_KP_MULTIPLY] = ImGuiKey_KeypadMultiply;
		mKeyMap[(uint32_t)KeyCode::Key_KP_SUBTRACT] = ImGuiKey_KeypadSubtract;
		mKeyMap[(uint32_t)KeyCode::Key_KP_ADD] = ImGuiKey_KeypadAdd;
		mKeyMap[(uint32_t)KeyCode::Key_KP_ENTER] = ImGuiKey_KeypadEnter;
		mKeyMap[(uint32_t)KeyCode::Key_KP_EQUAL] = ImGuiKey_KeypadEqual;
		mKeyMap[(uint32_t)KeyCode::Key_LEFT_SHIFT] = ImGuiKey_LeftShift;
		mKeyMap[(uint32_t)KeyCode::Key_LEFT_CONTROL] = ImGuiKey_LeftCtrl;
		mKeyMap[(uint32_t)KeyCode::Key_LEFT_ALT] = ImGuiKey_LeftAlt;
		mKeyMap[(uint32_t)KeyCode::Key_LEFT_SUPER] = ImGuiKey_LeftSuper;
		mKeyMap[(uint32_t)KeyCode::Key_RIGHT_SHIFT] = ImGuiKey_RightShift;
		mKeyMap[(uint32_t)KeyCode::Key_RIGHT_CONTROL] = ImGuiKey_RightCtrl;
		mKeyMap[(uint32_t)KeyCode::Key_RIGHT_ALT] = ImGuiKey_RightAlt;
		mKeyMap[(uint32_t)KeyCode::Key_RIGHT_SUPER] = ImGuiKey_RightSuper;
		mKeyMap[(uint32_t)KeyCode::Key_MENU] = ImGuiKey_Menu;
		mKeyMap[(uint32_t)KeyCode::Mod_SHIFT] = ImGuiKey_ModShift;
		mKeyMap[(uint32_t)KeyCode::Mod_CONTROL] = ImGuiKey_ModCtrl;
		mKeyMap[(uint32_t)KeyCode::Mod_ALT] = ImGuiKey_ModAlt;
		mKeyMap[(uint32_t)KeyCode::Mod_SUPER] = ImGuiKey_ModSuper;
		mKeyMap[(uint32_t)KeyCode::Mod_CAPS_LOCK] = ImGuiKey_CapsLock;
		mKeyMap[(uint32_t)KeyCode::Mod_NUM_LOCK] = ImGuiKey_NumLock;

#pragma endregion

	}

	ImGuiContext::~ImGuiContext()
	{
		vkDeviceWaitIdle(mContext->GetDevice());
		for (int i = 0; i < 2; i++)
		{
			mFrameData[i].CommandBuffer = nullptr;
			mFrameData[i].CommandPool = nullptr;
		}
		ImGui_ImplVulkan_Shutdown();
	}

	void ImGuiContext::NewFrame()
	{
		// We put this here so when users grab the rendering finished semaphore they get the right one
		mFrameIndex ^= 1;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiContext::EndFrame(const std::vector<WeakRef<Semaphore>>& waitSemaphores)
	{
		FrameData& frameData = mFrameData[mFrameIndex];
		mFrameBuffer->SetClearValue(0, glm::vec4(0.f, 0.f, 0.f, 1.f));
		mFrameBuffer = mContext->GetSwapchainFrameBuffer();
		
		frameData.Fence->Wait();
		frameData.Fence->Reset();
		frameData.CommandPool->Reset();
		frameData.CommandBuffer->Begin();
		frameData.CommandBuffer->TransitionSwapchainFrameBufferForRendering(mFrameBuffer);
		frameData.CommandBuffer->BeginRenderPass(mFrameBuffer);

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, frameData.CommandBuffer->GetHandle());

		frameData.CommandBuffer->EndRenderPass();
		frameData.CommandBuffer->TransitionSwapchainFrameBufferForPresent(mFrameBuffer);
		frameData.CommandBuffer->End();
		mGraphicsQueue->Submit(frameData.CommandBuffer, waitSemaphores, { frameData.Semaphore }, frameData.Fence);

		ImGui::UpdatePlatformWindows();
	}

	void ImGuiContext::Resize(uint32_t width, uint32_t height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = mContext->GetWindow()->GetWidth();
		io.DisplaySize.y = mContext->GetWindow()->GetHeight();
	}

	void ImGuiContext::OnEvent(Ref<Event> event)
	{
		switch (event->Type)
		{
		case EventType::WindowResizeEvent:
		{
			WeakRef<WindowResizeEvent> resizeEvent = event;
			Resize(resizeEvent->Width, resizeEvent->Height);
		}
			break;
		case EventType::MouseMoveEvent:
		{
			WeakRef<MouseMoveEvent> e = event;
			ImGui::GetIO().AddMousePosEvent(e->GetX(), e->GetY());
		}
			break;
		case EventType::CharEvent:
		{
			WeakRef<CharEvent> e = event;
			ImGui::GetIO().AddInputCharacter(e->GetChar());
		}
			break;
		case EventType::KeyboardEvent:
		{
			WeakRef<KeyboardEvent> e = event;
			ImGuiKey key = mKeyMap[(uint32_t)e->GetKey()];
			ImGui::GetIO().AddKeyEvent(key, e->IsKeyPressed());

			if (e->GetModifier() != KeyCode::None)
			{
				ImGuiKey key = mKeyMap[(uint32_t)e->GetModifier()];
				ImGui::GetIO().AddKeyEvent(key, e->IsKeyPressed());
			}
		}
			break;
		case EventType::MouseButtonEvent:
		{
			WeakRef<MouseButtonEvent> e = event;

			ImGui::GetIO().AddMouseButtonEvent((int)e->GetMouseButton(), e->IsButtonPressed());
		}
		break;
		}
	}
}