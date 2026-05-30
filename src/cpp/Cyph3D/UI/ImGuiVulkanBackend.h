#pragma once

#include <imgui.h>
#include <memory>

namespace c3d
{
class VKCommandBuffer;
class VKImage;

void ImGui_ImplVKObject_Init();
void ImGui_ImplVKObject_NewFrame();
void ImGui_ImplVKObject_RenderDrawData(const ImDrawData& drawData, const std::shared_ptr<VKCommandBuffer>& commandBuffer, const std::shared_ptr<VKImage>& outputImage);
void ImGui_ImplVKObject_Shutdown();

ImTextureID ImGui_ImplVKObject_ToTextureID(const std::shared_ptr<VKImage>& image);
}