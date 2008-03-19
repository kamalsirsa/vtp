/**
\mainpage Virtual Terrain Builder: Developer Documentation

\section overview Overview
VTBuilder is part of the <a href="http://vterrain.org/">Virtual Terrain Project</a>
and distributed under a completely free <a href="../license.txt">open source license</a>.
Although it is generally an end-user GUI application, it contains powerful methods
which can be called by any C++ programmer.  This developer documentation highlights
some of the classes and methods available.

\section top1 Notable
All of the classes are documented under the 'Classes' tab above, but here are
a few of the most potentially useful classes and methods:
	- Builder
		- Builder::ImportDataFromFile
		- Builder::AddLayer
		- Builder::RemoveLayer
		- Builder::DoSampleElevationToTilePyramids
		- Builder::DoSampleImageryToTilePyramids
	- vtLayer
		- vtLayer::GetExtent
		- vtLayer::GetProjection
		- vtLayer::TransformCoords
	- TilingOptions

\section top2 Functions
	- MatchTilingToResolution()

\section top3 Options
	The global object <b>g_Options</b> contains all the user-level options.
	It is of type <a href="http://vterrain.org/Doc/vtdata/classvtTagArray.html">vtTagArray</a>.

	The following values may be especially useful to set.  They are true/false except as noted.
	- TAG_USE_CURRENT_CRS
	- TAG_LOAD_IMAGES_ALWAYS
	- TAG_LOAD_IMAGES_NEVER
	- TAG_REPRO_TO_FLOAT_ALWAYS
	- TAG_REPRO_TO_FLOAT_NEVER
	- TAG_SLOW_FILL_GAPS
	- TAG_SAMPLING_N	(N > 1 for N*N multisampling)
	- TAG_BLACK_TRANSP

	<b>Example:</b>
	<code>g_Options.SetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS, true);</code>
 */*/
