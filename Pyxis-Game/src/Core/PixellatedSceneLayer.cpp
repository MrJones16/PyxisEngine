#include "PixellatedSceneLayer.h"
#include <Pyxis/Core/Application.h>

#include <Pyxis/Renderer/RenderCommand.h>
#include <Pyxis/Renderer/Renderer2D.h>

#include <Pyxis/Core/Input.h>
#include <memory>

namespace Pyxis {

PixellatedSceneLayer::PixellatedSceneLayer(bool debug) : m_Debug(debug) {}

PixellatedSceneLayer::~PixellatedSceneLayer() {}

void PixellatedSceneLayer::OnAttach() {
    m_ViewportSize = {Application::Get().GetWindow().GetWidth(),
                      Application::Get().GetWindow().GetHeight()};
    Renderer2D::Init();

    FrameBufferSpecification deferredGBufferSpec;
    deferredGBufferSpec.Attachments = {
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // position 0
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // normal 1
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // albedo 2
        {FrameBufferTextureFormat::R32UI,
         FrameBufferTextureType::Color}, // node id 3
        {FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}};
    deferredGBufferSpec.Width =
        m_RenderResolution.x + (m_RenderResolutionPadding * 2);
    deferredGBufferSpec.Height =
        m_RenderResolution.y + (m_RenderResolutionPadding * 2);
    m_DeferredGBuffer = FrameBuffer::Create(deferredGBufferSpec);

    FrameBufferSpecification lightingPassBufferSpec;
    lightingPassBufferSpec.Attachments = {
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // Color 0
        {FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}};
    lightingPassBufferSpec.Width =
        m_RenderResolution.x + (m_RenderResolutionPadding * 2);
    lightingPassBufferSpec.Height =
        m_RenderResolution.y + (m_RenderResolutionPadding * 2);
    m_DeferredLightingBuffer = FrameBuffer::Create(lightingPassBufferSpec);
}

void PixellatedSceneLayer::OnDetach() {}

void PixellatedSceneLayer::OnUpdate(Timestep ts) {
    PROFILE_SCOPE("PixellatedSceneLayer OnUpdate");

    // clear dead nodes
    while (Node::NodesToDestroyQueue.size() > 0) {
        Ref<Node> node = Node::Nodes[Node::NodesToDestroyQueue.front()];

        if (node != nullptr) {
            // clear parent for children
            for (auto child : node->m_Children) {
                child->m_Parent = nullptr;
            }
            // clear parent's child of this
            if (node->m_Parent != nullptr) {
                node->m_Parent->RemoveChild(node->shared_from_this());
            }
        }
        Node::Nodes.erase(Node::NodesToDestroyQueue.front());
        Node::NodesToDestroyQueue.pop();
    }

// rendering
#if STATISTICS
    Renderer2D::ResetStats();
#endif

    {
        PROFILE_SCOPE("Renderer Prep");
        // Checking for camera and updating buffers if necessary.
        PixelCameraNode *camera =
            dynamic_cast<PixelCameraNode *>(Camera::Main());
        if (!camera) {
            // Camera is null, or not a pixel camera!
            //
            PX_CORE_ASSERT(false, "There is no main camera!");
        } else {
            // Camera is a pixel camera. We need to check if the size/padding
            // has changed so that we can adjust the framebuffers
            if (m_RenderResolution != camera->GetSize() ||
                m_RenderResolutionPadding !=
                    camera->GetRenderResolutionPadding()) {
                m_RenderResolution = camera->GetSize();

                // Camera size changed. Resize the buffers!
                PX_CORE_TRACE(
                    "Camera size changed, resizing buffers to "
                    "{0}, {1}",
                    m_RenderResolution.x + (2 * m_RenderResolutionPadding),
                    m_RenderResolution.y + (2 * m_RenderResolutionPadding));
                m_DeferredGBuffer->Resize(
                    m_RenderResolution.x + (m_RenderResolutionPadding * 2),
                    m_RenderResolution.y + (m_RenderResolutionPadding * 2));
                m_DeferredLightingBuffer->Resize(
                    m_RenderResolution.x + (m_RenderResolutionPadding * 2),
                    m_RenderResolution.y + (m_RenderResolutionPadding * 2));
            }
        }

        m_PixelCamera = camera;
        RenderCommand::SetClearColor({0, 0, 0, 0});

        m_PixelCamera->RecalculateViewMatrix();

        Renderer2D::BeginScene(m_PixelCamera, m_DeferredGBuffer,
                               m_DeferredLightingBuffer);
    }

    // only run per tick rate
    auto time = std::chrono::high_resolution_clock::now();
    if (m_FixedUpdateRate > 0 &&
        std::chrono::time_point_cast<std::chrono::microseconds>(time)
                    .time_since_epoch()
                    .count() -
                std::chrono::time_point_cast<std::chrono::microseconds>(
                    m_FixedUpdateTime)
                    .time_since_epoch()
                    .count() >=
            (1.0f / m_FixedUpdateRate) * 1000000.0f) {
        PROFILE_SCOPE("Fixed Update");

        m_FixedUpdateTime = time;

        for (auto node : Node::Nodes) {
            if (node.second != nullptr) {
                node.second->OnUpdate(ts);
                node.second->OnFixedUpdate();
                node.second->OnRender();
            } else {
                // node was null so add it to list to delete
                m_NullNodeQueue.push(node.first);
            }
        }

    } else {
        for (auto node : Node::Nodes) {
            if (node.second != nullptr) {
                node.second->OnUpdate(ts);
                node.second->OnRender();
            } else {
                // node was null so add it to list to delete
                m_NullNodeQueue.push(node.first);
            }
        }
    }

    // remove null nodes from map
    while (!m_NullNodeQueue.empty()) {
        Node::Nodes.erase(m_NullNodeQueue.front());
        m_NullNodeQueue.pop();
    }

    /*auto[x,y] = Input::GetMousePosition();
    glm::vec2 worldPos = m_CameraNode->MouseToWorldPos({x, y});
    PX_TRACE("Mouse Pos: ({0},{1})", x, y);
    PX_TRACE("World Pos: ({0},{1})", worldPos.x, worldPos.y);

    Renderer2D::DrawText("Hello!", glm::mat4(1),
    ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"));*/

    Renderer2D::EndScene(); // finalizes rendering of normal objects.

    // test drawing lights.

    Renderer2D::DrawLight({-10, -10}, {0.5, 0.5, 0.5}, 2, 2000);

    Renderer2D::DrawDeferredLightingPass();

    // we need to get the scale and offset to render the output to, as the
    // render resolution and display are separate.

    float ScaleToFillDisplay =
        m_ViewportSize.x /
        m_RenderResolution.x; // maybe we should take the min with height? so it
                              // guarantees full screen coverage?
    glm::vec2 bufferPadding = glm::vec2(2, 2) * m_RenderResolutionPadding;
    glm::vec2 outputSize = ((m_RenderResolution + bufferPadding) *
                            ScaleToFillDisplay); // 642 * 4 = 2568

    glm::vec2 outputScale = outputSize / m_ViewportSize;
    // went from scaled render res to output, in amount to scale. a 640 at 1x
    // would be 0.25 of the screen space for a 2560 monitor

    glm::vec2 offset = m_PixelCamera->GetOffsetToGrid();
    offset /=
        (m_PixelCamera->GetSize() +
         bufferPadding);   // 0/642 - 1/642 ... how much of a pixel we shifted
    offset *= outputScale; // offset depends on scale too

    // grab the ID from the Deferred "G" buffer.
    //
    // Mouse position is weird in this setup, as the output / buffers/ game are
    // all kinda doing their own thing. I have to find the mouse position from
    // 0-output, which 0 may start offset from the bottom left corner! check
    // RenderingThoughts.png for an analysis, where x is that offset.
    glm::vec2 mp = Input::GetMousePosition();
    // flip the y so bottom left is 0,0
    mp.y = m_ViewportSize.y - mp.y;

    glm::vec2 offsetForMouse = (m_ViewportSize - outputSize) / 2.0f;
    mp -= offsetForMouse;
    mp /=
        ScaleToFillDisplay; // need to divide from that int scale to get back to
    // original 642/362

    m_DeferredGBuffer->Bind();
    if (mp.x >= 0 && mp.x < (outputSize.x / ScaleToFillDisplay) && mp.y >= 0 &&
        mp.y < (outputSize.y / ScaleToFillDisplay)) {
        UUID nodeID;
        m_DeferredGBuffer->ReadPixel(3, mp.x, mp.y, &nodeID);
        Node::s_HoveredNodeID = nodeID;
        // PX_CORE_TRACE("Mouse Position: ({0},{1})", mp.x, mp.y);
        // PX_CORE_TRACE("Read Pixel: {0}", (uint32_t)nodeID);
    }
    m_DeferredGBuffer->Unbind();

    Renderer2D::DrawScreenQuad(
        m_DeferredLightingBuffer->GetColorAttachmentRendererID(0), outputScale,
        offset * 2.0f); // offset * 2.0f - glm::vec2(0, 0)
}

void PixellatedSceneLayer::DrawNodeTree(Ref<Node> Node) {
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ((Node == m_SelectedNode) ? ImGuiTreeNodeFlags_Selected : 0);

    bool opened = ImGui::TreeNodeEx((void *)(UUID)Node->GetUUID(), flags,
                                    Node->m_Name.c_str());

    if (ImGui::IsItemClicked()) {
        m_SelectedNode = Node;
    }

    if (opened) {
        // get child entities displayed as well
        for (auto &node : Node->m_Children) {
            DrawNodeTree(node->shared_from_this());
        }
        ImGui::TreePop();
    }
}

void PixellatedSceneLayer::OnImGuiRender() {
    // check scenelayer.cpp for actual setup
}

void PixellatedSceneLayer::OnEvent(Event &e) {

    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(
        PX_BIND_EVENT_FN(PixellatedSceneLayer::OnWindowResizeEvent));
    dispatcher.Dispatch<KeyPressedEvent>(
        PX_BIND_EVENT_FN(PixellatedSceneLayer::OnKeyPressedEvent));
    dispatcher.Dispatch<MouseButtonPressedEvent>(
        PX_BIND_EVENT_FN(PixellatedSceneLayer::OnMouseButtonPressedEvent));
    dispatcher.Dispatch<MouseButtonReleasedEvent>(
        PX_BIND_EVENT_FN(PixellatedSceneLayer::OnMouseButtonReleasedEvent));
    dispatcher.Dispatch<MouseScrolledEvent>(
        PX_BIND_EVENT_FN(PixellatedSceneLayer::OnMouseScrolledEvent));
}

glm::ivec2 PixellatedSceneLayer::GetMousePositionImGui() {
    /// if not using a framebuffer / imgui image, just use
    /// Pyxis::Input::GetMousePosition();
    // TODO: Set up ifdef for using imgui? or just stop using imgui... lol
    auto [mx, my] = ImGui::GetMousePos();

    mx -= m_ViewportBounds[0].x;
    my -= m_ViewportBounds[0].y;

    return {(int)mx, (int)my};
}

bool PixellatedSceneLayer::OnWindowResizeEvent(WindowResizeEvent &event) {
    m_ViewportSize = {event.GetWidth(), event.GetHeight()};
    if (m_PixelCamera != nullptr) {
        glm::vec2 dynamicScale = m_ViewportSize / 480.0f;
        float dynamicScaleMin = std::min(dynamicScale.x, dynamicScale.y);
        dynamicScaleMin = (int)(dynamicScaleMin + 1);
        glm::vec2 newRenderResolution = m_ViewportSize / dynamicScaleMin;
        glm::ivec2 intRes = glm::floor(newRenderResolution);
        if (intRes.x % 2 != 0)
            intRes.x++;
        if (intRes.y % 2 != 0)
            intRes.y++;
        m_PixelCamera->SetWidth(intRes.x);
        m_PixelCamera->SetHeight(intRes.y);
    }

    // m_OrthographicCameraController.SetAspect(m_ViewportSize.y /
    // m_ViewportSize.x);
    return false;
}

bool PixellatedSceneLayer::OnKeyPressedEvent(KeyPressedEvent &event) {
    // Serialize Scene
    if (event.GetKeyCode() == PX_KEY_O) {
        json j;
        auto SceneNode = CreateRef<Node>("Scene");
        SceneNode->Serialize(j);

        for (auto &[id, ref] : Node::Nodes) {
            if (ref->m_Parent == nullptr) {
                json c;
                ref->Serialize(c);
                j["m_Children"] += c;
            }
        }
        PX_TRACE("Here is the serialized Scene: ");
        std::cout << j.dump(4) << std::endl;
    }
    return false;
}

bool PixellatedSceneLayer::OnMouseButtonPressedEvent(
    MouseButtonPressedEvent &event) {
    // let the UI keep track of what has been pressed, so that way buttons can
    // be on release!
    UI::UINode::s_MousePressedNodeID = Node::s_HoveredNodeID;
    PX_TRACE("Pressed Node: {0}", UI::UINode::s_MousePressedNodeID);

    // see if the node is valid, and if it is then send the event through.
    if (Node::Nodes.contains(Node::s_HoveredNodeID)) {
        // the hovered node is valid

        // try to cast it to a UI node, and call onMousePressed
        if (Ref<UI::UINode> uinode = dynamic_pointer_cast<UI::UINode>(
                Node::Nodes[Node::s_HoveredNodeID])) {
            uinode->OnMousePressed(event.GetMouseButton());
        }
    }
    return false;
}

/// <summary>
/// checks to see if we released on the same object we pressed, and if so calls
/// OnMouseReleased with continuous as true. otherwise, will call
/// onmousereleased on both the previously selected object, and the hovered
/// object.
/// </summary>
/// <param name="event"></param>
/// <returns></returns>
bool PixellatedSceneLayer::OnMouseButtonReleasedEvent(
    MouseButtonReleasedEvent &event) {
    PX_TRACE("Released when UINode is: {0}", UI::UINode::s_MousePressedNodeID);
    if (UI::UINode::s_MousePressedNodeID == Node::s_HoveredNodeID) {
        // the hovered node is what we pressed last
        if (Node::Nodes.contains(Node::s_HoveredNodeID)) {
            // the hovered node is valid

            // try to cast it to UI
            if (Ref<UI::UINode> uinode = dynamic_pointer_cast<UI::UINode>(
                    Node::Nodes[Node::s_HoveredNodeID])) {
                uinode->OnMouseReleased(event.GetMouseButton(), true);
            }
        }
    } else {
        PX_TRACE("Searching for that ID");
        // the hovered node is not what we pressed originally, so call released
        // on original press
        if (Node::Nodes.contains(UI::UINode::s_MousePressedNodeID)) {
            PX_TRACE("Found valid object. Attempting cast to UINode");
            // the hovered node is valid

            // try to cast it to UI
            if (Ref<UI::UINode> uinode = dynamic_pointer_cast<UI::UINode>(
                    Node::Nodes[UI::UINode::s_MousePressedNodeID])) {
                PX_TRACE("Cast success, calling mose released");
                uinode->OnMouseReleased(event.GetMouseButton(), false);
            }
        } else {
            PX_TRACE("Could not find object.");
        }

        /// here if we want to call released on UI nodes without having to press
        /// first?
        // if (Node::Nodes.contains(Node::s_HoveredNodeID))
        //{
        //	//the hovered node is valid

        //	//try to cast it to UI
        //	if (UI::UINode* uinode =
        // dynamic_cast<UI::UINode*>(Node::Nodes[Node::s_HoveredNodeID]))
        //	{
        //		uinode->OnMouseReleased(event.GetMouseButton(), false);
        //	}
        //}
    }

    return false;
}

bool PixellatedSceneLayer::OnMouseScrolledEvent(MouseScrolledEvent &event) {
    return false;
}

} // namespace Pyxis
