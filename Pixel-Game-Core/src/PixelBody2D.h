#pragma once

#include <Pyxis/Nodes/RigidBody2D.h>
#include "Element.h"
#include "VectorHash.h"
#include "World.h"


/// A pixel body is a rigidbody2D , aka a B2Body. 
/// 
/// The underlying b2body position is different than the world position,
/// because control over the PPU is helpful for deciding how many pixels
/// should be considered a 1x1 cube, because box2D is optimized for 1-50
/// i believe.
///


namespace Pyxis
{
	/// <summary>
	/// this is based on how many pixels represent 1 unit.
	/// if an object had a velocity of -10, it would fall 10 pixels at
	/// a PPU of 1, whereas it would fall 100 pixels at a PPU of 10;
	/// 
	/// basically, what matters is this is the value of an average
	/// width / size pixel body. like a 1x1 box is this wide/tall
	/// 
	/// I have it at 16 cause a 16x16 box is a box to me!
	/// </summary>
	static const float PPU = 16.0f; // pixels per unit for box2d sim

	struct PixelBodyElement
	{
		PixelBodyElement() {}
		PixelBodyElement(Element e)
			: element(e)
		{}
		PixelBodyElement(Element e, glm::ivec2 worldPosition)
			: element(e), worldPos(worldPosition)
		{}
		Element element = Element();
		bool hidden = false;
		glm::ivec2 worldPos = { 0,0 };

		//Write to_json and from_json functions
		friend void to_json(json& j, const PixelBodyElement& pbe)
		{
			j = json{ {"Element", pbe.element}, {"Hidden", pbe.hidden}, {"WorldPos", pbe.worldPos} };
		}
		friend void from_json(const json& j, PixelBodyElement& pbe)
		{
			j.at("Element").get_to(pbe.element);
			j.at("Hidden").get_to(pbe.hidden);
			j.at("WorldPos").get_to(pbe.worldPos);
		}
	};

	/*struct PixelBodyData
	{
		b2Vec2 linearVelocity;
		float angularVelocity;
		float angle;
		b2Vec2 position;
	};*/

	/// <summary>
	/// A rigid body, but it is tied to elements in the simulation.
	/// 
	/// When deserializing, you must set the m_PXWorld afterwards.
	/// </summary>
	class PixelBody2D : public RigidBody2D
	{
	protected:
		bool m_InWorld = true;

		float m_Width = 0;
		float m_Height = 0;

		/// <summary>
		/// m_Elements holds the LOCAL positions about the center.
		/// </summary>
		std::unordered_map<glm::ivec2, PixelBodyElement, HashVector> m_Elements;
		bool m_RecreateBody = false;

		bool m_DebugDisplay = false;

	public:
		float WorldToB2 = static_cast<float>(CHUNKSIZE) / PPU;
		float B2ToWorld = PPU / static_cast<float>(CHUNKSIZE);


		World* m_PXWorld = nullptr;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="type"></param>
		/// <param name="world"></param>
		/// <param name="elements"> elements should contain the element, and their pixel position in the world</param>
		PixelBody2D(const std::string& name, b2BodyType type, World* world, std::vector<PixelBodyElement>& elements, bool CreatedFromSplit = false);

		PixelBody2D(UUID id);

		//Serialize & Deserialize
		virtual void Serialize(json& j) override;
		virtual void Deserialize(json& j) override;

		/// <summary>
		/// Update called by Physics2D steps
		/// </summary>
		void OnPhysicsUpdate() override;

		void OnInspectorRender() override;

		void OnRender() override;
		

		/// <summary>
		/// places our map of elements into the world using our transform
		/// </summary>
		virtual void EnterWorld();


		/// <summary>
		/// Takes the elements in our pixel body out of the simulation
		/// 
		/// This will re-create the b2body shapes & create new pixel bodies if split!
		/// </summary>
		virtual void ExitWorld();


		/// <summary>
		/// Uses the transform of the rigidbody2D to calculate the new world
		/// positions for the elements in m_Elements
		/// </summary>
		void UpdateElementPositions();

		

		/// <summary>
		/// takes the current map of elements of the pixel body, and creates the b2body 
		/// fixtures and such based on them
		/// </summary>
		void GeneratePixelBody(bool SkipCalculations = false);

		
		////////////////////////////////////////////////
		/// Overrides for transforms, since
		/// Pixel bodies display in world, 
		/// so the PPU and chunksize are not aligned!
		////////////////////////////////////////////////

		virtual void TransferWorld(b2World* world) override;

		virtual glm::mat4 GetWorldTransform() override;

		virtual void Translate(const glm::vec2& translation) override;
		virtual void SetPosition(const glm::vec2& position) override;
		virtual glm::vec2 GetPosition();

		glm::vec2 GetB2Position();

		virtual void Rotate(const float radians) override;
		virtual void SetRotation(const float radians) override;
		virtual float GetRotation();

	private:


		/// <summary>
		/// takes a set of points, and uses QueuePull to separate it into different contiguous sections
		/// </summary>
		/// <param name="source"></param>
		/// <returns></returns>
		std::vector<std::unordered_set<glm::ivec2, HashVector>> GetContiguousAreas(std::unordered_set<glm::ivec2, HashVector> source);

		/// <summary>
		/// Volatile extraction of a contiguous region from the bitmap, put into the result, starting from a position, using a seed fill approach
		/// </summary>
		/// <param name="startPos">a starting position to make the first region from, otherwise will grab first element in bitmap</param>
		/// <param name="result">the vector you supply to fill with a contiguous region</param>
		/// <param name="bitmap">the source that the region will be extracted from, volatile!</param>
		void QueuePull(glm::ivec2 startPos, std::unordered_set<glm::ivec2, HashVector>& result, std::unordered_set<glm::ivec2, HashVector>& source);

		/// <summary>
		/// gathers the COUNTERCLOCKWISE outline points of the rigid body, and 
		/// returns a vector of size 0 if there was a failure
		/// 
		/// currently tries to grab as many diagonals as possible, but could be better to switch the two special cases to be inverted
		/// to ignore diagonals if possible
		/// </summary>
		/// <returns></returns>
		std::vector<p2t::Point> GetContourPoints();

		/// <summary>
		/// Gets the case for a marching squares algorithm, used by GetContourPoints;
		/// </summary>
		/// <param name="localPosition"></param>
		/// <returns></returns>
		int GetMarchingSquareCase(glm::ivec2 localPosition);

		/// <summary>
		/// Uses a Douglass-Peucker line simplification to simplify points of the pixelbody perimeter to create a collision shape
		/// 
		/// </summary>
		/// <param name="contourVector"></param>
		/// <param name="startIndex"></param>
		/// <param name="endIndex"></param>
		/// <param name="threshold"></param>
		/// <returns></returns>
		std::vector<p2t::Point> SimplifyPoints(const std::vector<p2t::Point>& contourVector, int startIndex, int endIndex, float threshold);

	};

	REGISTER_SERIALIZABLE_NODE(PixelBody2D);

}
