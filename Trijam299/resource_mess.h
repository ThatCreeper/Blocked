
#define S(a) T(a, #a ".png")
#define TEXTURES \
	T(frozen, "frozen.png") \
	T(baselut, "baselut.png") \
	S(house) \
	S(fire1) \
	S(fire2) \
	S(spot) \
	S(spot_wings) \
	S(inside) \
	S(glass_shards) \
	S(mushrooms) \
	S(potion_room) \
	S(soap) \
	S(chocolate) \
	S(potion) \
	S(spotroom) \
	S(hitler) \
	S(baby)
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
	T(blur, nullptr, "blur.fs")
#define UNIFORMS \
	U(blur, lut, "lut")
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
