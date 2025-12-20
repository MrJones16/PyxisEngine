#include "Element.h"

namespace Pyxis {
ElementProperties &Pyxis::ElementData::GetElementProperties(uint32_t id) {
    if (id >= s_ElementData.size()) {
        return s_ElementData[0];
    }
    return s_ElementData[id];
}

ElementProperties &ElementData::GetElementProperties(const std::string &name) {
    if (s_ElementNameToID.find(name) == s_ElementNameToID.end()) {
        return s_ElementData[0];
    }
    return s_ElementData[s_ElementNameToID[name]];
}

void ElementData::LoadElementData(const std::string &path) {
    // first, lets reset any current data we have.
    s_ElementData.clear();
    s_ElementNameToID.clear();
    s_TagElements.clear();
    s_Reactions.clear();
    s_ReactionTable.clear();

    // load the json file
    std::ifstream file(path);
    json j;
    file >> j;
    file.close();
    // de-serialize element data & reactions.
    for (auto &element : j) {
        if (element["Type"] == "ElementData") {

            ElementProperties properties = element;
            s_ElementData.push_back(properties);
            s_ElementNameToID[properties.name] = s_ElementData.size() - 1;
            for (auto &tag : properties.tags) {
                s_TagElements[tag].push_back(s_ElementData.size() - 1);
            }
        } else if (element["Type"] == "Reaction") {
            Reaction reaction = element;
            s_Reactions.push_back(reaction);
        }
    }
    BuildReactionTable();
}

Element ElementData::GetElement(uint32_t id, int x, int y) {
    Element element = Element();
    if (id < s_ElementData.size()) {
        s_ElementData[id].UpdateElementProperties(element, x, y);
        element.m_ID = id;
    }
    return element;
}

Element ElementData::GetElement(const std::string &name, int x, int y) {
    Element element = Element();
    if (s_ElementNameToID.find(name) != s_ElementNameToID.end()) {
        uint32_t id = s_ElementNameToID[name];
        GetElementProperties(id).UpdateElementProperties(element, x, y);
        element.m_ID = id;
    }

    return element;
}

void ElementData::BuildReactionTable() {
    std::string input0Tag, input1Tag;
    // loop over each reaction
    for (Reaction reaction : s_Reactions) {
        // get the first string, input 0
        if (StringContainsTag(reaction.input_cell_0)) {
            // contains a tag, so keep track of what the tag is and loop
            input0Tag = TagFromString(reaction.input_cell_0);
            for (uint32_t id0 : s_TagElements[input0Tag]) {
                uint32_t idOut0;
                if (StringContainsTag(reaction.output_cell_0)) {
                    idOut0 = s_ElementNameToID[ReplaceTagInString(
                        reaction.output_cell_0, s_ElementData[id0].name)];
                } else {
                    idOut0 = s_ElementNameToID[reaction.output_cell_0];
                }
                // id0 obtained, move onto next input
                if (StringContainsTag(reaction.input_cell_1)) {
                    input1Tag = TagFromString(reaction.input_cell_1);
                    // input 1 has tag, so loop over tag elements again
                    for (uint32_t id1 : s_TagElements[input1Tag]) {
                        uint32_t idOut1;
                        // id0 and 1 obtained
                        if (StringContainsTag(reaction.output_cell_1)) {
                            idOut1 = s_ElementNameToID[ReplaceTagInString(
                                reaction.output_cell_1,
                                s_ElementData[id1].name)];
                        } else {
                            idOut1 = s_ElementNameToID[reaction.output_cell_1];
                        }
                        // all id's obtained
                        s_ReactionTable[id0][id1] = ReactionResult(
                            reaction.probablility, idOut0, idOut1);
                    }
                } else {
                    uint32_t id1 = s_ElementNameToID[reaction.input_cell_1];
                    uint32_t idOut1 = s_ElementNameToID[reaction.output_cell_1];
                    // all id's obtained
                    s_ReactionTable[id0][id1] =
                        ReactionResult(reaction.probablility, idOut0, idOut1);
                    /*m_ReactionLookup[id1][id0] =
                     * ReactionResult(reaction.probablility, idOut1, idOut0);*/
                }
            }
        } else {
            // input 0 has no tag, so move on to next input
            uint32_t id0 = s_ElementNameToID[reaction.input_cell_0];
            uint32_t idOut0 = s_ElementNameToID[reaction.output_cell_0];
            if (StringContainsTag(reaction.input_cell_1)) {
                input1Tag = TagFromString(reaction.input_cell_1);
                // input 1 has tag, so loop over tag elements again
                for (uint32_t id1 : s_TagElements[input1Tag]) {
                    // id0 and 1 obtained
                    uint32_t idOut1;
                    if (StringContainsTag(reaction.output_cell_1)) {
                        idOut1 = s_ElementNameToID[ReplaceTagInString(
                            reaction.output_cell_1, s_ElementData[id1].name)];
                    } else {
                        idOut1 = s_ElementNameToID[reaction.output_cell_1];
                    }
                    // all id's obtained
                    s_ReactionTable[id0][id1] =
                        ReactionResult(reaction.probablility, idOut0, idOut1);
                }
            } else {
                uint32_t id1 = s_ElementNameToID[reaction.input_cell_1];
                uint32_t idOut1 = s_ElementNameToID[reaction.output_cell_1];
                // all id's obtained
                s_ReactionTable[id0][id1] =
                    ReactionResult(reaction.probablility, idOut0, idOut1);
            }
        }
    }
}

bool ElementData::StringContainsTag(const std::string &string) {
    if (string.find("[") != std::string::npos &&
        string.find("]") != std::string::npos)
        return true;
    return false;
}

std::string ElementData::TagFromString(const std::string &stringWithTag) {
    int start = stringWithTag.find("[");
    int end = stringWithTag.find("]");
    return stringWithTag.substr(start + 1, (end - start) - 1);
}

std::string ElementData::ReplaceTagInString(const std::string &stringToFill,
                                            const std::string &name) {
    int start = stringToFill.find("[");
    int end = stringToFill.find("]");
    std::string result = stringToFill.substr(0, start) + name;
    if (end == stringToFill.size() - 1)
        return result;
    return result +
           stringToFill.substr(end + 1, (stringToFill.size() - end) - 1);
}

void ElementProperties::SetTexture(std::string tex_path) {
    texture_path = tex_path;
    auto texture = Texture2D::Create(tex_path);
    int width = texture->GetWidth();
    int height = texture->GetHeight();
    texture_data = texture->GetData();
}

Element ElementProperties::GetElement(int x, int y) const {
    Element element;
    UpdateElementProperties(element, x, y);
    return element;
}

void ElementProperties::UpdateElementProperties(Element &element, int x,
                                                int y) const {
    if (texture_data != nullptr) {
        glm::ivec2 result;
        if (x < 0) {
            result.x = width - (std::abs(x) % width);
            result.x = result.x % width;
        } else
            result.x = x % width;
        if (y < 0) {
            result.y = height - (std::abs(y) % height);
            result.y = result.y % height;
        } else
            result.y = y % height;
        int index = (result.x * 4) + (result.y * width * 4);

        uint8_t r = texture_data[index];
        uint8_t g = texture_data[index + 1];
        uint8_t b = texture_data[index + 2];
        uint8_t a = texture_data[index + 3];

        element.m_BaseColor = (a << 24) | (b << 16) | (g << 8) | r;
    } else {
        element.m_BaseColor = color; // RandomizeABGRColor(color);
    }
    element.m_Color = element.m_BaseColor;
    element.m_Temperature = temperature;
    element.m_Ignited = ignited;
    element.m_Health = health;
}

void ElementProperties::UpdateElementProperties(Element *element, int x,
                                                int y) const {
    if (texture_data != nullptr) {
        glm::ivec2 result;
        if (x < 0) {
            result.x = width - (std::abs(x) % width);
            result.x = result.x % width;
        } else
            result.x = x % width;
        if (y < 0) {
            result.y = height - (std::abs(y) % height);
            result.y = result.y % height;
        } else
            result.y = y % height;

        int index = (result.x * 4) + (y * width * 4);

        uint8_t r = texture_data[index];
        uint8_t g = texture_data[index + 1];
        uint8_t b = texture_data[index + 2];
        uint8_t a = texture_data[index + 3];

        element->m_BaseColor = (a << 24) | (b << 16) | (g << 8) | r;
    } else {
        element->m_BaseColor = color; // RandomizeABGRColor(color);
    }
    element->m_Color = element->m_BaseColor;
    element->m_Temperature = temperature;
    element->m_Ignited = ignited;
    element->m_Health = health;
}

void to_json(json &j, const ElementProperties &e) {
    // Only adding the variables that are not default
    ElementProperties defaultProperties;
    j["Type"] = "ElementData";
    j["Name"] = e.name;
    if (e.texture_path != defaultProperties.texture_path)
        j["TexturePath"] = e.texture_path;
    switch (e.cell_type) {
    case ElementType::solid:
        j["CellType"] = "solid";
        break;
    case ElementType::movableSolid:
        j["CellType"] = "movableSolid";
        break;
    case ElementType::liquid:
        j["CellType"] = "liquid";
        break;
    case ElementType::gas:
        j["CellType"] = "gas";
        break;
    case ElementType::fire:
        j["CellType"] = "fire";
        break;
    default:
        break;
    }

    j["Tags"] = e.tags;
    if (e.color != defaultProperties.color)
        j["Color"] = e.color;
    if (e.density != defaultProperties.density)
        j["Density"] = e.density;
    if (e.friction != defaultProperties.friction)
        j["Friction"] = e.friction;
    if (e.health != defaultProperties.health)
        j["Health"] = e.health;
    if (e.flammable != defaultProperties.flammable)
        j["Flammable"] = e.flammable;
    if (e.ignited != defaultProperties.ignited)
        j["Ignited"] = e.ignited;
    if (e.spread_ignition != defaultProperties.spread_ignition)
        j["SpreadIgnition"] = e.spread_ignition;
    if (e.spread_ignition_chance != defaultProperties.spread_ignition_chance)
        j["SpreadIgnitionChance"] = e.spread_ignition_chance;
    if (e.ignited_color != defaultProperties.ignited_color)
        j["IgnitedColor"] = e.ignited_color;
    if (e.ignition_temperature != defaultProperties.ignition_temperature)
        j["IgnitionTemperature"] = e.ignition_temperature;
    if (e.fire_temperature != defaultProperties.fire_temperature)
        j["FireTemperature"] = e.fire_temperature;
    if (e.fire_color != defaultProperties.fire_color)
        j["FireColor"] = e.fire_color;
    if (e.fire_temperature_increase !=
        defaultProperties.fire_temperature_increase)
        j["FireTemperatureIncrease"] = e.fire_temperature_increase;
    if (e.burnt != defaultProperties.burnt)
        j["Burnt"] = e.burnt;
    if (e.glow != defaultProperties.glow)
        j["Glow"] = e.glow;
    if (e.conductivity != defaultProperties.conductivity)
        j["Conductivity"] = e.conductivity;
    if (e.temperature != defaultProperties.temperature)
        j["Temperature"] = e.temperature;
    if (e.melted != defaultProperties.melted)
        j["Melted"] = e.melted;
    if (e.melting_point != defaultProperties.melting_point)
        j["MeltingPoint"] = e.melting_point;
    if (e.frozen != defaultProperties.frozen)
        j["Frozen"] = e.frozen;
    if (e.freezing_point != defaultProperties.freezing_point)
        j["FreezingPoint"] = e.freezing_point;
}

void from_json(const json &j, ElementProperties &e) {
    j.at("Name").get_to(e.name);
    if (j.contains("TexturePath"))
        j.at("TexturePath").get_to(e.texture_path);
    if (e.texture_path != "") {
        auto texture = Texture2D::Create(e.texture_path);
        e.width = texture->GetWidth();
        e.height = texture->GetHeight();
        e.texture_data = texture->GetData();
    }
    std::string cell_type = "solid";
    if (j.contains("CellType"))
        j.at("CellType").get_to(cell_type);
    if (cell_type == "solid")
        e.cell_type = ElementType::solid;
    else if (cell_type == "movableSolid")
        e.cell_type = ElementType::movableSolid;
    else if (cell_type == "liquid")
        e.cell_type = ElementType::liquid;
    else if (cell_type == "gas")
        e.cell_type = ElementType::gas;
    else if (cell_type == "fire")
        e.cell_type = ElementType::fire;

    if (j.contains("Tags"))
        j.at("Tags").get_to(e.tags);

    if (j.contains("Color"))
        j.at("Color").get_to(e.color);
    if (j.contains("Density"))
        j.at("Density").get_to(e.density);
    if (j.contains("Friction"))
        j.at("Friction").get_to(e.friction);
    if (j.contains("Health"))
        j.at("Health").get_to(e.health);
    if (j.contains("Flammable"))
        j.at("Flammable").get_to(e.flammable);
    if (j.contains("Ignited"))
        j.at("Ignited").get_to(e.ignited);
    if (j.contains("SpreadIgnition"))
        j.at("SpreadIgnition").get_to(e.spread_ignition);
    if (j.contains("SpreadIgnitionChance"))
        j.at("SpreadIgnitionChance").get_to(e.spread_ignition_chance);
    if (j.contains("IgnitedColor"))
        j.at("IgnitedColor").get_to(e.ignited_color);
    if (j.contains("IgnitionTemperature"))
        j.at("IgnitionTemperature").get_to(e.ignition_temperature);
    if (j.contains("FireTemperature"))
        j.at("FireTemperature").get_to(e.fire_temperature);
    if (j.contains("FireColor"))
        j.at("FireColor").get_to(e.fire_color);
    if (j.contains("FireTemperatureIncrease"))
        j.at("FireTemperatureIncrease").get_to(e.fire_temperature_increase);
    if (j.contains("Burnt"))
        j.at("Burnt").get_to(e.burnt);
    if (j.contains("Glow"))
        j.at("Glow").get_to(e.glow);
    if (j.contains("Conductivity"))
        j.at("Conductivity").get_to(e.conductivity);
    if (j.contains("Temperature"))
        j.at("Temperature").get_to(e.temperature);
    if (j.contains("Melted"))
        j.at("Melted").get_to(e.melted);
    if (j.contains("MeltingPoint"))
        j.at("MeltingPoint").get_to(e.melting_point);
    if (j.contains("Frozen"))
        j.at("Frozen").get_to(e.frozen);
    if (j.contains("FreezingPoint"))
        j.at("FreezingPoint").get_to(e.freezing_point);
}

} // namespace Pyxis
