#include "application.h"
#include "gpk_bitmap_file.h"
#include "gpk_encoding.h"
#include "gpk_label.h"
#include "gpk_grid_copy.h"
#include "gpk_tcpip.h"

//#define GPK_AVOID_LOCAL_APPLICATION_MODULE_MODEL_EXECUTABLE_RUNTIME
#include "gpk_app_impl.h"

GPK_DEFINE_APPLICATION_ENTRY_POINT(::gme::SApplication, "Module Explorer");


::gpk::error_t													cleanup					(::gme::SApplication & app)						{ 
	::gpk::tcpipShutdown();
	::gpk::mainWindowDestroy(app.Framework.MainDisplay); 
	return 0;
}

int run(::gme::SHTTPServer& );

::gpk::error_t													setup					(::gme::SApplication & app)						{ 
	::gpk::SFramework													& framework				= app.Framework;
	::gpk::SDisplay														& mainWindow			= framework.MainDisplay;
	error_if(errored(::gpk::mainWindowCreate(mainWindow, framework.RuntimeValues.PlatformDetail, framework.Input)), "Failed to create main window why?????!?!?!?!?");
	::gpk::SGUI															& gui					= framework.GUI;
	const int32_t iShades = 16;
	gui.ThemeDefault												= 18 * iShades + 14;
	gui.ColorModeDefault											= ::gpk::GUI_COLOR_MODE_3D;
	int32_t																controlTestRoot			= ::gpk::controlCreate(gui);
	::gpk::SControl														& controlRoot			= gui.Controls.Controls[controlTestRoot];
	controlRoot.Area												= {{0, 0}, {320, 240}};
	controlRoot.Border												= {4, 4, 4, 4};
	controlRoot.Margin												= {20, 20, 20, 10};
	controlRoot.Align												= ::gpk::ALIGN_CENTER					;
	gui.Controls.Constraints[controlTestRoot].AttachSizeToControl	= {controlTestRoot, controlTestRoot};
	//gui.Controls.Modes	[controlTestRoot].Design				= true;
	::gpk::controlSetParent(gui, controlTestRoot, -1);

	{
		app.IdExit													= ::gpk::controlCreate(gui);
		::gpk::SControl													& controlExit			= gui.Controls.Controls[app.IdExit];
		controlExit.Align											= ::gpk::ALIGN_BOTTOM_RIGHT				;
		::gpk::SControlText												& controlText			= gui.Controls.Text[app.IdExit];
		controlText.Text											= "Exit";
		::gpk::controlSetParent(gui, app.IdExit, 0);
	}
	{
		app.IdTheme													= ::gpk::controlCreate(gui);
		::gpk::SControl													& controlExit			= gui.Controls.Controls[app.IdTheme];
		controlExit.Align											= ::gpk::ALIGN_CENTER_BOTTOM			;
		::gpk::SControlText												& controlText			= gui.Controls.Text[app.IdTheme];
		controlText.Text											= "Theme";
		::gpk::controlSetParent(gui, app.IdTheme, 0);
	}
	{
		app.IdMode													= ::gpk::controlCreate(gui);
		::gpk::SControl													& controlExit			= gui.Controls.Controls[app.IdMode];
		controlExit.Align											= ::gpk::ALIGN_BOTTOM_LEFT				;
		::gpk::SControlText												& controlText			= gui.Controls.Text[app.IdMode];
		controlText.Text											= "Mode";
		::gpk::controlSetParent(gui, app.IdMode, 0);
	}
	for(uint32_t iButton = app.IdExit; iButton < gui.Controls.Controls.size(); ++iButton) {
		::gpk::SControl													& control				= gui.Controls.Controls[iButton];
		::gpk::SControlText												& controlText			= gui.Controls.Text[iButton];
		control.Area												= {{0, 0}, {64, 20}};
		control.Border												= {1, 1, 1, 1};
		control.Margin												= {1, 1, 1, 1};
		controlText.Align											= ::gpk::ALIGN_CENTER;
	}
	
	::gpk::tcpipInitialize();

	run(app.Server);

	return 0; 
}

::gpk::error_t												update					(::gme::SApplication & app, bool exitSignal)	{ 
	//::gpk::STimer													timer;
	retval_info_if(::gpk::APPLICATION_STATE_EXIT, exitSignal, "Exit requested by runtime.");
	{
		::gme::mutex_guard												lock					(app.LockRender);
		app.Framework.MainDisplayOffscreen							= app.Offscreen;
	}
	::gpk::SFramework												& framework				= app.Framework;
	retval_info_if(::gpk::APPLICATION_STATE_EXIT, ::gpk::APPLICATION_STATE_EXIT == ::gpk::updateFramework(framework), "Exit requested by framework update.");
	::gpk::SGUI														& gui					= framework.GUI;

	{
		::gme::mutex_guard												lock					(app.LockGUI);
		::gpk::guiProcessInput(gui, *app.Framework.Input);
		for(uint32_t iControl = 0, countControls = gui.Controls.Controls.size(); iControl < countControls; ++iControl) {
			if(gui.Controls.States[iControl].Unused || gui.Controls.States[iControl].Disabled)
				continue;
			if(gui.Controls.States[iControl].Execute) {
				info_printf("Executed %u.", iControl);
				if(iControl == (uint32_t)app.IdExit)
					return 1;
				else if(iControl == (uint32_t)app.IdMode) {
					gui.Controls.Modes[iControl].ColorMode					= gui.Controls.Modes[iControl].ColorMode == ::gpk::GUI_COLOR_MODE_THEME ? ::gpk::GUI_COLOR_MODE_3D : ::gpk::GUI_COLOR_MODE_THEME;
					for(uint32_t iChild = 0; iChild < gui.Controls.Children[iControl].size(); ++iChild) 
						gui.Controls.Modes[gui.Controls.Children[iControl][iChild]].ColorMode = gui.Controls.Modes[iControl].ColorMode;
				}
				else if(iControl == (uint32_t)app.IdTheme) {
					++gui.ThemeDefault; 
					if(gui.ThemeDefault >= gui.ControlThemes.size())
						gui.ThemeDefault										= 0;
				}
				else if(iControl > (uint32_t)app.IdMode) {
					gui.Controls.Controls[iControl].ColorTheme						= iControl - app.IdMode; 
					for(uint32_t iChild = 0; iChild < gui.Controls.Children[iControl].size(); ++iChild) 
						gui.Controls.Controls[gui.Controls.Children[iControl][iChild]].ColorTheme = gui.Controls.Controls[iControl].ColorTheme;
				}
			}
		}
		if(app.Framework.Input->MouseCurrent.Deltas.z) {
			gui.Zoom.ZoomLevel										+= app.Framework.Input->MouseCurrent.Deltas.z * (1.0 / (120 * 4ULL));
			::gpk::guiUpdateMetrics(gui, framework.MainDisplay.Size, true);
		}
	}
	//timer.Frame();
	//warning_printf("Update time: %f.", (float)timer.LastTimeSeconds);
	return 0; 
}

::gpk::error_t													draw					(::gme::SApplication & app)						{ 
	::gpk::STimer														timer;
	app;
	::gpk::ptr_obj<::gpk::SRenderTarget<::gpk::SColorBGRA, uint32_t>>	target;
	target.create();
	target->Color		.resize(app.Framework.MainDisplay.Size);
	target->DepthStencil.resize(app.Framework.MainDisplay.Size);
	//::gpk::clearTarget(*target);
	{
		::gme::mutex_guard												lock					(app.LockGUI);
		::gpk::controlDrawHierarchy(app.Framework.GUI, 0, target->Color.View);
		::gpk::grid_copy(target->Color.View, app.VerticalAtlas.View);
	}
	{
		::gme::mutex_guard												lock					(app.LockRender);
		app.Offscreen												= target;
	}
	//timer.Frame();
	//warning_printf("Draw time: %f.", (float)timer.LastTimeSeconds);
	return 0; 
}