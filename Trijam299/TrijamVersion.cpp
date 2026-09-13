#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"
#include "entity.h"
#include "world.h"

#define Z_LAYER_NORMAL 0

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

	while ( !WindowShouldClose() )
	{
		// flux::update(GetFrameTime());
		gFlux.update( DELTA );

		gWorld.update();

		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( BLACK );

		gWorld.render();

#if _DEBUG
		gTex.Gui();
		gShd.Gui();
		ImGui::Begin( "Entities" );
		gWorld.forEach<entity>( []( entity *e ) { e->trueGui(); } );
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
