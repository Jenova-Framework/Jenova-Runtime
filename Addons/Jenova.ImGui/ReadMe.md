# Jenova Immediate Mode Graphical User Interface (ImGui)

An addon for the Jenova Framework that integrates [Dear ImGui](https://github.com/ocornut/imgui) into the programmer's pipeline.    
It is particularly useful for debugging and prototyping.

> Currently supports only **Windows x64** with **Vulkan** and **OpenGL** backends.

## Usage :

```CPP
// Events
void OnAwake(Caller* instance)
{
	// Create A User Interface Render Event
	JenovaImGui::AddUserInterfaceRenderEvent("ImGuiContext-Demo", [&]()
	{
		ImGui::SetNextWindowPos(ImVec2(300, 50));
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		ImGui::Begin("Jenova Real-Time Interface");
		ImGui::Text("Engine Framerate : %lf fps", Engine::get_singleton()->get_frames_per_second());
		ImGui::Text("GUI Framerate : %lf fps", ImGui::GetIO().Framerate);
		ImGui::End();
	});
}
```

---

Developed by Hamid.Memar

