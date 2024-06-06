#pragma once

#include "Panel.h"
#include "Pyxis/Core/Core.h"
#include <vector>

namespace Pyxis
{
	struct ProfileResult
	{
		const char* Name;
		float Time;
	};

	class ProfilingPanel : public Panel
	{
	public:

		

		inline ProfilingPanel()
		{

		}
		~ProfilingPanel() = default;

		inline virtual void OnImGuiRender() override
		{
			#if PX_PROFILING
			const int AverageAmount = 100;
			ImGui::Begin("Profiler");

			if (m_ProfileAverageValue.empty())
			{
				for (auto& result : m_ProfileResults)
				{
					m_ProfileAverageValue.push_back(result.Time);
				}
			}
			auto avgVal = m_ProfileAverageValue.begin();
			//update the average with current time
			for (auto& result : m_ProfileResults)
			{
				*avgVal = *avgVal + result.Time;
				avgVal++;
			}
			//check for every 100th update, and update storage
			if (m_ProfileAverageCount >= AverageAmount)
			{
				m_ProfileAverageValueStorage.clear();
				avgVal = m_ProfileAverageValue.begin();
				for (auto& result : m_ProfileResults)
				{
					//set the storage
					m_ProfileAverageValueStorage.push_back(*avgVal / AverageAmount);
					avgVal++;
				}
				m_ProfileAverageValue.clear();
				m_ProfileAverageCount = 0;
			}
			//loop back through profiles and print times
			auto avgValFromStorage = m_ProfileAverageValueStorage.begin();
			for (auto& result : m_ProfileResults)
			{
				//write the average
				char label[50];
				strcpy(label, "%.3fms ");
				strcat(label, result.Name);
				ImGui::Text(label, *avgValFromStorage);
				avgValFromStorage++;
			}
			m_ProfileAverageCount++;
			m_ProfileResults.clear();
			ImGui::End();
			#endif
		}

		std::vector<ProfileResult> m_ProfileResults;
	private:
		std::vector<float> m_ProfileAverageValue;
		std::vector<float> m_ProfileAverageValueStorage;
		int m_ProfileAverageCount = 100;
	};
}