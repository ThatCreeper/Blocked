add_subdirectory( ImGui )
add_subdirectory( Stable )

target_include_directories( Trijam299 PRIVATE . )

target_sources( Trijam299
	PRIVATE
		entity.h
		flux.cpp
		flux.h
		flux_easings.h
		global.h
		globstate.cpp
		globstate.h
		helpers.h
		resource_mess.h
		TrijamVersion.cpp
		Trijam291.cpp
)
