#include <Pyxis/UI/UILayer.h>

#define CLAY_IMPLEMENTATION
#include <Pyxis/UI/clay.h>

namespace Pyxis {
UILayer::UILayer() {}
UILayer::~UILayer() {}

void UILayer::OnAttach() {
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(
        totalMemorySize, malloc(totalMemorySize));
}
void UILayer::OnDetach() {}
void UILayer::OnUIUpdate() {}
void UILayer::OnEvent(Event &event) {}

void UILayer::BeginFrame(glm::vec2 screenSize, glm::vec2 mousePosition,
                         bool lmb, glm::vec2 mouseWheel, float deltaTime) {
    // Optional: Update internal layout dimensions to support resizing
    Clay_SetLayoutDimensions((Clay_Dimensions){screenSize.x, screenSize.y});
    // Optional: Update internal pointer position for handling mouseover / click
    // / touch events - needed for scrolling & debug tools
    Clay_SetPointerState((Clay_Vector2){mousePosition.x, mousePosition.y}, lmb);
    // Optional: Update internal pointer position for handling mouseover / click
    // / touch events - needed for scrolling and debug tools
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){mouseWheel.x, mouseWheel.y}, deltaTime);

    // All clay layouts are declared between Clay_BeginLayout and Clay_EndLayout
    Clay_BeginLayout();
}
void UILayer::EndFrame() {
    // All clay layouts are declared between Clay_BeginLayout and Clay_EndLayout
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    // directory
    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand *renderCommand = &renderCommands.internalArray[i];

        switch (renderCommand->commandType) {
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            // DrawRectangle(renderCommand->boundingBox,
            //               renderCommand->renderData.rectangle.backgroundColor);
        }
            // ... Implement handling of other command types
        case CLAY_RENDER_COMMAND_TYPE_BORDER:
        case CLAY_RENDER_COMMAND_TYPE_TEXT:
        case CLAY_RENDER_COMMAND_TYPE_IMAGE:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
        case CLAY_RENDER_COMMAND_TYPE_NONE:
            break;
        }
    }
}

} // namespace Pyxis
