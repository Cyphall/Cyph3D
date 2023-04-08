#include "UIViewport.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.h"
#include "Cyph3D/Rendering/SceneRenderer/RaytracingSceneRenderer.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

#include <imgui_internal.h>
#include <stb_image_write.h>
#include <GLFW/glfw3.h>
#include <magic_enum.hpp>
#include <glm/gtc/type_ptr.hpp>

std::unique_ptr<SceneRenderer> UIViewport::_sceneRenderer;
UIViewport::RendererType UIViewport::_sceneRendererType = UIViewport::RendererType::Rasterization;

Camera UIViewport::_camera;
bool UIViewport::_cameraFocused = false;
glm::vec2 UIViewport::_lockedCursorPos;

bool UIViewport::_fullscreen = false;

bool UIViewport::_leftClickPressedOnViewport = false;
glm::vec2 UIViewport::_leftClickPressPos;

ImGuizmo::OPERATION UIViewport::_gizmoMode = ImGuizmo::TRANSLATE;
ImGuizmo::MODE UIViewport::_gizmoSpace = ImGuizmo::LOCAL;

void UIViewport::show()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	bool open = ImGui::Begin("Viewport");
	
	ImGui::PopStyleVar();
	
	if (open)
	{
		drawHeader();
		
		glm::ivec2 viewportStartLocal = glm::vec2(ImGui::GetCursorPos());
		glm::ivec2 viewportEndLocal = glm::vec2(ImGui::GetWindowContentRegionMax());
		
		glm::uvec2 viewportSize = glm::max(viewportEndLocal - viewportStartLocal, glm::ivec2(0));
		
		glm::ivec2 viewportStartGlobal = glm::vec2(ImGui::GetCursorScreenPos());
		glm::ivec2 viewportEndGlobal = viewportStartGlobal + glm::ivec2(viewportSize);
		
		if (_sceneRenderer && _sceneRenderer->getSize() != viewportSize)
		{
			_sceneRenderer.reset();
		}
		
		Window& window = Engine::getWindow();
		
		if (_cameraFocused && window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == Window::MouseButtonState::Released)
		{
			_cameraFocused = false;
			window.setInputMode(GLFW_CURSOR_NORMAL);
			window.setCursorPos(_lockedCursorPos);
		}
		
		if (viewportSize.x > 0 && viewportSize.y > 0)
		{
			if (!_sceneRenderer)
			{
				switch (_sceneRendererType)
				{
					case RendererType::Rasterization:
						_sceneRenderer = std::make_unique<RasterizationSceneRenderer>(viewportSize);
						break;
					case RendererType::Raytracing:
//						_sceneRenderer = std::make_unique<RaytracingSceneRenderer>(viewportSize); //TODO: reimplement RaytracingSceneRenderer with hardware raytracing
						_sceneRenderer = std::make_unique<RasterizationSceneRenderer>(viewportSize);
						break;
				}
				
				_camera.setAspectRatio(static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y));
			}
			
			if (_cameraFocused)
			{
				_camera.update(window.getCursorPos() - _lockedCursorPos);
				window.setCursorPos(_lockedCursorPos);
			}
			
			_sceneRenderer->onNewFrame();
			Engine::getScene().onPreRender(*_sceneRenderer, _camera);
			const VKPtr<VKImageView>& textureView = _sceneRenderer->render(_camera);
			
			ImGui::Image(
				static_cast<ImTextureID>(const_cast<VKPtr<VKImageView>*>(&textureView)),
				glm::vec2(textureView->getImage()->getSize(0)),
				ImVec2(0, 0),
				ImVec2(1, 1));
			
			if (window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == Window::MouseButtonState::Clicked && ImGui::IsItemHovered())
			{
				_cameraFocused = true;
				_lockedCursorPos = window.getCursorPos();
				window.setInputMode(GLFW_CURSOR_DISABLED);
				
				_leftClickPressedOnViewport = false;
			}
			
			glm::vec2 viewportCursorPos = window.getCursorPos() - glm::vec2(viewportStartGlobal);
			
			if (!_cameraFocused && window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == Window::MouseButtonState::Clicked && ImGui::IsItemHovered())
			{
				_leftClickPressedOnViewport = true;
				_leftClickPressPos = viewportCursorPos;
			}
			
			if (_leftClickPressedOnViewport && window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == Window::MouseButtonState::Released)
			{
				_leftClickPressedOnViewport = false;
				if (ImGui::IsItemHovered() && glm::distance(_leftClickPressPos, viewportCursorPos) < 5.0f)
				{
					Entity* clickedEntity = _sceneRenderer->getClickedEntity(glm::uvec2(viewportCursorPos));
					UIInspector::setSelected(clickedEntity);
				}
			}
			
			drawGizmo(viewportStartGlobal, viewportSize);
		}
	}
	
	ImGui::End();
}

Camera& UIViewport::getCamera()
{
	return _camera;
}

void UIViewport::setCamera(Camera camera)
{
	_camera = camera;
	
	if (_sceneRenderer)
	{
		glm::uvec2 framebufferSize = _sceneRenderer->getSize();
		_camera.setAspectRatio(static_cast<float>(framebufferSize.x) / static_cast<float>(framebufferSize.y));
	}
}

void UIViewport::drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize)
{
	IInspectable* selected = UIInspector::getSelected();
	if (selected == nullptr)
		return;
	
	Entity* entity = dynamic_cast<Entity*>(selected);
	if (entity == nullptr)
		return;
	
	ImGuizmo::SetRect(viewportStart.x, viewportStart.y, viewportSize.x, viewportSize.y);
	
	Transform& transform = entity->getTransform();
	glm::mat4 worldModel = transform.getLocalToWorldMatrix();
	
	glm::mat4 view = _camera.getView();
	glm::mat4 projection = _camera.getProjection();
	projection[1][1] *= -1;
	
	ImGui::PushClipRect(viewportStart, viewportStart + viewportSize, false);
	
	ImGuizmo::SetDrawlist();
	bool changed = ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(projection),
		_gizmoMode,
		_gizmoSpace,
		glm::value_ptr(worldModel));
	
	ImGui::PopClipRect();
	
	if (changed)
	{
		glm::mat4 localModel = transform.getParent()->getWorldToLocalMatrix() * worldModel;
		
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localModel), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));
		
		if (position != transform.getLocalPosition())
		{
			transform.setLocalPosition(position);
		}
		else if (rotation != transform.getEulerLocalRotation())
		{
			transform.setEulerLocalRotation(rotation);
		}
		else if (scale != transform.getLocalScale())
		{
			transform.setLocalScale(scale);
		}
	}
}

void UIViewport::drawHeader()
{
	float pixelScale = Engine::getWindow().getPixelScale();
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::BeginChild("ViewportHeader", ImVec2(0, ImGui::GetFontSize() + style.FramePadding.y * 2.0f + style.WindowPadding.y * 2.0f), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
	
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;
	
	// gizmos
	if (ImGui::Button("T"))
	{
		_gizmoMode = ImGuizmo::TRANSLATE;
	}
	
	if (ImGui::Button("R"))
	{
		_gizmoMode = ImGuizmo::ROTATE;
	}
	
	if (ImGui::Button("S"))
	{
		_gizmoMode = ImGuizmo::SCALE;
	}
	
	ImGui::Dummy({20.0f * pixelScale, 0});
	
	if (ImGui::Button(_gizmoSpace == ImGuizmo::LOCAL ? "Local" : "Global"))
	{
		if (_gizmoSpace == ImGuizmo::LOCAL)
		{
			_gizmoSpace = ImGuizmo::WORLD;
		}
		else
		{
			_gizmoSpace = ImGuizmo::LOCAL;
		}
	}
	
	ImGui::Separator();
	
	ImGui::Checkbox("Fullscreen", &_fullscreen);
	
	ImGui::Separator();
	
	ImGui::SetNextItemWidth(130.0f * pixelScale);
	if (ImGui::BeginCombo("SceneRenderer", magic_enum::enum_name(_sceneRendererType).data()))
	{
		for (UIViewport::RendererType sceneRendererType : magic_enum::enum_values<UIViewport::RendererType>())
		{
			const bool is_selected = (sceneRendererType == _sceneRendererType);
			if (ImGui::Selectable(magic_enum::enum_name(sceneRendererType).data(), is_selected))
			{
				_sceneRenderer.reset();
				_sceneRendererType = sceneRendererType;
			}
			
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	
	ImGui::EndChild();
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
}

bool UIViewport::isFullscreen()
{
	return _fullscreen;
}

void UIViewport::renderToFile(glm::uvec2 resolution)
{//TODO: re-enable once shadow maps are managed by the renderer
//	std::optional<std::filesystem::path> filePath = FileHelper::fileDialogSave({
//		FileDialogFilter{
//			.fileTypeDisplayName = L"PNG Image",
//			.fileTypeExtensions = L"*.png"
//		},
//		FileDialogFilter{
//			.fileTypeDisplayName = L"JPG Image",
//			.fileTypeExtensions = L"*.jpg"
//		}
//	}, ".", "render");
//
//	if (!filePath)
//	{
//		return;
//	}
//
//	RaytracingSceneRenderer renderer(resolution);
//
//	Camera camera(_camera);
//	camera.setAspectRatio(static_cast<float>(resolution.x) / static_cast<float>(resolution.y));
//
//	renderer.onNewFrame();
//
//	Engine::getScene().onPreRender(renderer, camera);
//
//	const VKPtr<VKImageView>& textureView = renderer.render(camera);
//
//	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(
//		Engine::getVKContext(),
//		textureView->getImage()->getLevelByteSize(0),
//		vk::BufferUsageFlagBits::eTransferDst,
//		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
//
//	Engine::getVKContext().executeImmediate(
//		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
//		{
//			commandBuffer->imageMemoryBarrier(
//				textureView->getImage(),
//				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
//				vk::AccessFlagBits2::eColorAttachmentWrite,
//				vk::PipelineStageFlagBits2::eCopy,
//				vk::AccessFlagBits2::eTransferRead,
//				vk::ImageLayout::eTransferSrcOptimal,
//				0,
//				0);
//
//			commandBuffer->copyImageToBuffer(textureView->getImage(), 0, 0, stagingBuffer, 0);
//		});
//
//	glm::ivec2 textureSize = textureView->getImage()->getSize(0);
//
//	std::byte* ptr = stagingBuffer->map();
//	if (filePath->extension() == ".png")
//	{
//		stbi_write_png(filePath->generic_string().c_str(), textureSize.x, textureSize.y, 4, ptr, textureSize.x * 4);
//	}
//	else if (filePath->extension() == ".jpg")
//	{
//		stbi_write_jpg(filePath->generic_string().c_str(), textureSize.x, textureSize.y, 4, ptr, 95);
//	}
//	stagingBuffer->unmap();
}

const PerfStep* UIViewport::getPreviousFramePerfStep()
{
	return _sceneRenderer ? &_sceneRenderer->getRenderPerf() : nullptr;
}

void UIViewport::shutdown()
{
	_sceneRenderer = {};
}
