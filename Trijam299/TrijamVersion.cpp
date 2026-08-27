#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"
#include "entity.h"
#include "world.h"

#define NS_BEGIN namespace TrijamVersion {
#define NS_END }
NS_BEGIN

#include "resource_mess.h"

namespace State {
	Textures gTex;
	Shaders gShd;
	flux::Group gFlux;
	World gWorld;

	void stateReset()
	{
		gTex = {};
		gShd = {};
		gFlux = {};
		gWorld = {};

		gTex.Load();
		gShd.Load();
	}

	void stateClose()
	{
		gTex.Unload();
		gShd.Unload();
	}

	void stateGui() {
		ImGui::Begin("State");
		ImGui::End();
	}
} // namespace State
using namespace State;

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	stateReset();

	PlaySound(SND_START);

	RenderTexture2D render = LoadRenderTexture(SCRWID, SCRHEI);

	while (!WindowShouldClose()) {
		// flux::update(GetFrameTime());
		gFlux.update(GetFrameTime());

		gWorld.update();

		BeginTextureMode(render);

		ClearBackground(BLACK);

		gWorld.render();

		EndTextureMode();

		BeginDrawing();
		rlImGuiBegin();
		
		ClearBackground(BLACK);

		//BeginShaderMode(s.s.blur);
		//SetShaderValueTexture(s.s.blur, s.s.uniform_blur_lut, s.t.baselut);

		DrawTexturePro(render.texture, { 0, 0, SCRWID, -SCRHEI }, { 0, 0, SCRWID, SCRHEI }, { 0, 0 }, 0, WHITE);

		//EndShaderMode();

#if _DEBUG
		gTex.Gui();
		gShd.Gui();
		ImGui::Begin("Entities");
		gWorld.forEach<entity>([](entity *e) { e->gui(); });
		ImGui::End();
		stateGui();
#endif

		DoFadeInAnimation(fadein);

		rlImGuiEnd();
		EndDrawing();
	}

END:
	SaveGlobState();
	stateClose();

	return restart;
}

NS_END
