#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"
#include "entity.h"
#include "world.h"

namespace TrijamVersion
{

#include "resource_mess.h"

Textures gTex;
Shaders gShd;
flux::Group gFlux;
World gWorld;

struct State
{
	void reset()
	{
		gTex = {};
		gShd = {};
		gFlux = {};
		gWorld = {};

		gTex.Load();
		gShd.Load();
	}

	void close()
	{
		gTex.Unload();
		gShd.Unload();
	}

	void gui() {
#define FDRAG( f ) ImGui::DragFloat( #f, &f );
		ImGui::Begin( "State" );
		ImGui::End();
	}
} s;

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s.reset();

	PlaySound( SND_START );

	RenderTexture2D render = LoadRenderTexture( SCRWID, SCRHEI );

	while ( !WindowShouldClose() )
	{
		// flux::update(GetFrameTime());
		gFlux.update( DELTA );

		gWorld.update();

		BeginTextureMode( render );

		ClearBackground( BLACK );

		gWorld.render();

		EndTextureMode();

		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( BLACK );

		//BeginShaderMode(s.s.blur);
		//SetShaderValueTexture(s.s.blur, s.s.uniform_blur_lut, s.t.baselut);

		DrawTexturePro( render.texture, { 0, 0, SCRWID, -SCRHEI }, { 0, 0, SCRWID, SCRHEI }, { 0, 0 }, 0, WHITE );

		//EndShaderMode();

#if _DEBUG
		gTex.Gui();
		gShd.Gui();
		ImGui::Begin( "Entities" );
		gWorld.forEach<entity>( []( entity *e ) { e->gui(); } );
		ImGui::End();
		s.gui();
#endif

		DoFadeInAnimation( fadein );

		rlImGuiEnd();
		EndDrawing();
	}

END:
	SaveGlobState();
	s.close();

	return restart;
}

}
