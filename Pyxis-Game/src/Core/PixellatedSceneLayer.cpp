#include "PixellatedSceneLayer.h"
#include <Pyxis/Core/Application.h>

#include "Pyxis/Renderer/PixelRenderer2D.h"
#include <Pyxis/Renderer/RenderCommand.h>

#include <memory>

namespace Pyxis {

PixellatedSceneLayer::PixellatedSceneLayer(bool debug) : m_Debug(debug) {}

PixellatedSceneLayer::~PixellatedSceneLayer() { PixelRenderer2D::Shutdown(); }

void PixellatedSceneLayer::OnAttach() { PixelRenderer2D::Init(); }

void PixellatedSceneLayer::OnDetach() { PixelRenderer2D::Shutdown(); }

void PixellatedSceneLayer::OnUpdate(Timestep ts) {
    PROFILE_SCOPE("PixellatedSceneLayer OnUpdate");

    //////////////////////////////////
    ///  Simulation Pass
    //////////////////////////////////
    {
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
            m_PixelCamera = dynamic_cast<PixelCameraNode *>(Camera::Main());
            PixelRenderer2D::BeginFrame(m_PixelCamera);

            PixelRenderer2D::BeginSimulationPass();
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

        // welp this is borked atm
        Renderer2D::DrawPointLight({10, -10}, {1, 0, 1}, 1, 256);
        Renderer2D::DrawPointLight({10, 10}, {0, 1, 0}, 1, 128);

        PixelRenderer2D::EndSimulationPass();
    }

    //////////////////////////////////
    ///  Parallax Pass
    //////////////////////////////////
    /// Drawing multiple layers together
    ///
    /// In Buffer: List of "Parallax Layers" with their depth, distance, and
    /// texture Out Buffer: Framebuffer with all layers added together.
    /// Resolution: Pixel Resolution -> Display Resolution
    {
        PixelRenderer2D::BeginParallaxPass();
        PixelRenderer2D::DrawParallaxLayer(
            0, 0, PixelRenderer2D::s_RenderData.DeferredLightingBuffer);
        PixelRenderer2D::EndParallaxPass();
    }

    //////////////////////////////////
    ///  Post-Effects Pass
    //////////////////////////////////
    /// Bloom, Blur, Distortion, Etc.
    ///
    /// In Buffer: Compiled Parallax Buffer
    /// Out Buffer: Straight to output buffer
    /// Resolution: Pixel Resolution
    {
        PixelRenderer2D::BeginPostEffectsPass();
        PixelRenderer2D::EndPostEffectsPass();
    }

    //////////////////////////////////
    ///  HUD Pass
    //////////////////////////////////
    /// Drawing HUD & UI to screen
    ///
    /// In Buffer: None
    /// Out Buffer: Output Buffer
    /// Resolution: Display Resolution
    {
        PixelRenderer2D::BeginHUDPass();

        PixelRenderer2D::EndHUDPass();
    }
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

bool PixellatedSceneLayer::OnWindowResizeEvent(WindowResizeEvent &event) {
    glm::vec2 viewportSize = {event.GetWidth(), event.GetHeight()};
    if (m_PixelCamera != nullptr) {
        glm::vec2 dynamicScale = viewportSize / 480.0f;
        float dynamicScaleMin = std::min(dynamicScale.x, dynamicScale.y);
        dynamicScaleMin = (int)(dynamicScaleMin + 1);
        glm::vec2 newRenderResolution = viewportSize / dynamicScaleMin;
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
