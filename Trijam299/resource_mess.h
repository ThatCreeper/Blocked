
#define S(a) T(a, #a ".png")
#define TEXTURES \
	T(frozen, "frozen.png") \
	T(baselut, "baselut.png") \
	S(sheep_circle) \
	S(bridge) \
	S(bridge_over) \
	S(cs1_1) \
	S(cs_1_2) \
	S(cs1_3) \
	S(cs1_4) \
	S(cs1_5) \
	S(cs1_6) \
	S(cs1_7) \
	S(evil) \
	S(cs2) \
	S(bg1) \
	S(bg2)
struct Textures
{
#define T(a, b) Texture2D a;
	TEXTURES
#undef T

		void Load()
	{
#define T(a, b) a = LoadTexture(b);
		TEXTURES
#undef T
	}

	void Unload()
	{
#define T(a, b) UnloadTexture(a);
		TEXTURES
#undef T
	}

	void Gui()
	{
		ImGui::Begin( "Textures" );
		ImGui::BeginGroup();

#define T(a, b) if (ImGui::Button(#a)) { UnloadTexture(a); a = LoadTexture(b); }
		TEXTURES
#undef T

			ImGui::EndGroup();
		ImGui::End();
	}
};
#undef S
#undef TEXTURES

#define SHADERS \
	//T(blur, nullptr, "blur.fs")
#define UNIFORMS \
	//U(blur, lut, "lut")
struct Shaders
{
#define T(a, b, c) Shader a;
	SHADERS;
#undef T
#define U(a, b, c) int uniform_##a##_##b;
	UNIFORMS;
#undef U

	void Load()
	{
#define T(a, b, c) a = LoadShader(b, c);
		SHADERS;
#undef T
		LoadUniforms();
	}

	void LoadUniforms()
	{
#define U(a, b, c) uniform_##a##_##b = GetShaderLocation(a, c);
		UNIFORMS;
#undef U
	}

	void Unload()
	{
#define T(a, b, c) UnloadShader(a);
		SHADERS;
#undef T
	}

	void Gui()
	{
		ImGui::Begin( "Shaders" );
		ImGui::BeginGroup();

#define T(a, b, c) if (ImGui::Button("Reload " #a)) { Shader s = LoadShader(b, c); if (s.id) { UnloadShader(a); a = s; LoadUniforms(); } }
		SHADERS;
#undef T

		ImGui::EndGroup();
		ImGui::End();
	}
};
#undef SHADERS
#undef UNIFORMS
