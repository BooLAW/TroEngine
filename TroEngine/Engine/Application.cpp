#include "Application.h"
#include "Brofiler/Brofiler.h"
#include "SDL\include\SDL_cpuinfo.h"
#include "gpudetect\DeviceId.h"
#include <locale>
#include <codecvt>
#include "mmgr\mmgr.h"

#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "EditorScene.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleGUI.h"
#include "EditorConsole.h"
#include "EditorGUI.h"

#include "Algorithm\Random\LCG.h"
#include "imgui.h"

#define GRAPH_DATA 75

Application::Application()
{
	window = new ModuleWindow();
	input = new ModuleInput();
	audio = new ModuleAudio();
	scene_intro = new EditorScene();
	renderer3D = new ModuleRenderer3D();
	camera = new ModuleCamera3D();
	gui = new ModuleGUI();
	console = new EditorConsole();
	editor_gui = new EditorGUI();

	// The order of calls is very important!
	// Modules will Awake() Start() and Update() in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(gui);
	AddModule(editor_gui);
	AddModule(console);
	AddModule(camera);
	AddModule(input);
	AddModule(audio);
	
	// Scenes
	AddModule(scene_intro);

	// Renderer last!
	AddModule(renderer3D);

	organization = "UPC CITM";

	new_title = new char[150];
	std::strcpy(new_title, window->title.c_str());

	new_org = new char[150];
	std::strcpy(new_org, organization.c_str());
}

Application::~Application()
{
	std::list<Module*>::iterator item = list_modules.begin();

	while(item != list_modules.end())
	{
		delete *item;
		++item;
	}

	list_modules.clear();
}

bool Application::Init()
{
	bool ret = true;

	PERF_START(ptimer);

	// Call Awake() in all modules
	std::list<Module*>::iterator item = list_modules.begin();

	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Awake();
		++item;
	}

	// After all Awake calls we call Start() in all modules
	EDITOR_LOG("Application Start --------------");
	item = list_modules.begin();

	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Start();
		++item;
	}
	
	PERF_PEEK(ptimer);

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	frame_count++;
	last_sec_frame_count++;

	dt = frame_time.ReadSec();
	frame_time.Start();
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	FrameRateCalculations();
}

void Application::CapFPS(float fps)
{
	if (fps > 0)
	{
		capped_ms = (1000 / fps);
		EDITOR_LOG("FPS capped to %.2f", fps);
	}
	else if (fps == 0)
	{
		capped_ms = 0;
	}
}

void Application::FrameRateCalculations()
{
	if (last_sec_frame_time.Read() > 1000)
	{
		last_sec_frame_time.Start();
		prev_last_sec_frame_count = last_sec_frame_count;
		last_sec_frame_count = 0;
	}

	float avg_fps = float(frame_count) / startup_time.ReadSec();
	float seconds_since_startup = startup_time.ReadSec();
	Uint32 last_frame_ms = frame_time.Read();
	Uint32 frames_on_last_update = prev_last_sec_frame_count;

	if (capped_ms > 0 && last_frame_ms < capped_ms)
	{
		SDL_Delay(capped_ms - last_frame_ms);
	}

	fps_log.push_back(frames_on_last_update);

	if (fps_log.size() > GRAPH_DATA)
		fps_log.erase(fps_log.begin());

	if (ms_log.size() > GRAPH_DATA)
		ms_log.erase(ms_log.begin());
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	BROFILER_CATEGORY("UpdateLogic", Profiler::Color::Azure);

	update_status ret = UPDATE_CONTINUE;

	logic_timer.Start();

	PrepareUpdate();

	std::list<Module*>::iterator item = list_modules.begin();
	
	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PreUpdate(dt);
		++item;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->Update(dt);
		++item;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PostUpdate(dt);
		++item;
	}

	ms_log.push_back(logic_timer.ReadMs());
	
	mem_log.push_back(m_getMemoryStatistics().totalActualMemory);
	
	if (mem_log.size() > GRAPH_DATA)
		mem_log.erase(mem_log.begin());

	FinishUpdate();

	if (close_app)
	{
		ret = UPDATE_STOP;
	}

	return ret;
}

bool Application::CleanUp()
{
	PERF_START(ptimer);

	bool ret = true;

	for (std::list<Module*>::iterator it = list_modules.begin(); it != list_modules.end() && ret == true; ++it)
	{
		ret = (*it)->CleanUp();
	}

	return ret;

	PERF_PEEK(ptimer);
}

void Application::CloseApp()
{
	close_app = true;
}

float Application::RandFloat()
{ 
	return random_generator.FloatIncl(0.0f, 1.0f);
}

int Application::RandRange(int min, int max)
{
	return random_generator.Int(min, max);
}

void Application::ConfigGUI()
{
	if (ImGui::CollapsingHeader("Application"))
	{
		if (ImGui::InputText("App Name", new_title, 150))
		{
			if (App->window->title != new_title)
			{
				window->SetTitle(new_title);
			}
		}

		if (ImGui::InputText("Organization", new_org, 150))
		{
			if (App->organization != new_org)
			{
				organization = new_org;
			}
		}

		if (ImGui::SliderInt("Max FPS", &new_fps, 0, 200))
		{
			CapFPS(new_fps);
		}

		char title[25];
		sprintf_s(title, 25, "FPS: %.1f", fps_log[fps_log.size() - 1]);
		ImGui::PlotHistogram("##fps", &fps_log[0], fps_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
		sprintf_s(title, 25, "Logic ms: %.1f", ms_log[ms_log.size() - 1]);
		ImGui::PlotHistogram("##logicms", &ms_log[0], ms_log.size(), 0, title, 0.0f, 40.0f, ImVec2(310, 100));
		sprintf_s(title, 25, "Memory Consumption");
		ImGui::PlotHistogram("##memchart", &mem_log[0], mem_log.size(), 0, title, 0.0f, 20000.0f, ImVec2(310, 100));
		ImGui::Text("Total Reported Mem: %d", m_getMemoryStatistics().totalReportedMemory);
		ImGui::Text("Total Actual Mem: %d", m_getMemoryStatistics().totalActualMemory);
		ImGui::Text("Peak Reported Mem: %d", m_getMemoryStatistics().peakReportedMemory);
		ImGui::Text("Peak Actual Mem: %d", m_getMemoryStatistics().peakActualMemory);
		ImGui::Text("Accumulated Reported Mem: %d", m_getMemoryStatistics().accumulatedReportedMemory);
		ImGui::Text("Accumulated Actual Mem: %d", m_getMemoryStatistics().accumulatedActualMemory);
		ImGui::Text("Accumulated Alloc Unit Count: %d", m_getMemoryStatistics().accumulatedAllocUnitCount);
		ImGui::Text("Total Alloc Unit Count: %d", m_getMemoryStatistics().totalAllocUnitCount);
		ImGui::Text("Peak Alloc Unit Count: %d", m_getMemoryStatistics().peakAllocUnitCount);
	}
}

void Application::HardwareConfig()
{
	if (ImGui::CollapsingHeader("Hardware"))
	{
		//CPU
		ImGui::Text("CPU:");
		ImGui::Text("Cache: "); ImGui::SameLine(); ImGui::Text("%d", SDL_GetCPUCacheLineSize());
		ImGui::Text("CPUs: "); ImGui::SameLine(); ImGui::Text("%d", SDL_GetCPUCount());
		ImGui::Text("RAM Memory: "); ImGui::SameLine(); ImGui::Text("%d", SDL_GetSystemRAM());
		ImGui::Text("Caps: "); ImGui::SameLine();
		std::string caps;
		if (SDL_Has3DNow)
			caps += "3DNow";
		if (SDL_HasAVX())
			caps += ", AVX";
		if (SDL_HasAVX2())
			caps += ", AVX2";
		if (SDL_HasAltiVec())
			caps += ", AltiVec";
		if (SDL_HasMMX())
			caps += ", MMX";
		if (SDL_HasRDTSC())
			caps += ", RDTSC";
		if (SDL_HasSSE())
			caps += ", SSE";
		if (SDL_HasSSE2())
			caps += ", SSE2";
		if (SDL_HasSSE3())
			caps += ", SSE3";
		if (SDL_HasSSE41())
			caps += ", SSE41";
		if (SDL_HasSSE42())
			caps += ", SSE42";
		ImGui::TextWrapped(caps.c_str());

		ImGui::Separator();
		//GPU
		ImGui::Text("GPU:");
		
		uint vendorid, deviceid;
		Uint64 vm, vm_curr, vm_a, vm_r;
		std::wstring brand;

		getGraphicsDeviceInfo(&vendorid, &deviceid, &brand, &vm, &vm_curr, &vm_a, &vm_r);

		ImGui::Text("Vendor ID: "); ImGui::SameLine(); ImGui::Text("%d", vendorid);
		ImGui::Text("Device ID: "); ImGui::SameLine(); ImGui::Text("%d", deviceid);

		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;

		std::string brand_str = converter.to_bytes(brand);
		ImGui::Text("Brand: "); ImGui::SameLine(); ImGui::Text(brand_str.c_str());
		ImGui::Text("Video Memory: "); ImGui::SameLine(); ImGui::Text("%d", vm);
		ImGui::Text("Video Memory On Use: "); ImGui::SameLine(); ImGui::Text("%d", vm_curr);
		ImGui::Text("Video Memory Available: "); ImGui::SameLine(); ImGui::Text("%d", vm_a);
		ImGui::Text("Video Memory Reserved: "); ImGui::SameLine(); ImGui::Text("%d", vm_r);

	}
}

void Application::OpenWebPage(const char * url)
{
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWMAXIMIZED);
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}