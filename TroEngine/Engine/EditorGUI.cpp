#include "EditorGUI.h"
#include "Application.h"
#include "imgui.h"

EditorGUI::EditorGUI()
{
}

EditorGUI::~EditorGUI()
{
}

bool EditorGUI::Awake()
{
	return true;
}

update_status EditorGUI::Update(float dt)
{
	CreateGUI();
	AboutPanel();
	GUIConfig();
	MathTest();

	return UPDATE_CONTINUE;
}

void EditorGUI::CreateGUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Close App"))
			{
				App->CloseApp();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Math Test"), NULL, &show_math_test)
			{
			}
			

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About", NULL, &show_about))
			{
			}

			if (ImGui::MenuItem("GUI Config", NULL, &show_test_window))
			{	
			}


			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	};
}

void EditorGUI::AboutPanel()
{
	if (show_about)
	{
		ImGui::Begin("TroEngine v0.0.1", &show_about);

		ImGui::Text("Engine mantained by Iban Mas Ortega (Trodek)");

		if (ImGui::Button("GitHub Repository"))
		{
			ShellExecute(NULL, "open", "https://github.com/Trodek/TroEngine", NULL, NULL, SW_SHOWMAXIMIZED);
		}

		ImGui::End();
	}
}

void EditorGUI::GUIConfig()
{
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void EditorGUI::MathTest()
{
	if (!show_math_test)
	{
		ImGui::Begin("Math Test##panel", &show_math_test,ImGuiWindowFlags_AlwaysAutoResize);

		// Rand Float
		if (ImGui::Button("Generate random float"))
		{
			rand_float = App->RandFloat();
		}
		ImGui::SameLine(200);
		ImGui::Text("%.3f", rand_float); 
		ImGui::SameLine(250);
		ImGui::Text("float");

		ImGui::Separator();

		//Rand int range
		ImGui::PushItemWidth(75);
		ImGui::InputInt("Min", &min);
		ImGui::SameLine(125);
		ImGui::PushItemWidth(75);
		ImGui::InputInt("Max", &max);
		if (ImGui::Button("Generate random in range"))
		{
			rand_int = App->RandRange(min, max);
		}
		ImGui::SameLine(200);
		ImGui::Text("%d", rand_int);

		ImGui::Separator();

		//Create x bounding boxes n check contacts
		ImGui::PushItemWidth(75);
		ImGui::InputInt("Min X", &min_x);
		ImGui::SameLine(125);
		ImGui::PushItemWidth(75);
		ImGui::InputInt("Max X", &max_x);

		ImGui::PushItemWidth(75);
		ImGui::InputInt("Min Y", &min_y);
		ImGui::SameLine(125);
		ImGui::PushItemWidth(75);
		ImGui::InputInt("Max Y", &max_y);

		ImGui::PushItemWidth(75);
		ImGui::InputInt("Min Z", &min_z);
		ImGui::SameLine(125);
		ImGui::PushItemWidth(75);
		ImGui::InputInt("Max Z", &max_z);

		ImGui::PushItemWidth(75);
		ImGui::InputInt("Num Objects", &num_obj);
		if (num_obj > 0 && max_x>min_x && max_y>min_y && max_z>min_z)
		{
			if (ImGui::Button("Test objects"))
			{
				//Reset variables
				contacts = 0;
				aabb_objects.clear();
				graphical_objects.clear();
				EDITOR_LOG("--------Math Test---------");

				//Create Objects
				for (int object = 0; object < num_obj; ++object)
				{
					rand_x = App->RandRange(min_x, max_x);
					rand_y = App->RandRange(min_y, max_y);
					rand_z = App->RandRange(min_z, max_z);

					vec min_point(rand_x,rand_z,rand_y);

					rand_x = App->RandRange(min_x, max_x);
					rand_y = App->RandRange(min_y, max_y);
					rand_z = App->RandRange(min_z, max_z);

					vec max_point(rand_x, rand_z, rand_y);

					aabb_objects.push_back(AABB(min_point, max_point));

					PCube cube(max_point.x - min_point.x, max_point.y - min_point.y, max_point.z - min_point.z);
					cube.SetPos(min_point.x, min_point.y, min_point.z);

					graphical_objects.push_back(cube);
				}

				EDITOR_LOG("Created %d AABB objects", num_obj);

				//Check Colisions/Contacts
				int curr = 1;
				for (std::list<AABB>::iterator curr_object = aabb_objects.begin(); curr_object != --aabb_objects.end(); ++curr_object)
				{
					int test = curr + 1;
					for (std::list<AABB>::iterator test_object = ++curr_object; test_object != aabb_objects.end(); ++test_object)
					{
						if(curr_object->Intersects(*test_object))
						{
							EDITOR_LOG("Object %d intersects with object %d", curr, test);
							contacts++;
						}
						test++;
					}
					curr++;
				}
				EDITOR_LOG("-------------------------");
			}
			ImGui::SameLine(110);
			ImGui::Text("%d", contacts);
			ImGui::SameLine(125);
			ImGui::Text("Contacts");
			
		}


		ImGui::End();

		if (graphical_objects.size() > 0)
		{
			for (std::list<PCube>::iterator c = graphical_objects.begin(); c != graphical_objects.end(); ++c)
			{
				c->Render();
			}
		}
	}
}